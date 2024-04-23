#pragma once

#include "CoreMinimal.h"
#include "FBristleconePackingSystemShim.h"
#include "FCablePackedInput.h"
THIRD_PARTY_INCLUDES_START
#include <concrt.h>

#include "winrt/Windows.Gaming.Input.h"
THIRD_PARTY_INCLUDES_END
namespace RawInput = winrt::Windows::Gaming::Input;


class FCabling : public FRunnable {
public:
	FCabling();
	virtual ~FCabling() override;

	virtual bool Init() override;
	virtual uint32 Run() override
	{

		while (running)
		{
			//game pad reading
			//push to both queues.
			//wake bristlecone
			FPlatformProcess::Sleep(1/512);
		}
	};
	virtual void Exit() override;
	virtual void Stop() override;

private:
	void Cleanup();

	bool running;
};
