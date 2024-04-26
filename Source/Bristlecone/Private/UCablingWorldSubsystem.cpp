// Fill out your copyright notice in the Description page of Project Settings.


#include "UCablingWorldSubsystem.h"

//THIS IS A GENERALLY UNDESIRABLE INCLUDE PATTERN
#include "UBristleconeWorldSubsystem.h"

//Goal: Cabling is a thin threaded layer that pulls input from the controller, and provides it to: 
// the Cabling world subsystem for making accessible to the game thread...
// AND
// a threadsafe queue supporting a single consuming thread, then triggers an event.
 
//We're going to wire up the RT system and oversample at 3x expected control input hertz, so 360
//This is because XB1+ Controllers have a max sample rate of 120.
//If we don't have a new input after 3 polls, we ship the old one.
// 
//We only ship 128 inputs per second, so we'll need to do something about this.
void UCablingWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);
	GameThreadControlQueue = MakeShareable(new TCircularQueue<uint64_t>(256));
	CabledThreadControlQueue = MakeShareable(new TCircularQueue<uint64_t>(256));
	UE_LOG(LogTemp, Warning, TEXT("UCablingWorldSubsystem: Subsystem world initialized"));
}

void UCablingWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld) {
	if ([[maybe_unused]] const UWorld* World = InWorld.GetWorld()) {
		UE_LOG(LogTemp, Warning, TEXT("UCablingWorldSubsystem: World beginning play"));
		controller_runner.CabledThreadControlQueue = this->CabledThreadControlQueue;
		controller_runner.GameThreadControlQueue = this->GameThreadControlQueue;
		controller_thread.Reset(FRunnableThread::Create(&controller_runner, TEXT("Cabling hooked up!")));
	}
}

void UCablingWorldSubsystem::Deinitialize() {
	UE_LOG(LogTemp, Warning, TEXT("UCablingWorldSubsystem: Deinitializing Cabling subsystem"));

	controller_runner.Stop();
	Super::Deinitialize();
}

//once all tickable world subsystems are initialized, this is invoked.
//we use this to seek out and bind to the bristlecone subsystem.
//I recommend that you strongly consider generalizing this,
//but my normal approach would be template typing and initialization
//injection, which appears to be extremely Non-Unreal.
void UCablingWorldSubsystem::PostInitialize()
{
	UBristleconeWorldSubsystem* MySquire = GetTickableGameObjectWorld()->GetSubsystem< UBristleconeWorldSubsystem>();
	MySquire->QueueToSend = CabledThreadControlQueue;
}

void UCablingWorldSubsystem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: Subsystem world ticked"));
}

TStatId UCablingWorldSubsystem::GetStatId() const {
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFCablingWorldSubsystem, STATGROUP_Tickables);
}
