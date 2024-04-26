#pragma once

#include "FBristleconePackingSystemShim.h"
#include <bitset>

//thanks a lot, unreal! If you don't do ALL of this, you'll hit FP errors and this won't be platform independent.
//TODO: And clang may not honor these anyway, so have fun with that, Eliza!
#pragma fp_contract (off)
#pragma float_control (precise, on, push)

#pragma float_control (except, on, push)

//Packed as:
//
//MSB[sticks][buttons][Events]LSB
//integerized:
//11 bits, Left Stick	X
//11 bits, Left Stick	Y
// 
//11 bits, Right Stick	X
//11 bits, Right Stick	Y
//
//Buttons, please read:
//1 bit per button
//in LOWEST TO HIGHEST order
// Menu ,
// View 
// A,
// B,
// X,
// Y,
// DPadUp,
// DPadDown,
// DPadLeft,
// DPadRight,
// LeftShoulder,
// RightShoulder,
// LeftTrigger,
// RightTrigger,
// 
//6 bits:
//Events
// Used as needed.
// NOTE THIS CLASS IS NOT 8 BYTES
class FCableInputPacker : Packable8 {
public:
	std::bitset<11> lx;
	std::bitset<11> ly;
	std::bitset<11> rx;
	std::bitset<11> ry;
	std::bitset<14> buttons;
	std::bitset<6> events;
	//sums to 64! I counted. Like 30 times.
	
	uint64_t PackImpl() override
	{
		uint64_t box = 0;

		box |= lx.to_ullong();
		
		box <<= ly.size();
		box |= ly.to_ullong();

		box <<= rx.size();
		box |= rx.to_ullong();

		box <<= ry.size();
		box |= ry.to_ullong();


		box <<= buttons.size();
		box |= buttons.to_ullong();

		box <<= events.size();
		box |= events.to_ullong();

		return box;
	}

	//This needs to be replaced with a standard fp to fixed point routine, or dekker's algorithm.
	//While I'm fairly sure it "works" okay, I think we lose more precision than we need to
	//and that it's quite overcomplicated for what it does. To be honest, I can't guarantee that
	//we aren't accumulating floating point error when we do this, but the good news is that once it's 
	//packed, we never actually turn it back into a float. Still, as stick values are passed from
	//client a to client b, this is a serious risk to determinism. this will need to be revised.
	uint32_t IntegerizedStick(double axis)
	{
	//3/16 is a dyadic rational. it represents exactly in fp, so we'll use it prune
	//we need a deadzone anyway, and this means we don't need to deal with extremely small floats.
	//this helps ensure that we don't get hammered on bit corruption from FP rounding,
	//though we still get it pretty bad.
	if ((axis <= 0.1875f && axis >= -0.1875f) || (axis > 1.0f || axis < -1.0f))
	{
			return 0; // get deadzoned, lmao.
	}

	uint32_t patientNonZero = 0;
	double contaminatedAdjustment = axis * 1024.0f; // this contaminates QUITE A FEW bits. fortunately...
	int trunc = contaminatedAdjustment; //note this preserves sign as twos complement, so we actually need 11 bits a stick.
	memcpy(&patientNonZero, &trunc, sizeof(trunc));
	return patientNonZero;
	}
};

#pragma float_control(pop)
#pragma float_control(pop)