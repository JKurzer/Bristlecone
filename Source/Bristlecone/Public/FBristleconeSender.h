#pragma once
#include "FBristleconePacket.h"
#include "FControllerState.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

static constexpr uint8 CLONE_SIZE = 3;
static constexpr uint8 MAX_MIXED_CONSECUTIVE_PACKETS_ALLOWED = 100;

class FBristleconeSender : public FRunnable {
public:
	FBristleconeSender();
	
	virtual ~FBristleconeSender() override;

	void AddTargetAddress(FString target_address_str);
	void SetLocalSockets(
		const TSharedPtr<FSocket, ESPMode::ThreadSafe>& new_socket_high,
		const TSharedPtr<FSocket, ESPMode::ThreadSafe>& new_socket_low,
		const TSharedPtr<FSocket, ESPMode::ThreadSafe>& new_socket_adaptive
	);

	void ActivateDSCP();
	
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;

private:
	void Cleanup();

	FBristleconePacketContainer<FControllerState, 3> packet_container;
	
	TSharedPtr<FSocket, ESPMode::ThreadSafe> sender_socket_high;
	TSharedPtr<FSocket, ESPMode::ThreadSafe> sender_socket_low;
	TSharedPtr<FSocket, ESPMode::ThreadSafe> sender_socket_background;
	TArray<FIPv4Endpoint> target_endpoints;

	TUniquePtr<ISocketSubsystem> socket_subsystem;

	uint8 consecutive_zero_bytes_sent;

	bool running;
};
