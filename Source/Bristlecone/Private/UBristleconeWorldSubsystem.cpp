// Fill out your copyright notice in the Description page of Project Settings.


#include "UBristleconeWorldSubsystem.h"

#include "Common/UdpSocketBuilder.h"

void UBristleconeWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: Subsystem world initialized"));
}

void UBristleconeWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld) {
	if ([[maybe_unused]] const UWorld* World = InWorld.GetWorld()) {
		UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: World beginning play"));

		local_endpoint = FIPv4Endpoint(FIPv4Address::Any, DEFAULT_PORT);
		socket = MakeShareable(FUdpSocketBuilder(TEXT("Bristlecone.Receiver.Socket"))
						.AsNonBlocking()
						.AsReusable()
						.BoundToEndpoint(local_endpoint)
						.WithReceiveBufferSize(CONTROLLER_STATE_PACKET_SIZE * 4)
						.WithSendBufferSize(CONTROLLER_STATE_PACKET_SIZE * 4)
						.Build());
		
		//Get config and start sender thread
		ConfigVals = NewObject<UBristleconeConstants>();
		//TODO: refactor this to allow proper data driven construction.
		FString address = ConfigVals->default_address.IsEmpty() ? "1.2.3.4" : ConfigVals->default_address;
		sender_runner.AddTargetAddress(address);
		sender_runner.SetLocalSocket(socket);
		sender_thread.Reset(FRunnableThread::Create(&sender_runner, TEXT("Bristlecone.Sender")));

		// Start receiver thread
		receiver_runner.SetLocalSocket(socket);
		receiver_thread.Reset(FRunnableThread::Create(&receiver_runner, TEXT("Bristlecone.Receiver")));
	}
}

void UBristleconeWorldSubsystem::Deinitialize() {
	UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: Deinitializing Bristlecone subsystem"));
	
	if (sender_thread) {
		sender_thread->Kill();
	}
	if (receiver_thread) {
		receiver_thread->Kill();
	}

	if (socket.IsValid()) {
		socket.Get()->Close();
	}
	socket = nullptr;

	FSocket* sender_socket_obj = socket.Get();
	if (sender_socket_obj != nullptr) {
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(sender_socket_obj);
	}

	ConfigVals = nullptr;
	
	Super::Deinitialize();
}

void UBristleconeWorldSubsystem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: Subsystem world ticked"));
}

TStatId UBristleconeWorldSubsystem::GetStatId() const {
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFBristleconeWorldSubsystem, STATGROUP_Tickables);
}
