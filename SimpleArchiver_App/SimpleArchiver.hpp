/******************************************************************************
	Project	: archiver_SimpleArchiver (App)
	File	: SimpleArchiver.hpp
	Author	: himu62
******************************************************************************/

#include <map>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

namespace hm
{
namespace archiver
{

// アーカイブを読み込み，ファイルインデックスを生成する(mapの鍵はパス，値はアーカイブ先頭からの位置)
std::map<std::wstring, uintmax_t> ConvertIndex(const boost::filesystem::path& archivePath);

// パスがアーカイブか判定する
bool IsArchive(const boost::filesystem::path& srcPath);

// パス以下のファイルを探索してインデックスを生成する(サブディレクトリも探索)(mapの値は空っぽ)
std::map<std::wstring, uintmax_t> SearchFiles(const boost::filesystem::path& srcPath);

// 文字列をデリミタで区切って返す
std::vector<std::wstring> SplitString(const std::wstring& src, const std::wstring& delim);

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
	void ReadFile(char* dest, const std::wstring& filePath) const;

	// ディレクトリ以下のファイルをすべて含むアーカイブを指定パスに書き込む
	void WriteArchive(const std::wstring& destPath) const;

	void WriteArchive(const boost::filesystem::path& destPath) const;

	// アーカイブまたはディスク上のファイルのファイルサイズを取得する
	uintmax_t GetFileSize(const std::wstring& filePath) const;
	uintmax_t GetFileSize(const boost::filesystem::path& filePath) const;

private:
	const boost::filesystem::path basePath_;
	const std::map<std::wstring, uintmax_t> index_;
	const bool isArchive_;
};

} // namespace archiver
} // namespace hm