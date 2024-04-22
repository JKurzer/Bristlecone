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

