#pragma once
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "FControllerState.h"
#include "SocketSubsystem.h"
#include "Common/UdpSocketBuilder.h"
#include "BristleconeCommonTypes.h"

class FBristleconeReceiver : public FRunnable {
public:
	FBristleconeReceiver();

	void BindSink(TheCone::QueueRecvEight QueueCandidate);

	virtual ~FBristleconeReceiver() override;

	void SetLocalSocket(const TSharedPtr<FSocket, ESPMode::ThreadSafe>& new_socket);
	
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;

private:
	void Cleanup();

	TSharedPtr<FSocket, ESPMode::ThreadSafe> receiver_socket;
	TArray<uint8> received_data;
	TheCone::QueueRecvEight Queue;
	TUniquePtr<ISocketSubsystem> socket_subsystem;
	bool running;
};
