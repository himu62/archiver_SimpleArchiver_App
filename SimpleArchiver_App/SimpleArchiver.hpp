/******************************************************************************
	Project	: archiver_SimpleArchiver (App)
	File	: SimpleArchiver.hpp
	Author	: himu62
******************************************************************************/
#pragma once

#include <map>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

namespace hm
{
namespace archiver
{

/******************************************************************************
	IndexItem
	インデックス内の1つのアイテム
******************************************************************************/
struct IndexItem
{
	explicit IndexItem(const boost::filesystem::path& path, const std::size_t begin = 0, const std::size_t filesize = 0);

	const std::size_t begin_;
	const std::size_t fileSize_;
	const unsigned int pathLength_;
};

/******************************************************************************
	SimpleArchiver
	インデックスとデータを並べるだけの単純なアーカイバインタフェース
******************************************************************************/
class SimpleArchiver
{
public:
	// コンストラクタ：指定パスのディレクトリまたはアーカイブを操作対象に設定する
	explicit SimpleArchiver(const boost::filesystem::path& srcPath);

	// アーカイブからメモリにファイルを読み出す．パスがディレクトリの場合は無効．
	void ReadFile(char* dest, const boost::filesystem::path& filePath) const;

	// ディレクトリ以下のファイルをすべて含むアーカイブを指定パスに書き込む
	void WriteArchive(const boost::filesystem::path& destPath) const;

	// アーカイブまたはディスク上のファイルのファイルサイズを取得する
	std::size_t GetFileSize(const boost::filesystem::path& filePath) const;

private:
	const bool isArchive_;
	const boost::filesystem::path basePath_;
	const std::map<boost::filesystem::path, IndexItem> index_;
};

} // namespace archiver
} // namespace hm