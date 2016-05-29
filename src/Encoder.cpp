#include "Encoder.h"

Encoder::Encoder( FILE* output)
{
	out = output;
	Init();
}

Encoder::Encoder()
{
	Init();
}

void Encoder::Init()
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


Encoder::~Encoder()
{
}



void Encoder::Encode(int c)
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

		const int bit = c & 128;
		c += c;

		if (bit)
		{
			Encoder::EncodeBit1(p + ssep + ssep + ssep);

			counter0[ctx].UpdateBit1();
			counter1[c1][ctx].UpdateBit1();
			counter2[f][ctx][j].UpdateBit1();
			counter2[f][ctx][j + 1].UpdateBit1();

			ctx += ctx + 1;
		}
		else
		{
			Encoder::EncodeBit0(p + ssep + ssep + ssep);

			counter0[ctx].UpdateBit0();
			counter1[c1][ctx].UpdateBit0();
			counter2[f][ctx][j].UpdateBit0();
			counter2[f][ctx][j + 1].UpdateBit0();

			ctx += ctx;
		}
	}

	c2 = c1;
	c1 = ctx & 255;
}

void Encoder::Flush()
{
	for (int i = 0; i < 4; ++i)
	{
		putc(low >> 24, out);
		low <<= 8;
	}
}


void Encoder::EncodeBit1(unsigned int p)
{
#ifdef _WIN64
	high = low + ((QWORD(high - low)*p) >> 18);
#else
	high = low + ((QWORD(high - low)*(p << (32 - 18))) >> 32);
#endif
	while ((low^high) < (1 << 24))
	{
		putc(low >> 24, out);
		low <<= 8;
		high = (high << 8) + 255;
	}
}


void Encoder::EncodeBit0(unsigned int p)
{
#ifdef _WIN64
	low += ((QWORD(high - low)*p) >> 18) + 1;
#else
	low += ((QWORD(high - low)*(p << (32 - 18))) >> 32) + 1;
#endif
	while ((low^high) < (1 << 24))
	{
		putc(low >> 24, out);
		low <<= 8;
		high = (high << 8) + 255;
	}
}
