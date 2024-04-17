// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FBristleconePacket.h"
#include "FBristleconeReceiver.h"
#include "FBristleconeSender.h"
#include "Subsystems/WorldSubsystem.h"
#include "UBristleconeWorldSubsystem.generated.h"

typedef FBristleconePacket<FControllerState, 3> FControllerStatePacket;

static constexpr int CONTROLLER_STATE_PACKET_SIZE = sizeof(FControllerStatePacket);
static constexpr int DEFAULT_PORT = 40000;
static constexpr uint16 MAX_TARGET_COUNT = 1;
static constexpr float SLEEP_TIME_BETWEEN_THREAD_TICKS = 0.01f;

UCLASS()
class BRISTLECONEWORK_API UBristleconeWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

  private:
	FIPv4Endpoint local_endpoint;
	TSharedPtr<FSocket, ESPMode::ThreadSafe> socket;
	
	// Sender information
  	FBristleconeSender sender_runner;
  	TUniquePtr<FRunnableThread> sender_thread;

	// Receiver information
	FBristleconeReceiver receiver_runner;
	TUniquePtr<FRunnableThread> receiver_thread;
};
