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

uint32 FCabling::AddPad(winrt::Windows::Foundation::IInspectable const&, RawishInput::Gamepad const& addedController)
{

	concurrency::critical_section::scoped_lock{ controllerSetLock };
	if (myGamepads.find(addedController) == myGamepads.end())
	{
		myGamepads.insert(addedController);
	}
	return 0;
}

uint32 FCabling::DropPad(winrt::Windows::Foundation::IInspectable const&, RawishInput::Gamepad const& droppedcontroller)
{
	concurrency::critical_section::scoped_lock{ controllerSetLock };
	if (myGamepads.find(droppedcontroller) == myGamepads.end()) //contains was reading as absent, odd given that we should be on C++20.
	{
		myGamepads.erase(droppedcontroller);
	}
	return 0;
}


uint32 FCabling::Run() {
	int ticktrack = 0;
	
	RawishInput::Gamepad::GamepadAdded({ this,&FCabling::AddPad});
	RawishInput::Gamepad::GamepadRemoved({ this,&FCabling::DropPad});
	
	const RawishInput::Gamepad* current = nullptr;
	while (running)
	{
		
		ticktrack = (ticktrack + 1) % 512;
		if(ticktrack == 500)
		{
			//this is likely to be unsafe. I need to check what a disconnected gamepad actually looks like. is it null?
			//unless you use really broad locking, I don't see how you can avoid getting into trouble if so.
			concurrency::critical_section::scoped_lock{ controllerSetLock };
			current = myGamepads.empty() ? nullptr : &(*myGamepads.begin()); //this needs to be revised.
		}

		//game pad reading
		if (current != nullptr)
		{
			RawishInput::GamepadReading cur = current->GetCurrentReading();
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

