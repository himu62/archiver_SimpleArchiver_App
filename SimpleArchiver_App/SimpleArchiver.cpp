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

#include <boost/filesystem/fstream.hpp>

namespace fs = boost::filesystem;

namespace hm
{
namespace archiver
{

/******************************************************************************
	関数
******************************************************************************/

std::map<fs::path, IndexItem> ConvertIndex(const fs::path& archivePath)
{
	assert(&archivePath);

	fs::ifstream in_stream(archivePath, std::ios_base::in | std::ios_base::binary);
	if(in_stream.fail())
	{
		throw std::runtime_error("アーカイブの読み込みに失敗");
	}

	in_stream.seekg(0, fs::ifstream::beg);

	std::vector<char> readData(4);
	in_stream.read(&readData.front(), readData.size());

	if(memcmp(&readData.front(), "ARCH", readData.size()))
	{
		throw std::runtime_error("アーカイブの読込みに失敗");
	}

	uintmax_t indexSize;
	in_stream.read(reinterpret_cast<char*>(&indexSize), sizeof(indexSize));

	std::string indexBuffer(indexSize, '\0');
	in_stream.read(&indexBuffer.front(), indexSize);

	const std::string delim(1, 0x1b);
	std::vector<std::string> indexPieces = SplitString(indexBuffer, delim);

	std::map<fs::path, IndexItem> destIndex;
	for(const auto& piece : indexPieces)
	{
		uintmax_t fileBegin;
		memcpy_s(reinterpret_cast<char*>(&fileBegin), sizeof(uintmax_t), piece.substr(0, sizeof(uintmax_t)).c_str(), sizeof(uintmax_t));

		const fs::path filePath = fs::path(piece.substr(2 * sizeof(uintmax_t) - 1));

		IndexItem item(filePath, fileBegin);
		destIndex.insert(std::map<fs::path, IndexItem>::value_type(filePath, item));
	}

	return destIndex;
}

bool IsArchive(const fs::path& srcPath)
{
	assert(&srcPath);

	fs::ifstream in_stream(srcPath, std::ios_base::in | std::ios_base::binary);
	if(in_stream.fail())
	{
		return false;
	}

	in_stream.seekg(0, fs::ifstream::beg);

	std::vector<char> data(4);
	in_stream.read(&data.front(), data.size());

	if(memcmp(&data.front(), "ARCH", data.size()))
	{
		return false;
	}

	return true;
}

std::map<fs::path, IndexItem> SearchFiles(const fs::path& srcPath)
{
	assert(&srcPath);

	std::map<fs::path, IndexItem> destIndex;

	if(!fs::is_directory(srcPath))
	{
		IndexItem item(srcPath, 0);
		destIndex.insert(std::map<fs::path, IndexItem>::value_type(srcPath, item));
	}
	else
	{
		fs::recursive_directory_iterator end;
		for(fs::recursive_directory_iterator i(srcPath); i != end; ++i)
		{
			const auto p = i->path();
			if(!fs::is_directory(p))
			{
				IndexItem item(p, 0);
				destIndex.insert(std::map<fs::path, IndexItem>::value_type(p, item));
			}
		}
	}

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
		next = src.find(delim.c_str(), begin);
	}

	destIndex.push_back(src.substr(begin));

	return destIndex;
}

/******************************************************************************
	IndexItem
******************************************************************************/

IndexItem::IndexItem(const fs::path& path, const uintmax_t begin) :
	begin_(begin),
	fileSize_(fs::exists(path) ? fs::file_size(path) : 0)
{}

/******************************************************************************
	SimpleArchiver
******************************************************************************/

SimpleArchiver::SimpleArchiver(const fs::path& srcPath) :
	isArchive_(IsArchive(srcPath)),
	basePath_(srcPath),
	index_(isArchive_ ? ConvertIndex(srcPath) : SearchFiles(srcPath))
{}

void SimpleArchiver::ReadFile(char* dest, const fs::path& filePath) const
{
	assert(dest);
	assert(&filePath);

	fs::ifstream in_stream(basePath_, std::ios_base::in | std::ios_base::binary);
	if(in_stream.fail())
	{
		throw std::runtime_error("ファイルの読み込みに失敗");
	}

	IndexItem item = index_.at(filePath.string());

	in_stream.seekg(item.begin_, fs::ifstream::beg);
	in_stream.read(dest, item.fileSize_);
}

void SimpleArchiver::WriteArchive(const fs::path& destPath) const
{
	assert(&destPath);

	fs::ofstream out_stream(destPath, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
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

	uintmax_t indexSize = 0;
	for(const auto& record : index_)
	{
		indexSize += sizeof(record.second);
		indexSize += record.first.string().size();
		indexSize += sizeof(delim);
	}
	out_stream.write(reinterpret_cast<const char*>(&indexSize), sizeof(indexSize));

	//****************************************************************************
	// [INDEX]
	for(const auto& piece : index_)
	{
		out_stream.write(reinterpret_cast<const char*>(&piece.second), sizeof(piece.second));

		out_stream.write(reinterpret_cast<const char*>(piece.first.string().c_str()), piece.first.string().size());
		
		out_stream.write(reinterpret_cast<const char*>(&delim), sizeof(delim));
	}

	//****************************************************************************
	// [DATA]
	for(const auto& piece : index_)
	{
		uintmax_t fileSize = fs::file_size(piece.first);

		fs::ifstream in_stream(piece.first, std::ios_base::in | std::ios_base::binary);
		if(in_stream.fail())
		{
			throw std::runtime_error("ファイルの書込みに失敗");
		}
		
		in_stream.seekg(0, fs::ifstream::beg);
		const auto inBeg = in_stream.tellg();

		// I/Oサイズ: 1024KB
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
				buffer.resize(fileSize - wroteSize);
			}

			in_stream.read(&buffer.front(), buffer.size());
			out_stream.write(&buffer.front(), buffer.size());
		}
	}
}

uintmax_t SimpleArchiver::GetFileSize(const fs::path& filePath) const
{
	assert(&filePath);

	if(!fs::exists(filePath))
	{
		return index_.at(filePath.string()).fileSize_;
	}

	return fs::file_size(filePath);
}

} // namespace archiver
} // namespace hm