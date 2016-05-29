#pragma once
#include "Compressor.h"

using std::wstring;

#define ENCODED_FILES_DIR "EncodedFiles"
#define DECODED_FILES_DIR "DecodedFiles"
#define TEST_FILES "test"
#define DECODE_COMMAND "-d"
#define ENCODE_COMMAND "-c"

class AppManager
{

private:
	bool bDecode;
	double totalCompressed,totalUncompressed;
	std::vector< std::pair<wstring, wstring> > filePaths;

	//Print the Final Reduction and Compression ratios
	void PrintFinalStatistics();

public:
	AppManager();
	~AppManager();

	// Read command line arguments (i.e. whether to decode or encode files and build files paths)
	void ReadCMD(int argc, char** argv);
	
	// Compressess/Decompress files and Print final statistics
	void Execute();
	
};

