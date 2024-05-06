// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FBristleconePacket.h"
#include "FBristleconeReceiver.h"
#include "FBristleconeSender.h"
#include "Subsystems/WorldSubsystem.h"
#include "UBristleconeConstants.h"
#include "BristleconeCommonTypes.h"

//This implementation suffers badly from the general resistance to correct template support
//in the unreal engine. As a result, this subsystem only supports 8 byte messages.
//Later, I'll defactor this into a pair of base classes that compose the implementation
//of UObjects, but for now, I'm leaving it. My use-cases only require the 8byte.
#include "UBristleconeWorldSubsystem.generated.h"
using namespace TheCone;



UCLASS()
class BRISTLECONE_API UBristleconeWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

public:
	// In and out queues allowing one producer and one consumer.
	// Subsystems should provide allocated queues. Bristlecone only provides for receives,
	// and will not run if nothing binds the source queue "QueueToSend."
	// 
	// Inbound is produced by another subsystem that is responsible for creating and owning the queue lifecycle
	// It is consumed by the sender thread. Input is expected to be 8 byte packets at 120hz. 
	// An event is provided, as well, to wake the sender.
	// Adding more producers WILL cause concurrency bugs immediately.
	FSharedEventRef WakeSender; //DO NOT USE AN INFINITE WAIT ON THIS. IT IS NOT EVER SAFE TO USE INFINITE WAITS ON FEVENTS.
	TheCone::SendQueue QueueToSend;
	TheCone::SendQueue DebugSend;

	//This is the outbound queue of received packets, produced by the receiver thread. technically, bristlecone doesn't guarantee
	//that you will have both a sender and a receiver for each datagram, but in practice, it happens enough
	//that I've paired them in this subsystem explicitly.
	//Again, only 1p1c patterns are supported by this lockless design. The receiver waits on its socket, not this queue.
	TheCone::RecvQueue QueueOfReceived;
	TheCone::RecvQueue SelfBind;
	bool LogOnReceive;

  private:
	FIPv4Endpoint local_endpoint;
	const UBristleconeConstants* ConfigVals;
	TSharedPtr<FSocket, ESPMode::ThreadSafe> socketHigh;
	TSharedPtr<FSocket, ESPMode::ThreadSafe> socketLow;
	TSharedPtr<FSocket, ESPMode::ThreadSafe> socketBackground;
	


	// Sender information
  	FBristleconeSender sender_runner;
  	TUniquePtr<FRunnableThread> sender_thread;

	// Receiver information
	FBristleconeReceiver receiver_runner;
	TUniquePtr<FRunnableThread> receiver_thread;
};
