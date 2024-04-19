// Fill out your copyright notice in the Description page of Project Settings.


#include "UBristleconeWorldSubsystem.h"

#include "Common/UdpSocketBuilder.h"

#if PLATFORM_HAS_BSD_SOCKET_FEATURE_WINSOCKETS
#include "Windows/WindowsHWrapper.h"
#include "Windows/AllowWindowsPlatformTypes.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <qos2.h>

typedef int32 SOCKLEN;

#include "Windows/HideWindowsPlatformTypes.h"

#endif
#include <Runtime/Sockets/Private/BSDSockets/SocketsBSD.h>


void UBristleconeWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: Subsystem world initialized"));
}

void UBristleconeWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld) {
	if ([[maybe_unused]] const UWorld* World = InWorld.GetWorld()) {
		UE_LOG(LogTemp, Warning, TEXT("Bristlecone:Subsystem: World beginning play"));

		local_endpoint = FIPv4Endpoint(FIPv4Address::Any, DEFAULT_PORT);
		FUdpSocketBuilder socket_factory = FUdpSocketBuilder(TEXT("Bristlecone.Receiver.Socket"))
						.AsNonBlocking()
						.AsReusable()
						.BoundToEndpoint(local_endpoint)
						.WithReceiveBufferSize(CONTROLLER_STATE_PACKET_SIZE * 4)
						.WithSendBufferSize(CONTROLLER_STATE_PACKET_SIZE * 4);
		socketHigh = MakeShareable(socket_factory.Build());
		socketLow = MakeShareable(socket_factory.Build());
		socketFast = MakeShareable(socket_factory.Build());
		//quite a lot of ungood things have to happen for us to do this. As a result, I'll be writing out what we're doing.
		#if PLATFORM_HAS_BSD_SOCKET_FEATURE_WINSOCKETS
		QOS_VERSION Version;
		HANDLE      QoSHandle = NULL;

		// Initialize the QoS version parameter.
		Version.MajorVersion = 1;
		Version.MinorVersion = 0;

		// Get a handle to the QoS subsystem. this requires us to have the qwave lib file loaded to resolve the symbol. Oddly, you can't load the dll.
		QOSCreateHandle(
			&Version,
			&QoSHandle);
		//this is necessary because fsocket does not have a get native, as not all abstracted sockets actually have a native file-like socket
		//our build mechanism has to break encapsulation pretty aggressively to resolve this, and it's quite ugly.
		SOCKET underlyingHigh =	((FSocketBSD*)	(socketHigh.Get()))->GetNativeSocket();// Time to go for a very bad ride.
		SOCKET underlyingLow =	((FSocketBSD*)	(socketLow.Get()))->GetNativeSocket();
		SOCKET underlyingFast =	((FSocketBSD*)	(socketFast.Get()))->GetNativeSocket();
		
		#endif

		//Get config and start sender thread
		ConfigVals = NewObject<UBristleconeConstants>();
		//TODO: refactor this to allow proper data driven construction.
		FString address = ConfigVals->default_address.IsEmpty() ? "1.2.3.4" : ConfigVals->default_address;
		sender_runner.AddTargetAddress(address);
		sender_runner.SetLocalSocket(socketHigh);
		sender_thread.Reset(FRunnableThread::Create(&sender_runner, TEXT("Bristlecone.Sender")));

		// Start receiver thread
		receiver_runner.SetLocalSocket(socketHigh);
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
