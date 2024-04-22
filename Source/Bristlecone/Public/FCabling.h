#pragma once

#include "CoreMinimal.h"

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
