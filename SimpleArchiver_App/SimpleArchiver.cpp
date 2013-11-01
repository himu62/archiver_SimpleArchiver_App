/******************************************************************************
	Project	: archiver_SimpleArchiver (App)
	File	: SimpleArchiver.cpp
	Author	: himu62
******************************************************************************/

#include "SimpleArchiver.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace hm
{
namespace archiver
{

/******************************************************************************
	関数
******************************************************************************/


std::map<std::wstring, std::size_t> ConvertIndex(const std::wstring& archivePath)
{
	assert(&archivePath);

	std::wifstream in_stream(archivePath.c_str(), std::wios::in | std::wios::binary);
	if(!in_stream)
	{
		throw std::runtime_error("アーカイブの読込みに失敗");
	}

	in_stream.seekg(0, std::wios::beg);

	std::vector<wchar_t> readData(4);
	in_stream.read(&readData.front(), readData.size());

	if(!wmemcmp(&readData.front(), L"ARCH", readData.size()))
	{
		throw std::runtime_error("アーカイブの読込みに失敗");
	}

	std::size_t indexSize;
	in_stream.read(reinterpret_cast<wchar_t*>(&indexSize), sizeof(indexSize));

	std::wstring indexBuffer(indexSize, L'');
	in_stream.read(&indexBuffer.front(), indexSize);

	const std::wstring delim(1, 0x1b);
	std::vector<std::wstring> indexPieces;
	SplitString(indexPieces, indexBuffer, delim);

}

bool IsArchive(const std::wstring& srcPath)
{
	assert(&srcPath);

	std::wifstream in_stream(srcPath.c_str(), std::wios::in | std::wios::binary);
	if(!in_stream)
	{
		return false;
	}

	in_stream.seekg(0, std::wios::beg);

	std::vector<wchar_t> data(4);
	in_stream.read(&data.front(), data.size());

	if(!wmemcmp(&data.front(), L"ARCH", data.size()))
	{
		return false;
	}

	return true;
}

std::map<std::wstring, std::size_t> SearchFiles(const std::wstring& srcPath)
{
	assert(&srcPath);
}

void SplitString(std::vector<std::wstring>& destIndex, const std::wstring& src, const std::wstring& delim)
{
	assert(&destIndex);
	assert(&src);
	assert(&delim);

	destIndex.clear();
	destIndex.resize(0);

	unsigned int begin = 0;
	unsigned int next = src.find(delim.c_str(), begin);

	while(next != std::wstring::npos)
	{
		destIndex.push_back(src.substr(begin, next));
		begin += next;
	}

	destIndex.push_back(src.substr(begin));
}

/******************************************************************************
	SimpleArchiver
******************************************************************************/

SimpleArchiver::SimpleArchiver(const std::wstring& srcPath) :
	isArchive_(IsArchive(srcPath)),
	basePath_(srcPath),
	index_(isArchive_ ? ConvertIndex(srcPath) : SearchFiles(srcPath))
{}

void SimpleArchiver::ReadFile(wchar_t* dest, const std::wstring& filePath) const
{
}

void SimpleArchiver::WriteArchive(const std::wstring& destPath) const
{
}

void SimpleArchiver::GetFileSize(std::size_t* destSize, const std::wstring& filePath) const
{
	std::wifstream in_stream(filePath.c_str(), std::wios::in | std::wios::binary);

}

}
}