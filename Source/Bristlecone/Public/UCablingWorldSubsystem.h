// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FCabling.h"
#include "UCablingWorldSubsystem.generated.h"




UCLASS()
class  UCablingWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

  private:
	

	// Receiver information
	FCabling control_runner;
	TUniquePtr<FRunnableThread> receiver_thread;
};
