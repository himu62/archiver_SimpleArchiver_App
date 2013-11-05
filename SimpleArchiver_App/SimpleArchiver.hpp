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

// �A�[�J�C�u��ǂݍ��݁C�t�@�C���C���f�b�N�X�𐶐�����(map�̌��̓p�X�C�l�̓A�[�J�C�u�擪����̈ʒu)
std::map<std::string, std::size_t> ConvertIndex(const std::string& archivePath);

// �p�X���A�[�J�C�u�����肷��
bool IsArchive(const std::string& srcPath);

// �p�X�ȉ��̃t�@�C����T�����ăC���f�b�N�X�𐶐�����(�T�u�f�B���N�g�����T��)(map�̒l�͋����)
std::map<std::string, std::size_t> SearchFiles(const std::string& srcPath);

// ��������f���~�^�ŋ�؂��ĕԂ�
std::vector<std::string> SplitString(const std::string& src, const std::string& delim);

/******************************************************************************
	SimpleArchiver
	�C���f�b�N�X�ƃf�[�^����ׂ邾���̒P���ȃA�[�J�C�o�C���^�t�F�[�X
******************************************************************************/
class SimpleArchiver
{
public:
	// �R���X�g���N�^�F�w��p�X�̃f�B���N�g���܂��̓A�[�J�C�u�𑀍�Ώۂɐݒ肷��
	explicit SimpleArchiver(const std::string& srcPath);

	// �A�[�J�C�u���烁�����Ƀt�@�C����ǂݏo���D�p�X���f�B���N�g���̏ꍇ�͖����D
	void ReadFile(char* dest, const std::string& filePath) const;

	// �f�B���N�g���ȉ��̃t�@�C�������ׂĊ܂ރA�[�J�C�u���w��p�X�ɏ�������
	void WriteArchive(const std::string& destPath) const;

	// �A�[�J�C�u�܂��̓f�B�X�N��̃t�@�C���̃t�@�C���T�C�Y���擾����
	std::size_t GetFileSize(const std::string& filePath) const;

private:
	const std::string basePath_;
	const std::map<std::string, std::size_t> index_;
	const bool isArchive_;
};

} // namespace archiver
} // namespace hm