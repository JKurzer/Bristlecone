// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FBristleconeReceiver.h"
#include "FBristleconeSender.h"
#include "Subsystems/WorldSubsystem.h"
#include "FBristleconeWorldSubsystem.generated.h"

// TODO - Update to header_size + (data_packet * CLONE_COUNT)
static constexpr int PACKET_SIZE = 2 * 1024 * 1024;
static constexpr int DEFAULT_PORT = 40000;

/**
 * 
 */
UCLASS()
class BRISTLECONEWORK_API UFBristleconeWorldSubsystem : public UTickableWorldSubsystem
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
