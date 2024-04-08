#pragma once
#include "Interfaces/IPv4/IPv4Endpoint.h"

class FBristleconeReceiver : public FRunnable {
public:
	FBristleconeReceiver();

	virtual ~FBristleconeReceiver() override;

	void SetLocalSocket(const TSharedPtr<FSocket, ESPMode::ThreadSafe>& new_socket);
	
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;

private:
	void Cleanup();

	//FIPv4Endpoint local_endpoint;

	//TUniquePtr<FSocket> receiver_socket;
	TSharedPtr<FSocket, ESPMode::ThreadSafe> receiver_socket;
	TArray<uint8> received_data;

	TUniquePtr<ISocketSubsystem> socket_subsystem;
	
	bool running;
};
