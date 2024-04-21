#include "FBristleconeSender.h"

#include "UBristleconeWorldSubsystem.h"
#include "Common/UdpSocketBuilder.h"

//these includes shouldn't be moved to the .h, due to odd declaration behaviors.
//the same pattern can be seen, executed differently, in the socket library.
//it looks like they basically "shade" them from being included in some TLUs.
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

FBristleconeSender::FBristleconeSender()
: consecutive_zero_bytes_sent(0), running(false) {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Sender: Constructing Bristlecone Sender"));

	target_endpoints.Reserve(MAX_TARGET_COUNT);
}

FBristleconeSender::~FBristleconeSender() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Sender: Destructing Bristlecone Sender"));
}

void FBristleconeSender::AddTargetAddress(FString target_address_str) {
	FIPv4Address target_address;
	FIPv4Address::Parse(target_address_str, target_address);
	FIPv4Endpoint target_endpoint = FIPv4Endpoint(target_address, DEFAULT_PORT);
	target_endpoints.Emplace(target_endpoint);
}

void FBristleconeSender::SetLocalSockets(
	const TSharedPtr<FSocket, ESPMode::ThreadSafe>& new_socket_high,
	const TSharedPtr<FSocket, ESPMode::ThreadSafe>& new_socket_low,
	const TSharedPtr<FSocket, ESPMode::ThreadSafe>& new_socket_adaptive
) {
	sender_socket_high = new_socket_high;
	sender_socket_low = new_socket_low;
	sender_socket_adaptive = new_socket_adaptive;
}

void FBristleconeSender::ActivateDSCP()
{

	//this no-ops on many platforms.
	//TODO: If you want to take this to general production grade, this will need a behavior for most platforms.
	//Fortunately, Linux, mac, steam deck, and many other platforms will actually be simpler, as those allow dscp
	//to be set normally, instead of requiring qos manipulation.
	// 
	//The outliers are switch and SDR, and I don't even know that you'd ever use bristlecone with SDR, as it's basically
	// a successor system with a narrow application space.
	
	//quite a lot of ungood things have to happen for us to do this. As a result, I'll be writing out what we're doing.
	// https://learn.microsoft.com/en-us/windows/win32/api/qos2/nf-qos2-qosaddsockettoflow
	//https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/netds/Qos/Qos2/qossample.c
	//This is probably the best example I can provide for WHAT is happening.
	//Bear in mind that DSCP settings may literally do nothing, so this is quite a bit of work to find out.
#if PLATFORM_HAS_BSD_SOCKET_FEATURE_WINSOCKETS
	QOS_VERSION Version;
	SOCKADDR_IN destination;
	HANDLE      QoSHandle = NULL;


	// Initialize the QoS version parameter.
	Version.MajorVersion = 1;
	Version.MinorVersion = 0;
	QOS_FLOWID     QoSFlowId = 0;
	destination.sin_family = AF_INET;
	destination.sin_port = target_endpoints.Last().Port;
	destination.sin_addr.s_addr = target_endpoints.Last().Address.Value;
	// Get a handle to the QoS subsystem. this requires us to have the qwave lib file loaded to resolve the symbol. Oddly, you can't load the dll.

	QOSCreateHandle(
		&Version,
		&QoSHandle);
	//this is necessary because fsocket does not have a get native, as not all abstracted sockets actually have a native file-like socket
	//our build mechanism has to break encapsulation pretty aggressively to resolve this, and it's quite ugly.
	SOCKET underlyingHigh = ((FSocketBSD*)(sender_socket_high.Get()))->GetNativeSocket();// Time to go for a very bad ride.
	SOCKET underlyingLow = ((FSocketBSD*)(sender_socket_low.Get()))->GetNativeSocket();
	SOCKET underlyingAdaptive = ((FSocketBSD*)(sender_socket_adaptive.Get()))->GetNativeSocket();
	//qwave MAY need destination and point. as a result, we had to wait to perform this until now.

	QOSAddSocketToFlow(QoSHandle, underlyingHigh, (SOCKADDR*)&destination, QOS_TRAFFIC_TYPE::QOSTrafficTypeControl, QOS_NON_ADAPTIVE_FLOW, &QoSFlowId);
	QoSFlowId = 0;
	QOSAddSocketToFlow(QoSHandle, underlyingLow, (SOCKADDR*)&destination, QOS_TRAFFIC_TYPE::QOSTrafficTypeControl, QOS_NON_ADAPTIVE_FLOW, &QoSFlowId);
	QoSFlowId = 0;
	QOSAddSocketToFlow(QoSHandle, underlyingAdaptive, (SOCKADDR*)&destination, QOS_TRAFFIC_TYPE::QOSTrafficTypeExcellentEffort, 0, &QoSFlowId);
#endif
}

bool FBristleconeSender::Init() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Sender: Initializing Bristlecone Sender thread"));
	socket_subsystem.Reset(ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM));
	
	running = true;
	return true;
}

uint32 FBristleconeSender::Run() {
	int counter = 0;
	FControllerState sending_state;
	
	while(sender_socket_high) {
		// Update ring array
		counter++;
		sending_state.controller_arr[0] = counter;
		packet_container.InsertNewDatagram(&sending_state);
		
		for (auto& endpoint : target_endpoints) {
			int32 bytes_sent;

			const FControllerStatePacket* current_controller_state = packet_container.GetPacket();

			bool packet_sent = sender_socket_high->SendTo(reinterpret_cast<const uint8*>(current_controller_state), sizeof(FControllerStatePacket),
														   bytes_sent, *endpoint.ToInternetAddr());
			
			packet_sent = sender_socket_low->SendTo(reinterpret_cast<const uint8*>(current_controller_state), sizeof(FControllerStatePacket),
				bytes_sent, *endpoint.ToInternetAddr());
			
			packet_sent = sender_socket_adaptive->SendTo(reinterpret_cast<const uint8*>(current_controller_state), sizeof(FControllerStatePacket),
				bytes_sent, *endpoint.ToInternetAddr());
			UE_LOG(LogTemp, Warning, TEXT("Sent messages: %s : %s : Address - %s : BytesSent - %d"), *current_controller_state->ToString(),
				(packet_sent ? TEXT("true") : TEXT("false")), *endpoint.ToString(), bytes_sent);

			if (bytes_sent == 0) {
				consecutive_zero_bytes_sent++;
			} else {
				consecutive_zero_bytes_sent = 0;
			}
			
		}

		FPlatformProcess::Sleep(SLEEP_TIME_BETWEEN_THREAD_TICKS);
	}
	
	return 0;
}

void FBristleconeSender::Exit() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Sender: Exiting Bristlecone sender thread."));
	Cleanup();
}

void FBristleconeSender::Stop() {
	UE_LOG(LogTemp, Display, TEXT("Bristlecone:Sender: Stopping Bristlecone sender thread."));
	Cleanup();
}

void FBristleconeSender::Cleanup() {
	sender_socket_high = nullptr;
	sender_socket_low = nullptr;
	sender_socket_adaptive = nullptr;
	const ISocketSubsystem* socket_subsystem_obj = socket_subsystem.Release();
	if (socket_subsystem_obj != nullptr) {
		socket_subsystem_obj = nullptr;
	}
	target_endpoints.Empty();
	running = false;
}

