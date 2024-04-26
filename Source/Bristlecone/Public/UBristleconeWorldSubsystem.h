// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FBristleconePacket.h"
#include "FBristleconeReceiver.h"
#include "FBristleconeSender.h"
#include "Subsystems/WorldSubsystem.h"
#include "UBristleconeConstants.h"
#include "Containers/CircularQueue.h"

//This implementation suffers badly from the general resistance to correct template support
//in the unreal engine. As a result, this subsystem only supports 8 byte messages.
//Later, I'll defactor this into a pair of base classes that compose the implementation
//of UObjects, but for now, I'm leaving it. My use-cases only require the 8byte.
#include "UBristleconeWorldSubsystem.generated.h"


typedef FBristleconePacket<FControllerState, 3> FControllerStatePacket;

static constexpr int CONTROLLER_STATE_PACKET_SIZE = sizeof(FControllerStatePacket);
static constexpr int DEFAULT_PORT = 40000;
static constexpr uint16 MAX_TARGET_COUNT = 1;
static constexpr float SLEEP_TIME_BETWEEN_THREAD_TICKS = 0.016f;

UCLASS()
class  UBristleconeWorldSubsystem : public UTickableWorldSubsystem
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
	// Subsystems should provide allocated queues. Bristlecone does not, and will not run if nothing binds.
	// 
	//Inbound is produced by another subsystem that is responsible for creating and owning the queue lifecycle
	//It is consumed by the sender thread. Input is expected to be 8 byte packets at 120hz. 
	//An event is provided, as well, to wake the sender.
	//Adding more producers WILL cause concurrency bugs immediately.
	TSharedPtr<TCircularQueue<uint64_t>> QueueToSend;
	//This is the outbound queue, used by the receiver thread. technically, bristlecone doesn't guarantee
	//that you will have both a sender and a receiver for each datagram, but in practice, it happens enough
	//that I've paired them in this subsystem explicitly. The sender thread produces, this subsystem consumes for now.
	//Again, only 1p1c patterns are supported by this lockless design. The receiver waits on its socket, not this queue.
	typedef FBristleconePacket<uint64_t, 3> Packet_tpl;
	typedef TCircularQueue<Packet_tpl> PacketQ;
	typedef TSharedPtr<PacketQ> QueueRecvEight;
	QueueRecvEight QueueOfReceived;
	QueueRecvEight SelfBind;
  private:
	FIPv4Endpoint local_endpoint;
	UBristleconeConstants* ConfigVals;
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
