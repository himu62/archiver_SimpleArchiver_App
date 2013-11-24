/******************************************************************************
	Project	: archiver_SimpleArchiver (App)
	File	: SimpleArchiver.cpp
	Author	: himu62
******************************************************************************/

#include "SimpleArchiver.hpp"

#include <cassert>
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

//*****************************************************************************
// 文字列を分割する
std::vector<std::wstring> SplitString(const std::wstring& src, const std::wstring& delim, const std::wstring& splitter = L"")
{
	using namespace std;

	vector<wstring> dest;

	wstring::size_type begin = 0;
	auto next = src.find_first_of(delim, begin);

	if(next == wstring::npos)
	{
		dest.push_back(src);
	}
	else
	{
		while(next != wstring::npos)
		{
			dest.push_back(src.substr(begin, next - begin));
			begin = next + 1;
			next = src.find_first_of(delim, begin);

			if(!splitter.empty())
			{
				dest.push_back(splitter);
			}
		}

		dest.push_back(src.substr(begin));
	}

	return dest;
}

//*****************************************************************************
// パスを符号化する
// first: 符号化参照, second: 符号化対象
void EncodePath(std::vector<std::wstring>& first, std::vector<std::wstring>& second)
{
	for(
		auto& it_first = first.begin(), it_second = second.begin();
		it_first < first.end() && it_second < second.end();
		++it_first, ++it_second
		)
	{
		if(*it_first == *it_second)
		{
			*it_second = 0x1b;
		}
	}
}

//*****************************************************************************
// パスを符号化する
std::vector<IndexItem> EncodeIndex(const std::vector<IndexItem>& src)
{
	using namespace std;

	vector<wstring> srcPaths;
	for(const auto& record : src)
	{
		srcPaths.push_back(record.path_.wstring());
	}

	const wstring delim(1, 0x1b);
	vector<vector<wstring>> pathPieces;
	for(const auto& path : srcPaths)
	{
		pathPieces.push_back(SplitString(path, L"\\", delim));
	}

	for(auto& it = pathPieces.rbegin(); it < pathPieces.rend() - 1; ++it)
	{
		EncodePath(*(it + 1), *it);
	}

	vector<wstring> paths;
	for(const auto& path : pathPieces)
	{
		std::wstring buffer;
		buffer.clear();
		for(const auto& piece : path)
		{
			buffer += piece;
		}
		paths.push_back(buffer);
	}

	vector<IndexItem> dest;
	for(const auto& record : src)
	{
		IndexItem item(paths.front(), record.begin_, record.fileSize_);
		paths.erase(paths.begin());
		dest.push_back(item);
	}

	return dest;
}

//*****************************************************************************
// パスを復号化する
std::vector<IndexItem> DecodeIndex(const std::vector<IndexItem>& src)
{
	std::vector<IndexItem> dest;
	return dest;
}

//*****************************************************************************
// アーカイブを読み込み，ファイルインデックスを生成する(mapの鍵はパス，値はアーカイブ先頭からの位置)
std::vector<IndexItem> ConvertIndex(const fs::path& archivePath)
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

	std::size_t indexSize;
	in_stream.read(reinterpret_cast<char*>(&indexSize), sizeof(indexSize));

	std::vector<IndexItem> destIndex;
	for(unsigned int readSize = 0; readSize < indexSize;)
	{
		std::size_t fileBegin;
		in_stream.read(reinterpret_cast<char*>(&fileBegin), sizeof(fileBegin));
		readSize += sizeof(fileBegin);

		std::size_t fileSize;
		in_stream.read(reinterpret_cast<char*>(&fileSize), sizeof(fileSize));
		readSize += sizeof(fileSize);

		unsigned int pathLength;
		in_stream.read(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));
		readSize += sizeof(pathLength);

		std::vector<char> pathBuffer(pathLength);
		in_stream.read(&pathBuffer.front(), pathLength);
		readSize += pathLength;

		IndexItem item(pathBuffer, fileBegin, fileSize);

		destIndex.push_back(item);
	}

	return DecodeIndex(destIndex);
}

//*****************************************************************************
// パスがアーカイブか判定する
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

//*****************************************************************************
// パス以下のファイルを探索してインデックスを生成する(サブディレクトリも探索)
void SearchFilesSubdir(const fs::path& srcPath, std::vector<IndexItem>& destIndex)
{
	assert(&srcPath);
	assert(&destIndex);

	const auto full_src_path = fs::absolute(srcPath);

	fs::directory_iterator end;
	for(fs::directory_iterator it(full_src_path); it != end; ++it)
	{
		if(fs::is_directory(fs::absolute(it->path())))
		{
			SearchFilesSubdir(srcPath / it->path().leaf(), destIndex);
		}
	}
	for(fs::directory_iterator it(full_src_path); it != end; ++it)
	{
		const auto full_path = fs::absolute(it->path());

		if(!fs::is_directory(full_path))
		{
			const auto buffer = srcPath / full_path.leaf();
			IndexItem item(srcPath / full_path.leaf(), 0, static_cast<const size_t>(fs::file_size(full_path)));
			destIndex.push_back(item);
		}
	}
}

