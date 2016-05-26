#include "Decoder.h"

Decoder::Decoder(FILE* input)
{
	low = 0;
	high = unsigned int(-1);
	code = 0;
	in = input;
}

Decoder::Decoder()
{
	low = 0;
	high = unsigned int(-1);
	code = 0;

	c1 = 0;
	c2 = 0;
	run = 0;

	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			for (int k = 0; k < 17; ++k)
				counter2[i][j][k].p = (k - (k == 16)) << 12;
		}
	}
}


Decoder::~Decoder()
{
}

void Decoder::Init()
{
	for (int i = 0; i < 4; ++i)
		code = (code << 8) + getc(in);
}


int Decoder::DecodeBit(unsigned int p)
{
#ifdef _WIN64
	const unsigned int mid = low + ((QWORD(high - low)*p) >> 18);
#else
	const unsigned int mid = low + ((QWORD(high - low)*(p << (32 - 18))) >> 32);
#endif
	const int bit = (code <= mid);
	if (bit)
		high = mid;
	else
		low = mid + 1;

	while ((low^high) < (1 << 24))
	{
		low <<= 8;
		high = (high << 8) + 255;
		code = (code << 8) + getc(in);
	}

	return bit;
}


int Decoder::Decode()
{
	if (c1 == c2)
		++run;
	else
		run = 0;
	const int f = (run > 2);

	int ctx = 1;
	while (ctx < 256)
	{
		const int p0 = counter0[ctx].p;
		const int p1 = counter1[c1][ctx].p;
		const int p2 = counter1[c2][ctx].p;
		const int p = (p0 + p0 + p0 + p0 + p1 + p1 + p1 + p2) >> 3;

		const int j = p >> 12;
		const int x1 = counter2[f][ctx][j].p;
		const int x2 = counter2[f][ctx][j + 1].p;
		const int ssep = x1 + (((x2 - x1)*(p & 4095)) >> 12);

		const int bit = DecodeBit(p + ssep + ssep + ssep);

		if (bit)
		{
			counter0[ctx].UpdateBit1();
			counter1[c1][ctx].UpdateBit1();
			counter2[f][ctx][j].UpdateBit1();
			counter2[f][ctx][j + 1].UpdateBit1();
			ctx += ctx + 1;
		}
		else
		{
			counter0[ctx].UpdateBit0();
			counter1[c1][ctx].UpdateBit0();
			counter2[f][ctx][j].UpdateBit0();
			counter2[f][ctx][j + 1].UpdateBit0();
			ctx += ctx;
		}
	}

	c2 = c1;
	return c1 = ctx & 255;
}
