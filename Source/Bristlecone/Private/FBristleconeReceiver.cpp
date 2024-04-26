#include "FBristleconeReceiver.h"

#include "FControllerState.h"
#include "SocketSubsystem.h"
#include "UBristleconeWorldSubsystem.h"
#include "Common/UdpSocketBuilder.h"

FBristleconeReceiver::FBristleconeReceiver() : running(false) {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Receiver: Constructing Bristlecone Receiver"));
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

	running = true;
	return true;
}

uint32 FBristleconeReceiver::Run() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Receiver: Running receiver thread"));
	const TSharedRef<FInternetAddr> targetAddr = socket_subsystem->CreateInternetAddr();
	
	while (running && receiver_socket) {
		FBristleconePacketContainer<FControllerState, 3> receiving_state;
	
		uint32 socket_data_size;
		while (receiver_socket.IsValid() && receiver_socket->HasPendingData(socket_data_size)) {
			int32 bytes_read = 0;
			
			received_data.SetNumUninitialized(FMath::Min(socket_data_size, 65507u));
			receiver_socket->RecvFrom(received_data.GetData(), received_data.Num(), bytes_read, *targetAddr);
			
			memcpy(receiving_state.GetPacket(), received_data.GetData(), bytes_read);
			//this & logging are VERY slow, like potentially reordering our perceived timings slow. We need to be careful as hell interacting
			//with time and logging, since we're now operating in the lock-sensitive time regime. we'll need a solution.
			long long round_trip_time = std::chrono::steady_clock::now().time_since_epoch().count() - receiving_state.GetSendTimeStamp();
			UE_LOG(LogTemp, Warning, TEXT("@, Received, %lld, %lld"), receiving_state.GetSendTimeStamp(), round_trip_time);
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
