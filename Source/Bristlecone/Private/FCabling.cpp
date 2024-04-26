#include "FCabling.h"


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



uint32 FCabling::Run() {
	
	
	bool sent = false;
	int seqNumber = 0;
	uint64_t priorReading = 0;

	while (running)
	{
		// need to solve : LogLoad: (Engine Initialization) Total time: 44.80 seconds
		// LogGameInput: Error: [FGameInputWindowsModule] Failed to create a GameInput device!GAME_INPUT_SUPPORT is false!
		if((seqNumber%512) == 500)
		{
			//this is was indeed unsafe.
			//concurrency::critical_section::scoped_lock{ controllerSetLock };
			// current = myGamepads.empty() ? nullptr : &(*myGamepads.begin()); //this needs to be revised.
		}

		//game pad reading
		if (false)///current != nullptr)
		{
			if (!sent)
			{
				//TODO: all of this needs to swap to 
				//https://learn.microsoft.com/en-us/gaming/gdk/_content/gc/input/overviews/input-overview
				//because winrt is basically unmaintained and doesn't work unless you have both a window and a message pump
				//since those are exactly what we wanna avoid, welp.... RIP AND TEAR.
				//this is why we fucking encapsulated this, though.
				//FCableInputPacker boxing;
				//boxing.lx = boxing.IntegerizedStick(cur.LeftThumbstickX);
				////these are coming up empty.
				//boxing.ly = boxing.IntegerizedStick(cur.LeftThumbstickY);
				//boxing.rx = boxing.IntegerizedStick(cur.RightThumbstickX);
				//boxing.ry = boxing.IntegerizedStick(cur.RightThumbstickY);
				//boxing.buttons = (uint32_t)cur.Buttons;
				//boxing.buttons.set(12, (cur.LeftTrigger > 0.5)); //check the bitfield.
				//boxing.buttons.set(13, (cur.RightTrigger > 0.5));
				//boxing.events = boxing.events.none();
				//uint64_t currentRead = boxing.PackImpl();
				/*
				current = current	<< 1;
				current |= (cur.RightTrigger > 0.5);
				current = current	<< 1;
				current |= (cur.LeftTrigger	 > 0.5);
				current = current << 14;
				current |= (uint32_t)cur.Buttons; // upcast to uint, we use their ordering. then mask away the paddles.
				current &= 0b00000000000000000011111111111111;	//00000000 00000000 00111111 11111111
				current = current << 10;
				*/
				//events go here.
				if ((seqNumber % 4) == 0 )//|| (currentRead != priorReading))
				{
					//UE_LOG(LogTemp, Warning, TEXT("@, Received,  %lld"), currentRead);
				}
				//priorReading = currentRead;
			}		
		}
		

		//obv, we'll need some way to figure out WHICH pad. elided for now!
		//disconnected pads is actually easy, that's handled by the api.
		//oddly though, it's hard to figure out a good heuristic for pads that
		//aren't in use but are connected, like wired gamepads. 
		
		//initial packing goes here!

		//push to both queues.
		//wake bristlecone

		// originally this was 512, but I don't like acquiring a lock 512 times a second much
		//no great reason, it's pretty fast these days. we should crank this up and sample more but we
		//only plan to transmit at about 120hz. We're just sampling often to try to catch input
		//as early as we can. We can also make use of highly granular movement to do some
		//input prediction. That gets easier the more granularity you have.
		++seqNumber;
		if((seqNumber % 4) == 0)
		{
			sent = false;
		}
		FPlatformProcess::Sleep(1.0f / 512); 

	}
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

