#include "FBristleconeSender.h"

#include "UBristleconeWorldSubsystem.h"
#include "Common/UdpSocketBuilder.h"

FBristleconeSender::FBristleconeSender() : running(false) {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Sender: Constructing Bristlecone Sender"));

	// Datagram clone
	clone_state_ring_index = 0;
	for (uint32 ring_idx = 0; ring_idx < CLONE_SIZE; ++ring_idx) {
		clone_state_ring[ring_idx].clear();
	}
	
	// Endpoints
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
		clone_state_ring_index = (clone_state_ring_index + 1) % CLONE_SIZE;
		clone_state_ring[clone_state_ring_index].controller_arr[0] = counter;
		
		for (auto& endpoint : target_endpoints) {
			int32 bytes_sent;

			uint8 send_attempts_made = 0;
			for (uint32 clone_send_index = (clone_state_ring_index + 1) % CLONE_SIZE;
				send_attempts_made < 3 && running;
				clone_send_index = (clone_send_index + 1) % CLONE_SIZE) {
				const FControllerState* current_controller_state = &clone_state_ring[clone_send_index];

				const bool packet_sent = sender_socket->SendTo(reinterpret_cast<const uint8*>(current_controller_state->controller_arr), sizeof(FControllerState),
				                                               bytes_sent, *endpoint.ToInternetAddr());
				UE_LOG(LogTemp, Warning, TEXT("Sent message: %s : %s : Address - %s : BytesSent - %d"), *current_controller_state->to_string(),
					   (packet_sent ? TEXT("true") : TEXT("false")), *endpoint.ToString(), bytes_sent);
				running = packet_sent && bytes_sent > 0;
				send_attempts_made++;
			}
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

