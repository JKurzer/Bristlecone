#include "FCabling.h"
THIRD_PARTY_INCLUDES_START
#include "Microsoft/AllowMicrosoftPlatformTypes.h"
#include "GameInput.h"
#include "Microsoft/HideMicrosoftPlatformTypes.h"
THIRD_PARTY_INCLUDES_END
#include <bitset>
#include <thread>
using std::bitset;

FCabling::FCabling()
: running(false) {

}

FCabling::~FCabling() {
}



bool FCabling::Init() {
	UE_LOG(LogTemp, Display, TEXT("FCabling: Initializing Control Intercept thread"));
	
	running = true;
	return true;
}


//this is based directly on the gameinput sample code.
uint32 FCabling::Run() {
	
	IGameInput* g_gameInput = nullptr;
	HRESULT gameInputSpunUp = GameInputCreate(&g_gameInput);
	IGameInputDevice* g_gamepad = nullptr;
	IGameInputReading* reading;
	bool sent = false;
	int seqNumber = 0;
	uint64_t priorReading = 0;
	uint64_t currentRead = 0;
	//Hi! Jake here! Reminding you that this will CYCLE every 2100 milliseconds or so.
	//That's known. Isn't that fun? :) 
	uint32_t lsbTime = 0x00000000FFFFFFFF & (std::chrono::steady_clock::now().time_since_epoch().count());
	uint32_t lastPoll = 0;
	uint32_t periodInNano = 1900000;
	uint32_t hertz = 512;
	

	//We're using the GameInput lib.
	//https://learn.microsoft.com/en-us/gaming/gdk/_content/gc/input/overviews/input-overview
	while (running)
	{
		if ((lastPoll + periodInNano) <= lsbTime && SUCCEEDED(g_gameInput->GetCurrentReading(GameInputKindGamepad, g_gamepad, &reading)))
		{
			lastPoll = lsbTime;
			// If no device has been assigned to g_gamepad yet, set it
			// to the first device we receive input from. (This must be
			// the one the player is using because it's generating input.)
			if (!g_gamepad) reading->GetDevice(&g_gamepad);

			// Retrieve the fixed-format gamepad state from the reading.
			GameInputGamepadState state;
			//This is a struct of form:
			//GameInputGamepadButtons buttons
			//float leftTrigger
			//float rightTrigger
			//float leftThumbstickX
			//float leftThumbstickY
			//float rightThumbstickX
			//float rightThumbstickY
			//And we'll integerize the sticks, convert the triggers into thresholded buttons
			//then repack the buttons and ship.

			reading->GetGamepadState(&state);
			reading->Release();

			if (!sent)
			{
				FCableInputPacker boxing;
				//very fun story. unless you explicitly import and use std::bitset
				//the wrong thing happens here. I'm not going to speculate on why, because
				//I don't think I can do so without swearing extensively.
				boxing.lx = (uint32_t)boxing.IntegerizedStick(state.leftThumbstickX);
				boxing.ly = (uint32_t)boxing.IntegerizedStick(state.leftThumbstickY);
				boxing.rx = (uint32_t)boxing.IntegerizedStick(state.rightThumbstickX);
				boxing.ry = (uint32_t)boxing.IntegerizedStick(state.rightThumbstickY);
				boxing.buttons = (uint32_t)state.buttons; //strikingly, there's no paddle field.
				boxing.buttons.set(12, (state.leftTrigger > 0.55)); //check the bitfield.
				boxing.buttons.set(13, (state.rightTrigger > 0.55));
				boxing.events = 0;
				currentRead = boxing.PackImpl();

				//because we deadzone and integerize, we actually have a pretty good idea
				//of when input actually changes. 2048 positions for the stick along each axis
				//actually looks like it's enough to give us precise movement while still
				//excising some amount of jitter. Because we always round down, you have to move
				//fully to a new position and this seems to be a larger delta than the average
				//heart-rate jitter or control noise.
				if ((seqNumber % 4) == 0 || (currentRead != priorReading))
				{
					//push to both queues.
					this->CabledThreadControlQueue.Get()->Enqueue(currentRead);
					this->GameThreadControlQueue.Get()->Enqueue(currentRead);
					WakeTransmitThread->Trigger();
					sent = true;
					//wake bristlecone
				}
				priorReading = currentRead;
			}

			//if this is the case, we've looped round. rather than verifying, we'll just miss one chance to poll.
			//sequence number is still the actual arbiter, so we'll only send every 4 periods, even if we poll
			//one less or one more time.
			if (lsbTime < lastPoll)
			{
				lastPoll = 0;
			}

			if ((seqNumber % hertz) == 0)
			{
				long long now = std::chrono::steady_clock::now().time_since_epoch().count();
				UE_LOG(LogTemp, Display, TEXT("Cabling seq int 128, %lld"), (now/1000000000));
			}

			if ((seqNumber % 4) == 0)
			{
				sent = false;
			}
			++seqNumber;
		}
		// this is technically a kind of spin lock,
		// checking the steady clock is actually quite a long operation
		std::this_thread::yield(); // but this gets... weird.
		lsbTime = 0x00000000FFFFFFFF & std::chrono::steady_clock::now().time_since_epoch().count(); 
	}
	if (g_gamepad) g_gamepad->Release();
	if (g_gameInput) g_gameInput->Release();
	return 0;
}

void FCabling::Exit() {
	Cleanup();
}

void FCabling::Stop() {
	Cleanup();
}

void FCabling::Cleanup() {
	running = false;
}

