/******************************************************************************
	Project	: archiver_SimpleArchiver (App)
	File	: SimpleArchiver.hpp
	Author	: himu62
******************************************************************************/

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
	�C���f�b�N�X����1�̃A�C�e��
******************************************************************************/
struct IndexItem
{
	explicit IndexItem(const boost::filesystem::path& path, const uintmax_t begin = 0);

	const uintmax_t begin_;
	const uintmax_t fileSize_;
};

/******************************************************************************
	SimpleArchiver
	�C���f�b�N�X�ƃf�[�^����ׂ邾���̒P���ȃA�[�J�C�o�C���^�t�F�[�X
******************************************************************************/
class SimpleArchiver
{
public:
	// �R���X�g���N�^�F�w��p�X�̃f�B���N�g���܂��̓A�[�J�C�u�𑀍�Ώۂɐݒ肷��
	explicit SimpleArchiver(const boost::filesystem::path& srcPath);

	// �A�[�J�C�u���烁�����Ƀt�@�C����ǂݏo���D�p�X���f�B���N�g���̏ꍇ�͖����D
	void ReadFile(char* dest, const boost::filesystem::path& filePath) const;

	// �f�B���N�g���ȉ��̃t�@�C�������ׂĊ܂ރA�[�J�C�u���w��p�X�ɏ�������
	void WriteArchive(const boost::filesystem::path& destPath) const;

	// �A�[�J�C�u�܂��̓f�B�X�N��̃t�@�C���̃t�@�C���T�C�Y���擾����
	uintmax_t GetFileSize(const boost::filesystem::path& filePath) const;

private:
	const bool isArchive_;
	const boost::filesystem::path basePath_;
	const std::map<boost::filesystem::path, IndexItem> index_;
};

// �A�[�J�C�u��ǂݍ��݁C�t�@�C���C���f�b�N�X�𐶐�����(map�̌��̓p�X�C�l�̓A�[�J�C�u�擪����̈ʒu)
std::map<boost::filesystem::path, IndexItem> ConvertIndex(const boost::filesystem::path& archivePath);

// �p�X���A�[�J�C�u�����肷��
bool IsArchive(const boost::filesystem::path& srcPath);

// �p�X�ȉ��̃t�@�C����T�����ăC���f�b�N�X�𐶐�����(�T�u�f�B���N�g�����T��)(map�̒l�͋����)
std::map<boost::filesystem::path, IndexItem> SearchFiles(const boost::filesystem::path& srcPath);

// ��������f���~�^�ŋ�؂��ĕԂ�
std::vector<std::string> SplitString(const std::string& src, const std::string& delim);

} // namespace archiver
} // namespace hm