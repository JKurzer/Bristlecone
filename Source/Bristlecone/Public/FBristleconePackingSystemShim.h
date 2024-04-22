#pragma once

#include <chrono>

#pragma pack(0)
struct SixteenByter {
	uint64_t high;
	uint64_t low;
};
typedef SixteenByter Bigby;
#pragma pack (pop)
/**
 * 
 * In the long term, we'll wish to support SBE (SimpleBinaryEncoding) :
 * https://github.com/real-logic/simple-binary-encoding/tree/master
 
 * However, in the meantime, I've opted to follow my own advice in the standard. 
 * So: This ENTIRE implementation ONLY supports 8 and 16 byte messages.
 * 
 * The problem with SBE  is that it still labels each message with the wireformat, which we actually don't need!
 * 
 * Let me explain. Because each datagram gets a unique port:ip combo, we always know what schema is being
 * transmitted to us and by us. Datagram sizes are deterministic. Any datagrams that don't match a known
 * {size, port, ip, frequency} -> schema mapping? They're just rejected.
 * 
 * This matters because our messages are so small that including an 8byte header like SBE would either double
 * the size of our message if we did it per clone_type, or significantly increase it if we did it on a packet
 * level. Specifically, about a 10% increase even at 16b messages. We'll eventually switch to it, but I'll need
 * time I don't have right now to find a way around this. As a result, there's a shim here where SBE will go.
 * 
 * 
 * Implementation Notes:
 * The right way to do this is with concepts and constraints, but unfortunately, UE is highly resistant to
 * the use of templated classes in ways that horrify me. These shims will allow us to cleanly add ACTUAL
 * schemas later while getting solid-ish typesafety now. I've provided tag templates as well, to meet
 * the needs of classes that have an existing base class without requiring multiple inheritance,
 * as the diamond pattern is considered undesirable in libraries.
 * 
 * These types may add an RVal copy op. it shouldn't really matter at our scale, but if you're profiling this
 * and looking for perf, that's a candidate. I don't want to make any assumptions about memory management, so I've left it.
 * 
 * Wire Size Note: 
 * Generally the minimum size of an ethernet packet is 64b. This means that pack8 and pack16 don't have much diff
 * in size on the wire, 64b vs 77b but pack8 bristlecone payloads also leave more space for additional per clone-set data. 
 *  
 * Like MTU, minimums aren't specified that I could find, more just settled on? So I can't say what the wifi min is.
 * 
 */

class Packable8 : Packable {
public:
	char* Pack() override
	{
		return (char*)PackImpl();
	};
	int GetPackedSize() override
	{
		return 8;
	}

	virtual uint64_t PackImpl() = 0;

};



class Packable16: Packable {
public:
	char* Pack() override
	{
		return (char*)PackImpl();
	};
	int GetPackedSize() override
	{
		return 16;
	}

	virtual Bigby* PackImpl() = 0;
};

template <class T> class TagPack16 :Packable16
{
	Bigby* PackImpl() override
	{
		return T.PackImpl();
	};
};

template <class T> class TagPack8 :Packable8
{
	uint64_t* PackImpl() override
	{
		return T.PackImpl();
	}
};

// Direct Inheritance Contraindicated
// You should not inherit directly from packable. I'm allowing it because it MIGHT be necessary
// but it's not a good idea and you should consider it carefully.
// 
// WARNING: High Change Zone 
// Packable will see virtual members added when we implement SBE!
class Packable {
	virtual char* Pack() = 0;
	virtual int GetPackedSize() = 0;
};