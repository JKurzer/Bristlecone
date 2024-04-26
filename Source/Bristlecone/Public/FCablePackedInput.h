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
//10 bits, Left Stick	X
//10 bits, Left Stick	Y
// 
//10 bits, Right Stick	X
//10 bits, Right Stick	Y
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
//10 bits:
//Events
// Used as needed.
// NOTE THIS CLASS IS NOT 8 BYTES
class FCableInputPacker : Packable8 {
public:
	std::bitset<10> lx;
	std::bitset<10> ly;
	std::bitset<10> rx;
	std::bitset<10> ry;
	std::bitset<14> buttons;
	std::bitset<10> events;
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

	// do not use this on a double. You will have a bad time.
	// I did it this way because it was fun. 
	short IntegerizedStick(float axis)
	{
	#define SIGNSLICE_J (1U << 31)
	#define LEADING_ONE_J (1U << 23)
	#define EXPSLICE_J  (0xff << 23)
	#define MANTSLICE_J (0x7FFFFF) //  grabs the mantissa of a float. We're going to need that where we're going.
	#define DEBIAS_J (-127)
	//3/16 is a dyadic rational. it represents exactly in fp, so we'll use it prune
	//we need a deadzone anyway, and this means we don't need to deal with extremely small floats.
	if ((axis <= 0.1875f && axis >= -0.1875f) || (axis > 1.0f || axis < -1.0f))
	{
			return 0; // get deadzoned, lmao.
	}

	//TODO: we really should normalize around the deadzone to recover that part of the range? i think?
	uint32_t patientNonZero = 0;
	memcpy(&patientNonZero, &axis, sizeof(axis));
	uint32_t sign	= (patientNonZero & SIGNSLICE_J);
	uint32_t exp	= (patientNonZero & EXPSLICE_J) >> 23;
	//"in order to get the true exponent as defined by the offset-binary representation, 
	// the offset of 127 has to be subtracted from the stored exponent."
	exp += DEBIAS_J;
	uint32_t mant	= (patientNonZero & MANTSLICE_J); //note the implicit leading one is not automatically captured.
	mant += LEADING_ONE_J; // equivalent to bit-and here. this is now the actual significand.

	//I actually think this line is wrong. Something about the exp shift bugs me. I'm gonna need to sit with it.
	// for now, it's fine, we aren't moving data around yet, this is just proving the packing out.
	int16_t box = sign * ((mant >> 12) << exp); // trim to 10 bits of precision then exp, sign, and truncate. i think?
	//that's intentionally an int32. we're basically going to end up with a number from 512 to -511. I hope.
	return box;
	#undef SIGNSLICE_J
	#undef EXPSLICE_J
	#undef MANTSLICE_J
	#undef DEBIAS_J
	}
};

#pragma float_control(pop)
#pragma float_control(pop)