#pragma once

#include "CoreMinimal.h"
#include "FBristleconePackingSystemShim.h"
#include "FCablePackedInput.h"
#include <chrono> 

#include "Containers/CircularQueue.h"


//why do it this way?
//well, unfortunately, input in UE runs through the event loop.
//at high sample rates, this can blow the event loop to hell, 
//and it's always going to be dependent on the gamethread.
//at best, it's a tidge slow and tick-rate dependent.
//at worst, it goes boom. 
//Cabling is NOT a general purpose or complete solution. It is a thin layer
//provided as a jumping off point for working in this space.
class FCabling : public FRunnable {
public:
	FCabling();
	virtual ~FCabling() override;

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;
	uint32_t getSlicedMicrosecondNow()
	{

		using namespace std::chrono;
		return duration_cast<std::chrono::duration<uint32_t, std::micro>>(steady_clock::now().time_since_epoch()).count();
	};

	bool running;//cabling will let anyone unplug it. cabling is inanimate. cabling has no opinions on this.
	TSharedPtr<TCircularQueue<uint64_t>> GameThreadControlQueue;
	TSharedPtr<TCircularQueue<uint64_t>> CabledThreadControlQueue;
	FSharedEventRef WakeTransmitThread;
private:
	void Cleanup();

	};
