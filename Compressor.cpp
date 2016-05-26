#include "Compressor.h"

Compressor::Compressor(bool b)
{
	bDecode = b;
	blockSize = 32 << 20; // Default size is 32MGB

	compressionRatio = reductionRatio = 0;
}

Compressor::~Compressor()
{

}

void Compressor::Run(std::string sourceStr, std::string destStr)
{
	InitializeInputOutputFiles(sourceStr.c_str(), destStr.c_str());

	if (bDecode)
	{
		Decompress();
	}
	else
	{
		Compress();
	}
}

void Compressor::Compress()
{

	if (_fseeki64(inputFile, 0, SEEK_END))
	{
		perror("Fseek() failed");
		exit(1);
	}

	const long long fileLengtth = _ftelli64(inputFile);
	if (fileLengtth <= 0)
	{
		perror("Ftell() failed");
		exit(1);
	}

	if (blockSize > fileLengtth)
		blockSize = int(fileLengtth);
	rewind(inputFile);

	buffer = (BYTE*)calloc(blockSize, 5);

	if (!buffer)
	{
		fprintf(stderr, "Out of memory!\n");
		exit(1);
	}

	int blocksCount;
	while ((blocksCount = fread(buffer, 1, blockSize, inputFile)) > 0) //reading blocks count and transofrm it.
	{
		// returns the original string id in the transoformation matrix
		const int idx = divbwt(buffer, buffer, (int*)&buffer[blockSize], blocksCount); 
		if (idx < 1)
		{
			perror("Transformation Failed!");
			exit(1);
		}

		//DMC starts
		encoder.Encode(blocksCount >> 24);
		encoder.Encode(blocksCount >> 16);
		encoder.Encode(blocksCount >> 8);
		encoder.Encode(blocksCount);
		encoder.Encode(idx >> 24);
		encoder.Encode(idx >> 16);
		encoder.Encode(idx >> 8);
		encoder.Encode(idx);

		for (int i = 0; i < blocksCount; ++i)
			encoder.Encode(buffer[i]);
	}

	//END OF FILE
	encoder.Encode(0);

	encoder.Flush();
}

void Compressor::Decompress()
{
	decoder.Init();

	int bsize = 0;

	while(1)
	{
		//Inverse DMC	
		const int n = (decoder.Decode() << 24)
			| (decoder.Decode() << 16)
			| (decoder.Decode() << 8)
			| decoder.Decode();

		if (!n) // EOF
			break;
		if (!bsize)
		{
			buffer = (BYTE*)calloc(bsize = n, 5);
			if (!buffer)
			{
				fprintf(stderr, "Out of memory!\n");
				exit(1);
			}
		}
		const int idx = (decoder.Decode() << 24)
			| (decoder.Decode() << 16)
			| (decoder.Decode() << 8)
			| decoder.Decode();
		if (n<1 || n>bsize || idx<1 || idx>n)
		{
			fprintf(stderr, "File corrupted!\n");
			exit(1);
		}


		// Inverse BWT
		int t[257] = { 0 };
		for (int i = 0; i < n; ++i)
			++t[(buffer[i] = decoder.Decode()) + 1];

		for (int i = 1; i < 256; ++i)
			t[i] += t[i - 1];

		int* next = (int*)&buffer[bsize];
		for (int i = 0; i < n; ++i)
			next[t[buffer[i]]++] = i + (i >= idx);

		for (int p = idx; p;)
		{
			p = next[p - 1];
			putc(buffer[p - (p >= idx)], outputFile);
		}
	}
}


#pragma region Helpers

void Compressor::InitializeInputOutputFiles(const char* sourcePath, const char* destPath)
{
	inputFile = fopen(sourcePath, "rb");

	if (!inputFile)
	{
		perror(sourcePath);
		exit(1);
	}

	std::string outputFileName = destPath;
	outputFileName = outputFileName.substr(0, outputFileName.size() - 4);

	outputFileName += ((!bDecode) ? ".bin" : ".txt");

	outputFile = fopen(outputFileName.c_str(), "wb"); //write binary
	if (!outputFile)
	{
		perror(outputFileName.c_str());
		exit(1);
	}

	encoder.out = outputFile;
	decoder.in = inputFile;

	fprintf(stderr, "%*s:\t", 50, sourcePath);
	fflush(stderr);
}

void Compressor::CalculateRatios()
{
	compressionRatio = !bDecode ? double(_ftelli64(inputFile)) / _ftelli64(outputFile) : double(_ftelli64(outputFile)) / _ftelli64(inputFile);
	reductionRatio = !bDecode ? double(_ftelli64(outputFile)) / _ftelli64(inputFile) : double(_ftelli64(inputFile)) / _ftelli64(outputFile);
	reductionRatio *= 100;
}

double Compressor::GetCompressedSize()
{
	return !bDecode ? double(_ftelli64(outputFile)) : _ftelli64(inputFile);
}

double Compressor::GetUncompressedSize()
{
	return !bDecode ? double(_ftelli64(inputFile)) : _ftelli64(outputFile);
}

void Compressor::Finalize()
{
	CalculateRatios();

	fprintf(stderr, "%lld -> %lld \t Ratio=%.2f \t C/U=%.2f%%\n", _ftelli64(inputFile), _ftelli64(outputFile), compressionRatio, reductionRatio);

	fclose(inputFile);
	fclose(outputFile);

	free(buffer);
}

#pragma endregion
