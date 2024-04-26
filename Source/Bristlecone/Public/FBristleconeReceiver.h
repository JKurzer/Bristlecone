﻿#pragma once
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "UBristleconeWorldSubsystem.h"

class FBristleconeReceiver : public FRunnable {
public:
	FBristleconeReceiver();

	void BindSink(UBristleconeWorldSubsystem::QueueRecvEight QueueCandidate);

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
	UBristleconeWorldSubsystem::QueueRecvEight Queue;
	TUniquePtr<ISocketSubsystem> socket_subsystem;
	bool running;
};
