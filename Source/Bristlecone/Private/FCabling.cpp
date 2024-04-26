#include "FCabling.h"
THIRD_PARTY_INCLUDES_START
#include "Microsoft/AllowMicrosoftPlatformTypes.h"
#include "GameInput.h"
#include "Microsoft/HideMicrosoftPlatformTypes.h"
THIRD_PARTY_INCLUDES_END

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

	//We're using the GameInput lib.
	//https://learn.microsoft.com/en-us/gaming/gdk/_content/gc/input/overviews/input-overview
	while (running)
	{
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
				boxing.lx = boxing.IntegerizedStick(state.leftThumbstickX);
				boxing.ly = boxing.IntegerizedStick(state.leftThumbstickY);
				boxing.rx = boxing.IntegerizedStick(state.rightThumbstickX);
				boxing.ry = boxing.IntegerizedStick(state.rightThumbstickY);
				boxing.buttons = (uint32_t)state.buttons; //strikingly, there's no paddle field.
				boxing.buttons.set(12, (state.leftTrigger > 0.5)); //check the bitfield.
				boxing.buttons.set(13, (state.rightTrigger > 0.5));
				boxing.events = boxing.events.none();
				uint64_t currentRead = boxing.PackImpl();

				if ((seqNumber % 4) == 0 || (currentRead != priorReading))
				{
					//push to both queues.
					//wake bristlecone
					UE_LOG(LogTemp, Warning, TEXT("@, Received,  %lld"), currentRead);
				}
				priorReading = currentRead;
			}



			++seqNumber;
			if ((seqNumber % 4) == 0)
			{
				sent = false;
			}
			FPlatformProcess::Sleep(1.0f / 512);
		}
		else if (g_gamepad)
		{
			g_gamepad->Release();
			g_gamepad = nullptr;
			FPlatformProcess::Sleep(1.0f / 512);
			continue;
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

