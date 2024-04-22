// Fill out your copyright notice in the Description page of Project Settings.


#include "UCablingWorldSubsystem.h"

THIRD_PARTY_INCLUDES_START
#include <concrt.h>

#include "winrt/Windows.Gaming.Input.h"
THIRD_PARTY_INCLUDES_END
namespace RawInput = winrt::Windows::Gaming::Input;

void UCablingWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning, TEXT("UCablingWorldSubsystem: Subsystem world initialized"));
}

void UCablingWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld) {
	if ([[maybe_unused]] const UWorld* World = InWorld.GetWorld()) {
		UE_LOG(LogTemp, Warning, TEXT("UCablingWorldSubsystem: World beginning play"));

	
	}
}

void UCablingWorldSubsystem::Deinitialize() {
	UE_LOG(LogTemp, Warning, TEXT("UCablingWorldSubsystem: Deinitializing Bristlecone subsystem"));

	Super::Deinitialize();
}

void UCablingWorldSubsystem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: Subsystem world ticked"));
}

TStatId UCablingWorldSubsystem::GetStatId() const {
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFBristleconeWorldSubsystem, STATGROUP_Tickables);
}
