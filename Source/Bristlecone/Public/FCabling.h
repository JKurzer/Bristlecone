#pragma once

#include "CoreMinimal.h"
#include "FBristleconePackingSystemShim.h"
#include "FCablePackedInput.h"
THIRD_PARTY_INCLUDES_START
#include <concrt.h>

#include "winrt/Windows.Gaming.Input.h"

#include "winrt/Windows.Foundation.h"
THIRD_PARTY_INCLUDES_END
namespace RawishInput = winrt::Windows::Gaming::Input;

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
	uint32 AddPad(winrt::Windows::Foundation::IInspectable const&, RawishInput::Gamepad const& addedController);

	uint32 DropPad(winrt::Windows::Foundation::IInspectable const&, RawishInput::Gamepad const& droppedcontroller);
private:
	void Cleanup();


	concurrency::critical_section controllerSetLock;
	std::unordered_set<RawishInput::Gamepad> myGamepads;
	bool running;
};
