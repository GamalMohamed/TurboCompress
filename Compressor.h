#pragma once
#include "Encoder.h"
#include "Decoder.h"

class Compressor
{
public:
	Compressor(bool b);
	~Compressor();

	void Compress();
	void Decompress();
	void Run(std::string sourceStr, std::string destStr);

	void Finalize();

	void CalculateRatios();
	double GetCompressedSize();
	double GetUncompressedSize();


private:

	bool bDecode;
	int blockSize;
	double compressionRatio, reductionRatio;

	BYTE* buffer;
	FILE *inputFile, *outputFile;

	Encoder encoder;
	Decoder decoder;

	void InitializeInputOutputFiles(const char* filePath, const char* destPath);

};
