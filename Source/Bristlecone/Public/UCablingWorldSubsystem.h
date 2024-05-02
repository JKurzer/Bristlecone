// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FCabling.h"
#include "HAL/Runnable.h"
#include "Containers/CircularQueue.h"
#include "UCablingWorldSubsystem.generated.h"


//Goal: The Cabling Subsystem maintains the cabling thread and provides the output of
//the control polling that it performs to the normal input system. Cabling is not
//intended to replace a full input system, just provide a threaded flow


UCLASS()
class  UCablingWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	virtual void PostInitialize();
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	//THIS IS THE SHARED SESSION ID. THIS IS NOT IMPLEMENTED YET.
	//THESE ARE ONLY GUARANTEED TO BE UNIQUE PER REFLECTOR ATM.
	//ULTIMATELY, THESE SHOULD BE THE CURRENT MATCH ID AS BRISTLECONE
	//SHOULD NOT BE RUNNING OUTSIDE OF A MATCH.

  private:
	

	// Receiver information

	FCabling controller_runner;
	TSharedPtr<TCircularQueue<uint64_t>> GameThreadControlQueue;
	TSharedPtr<TCircularQueue<uint64_t>> CabledThreadControlQueue;
	TUniquePtr<FRunnableThread> controller_thread;
};
