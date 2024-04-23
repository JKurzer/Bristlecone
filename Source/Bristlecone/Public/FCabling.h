#pragma once

#include "CoreMinimal.h"
#include "FBristleconePackingSystemShim.h"
#include "FCablePackedInput.h"
THIRD_PARTY_INCLUDES_START
#include <concrt.h>

#include "winrt/Windows.Gaming.Input.h"
THIRD_PARTY_INCLUDES_END
namespace RawInput = winrt::Windows::Gaming::Input;

//((unsigned long&)y) & 0x7FFFFF grabs the last 23 bits of a 


class FCabling : public FRunnable {
public:
	FCabling();

	virtual ~FCabling() override;

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Exit() override;
	virtual void Stop() override;

private:
	void Cleanup();

	bool running;
};
