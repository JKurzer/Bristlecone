// Fill out your copyright notice in the Description page of Project Settings.


#include "UCablingWorldSubsystem.h"

THIRD_PARTY_INCLUDES_START
#include <concrt.h>

#include "winrt/Windows.Gaming.Input.h"
THIRD_PARTY_INCLUDES_END
namespace RawInput = winrt::Windows::Gaming::Input;

//Goal: Cabling is a thin threaded layer that pulls input from the controller, and provides it to: 
// the Cabling world subsystem for making accessible to the game thread...
// AND
// a threadsafe queue supporting a single consuming thread, then triggers an event.
 
//We're going to wire up the RT system and oversample at 3x expected control input hertz, so 360
//This is because XB1+ Controllers have a max sample rate of 120.
//If we don't have a new input after 3 polls, we ship the old one.
// 
//We only ship 120 inputs per second, so we'll need to do something about this.
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
