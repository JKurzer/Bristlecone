// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "FBristleconePacket.h"
#include "FControllerState.h"
#include "Containers/CircularQueue.h"
#include <cstdint>

//centralizing the typedefs to avoid circularized header includes
//and further ease swapping over between 8 and 16 byte modes. IWYU!
namespace TheCone {
	typedef FBristleconePacket<uint64_t, 3> Packet_tpl;
	typedef TCircularQueue<Packet_tpl> PacketQ;
	typedef TSharedPtr<PacketQ> RecvQueue;
	typedef TSharedPtr<TCircularQueue<uint64_t>> SendQueue;
	typedef FBristleconePacket<FControllerState, 3> FControllerStatePacket;

	static constexpr int CONTROLLER_STATE_PACKET_SIZE = sizeof(FControllerStatePacket);
	static constexpr int DEFAULT_PORT = 40000;
	static constexpr uint16 MAX_TARGET_COUNT = 1;
	static constexpr float SLEEP_TIME_BETWEEN_THREAD_TICKS = 0.016f;
	static constexpr uint8 CLONE_SIZE = 3;
	static constexpr uint8 MAX_MIXED_CONSECUTIVE_PACKETS_ALLOWED = 100;
}