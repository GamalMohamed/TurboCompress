#include "AppManager.h"

//Construct files directories
static void GetFilesInDirectory(std::vector< std::pair<wstring, wstring> > &out, const wstring &sourceDirectory, const wstring &destDirectory)
{
	HANDLE dir;
	WIN32_FIND_DATA file_data;

	if ((dir = FindFirstFile((sourceDirectory + L"/*").c_str(), &file_data)) == INVALID_HANDLE_VALUE)
		return; /* No files found */

	do
	{
		const wstring file_name = file_data.cFileName;
		const wstring source_file_name = sourceDirectory + L"\\" + file_name;
		const wstring dest_file_name = destDirectory + L"\\" + file_name;
		const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		if (file_name[0] == '.')
			continue;

		if (is_directory)
			continue;

		out.push_back(make_pair(source_file_name, dest_file_name));

	} while (FindNextFile(dir, &file_data));

	FindClose(dir);

}


AppManager::AppManager()
{
	bDecode = false;
	totalCompressed = totalUncompressed = 0;
}

AppManager::~AppManager()
{
}

void AppManager::ReadCMD(int argc, char** argv)
{
	argc = 4;
	
	argv[1] = ENCODE_COMMAND;
	argv[2] = TEST_FILES;
	argv[3] = ENCODED_FILES_DIR;

	/*
	argv[1] = DECODE_COMMAND;
	argv[2] = ENCODED_FILES_DIR;
	argv[3] = DECODED_FILES_DIR;
	*/

	if (argc > 1 && *argv[1] == '-')
	{
		if (argv[1][1] == 'd')
		{
			bDecode = true;
		}
		else if (argv[1][1] == 'c')
		{
			bDecode = false;
		}
		else
		{
			fprintf(stderr, "Unknown option: %s\n", argv[1]);
			exit(1);
		}
	}
	else
	{
		fprintf(stderr, "Usage: BWTCompression [options] inputFolder outputFolder\nOptions:\n-c\tCompress\n-d\tDecompress\n");
		exit(1);
	}

	//Construct Paths
	std::string sourcePath = argv[2];
	std::string destPath = argc > 3 ? argv[3] : (bDecode ? DECODED_FILES_DIR : ENCODED_FILES_DIR);
	GetFilesInDirectory(filePaths, std::wstring(sourcePath.begin(), sourcePath.end()), std::wstring(destPath.begin(), destPath.end()));

}

void AppManager::Execute()
{
	for (size_t i = 0; i < filePaths.size(); i++)
	{
		std::string sourceStr = std::string(filePaths[i].first.begin(), filePaths[i].first.end());
		std::string destStr = std::string(filePaths[i].second.begin(), filePaths[i].second.end());

		Compressor compressor(bDecode);
		compressor.Run(sourceStr, destStr);

		totalCompressed += compressor.GetCompressedSize();
		totalUncompressed += compressor.GetUncompressedSize();

		compressor.Finalize();
	}

	PrintFinalStatistics();
}

void AppManager::PrintFinalStatistics()
{
	fprintf(stderr, "\nTotal Uncompressed Files Size = %.0f Bytes\n", totalUncompressed);
	fprintf(stderr, "\nTotal Compressed Files Size = %.0f Bytes\n", totalCompressed);
	fprintf(stderr, "\nTotal Compression Ratio = %.2f\n", totalUncompressed / totalCompressed);
	fprintf(stderr, "\nTotal Reduction Ratio = %.2f%%\n", (totalCompressed / totalUncompressed) * 100);
}