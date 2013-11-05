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

std::map<std::string, std::size_t> ConvertIndex(const std::string& archivePath)
{
	assert(&archivePath);

	std::ifstream in_stream(archivePath.c_str(), std::ios::in | std::ios::binary);
	if(!in_stream)
	{
		throw std::runtime_error("アーカイブの読込みに失敗");
	}

	in_stream.seekg(0, std::ios::beg);

	std::vector<char> readData(4);
	in_stream.read(&readData.front(), readData.size());

	if(!memcmp(&readData.front(), "ARCH", readData.size()))
	{
		throw std::runtime_error("アーカイブの読込みに失敗");
	}

	std::size_t indexSize;
	in_stream.read(reinterpret_cast<char*>(&indexSize), sizeof(indexSize));

	std::string indexBuffer(indexSize, '\0');
	in_stream.read(&indexBuffer.front(), indexSize);

	const std::string delim(1, 0x1b);
	std::vector<std::string> indexPieces = SplitString(indexBuffer, delim);

	std::map<std::string, std::size_t> destIndex;
	for(const auto& piece : indexPieces)
	{
		std::size_t fileBegin;
		memcpy_s(reinterpret_cast<char*>(&fileBegin), sizeof(std::size_t), piece.substr(0, sizeof(std::size_t)).c_str(), sizeof(std::size_t));

		destIndex.insert(std::map<std::string, std::size_t>::value_type(piece.substr(2 * sizeof(std::size_t)), fileBegin));
	}

	return destIndex;
}

bool IsArchive(const std::string& srcPath)
{
	assert(&srcPath);

	std::ifstream in_stream(srcPath.c_str(), std::ios::in | std::ios::binary);
	if(!in_stream)
	{
		return false;
	}

	in_stream.seekg(0, std::ios::beg);

	std::vector<char> data(4);
	in_stream.read(&data.front(), data.size());

	if(!memcmp(&data.front(), "ARCH", data.size()))
	{
		return false;
	}

	return true;
}

std::map<std::string, std::size_t> SearchFiles(const std::string& srcPath)
{
	assert(&srcPath);

	std::map<std::string, std::size_t> destIndex;


	return destIndex;
}

std::vector<std::string> SplitString(const std::string& src, const std::string& delim)
{
	assert(&src);
	assert(&delim);

	std::vector<std::string> destIndex;

	unsigned int begin = 0;
	unsigned int next = src.find(delim.c_str(), begin);

	while(next != std::string::npos)
	{
		destIndex.push_back(src.substr(begin, next));
		begin += next;
	}

	destIndex.push_back(src.substr(begin));

	return destIndex;
}

/******************************************************************************
	SimpleArchiver
******************************************************************************/

SimpleArchiver::SimpleArchiver(const std::string& srcPath) :
	isArchive_(IsArchive(srcPath)),
	basePath_(srcPath),
	index_(isArchive_ ? ConvertIndex(srcPath) : SearchFiles(srcPath))
{}

void SimpleArchiver::ReadFile(char* dest, const std::string& filePath) const
{
	assert(dest);
	assert(&filePath);

	std::ifstream in_stream(filePath.c_str(), std::ios::in | std::ios::binary);
	if(!in_stream)
	{
		try
		{
			throw std::runtime_error("Cannot read the file from the archive.");
		}
		catch(const std::runtime_error& e)
		{
			std::wcout << e.what() << std::endl;
		}
	}


}

void SimpleArchiver::WriteArchive(const std::string& destPath) const
{
	assert(&destPath);

	std::ofstream out_stream(destPath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	if(!out_stream)
	{
		try
		{
			throw std::runtime_error("Cannot write to the file.");
		}
		catch(const std::runtime_error& e)
		{
			std::wcout << e.what() << std::endl;
		}
	}

	/**************************************************************************
		アーカイブの構造
		[SIGN][INDEX_SIZE][INDEX][DATA]

			SIGN		: "ARCH" (4 bytes)
			INDEX_SIZE	: [INDEX]部のサイズ (std::size_t, 8 bytes)
			INDEX		: ファイルインデックス(後述)
			DATA		: データ

		インデックスの構造
		[BEGIN][SIZE][PATH][DELIM] [BEGIN][SIZE]... [PATH][DELIM]

			BEGIN	: アーカイブ先頭からのオフセット (std::size_t, 8 bytes)
			SIZE	: ファイルサイズ (std::size_t, 8 bytes)
			PATH	: ファイルパス
			DELIM	: デリミタ 0x1b (char, 1 bytes)
	**************************************************************************/

	out_stream.seekp(0, std::ios::beg);

	//****************************************************************************
	// [SIGN]
	out_stream.write("ARCH", 4);

	//****************************************************************************
	// [INDEX_SIZE]
	const char delim = 0x1b;

	std::size_t indexSize = sizeof(index_) + (sizeof(std::size_t) + sizeof(delim)) * index_.size();
	out_stream.write(reinterpret_cast<const char*>(&indexSize), sizeof(indexSize));

	//****************************************************************************
	// [INDEX]
	for(const auto& piece : index_)
	{
		out_stream.write(reinterpret_cast<const char*>(&piece.second), sizeof(piece.second));

		std::size_t fileSize = GetFileSize(piece.first);
		out_stream.write(reinterpret_cast<const char*>(&fileSize), sizeof(fileSize));

		out_stream.write(piece.first.c_str(), piece.first.size());
		
		out_stream.write(&delim, 1);
	}

	//****************************************************************************
	// [DATA]
	for(const auto& piece : index_)
	{
		std::size_t fileSize = GetFileSize(piece.first);

		// I/Oサイズ 8KB
		std::vector<char> fileData(8192);

		std::ifstream in_stream(piece.first.c_str(), std::ios::in | std::ios::binary);
		if(!in_stream)
		{
			try
			{
				throw std::runtime_error("Cannot write to the file.");
			}
			catch(const std::runtime_error& e)
			{
				std::wcout << e.what() << std::endl;
			}
		}

		in_stream.seekg(0, std::ios::beg);

		std::size_t begin = 0;
		while(begin <= fileSize)
		{
			in_stream.read(&fileData.front(), fileData.size());
			out_stream.write(&fileData.front(), fileData.size());
			begin += fileData.size();
		}
		if(begin > fileSize)
		{
			unsigned int leftSize = static_cast<unsigned int>(begin - fileSize);
			fileData.resize(leftSize);

			in_stream.read(&fileData.front(), fileData.size());
			out_stream.write(&fileData.front(), fileData.size());
		}
	}
}

std::size_t SimpleArchiver::GetFileSize(const std::string& filePath) const
{
	assert(&filePath);

	std::ifstream in_stream(filePath.c_str(), std::ios::in | std::ios::binary);
	if(!in_stream)
	{
		try
		{
			throw std::runtime_error("Cannot read the file.");
		}
		catch(const std::runtime_error& e)
		{
			std::wcout << e.what() << std::endl;
		}
	}

	in_stream.seekg(0, std::ios::end);
	const auto endPt = in_stream.tellg();
	in_stream.seekg(0, std::ios::beg);
	const auto begPt = in_stream.tellg();

	return static_cast<std::size_t>(endPt - begPt);
}

} // namespace archiver
} // namespace hm