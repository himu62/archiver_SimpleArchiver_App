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
	�C���f�b�N�X����1�̃A�C�e��
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
	std::size_t GetFileSize(const boost::filesystem::path& filePath) const;

private:
	const bool isArchive_;
	const boost::filesystem::path basePath_;
	const std::map<boost::filesystem::path, IndexItem> index_;
};

} // namespace archiver
} // namespace hm