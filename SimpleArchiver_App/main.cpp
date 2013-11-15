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
	�G���g���|�C���g
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
		archiver.WriteArchive(fs::path(dest));

		SimpleArchiver archive(dest);
		std::vector<char> data(archive.GetFileSize("C:\\Applications\\7-zip\\readme.txt"));
		archive.ReadFile(&data.front(), "C:\\Applications\\7-zip\\readme.txt");

		boost::filesystem::ofstream out_stream("C:\\Applications\\test.txt", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
 		out_stream.write(&data.front(), data.size());
	}
	catch(const std::runtime_error& e)
	{
		std::wcout << e.what() << std::endl;
		rewind(stdin);
		getchar();
	}

	return 0;
}