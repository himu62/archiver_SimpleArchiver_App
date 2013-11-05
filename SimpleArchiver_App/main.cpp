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
int main(int argc, char** argv)
{
	using namespace hm::archiver;

	std::vector<char> buffer(1024);

	try
	{
		SimpleArchiver archiver("test");
		archiver.WriteArchive("test.arc");

		SimpleArchiver reader("test.arc");

		reader.ReadFile(&buffer.front(), "text.txt");
	}
	catch(const std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
		exit(0);
	}

	for(const auto& dest : buffer)
	{
		std::cout << dest;
	}
	std::cout << std::endl;

	std::cin >> buffer.front();

	return 0;
}