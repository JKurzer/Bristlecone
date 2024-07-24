﻿#include "FCabling.h"
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
	//TODO: why does this seem to need to be an int? I'm assuming something about type 64/32 coercion or
	//the mod operator is that I knew when I wrote this code, but I no longer remember and it should get a docs note.
	int seqNumber = 0;
	uint64_t priorReading = 0;
	uint64_t currentRead = 0;
	//Hi! Jake here! Reminding you that this will CYCLE
	//That's known. Isn't that fun? :) Don't reorder these, by the way.
	uint32_t lastPollTime = NarrowClock::getSlicedMicrosecondNow();
	uint32_t lsbTime = NarrowClock::getSlicedMicrosecondNow();
	constexpr uint32_t sampleHertz = TheCone::CablingSampleHertz;
	constexpr uint32_t sendHertz = TheCone::BristleconeSendHertz;
	constexpr uint32_t sendHertzFactor = sampleHertz/sendHertz; 
	constexpr uint32_t periodInNano = 1000000 / sampleHertz; //swap to microseconds. standardizing.


	//We're using the GameInput lib.
	//https://learn.microsoft.com/en-us/gaming/gdk/_content/gc/input/overviews/input-overview
	//https://learn.microsoft.com/en-us/gaming/gdk/_content/gc/input/advanced/input-keyboard-mouse will be fun
	/*
	* 
	* Per the docs:
	Rather than using GetNextReading to walk through potentially dozens of historical mouse readings and 
	adding up the deltas, they're accumulated into a virtual positionX and positionY value. From there,
	deltas are calculated by subtracting the positionX and positionY value from the previous readings that 
	were obtained. Individual deltas are accessible when iterating through all intermediate readings, or
	through the accumulated deltas when intermediate readings have been skipped.
	
	The positionX and positionY values are the sum of all movement deltas.
	These values don't correlate with screen-space coordinates in any way. 
	The accumulated delta value is only for the mouse events that a process receives while it has input focus.
	*/

	//https://handmade.network/forums/t/8710-using_microsoft_gameinput_api_with_multiple_controllers#29361
	//Looks like PS4/PS5 won't be too bad, just gotta watch out for Fun Device ID changes.
	while (running)
	{
		if ((lastPollTime + periodInNano) <= lsbTime)
		{

			lastPollTime = lsbTime;
			if (SUCCEEDED(g_gameInput->GetCurrentReading(GameInputKindGamepad, g_gamepad, &reading)))
			{
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
					//TODO: this line might actually be wrong. I just redid the math, and it looks right but...
					if ((seqNumber % sendHertzFactor) == 0 || (currentRead != priorReading))
					{
						//push to both queues.
						this->CabledThreadControlQueue.Get()->Enqueue(currentRead);
						this->GameThreadControlQueue.Get()->Enqueue(currentRead);
						WakeTransmitThread->Trigger();
						sent = true;
					}
					priorReading = currentRead;
				}

			}
			else if (g_gamepad != nullptr)
			{
				g_gamepad->Release();
				g_gamepad = nullptr;
			}

			if ((seqNumber % sampleHertz) == 0)
			{
				long long now = std::chrono::steady_clock::now().time_since_epoch().count();
				UE_LOG(LogTemp, Display, TEXT("Cabling hertz cycled: %lld against lsb %lld with last poll as %lld"), (now), lsbTime, lastPollTime);
			}

			if ((seqNumber % sendHertzFactor) == 0)
			{
				sent = false;
			}
			++seqNumber;
		}
		//if this is the case, we've looped round. rather than verifying, we'll just miss one chance to poll.
		//sequence number is still the actual arbiter, so we'll only send every 4 periods, even if we poll
		//one less or one more time.

		// this is technically a kind of spin lock,
		// checking the steady clock is actually quite a long operation
		std::this_thread::yield(); // but this gets... weird.
		lsbTime = NarrowClock::getSlicedMicrosecondNow();
		
		if (lsbTime < lastPollTime)
		{
			lastPollTime = lsbTime;
		}
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

