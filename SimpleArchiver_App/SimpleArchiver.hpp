/******************************************************************************
	Project	: archiver_SimpleArchiver (App)
	File	: SimpleArchiver.hpp
	Author	: himu62
******************************************************************************/

#include <map>
#include <string>
#include <vector>

/******************************************************************************
	SimpleArchiver
******************************************************************************/

namespace hm
{
namespace archiver
{

// アーカイブを読み込み，ファイルインデックスを生成する(mapの鍵はパス，値はアーカイブ先頭からの位置)
std::map<std::wstring, std::size_t> ConvertIndex(const std::wstring& archivePath);

// パスがアーカイブか判定する
bool IsArchive(const std::wstring& srcPath);

// パス以下のファイルを探索してインデックスを生成する(サブディレクトリも探索)(mapの値は空っぽ)
std::map<std::wstring, std::size_t> SearchFiles(const std::wstring& srcPath);

// 文字列をデリミタで区切って返す
void SplitString(std::vector<std::wstring>& destIndex, const std::wstring& src, const std::wstring& delim);

//*****************************************************************************
// インデックスとデータを並べるだけの単純なアーカイバインタフェース
class SimpleArchiver
{
public:
	// コンストラクタ：指定パスのディレクトリまたはアーカイブを操作対象に設定する
	explicit SimpleArchiver(const std::wstring& srcPath);

	// アーカイブからメモリにファイルを読み出す．パスがディレクトリの場合は無効．
	void ReadFile(wchar_t* dest, const std::wstring& filePath) const;

	// ディレクトリ以下のファイルをすべて含むアーカイブを指定パスに書き込む
	void WriteArchive(const std::wstring& destPath) const;

	// アーカイブまたはディスク上のファイルのファイルサイズを取得する
	void GetFileSize(std::size_t* destSize, const std::wstring& filePath) const;

private:

	
	const std::wstring basePath_;
	const std::map<std::wstring, std::size_t> index_;
	const bool isArchive_;
};

}
}