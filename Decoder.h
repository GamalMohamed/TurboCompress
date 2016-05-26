#pragma once
#include "Counter.h"

class Decoder
{
public:
	FILE* in;

	Decoder();
	Decoder(FILE*);
	~Decoder();

	void Init();

	int Decode();
	int DecodeBit(unsigned int p);

private:
	unsigned int low, high, code;
	int c1, c2, run;

	Counter<2> counter0[256];
	Counter<4> counter1[256][256];
	Counter<6> counter2[2][256][17];


};