//*****************************************************************************
// パス以下のファイルを探索してインデックスを生成する(サブディレクトリも探索)
std::vector<IndexItem> SearchFiles(const fs::path& srcPath)
{
	assert(&srcPath);

	std::vector<IndexItem> destIndex;

	if(!fs::is_directory(srcPath))
	{
		IndexItem item(srcPath.leaf() , 0, static_cast<std::size_t>(fs::file_size(srcPath)));
		destIndex.push_back(item);
	}
	else
	{
		const auto branch = srcPath.branch_path();

		fs::current_path(srcPath);

		fs::directory_iterator end;
		for(fs::directory_iterator it(srcPath); it != end; ++it)
		{
			const auto full_path = fs::absolute(it->path());

			if(fs::is_directory(full_path))
			{
				SearchFilesSubdir(it->path().leaf(), destIndex);
			}
		}

		fs::current_path(srcPath.branch_path());

		for(fs::directory_iterator it(srcPath); it != end; ++it)
		{
			const auto full_path = fs::absolute(it->path());

			if(!fs::is_directory(full_path))
			{
				IndexItem item(full_path.leaf(), 0, static_cast<const size_t>(fs::file_size(full_path)));
				destIndex.push_back(item);
			}
		}
	}

	return EncodeIndex(destIndex);
}

/******************************************************************************
	IndexItem
******************************************************************************/

IndexItem::IndexItem(const fs::path& path, const std::size_t begin, const std::size_t filesize) :
	path_(path),
	begin_(begin),
	fileSize_(filesize),
	pathLength_(path.string().size())
{}

bool IndexItem::operator!()
{
	if(path_.empty())
	{
		return true;
	}
	else
	{
		return false;
	}
}

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
	if(!isArchive_)
	{
		std::wcout << L"アーカイブではない" << std::endl;
		return;
	}

	assert(dest);
	assert(&filePath);

	fs::ifstream in_stream(basePath_, std::ios_base::in | std::ios_base::binary);
	if(in_stream.fail())
	{
		throw std::runtime_error("アーカイブの読み込みに失敗");
	}

	if(!GetItem(filePath))
	{
		std::wcout << L"ファイルが見つからない" << std::endl;
		return;
	}

	IndexItem item = GetItem(filePath);

	in_stream.seekg(item.begin_, fs::ifstream::beg);
	in_stream.read(dest, item.fileSize_);
}

void SimpleArchiver::WriteArchive(const fs::path& destPath) const
{
	if(isArchive_)
	{
		std::wcout << L"アーカイブからアーカイブは生成できない" << std::endl;
		return;
	}

	assert(&destPath);

	fs::ofstream out_stream(destPath, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
	if(out_stream.fail())
	{
		throw std::runtime_error("ファイルの書込みに失敗");
	}

	/**************************************************************************
		アーカイブの構造
		[SIGN][INDEX_SIZE][INDEX][DATA]

			SIGN		: "ARCH"
			INDEX_SIZE	: [INDEX]部のサイズ
			INDEX		: ファイルインデックス(後述)
			DATA		: データ

		インデックスの構造
		[BEGIN][SIZE][PATH_LENGTH][PATH][BEGIN][SIZE]...

			BEGIN		: アーカイブ先頭からのオフセット
			SIZE		: ファイルサイズ
			PATH_LENGTH	: ファイルパスの長さ
			PATH		: ファイルパス
	**************************************************************************/

	out_stream.seekp(0, fs::ofstream::beg);

	//****************************************************************************
	// [SIGN]
	std::string sign("ARCH");
	out_stream.write(&sign.front(), sign.size());

	//****************************************************************************
	// [INDEX_SIZE]
	std::size_t indexSize = 0;
	for(const auto& record : index_)
	{
		indexSize += sizeof(record);
	}
	out_stream.write(reinterpret_cast<const char*>(&indexSize), sizeof(indexSize));

	//****************************************************************************
	// [INDEX]
	std::size_t begin = sign.size() + sizeof(indexSize) + indexSize;

	for(const auto& record : index_)
	{
		IndexItem buffer(record.path_, begin, record.fileSize_);

		out_stream.write(reinterpret_cast<const char*>(&buffer), sizeof(buffer));
		out_stream.write(reinterpret_cast<const char*>(record.path_.string().c_str()), record.pathLength_);

		begin += record.fileSize_;
	}

	//****************************************************************************
	// [DATA]
	for(const auto& record : index_)
	{
		std::size_t fileSize = record.fileSize_;

		fs::ifstream in_stream(basePath_.wstring() + L'\\' + record.path_.wstring(), std::ios_base::in | std::ios_base::binary);
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
				buffer.resize(static_cast<unsigned int>(fileSize - wroteSize));
			}

			in_stream.read(&buffer.front(), buffer.size());
			out_stream.write(&buffer.front(), buffer.size());
		}
	}
}

std::size_t SimpleArchiver::GetFileSize(const fs::path& filePath) const
{
	assert(&filePath);

	if(!fs::exists(filePath))
	{
		return GetItem(filePath).fileSize_;
	}

	return static_cast<std::size_t>(fs::file_size(filePath));
}

IndexItem SimpleArchiver::GetItem(const boost::filesystem::path& filePath) const
{
	for(const auto& item : index_)
	{
		if(item.path_ == filePath)
		{
			return item;
		}
	}

	return IndexItem(L"");
}

} // namespace archiver
} // namespace hm