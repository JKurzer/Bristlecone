#include "FBristleconeReceiver.h"



FBristleconeReceiver::FBristleconeReceiver() : running(false) {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Receiver: Constructing Bristlecone Receiver"));
}

void FBristleconeReceiver::BindSink(TheCone::RecvQueue QueueCandidate)
{
	Queue.Reset();
	Queue = QueueCandidate;
}


FBristleconeReceiver::~FBristleconeReceiver() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Receiver: Destructing Bristlecone Receiver"));
}

void FBristleconeReceiver::SetLocalSocket(const TSharedPtr<FSocket, ESPMode::ThreadSafe>& new_socket) {
	receiver_socket = new_socket;
}

bool FBristleconeReceiver::Init() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Receiver: Initializing Bristlecone receiver thread"));
	socket_subsystem.Reset(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM));
	SeenCycles = 0;
	HighestSeen = 0;
	running = true;
	return true;
}

uint32 FBristleconeReceiver::Run() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Receiver: Running receiver thread"));
	const TSharedRef<FInternetAddr> targetAddr = socket_subsystem->CreateInternetAddr();
	
	while (running && receiver_socket) {
		TheCone::Packet_tpl receiving_state;
	
		uint32 socket_data_size;
		while (receiver_socket.IsValid() && receiver_socket->HasPendingData(socket_data_size)) {
			int32 bytes_read = 0;
			
			received_data.SetNumUninitialized(FMath::Min(socket_data_size, 65507u));
			receiver_socket->RecvFrom(received_data.GetData(), received_data.Num(), bytes_read, *targetAddr);
			
			memcpy(&receiving_state, received_data.GetData(), bytes_read);
			//this & logging are VERY slow, like potentially reordering our perceived timings slow. We need to be careful as hell interacting
			//with time and logging, since we're now operating in the lock-sensitive time regime. we'll need a solution.
			if (LogOnReceive)
			{
				uint32_t lsbTime = 0x00000000FFFFFFFF & std::chrono::steady_clock::now().time_since_epoch().count();
				UE_LOG(LogTemp, Warning, TEXT("Bristlecone, %ld, %ld"), lsbTime - receiving_state.GetTransferTime(), receiving_state.GetCycleMeta());
			}
			const uint64_t cycle = receiving_state.GetCycleMeta();
			//we keep a mask of the 64 cycles before the highest seen to make sure we don't emit more than once.
			//if it's higher, we slide forwards and don't need to check the mask
			if ((cycle < HighestSeen))
			{
				//this isn't strictly needed but you shouldn't shift more than the width.
				cycle - HighestSeen > 64 ? SeenCycles = 0 : SeenCycles >>= (cycle - HighestSeen);
				HighestSeen = cycle;
			}
			//if it's off the bottom of the mask, we discard it.
			else if ((cycle - HighestSeen) < -64)
			{
				continue; //we don't accept inputs older than 64 cycles right now. TODO: make this configurable.
			}
			// if we've seen it, we discard it. Also, 1ull is awful.
			else if (SeenCycles & (1ull << (HighestSeen - cycle)))
			{
				continue;
			}
			else
			{
				//If we hadn't seen it, we set it now.
				SeenCycles |= (1ull << (HighestSeen - cycle));
			}


			Queue.Get()->Enqueue(receiving_state);//this actually provokes a copy, which can be removed, I think, by not doing the mcpy
			
		}

		receiver_socket.IsValid() ? receiver_socket.Get()->Wait(ESocketWaitConditions::WaitForRead, 0.01f) : 0;
	}
	
	return 0;
}

void FBristleconeReceiver::Exit() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Receiver: Stopping Bristlecone receiver thread."));
	Cleanup();
}

void FBristleconeReceiver::Stop() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Receiver: Stopping Bristlecone receiver thread."));
	Cleanup();
}

void FBristleconeReceiver::Cleanup() {
	receiver_socket = nullptr;
	const ISocketSubsystem* socket_subsystem_obj = socket_subsystem.Release();
	if (socket_subsystem_obj != nullptr) {
		socket_subsystem_obj = nullptr;
	}
	running = false;
}
