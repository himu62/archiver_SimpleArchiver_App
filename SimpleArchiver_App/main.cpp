/******************************************************************************
	Project	: archiver_SimpleArchiver (App)
	File	: main.cpp
	Author	: himu62
******************************************************************************/

#include "SimpleArchiver.hpp"

#include <iostream>
#include <stdexcept>

#include <boost/filesystem/fstream.hpp>

/******************************************************************************
	エントリポイント
******************************************************************************/
int wmain(int argc, wchar_t* argv[])
{
	if(argc != 3)
	{
		std::wcout
			<< std::endl
			<< L"usage: SimpleArchiver src dest"
			<< std::endl;
		exit(0);
	}

	using namespace hm::archiver;
	namespace fs = boost::filesystem;

	std::wstring src(argv[1]);
	std::wstring dest(argv[2]);

	try
	{
		SimpleArchiver archiver(src);
		archiver.WriteArchive(dest);

		SimpleArchiver archive(dest);
		std::vector<char> data(archive.GetFileSize(L"readme.txt"));
		archive.ReadFile(&data.front(), L"readme.txt");

		fs::ofstream out_stream(dest.substr(0, dest.find(L'\\', 4)) + L"\\test.txt", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
 		out_stream.write(&data.front(), data.size());

		data.resize(archive.GetFileSize(L"7-zip.chm"));
		archive.ReadFile(&data.front(), L"7-zip.chm");

		fs::ofstream out_stream2(dest.substr(0, dest.find(L'\\', 4)) + L"\\test.chm", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
		out_stream2.write(&data.front(), data.size());
	}
	catch(const std::runtime_error& e)
	{
		std::wcout << e.what() << std::endl;
		rewind(stdin);
		getchar();
	}

	return 0;
}