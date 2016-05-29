#pragma once
#include "DEFS.h"

template<int RATE>
struct Counter
{
	WORD p;

	Counter()
	{
		p = 1 << 15;
	}

	void UpdateBit0()
	{
		p -= p >> RATE;
	}

	void UpdateBit1()
	{
		p += (p ^ 65535) >> RATE;
	}
};
