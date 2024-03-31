// Fill out your copyright notice in the Description page of Project Settings.
#include "FBristlecone.h"

#include "Common/UdpSocketBuilder.h"

FBristlecone::FBristlecone() {
	UE_LOG(LogTemp, Display, TEXT("Constructing Bristlecone"));

	// Datagram clone
	clone_state_ring_index = 0;
	for (uint32 ring_idx = 0; ring_idx < CLONE_SIZE; ++ring_idx) {
		clone_state_ring[ring_idx].clear();
	}
	
	// Start thread
	sender_thread.Reset(FRunnableThread::Create(this, TEXT("Bristlecone.Sender")));

	// Endpoints
	target_endpoints.Reserve(MAX_TARGET_COUNT);
}

FBristlecone::FBristlecone(FName init_target_address) : FBristlecone(init_target_address, DEFAULT_PORT) {
}

FBristlecone::FBristlecone(FName init_target_address, uint16 init_target_port) : FBristlecone() {
	FIPv4Address target_address;
	FIPv4Address::Parse(init_target_address.ToString(), target_address);
	FIPv4Endpoint target_endpoint = FIPv4Endpoint(target_address, init_target_port);
	target_endpoints.Emplace(target_endpoint);
}

FBristlecone::~FBristlecone() {
	UE_LOG(LogTemp, Display, TEXT("Destructing Bristlecone"));

	if (sender_thread) {
		sender_thread->Kill();
	}
}

bool FBristlecone::Init() {
	UE_LOG(LogTemp, Display, TEXT("Initializing Bristlecone"));
	socket_subsystem.Reset(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM));
	local_endpoint = FIPv4Endpoint(FIPv4Address::Any, DEFAULT_PORT);

	sender_socket.Reset(FUdpSocketBuilder(TEXT("Bristlecone.Sender.Socket"))
					.AsNonBlocking()
					.AsReusable()
					.BoundToEndpoint(local_endpoint)
					.WithReceiveBufferSize(2 * 1024 * 1024)
					.WithSendBufferSize(2 * 1024 * 1024)
					.Build());
	
	running = true;
	return true;
}

uint32 FBristlecone::Run() {
	int counter = 0;

	const TSharedRef<FInternetAddr> targetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	FControllerState sending_state;

	while (running && sender_socket) {
		// Listening for message
		ReceiveTraffic(targetAddr);

		// Update ring array
		counter++;
		sending_state.controller_arr[0] = counter;
		clone_state_ring_index = (clone_state_ring_index + 1) % CLONE_SIZE;
		clone_state_ring[clone_state_ring_index].controller_arr[0] = counter;
		
		// Sending mesasag
		SendClone();
		
		FPlatformProcess::Sleep(0.1f);
	}

	return 0;
}

void FBristlecone::SendClone() {
	for (auto& endpoint : target_endpoints) {
		int32 bytes_sent;

		uint8 send_attempts_made = 0;
		for (uint32 clone_send_index = (clone_state_ring_index + 1) % CLONE_SIZE;
			send_attempts_made < 3 && running;
			clone_send_index = (clone_send_index + 1) % CLONE_SIZE) {
			const FControllerState* current_controller_state = &clone_state_ring[clone_send_index];
			
			bool packet_sent = sender_socket->SendTo(reinterpret_cast<const uint8*>(current_controller_state->controller_arr), sizeof(FControllerState),
											 bytes_sent, *endpoint.ToInternetAddr());
			UE_LOG(LogTemp, Warning, TEXT("Sent message: %s : %s : Address - %s : BytesSent - %d"), *current_controller_state->to_string(),
				   (packet_sent ? TEXT("true") : TEXT("false")), *endpoint.ToString(), bytes_sent);
			running = packet_sent && bytes_sent > 0;
			send_attempts_made++;
		}
	} 
}

void FBristlecone::ReceiveTraffic(const TSharedRef<FInternetAddr>& receive_from_address) {
	FControllerState receiving_state;
	
	uint32 socket_data_size;
	while (sender_socket->HasPendingData(socket_data_size)) {
		int32 bytes_read = 0;
			
		received_data.SetNumUninitialized(FMath::Min(socket_data_size, 65507u));
		sender_socket->RecvFrom(received_data.GetData(), received_data.Num(), bytes_read, *receive_from_address);

		receiving_state.clear();
		memcpy(&receiving_state.controller_arr, received_data.GetData(), bytes_read);
		UE_LOG(LogTemp, Warning, TEXT("Received %s in %d bytes from target"), *receiving_state.to_string(), bytes_read);
	}
}

void FBristlecone::Exit() {
	UE_LOG(LogTemp, Display, TEXT("Exiting Bristlecone."));
	Cleanup();
}

void FBristlecone::Stop() {
	UE_LOG(LogTemp, Display, TEXT("Stopping Bristlecone."));
	Cleanup();
}

void FBristlecone::Cleanup() {
	if (sender_socket != nullptr) {
		sender_socket->Close();
	}
	FSocket* sender_socket_obj = sender_socket.Release();
	if (sender_socket_obj != nullptr) {
		socket_subsystem->DestroySocket(sender_socket_obj);
	}
	//socket_subsystem = nullptr;
	ISocketSubsystem* sockect_subsystem_obj = socket_subsystem.Release();
	if (sockect_subsystem_obj != nullptr) {
		sockect_subsystem_obj = nullptr;
	}
	target_endpoints.Empty();
	running = false;
}
