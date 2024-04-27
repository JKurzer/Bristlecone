#pragma once

class FControllerState {
public:
	// this needs to be defactored but it isn't totally obvious how it SHOULD be set.
	//this might be a good place to refactor the packing shim for, so that we can inject the type elegantly.
	char controller_arr[8];
	
	FControllerState() {
		clear();
	}

	void clear() {
		memset(controller_arr, 0, sizeof(FControllerState));
	}

	FString ToString() const {
		return FString::Printf(TEXT("{ %d, %d, %d, %d }"), controller_arr[0], controller_arr[1], controller_arr[2], controller_arr[3]);
	}
};