// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "FBristleconePacket.h"
#include "Containers/CircularQueue.h"
#include <cstdint>

//centralizing the typedefs to avoid circularized header includes
//and further ease swapping over between 8 and 16 byte modes. IWYU!
namespace TheCone {
	typedef FBristleconePacket<uint64_t, 3> Packet_tpl;
	typedef TCircularQueue<Packet_tpl> PacketQ;
	typedef TSharedPtr<PacketQ> QueueRecvEight;
	typedef TSharedPtr<TCircularQueue<uint64_t>> SendQueue;
	typedef FBristleconePacket<FControllerState, 3> FControllerStatePacket;

	static constexpr int CONTROLLER_STATE_PACKET_SIZE = sizeof(FControllerStatePacket);
	static constexpr int DEFAULT_PORT = 40000;
	static constexpr uint16 MAX_TARGET_COUNT = 1;
	static constexpr float SLEEP_TIME_BETWEEN_THREAD_TICKS = 0.016f;
}