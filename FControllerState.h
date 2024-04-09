#pragma once

class FControllerState {
public:
	// for now just mocking the size
	int controller_arr[4];
	
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