#pragma once
#include "Counter.h"

class Encoder
{
public:
	FILE* out;

	Encoder();
	Encoder(FILE*);
	~Encoder();

	void Init();

	void Encode(int c);
	void EncodeBit1(unsigned int p);
	void EncodeBit0(unsigned int p);

	void Flush();


private:
	unsigned int low, high, code;
	int c1, c2, run;

	Counter<2> counter0[256];
	Counter<4> counter1[256][256];
	Counter<6> counter2[2][256][17];

};

