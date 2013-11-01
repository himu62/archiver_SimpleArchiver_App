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

// �A�[�J�C�u��ǂݍ��݁C�t�@�C���C���f�b�N�X�𐶐�����(map�̌��̓p�X�C�l�̓A�[�J�C�u�擪����̈ʒu)
std::map<std::wstring, std::size_t> ConvertIndex(const std::wstring& archivePath);

// �p�X���A�[�J�C�u�����肷��
bool IsArchive(const std::wstring& srcPath);

// �p�X�ȉ��̃t�@�C����T�����ăC���f�b�N�X�𐶐�����(�T�u�f�B���N�g�����T��)(map�̒l�͋����)
std::map<std::wstring, std::size_t> SearchFiles(const std::wstring& srcPath);

// ��������f���~�^�ŋ�؂��ĕԂ�
void SplitString(std::vector<std::wstring>& destIndex, const std::wstring& src, const std::wstring& delim);

//*****************************************************************************
// �C���f�b�N�X�ƃf�[�^����ׂ邾���̒P���ȃA�[�J�C�o�C���^�t�F�[�X
class SimpleArchiver
{
public:
	// �R���X�g���N�^�F�w��p�X�̃f�B���N�g���܂��̓A�[�J�C�u�𑀍�Ώۂɐݒ肷��
	explicit SimpleArchiver(const std::wstring& srcPath);

	// �A�[�J�C�u���烁�����Ƀt�@�C����ǂݏo���D�p�X���f�B���N�g���̏ꍇ�͖����D
	void ReadFile(wchar_t* dest, const std::wstring& filePath) const;

	// �f�B���N�g���ȉ��̃t�@�C�������ׂĊ܂ރA�[�J�C�u���w��p�X�ɏ�������
	void WriteArchive(const std::wstring& destPath) const;

	// �A�[�J�C�u�܂��̓f�B�X�N��̃t�@�C���̃t�@�C���T�C�Y���擾����
	void GetFileSize(std::size_t* destSize, const std::wstring& filePath) const;

private:

	
	const std::wstring basePath_;
	const std::map<std::wstring, std::size_t> index_;
	const bool isArchive_;
};

}
}