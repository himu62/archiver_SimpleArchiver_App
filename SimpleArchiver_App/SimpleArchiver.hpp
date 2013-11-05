/******************************************************************************
	Project	: archiver_SimpleArchiver (App)
	File	: SimpleArchiver.hpp
	Author	: himu62
******************************************************************************/

#include <map>
#include <string>
#include <vector>

namespace hm
{
namespace archiver
{

// アーカイブを読み込み，ファイルインデックスを生成する(mapの鍵はパス，値はアーカイブ先頭からの位置)
std::map<std::string, std::size_t> ConvertIndex(const std::string& archivePath);

// パスがアーカイブか判定する
bool IsArchive(const std::string& srcPath);

// パス以下のファイルを探索してインデックスを生成する(サブディレクトリも探索)(mapの値は空っぽ)
std::map<std::string, std::size_t> SearchFiles(const std::string& srcPath);

// 文字列をデリミタで区切って返す
std::vector<std::string> SplitString(const std::string& src, const std::string& delim);

/******************************************************************************
	SimpleArchiver
	インデックスとデータを並べるだけの単純なアーカイバインタフェース
******************************************************************************/
class SimpleArchiver
{
public:
	// コンストラクタ：指定パスのディレクトリまたはアーカイブを操作対象に設定する
	explicit SimpleArchiver(const std::string& srcPath);

	// アーカイブからメモリにファイルを読み出す．パスがディレクトリの場合は無効．
	void ReadFile(char* dest, const std::string& filePath) const;

	// ディレクトリ以下のファイルをすべて含むアーカイブを指定パスに書き込む
	void WriteArchive(const std::string& destPath) const;

	// アーカイブまたはディスク上のファイルのファイルサイズを取得する
	std::size_t GetFileSize(const std::string& filePath) const;

private:
	const std::string basePath_;
	const std::map<std::string, std::size_t> index_;
	const bool isArchive_;
};

} // namespace archiver
} // namespace hm