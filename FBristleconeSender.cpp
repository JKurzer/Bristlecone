#include "FBristleconeSender.h"

#include "UBristleconeWorldSubsystem.h"
#include "Common/UdpSocketBuilder.h"

FBristleconeSender::FBristleconeSender() : running(false) {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Sender: Constructing Bristlecone Sender"));

	target_endpoints.Reserve(MAX_TARGET_COUNT);
}

FBristleconeSender::~FBristleconeSender() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Sender: Destructing Bristlecone Sender"));
}

void FBristleconeSender::AddTargetAddress(FName target_address_str) {
	FIPv4Address target_address;
	FIPv4Address::Parse(target_address_str.ToString(), target_address);
	FIPv4Endpoint target_endpoint = FIPv4Endpoint(target_address, DEFAULT_PORT);
	target_endpoints.Emplace(target_endpoint);
}

void FBristleconeSender::SetLocalSocket(const TSharedPtr<FSocket, ESPMode::ThreadSafe>& new_socket) {
	sender_socket = new_socket;
}

bool FBristleconeSender::Init() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Sender: Initializing Bristlecone Sender thread"));
	socket_subsystem.Reset(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM));
	
	running = true;
	return true;
}

uint32 FBristleconeSender::Run() {
	int counter = 0;
	FControllerState sending_state;
	
	while(running && sender_socket) {
		// Update ring array
		counter++;
		sending_state.controller_arr[0] = counter;
		packet_container.InsertNewDatagram(&sending_state);
		
		for (auto& endpoint : target_endpoints) {
			int32 bytes_sent;

			const FControllerStatePacket* current_controller_state = packet_container.GetPacket();

			const bool packet_sent = sender_socket->SendTo(reinterpret_cast<const uint8*>(current_controller_state), sizeof(FControllerStatePacket),
														   bytes_sent, *endpoint.ToInternetAddr());
			UE_LOG(LogTemp, Warning, TEXT("Sent message: %s : %s : Address - %s : BytesSent - %d"), *current_controller_state->ToString(),
				   (packet_sent ? TEXT("true") : TEXT("false")), *endpoint.ToString(), bytes_sent);
			
			running = packet_sent && bytes_sent > 0;
		}

		FPlatformProcess::Sleep(0.1f);
	}
	
	return 0;
}

void FBristleconeSender::Exit() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Sender: Exiting Bristlecone sender thread."));
	Cleanup();
}

void FBristleconeSender::Stop() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Sender: Stopping Bristlecone sender thread."));
	Cleanup();
}

void FBristleconeSender::Cleanup() {
	sender_socket = nullptr;
	const ISocketSubsystem* socket_subsystem_obj = socket_subsystem.Release();
	if (socket_subsystem_obj != nullptr) {
		socket_subsystem_obj = nullptr;
	}
	target_endpoints.Empty();
	running = false;
}

