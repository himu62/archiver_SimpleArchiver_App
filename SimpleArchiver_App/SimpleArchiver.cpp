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

std::map<std::wstring, uintmax_t> ConvertIndex(const boost::filesystem::path& archivePath)
{
	assert(&archivePath);

	namespace fs = boost::filesystem;

	fs::ifstream in_stream(archivePath);
	if(in_stream.fail())
	{
		throw std::runtime_error("アーカイブの読み込みに失敗");
	}

	in_stream.seekg(0, fs::ifstream::beg);

	std::vector<char> readData(4);
	in_stream.read(&readData.front(), readData.size());

	if(!memcmp(&readData.front(), "ARCH", readData.size()))
	{
		throw std::runtime_error("アーカイブの読込みに失敗");
	}

	uintmax_t indexSize;
	in_stream.read(reinterpret_cast<char*>(&indexSize), sizeof(indexSize));

	std::wstring indexBuffer(indexSize, '\0');
	in_stream.read(reinterpret_cast<char*>(&indexBuffer.front()), indexSize);

	const std::wstring delim(1, 0x1b);
	std::vector<std::wstring> indexPieces = SplitString(indexBuffer, delim);

	std::map<std::wstring, uintmax_t> destIndex;
	for(const auto& piece : indexPieces)
	{
		uintmax_t fileBegin;
		memcpy_s(reinterpret_cast<char*>(&fileBegin), sizeof(uintmax_t), piece.substr(0, sizeof(uintmax_t)).c_str(), sizeof(uintmax_t));

		destIndex.insert(std::map<std::wstring, uintmax_t>::value_type(piece.substr(2 * sizeof(uintmax_t)), fileBegin));
	}

	return destIndex;
}

bool IsArchive(const boost::filesystem::path& srcPath)
{
	assert(&srcPath);

	namespace fs = boost::filesystem;

	fs::ifstream in_stream(srcPath);
	if(in_stream.fail())
	{
		return false;
	}

	in_stream.seekg(0, fs::ifstream::beg);

	std::vector<char> data(4);
	in_stream.read(&data.front(), data.size());

	if(!memcmp(&data.front(), "ARCH", data.size()))
	{
		return false;
	}

	return true;
}

std::map<std::wstring, uintmax_t> SearchFiles(const boost::filesystem::path& srcPath)
{
	assert(&srcPath);

	namespace fs = boost::filesystem;

	std::map<std::wstring, uintmax_t> destIndex;

	if(!fs::is_directory(srcPath))
	{
		destIndex[srcPath.wstring()] = fs::file_size(srcPath);
	}
	else
	{
		fs::recursive_directory_iterator end;
		for(fs::recursive_directory_iterator i(srcPath); i != end; ++i)
		{
			const auto p = i->path();
			if(!fs::is_directory(p))
			{
				destIndex[p.wstring()] = fs::file_size(p);
			}
		}
	}

	return destIndex;
}

std::vector<std::wstring> SplitString(const std::wstring& src, const std::wstring& delim)
{
	assert(&src);
	assert(&delim);

	std::vector<std::wstring> destIndex;

	unsigned int begin = 0;
	unsigned int next = src.find(delim.c_str(), begin);

	while(next != std::wstring::npos)
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

SimpleArchiver::SimpleArchiver(const boost::filesystem::path& srcPath) :
	isArchive_(IsArchive(srcPath)),
	basePath_(srcPath),
	index_(isArchive_ ? ConvertIndex(srcPath) : SearchFiles(srcPath))
{}

void SimpleArchiver::ReadFile(char* dest, const std::wstring& filePath) const
{
	assert(dest);
	assert(&filePath);

	namespace fs = boost::filesystem;

	const auto p = fs::path(filePath);

	fs::ifstream in_stream(p);
	if(in_stream.fail())
	{
		throw std::runtime_error("ファイルの読込みに失敗");
	}

	


}

void SimpleArchiver::WriteArchive(const boost::filesystem::path& destPath) const
{
	assert(&destPath);

	namespace fs = boost::filesystem;

	fs::ofstream out_stream(destPath);
	if(out_stream.fail())
	{
		throw std::runtime_error("ファイルの書込みに失敗");
	}

	/**************************************************************************
		アーカイブの構造
		[SIGN][INDEX_SIZE][INDEX][DATA]

			SIGN		: "ARCH" (4 bytes)
			INDEX_SIZE	: [INDEX]部のサイズ (uintmax_t, 8 bytes)
			INDEX		: ファイルインデックス(後述)
			DATA		: データ

		インデックスの構造
		[BEGIN][SIZE][PATH][DELIM] [BEGIN][SIZE]... [PATH][DELIM]

			BEGIN	: アーカイブ先頭からのオフセット (uintmax_t, 8 bytes)
			SIZE	: ファイルサイズ (uintmax_t, 8 bytes)
			PATH	: ファイルパス
			DELIM	: デリミタ 0x1b (char, 1 bytes)
	**************************************************************************/

	out_stream.seekp(0, fs::ofstream::beg);

	//****************************************************************************
	// [SIGN]
	out_stream.write("ARCH", 4);

	//****************************************************************************
	// [INDEX_SIZE]
	const char delim = 0x1b;

	uintmax_t indexSize = sizeof(index_) + (sizeof(uintmax_t) + sizeof(delim)) * index_.size();
	out_stream.write(reinterpret_cast<const char*>(&indexSize), sizeof(indexSize));

	//****************************************************************************
	// [INDEX]
	for(const auto& piece : index_)
	{
		out_stream.write(reinterpret_cast<const char*>(&piece.second), sizeof(piece.second));

		uintmax_t fileSize = GetFileSize(piece.first);
		out_stream.write(reinterpret_cast<const char*>(&fileSize), sizeof(fileSize));

		out_stream.write(reinterpret_cast<const char*>(piece.first.c_str()), piece.first.size());
		
		out_stream.write(&delim, 1);
	}

	//****************************************************************************
	// [DATA]
	for(const auto& piece : index_)
	{
		const auto p = fs::path(piece.first);

		uintmax_t fileSize = fs::file_size(p);

		fs::ifstream in_stream(p);
		if(in_stream.fail())
		{
			throw std::runtime_error("ファイルの書込みに失敗");
		}
		
		in_stream.seekg(0, fs::ifstream::beg);
		const auto inBeg = in_stream.tellg();

		// I/Oサイズ: 1MB
		std::vector<char> buffer(1024 * 1024);

		while(true)
		{
			const auto wroteSize = in_stream.tellg() - inBeg;

			if(wroteSize >= fileSize)
			{
				break;
			}
			else if(wroteSize + buffer.size() > fileSize)
			{
				buffer.resize(static_cast<unsigned int>(fileSize - wroteSize));
			}

			in_stream.read(&buffer.front(), buffer.size());
			out_stream.write(&buffer.front(), buffer.size());
		}
	}
}

uintmax_t SimpleArchiver::GetFileSize(const std::wstring& filePath) const
{
	assert(&filePath);

	namespace fs = boost::filesystem;

	const auto p = fs::path(filePath);

	if(!fs::exists(p))
	{
		throw std::runtime_error("ファイルの読込みに失敗");
	}

	return fs::file_size(p);
}

} // namespace archiver
} // namespace hm