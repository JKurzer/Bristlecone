#pragma once
#include "FControllerState.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

static constexpr uint8 CLONE_SIZE = 3;

class FBristleconeSender : public FRunnable {
public:
	FBristleconeSender();
	
	virtual ~FBristleconeSender() override;

	void AddTargetAddress(FName target_address_str);
	void SetLocalSocket(const TSharedPtr<FSocket, ESPMode::ThreadSafe>& new_socket);
	
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;

private:
	void Cleanup();
	
	uint32 clone_state_ring_index;
	FControllerState clone_state_ring[CLONE_SIZE];
	
	//TUniquePtr<FSocket> sender_socket;
	TSharedPtr<FSocket, ESPMode::ThreadSafe> sender_socket;
	//FIPv4Endpoint local_endpoint;
	TArray<FIPv4Endpoint> target_endpoints;

	TUniquePtr<ISocketSubsystem> socket_subsystem;

	bool running;
};
