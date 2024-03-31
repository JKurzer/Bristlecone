// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

#include "FControllerState.h"

// We're not implementing multiple targets (yet), but for future when we do it'll be easier if we have to refactor less
// code to work with arrays as it is
static constexpr uint16 MAX_TARGET_COUNT = 1;
static constexpr int DEFAULT_PORT = 40000;
static constexpr uint8 CLONE_SIZE = 3;

class BRISTLECONEWORK_API FBristlecone : public FRunnable
{
public:
	FBristlecone();
	explicit FBristlecone(FName init_target_address);
	explicit FBristlecone(FName init_target_address, uint16 init_target_port);

	virtual ~FBristlecone() override;

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;

private:
	void SendClone();
	void ReceiveTraffic(const TSharedRef<FInternetAddr>& receive_from_address);
	void Cleanup();

	// We'll be sending more than one control state per update tick to account for dropped UDP packets
	FControllerState clone_state_ring[CLONE_SIZE];
	uint32 clone_state_ring_index;

	// Information for local socket
	TUniquePtr<ISocketSubsystem> socket_subsystem;
	FIPv4Address local_address;
	FIPv4Endpoint local_endpoint;
	TUniquePtr<FSocket> sender_socket;

	// Target information
	TArray<FIPv4Endpoint> target_endpoints;
	TArray<uint8> received_data;

	// Thread information
	TUniquePtr<FRunnableThread> sender_thread;
	bool running;
};
