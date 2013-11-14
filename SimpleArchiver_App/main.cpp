/******************************************************************************
	Project	: archiver_SimpleArchiver (App)
	File	: main.cpp
	Author	: himu62
******************************************************************************/

#include "SimpleArchiver.hpp"

#include <iostream>
#include <stdexcept>

/******************************************************************************
	エントリポイント
******************************************************************************/
int wmain(int argc, wchar_t* argv[])
{
	if(argc < 2 || argc > 3)
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
	}
	catch(const std::runtime_error& e)
	{
		std::wcout << e.what() << std::endl;
		rewind(stdin);
		getchar();
	}

	return 0;
}