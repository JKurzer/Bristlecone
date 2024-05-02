// Fill out your copyright notice in the Description page of Project Settings.


#include "UBristleconeWorldSubsystem.h"

#include "Common/UdpSocketBuilder.h"


uint64_t UBristleconeWorldSubsystem::getSessionID()
{
#if UE_BUILD_SHIPPING
	throw();// IF YOU SEE THIS, YOU SHIPPED WITHOUT ADDING SESSION MANAGEMENT. THAT IS NOT GOOD.
#endif
	return 0xDEADBEEF;
} 

void UBristleconeWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: Inbound and Outbound Queues set to null."));
	QueueOfReceived = nullptr;
	QueueToSend = nullptr;
	SelfBind = nullptr;
	DebugSend = nullptr;
	ConfigVals = GetDefault<UBristleconeConstants>();
	LogOnReceive = ConfigVals->log_receive_c;
	WakeSender = FPlatformProcess::GetSynchEventFromPool(true);
	UE_LOG(LogTemp, Warning, TEXT("BCN will not start unless another subsystem creates and binds queues during PostInitialize."));
	UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: Subsystem world initialized"));
}

void UBristleconeWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld) {
	if ([[maybe_unused]] const UWorld* World = InWorld.GetWorld() ){
		UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: World beginning play"));
		if (!QueueToSend.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: Inbound queue not allocated. Debug mode only. Connect a controller next time?"));
			QueueToSend = MakeShareable(new TheCone::IncQ(256));
			DebugSend = QueueToSend;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: Good bind for send queue."));
		}


		if (!QueueOfReceived.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: No bind for received queue. Self-binding for debug."));
			QueueOfReceived = MakeShareable(new TheCone::PacketQ(256));
			SelfBind = QueueOfReceived;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: Good bind for received queue."));
		}

		local_endpoint = FIPv4Endpoint(FIPv4Address::Any, DEFAULT_PORT);
		FUdpSocketBuilder socket_factory = FUdpSocketBuilder(TEXT("Bristlecone.Receiver.Socket"))
						.AsNonBlocking()
						.AsReusable()
						.BoundToEndpoint(local_endpoint)
						.WithReceiveBufferSize(CONTROLLER_STATE_PACKET_SIZE * 10)
						.WithSendBufferSize(CONTROLLER_STATE_PACKET_SIZE * 10);
		socketHigh = MakeShareable(socket_factory.Build());
		socketLow = MakeShareable(socket_factory.Build());
		socketBackground = MakeShareable(socket_factory.Build());

		sender_runner.WakeSender = WakeSender;
		//Get config and start sender thread
		
		//TODO: refactor this to allow proper data driven construction.
		FString address = ConfigVals->default_address_c.IsEmpty() ? "52.87.255.239" : ConfigVals->default_address_c;
		sender_runner.AddTargetAddress(address);
		sender_runner.BindSource(QueueToSend);
		sender_runner.SetLocalSockets(socketHigh, socketLow, socketBackground);
		sender_runner.ActivateDSCP();
		sender_thread.Reset(FRunnableThread::Create(&sender_runner, TEXT("Bristlecone.Sender")));

		// Start receiver thread
		receiver_runner.LogOnReceive = LogOnReceive;
		receiver_runner.SetLocalSocket(socketHigh);
		receiver_runner.BindSink(QueueOfReceived);
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

	if (socketHigh.IsValid()) {
		socketHigh.Get()->Close();
	}
	if (socketLow.IsValid()) {
		socketLow.Get()->Close();
	}
	if (socketBackground.IsValid()) {
		socketBackground.Get()->Close();
	}
	socketHigh = nullptr;
	socketLow = nullptr;
	socketBackground = nullptr;

	//this will all need to be refactored, but tbh, I'm not sure we'll keep this for long enough to do it.
	FSocket* sender_socket_obj = socketHigh.Get();
	if (sender_socket_obj != nullptr) {
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(sender_socket_obj);
	}
	sender_socket_obj = socketLow.Get();
	if (sender_socket_obj != nullptr) {
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(sender_socket_obj);
	}
	sender_socket_obj = socketBackground.Get();
	if (sender_socket_obj != nullptr) {
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(sender_socket_obj);
	}

	ConfigVals = nullptr;
	FPlatformProcess::ReturnSynchEventToPool(WakeSender);
	Super::Deinitialize();
}

void UBristleconeWorldSubsystem::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (DebugSend.IsValid())
	{
		for (int i = 0; i < 50; ++i)
		{
			DebugSend.Get()->Enqueue(0xDB000000DEADBEEF);
			WakeSender->Trigger();
		}
	}
	if (SelfBind.IsValid())
	{
		while (!SelfBind->IsEmpty())
		{
			const TheCone::Packet_tpl* current = SelfBind->Peek();
			if (LogOnReceive)
			{
				uint32_t lsbTime = 0x00000000FFFFFFFF & std::chrono::steady_clock::now().time_since_epoch().count();
				UE_LOG(LogTemp, Warning, TEXT("Bristlecone: With UE Frame Latency, %ld, %ld"), lsbTime - current->GetTransferTime(), current->GetCycleMeta());
			}
			SelfBind->Dequeue();
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: Subsystem world ticked"));
}

TStatId UBristleconeWorldSubsystem::GetStatId() const {
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFBristleconeWorldSubsystem, STATGROUP_Tickables);
}
