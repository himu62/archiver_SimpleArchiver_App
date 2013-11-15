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
	�֐�
******************************************************************************/

//*****************************************************************************
// �A�[�J�C�u��ǂݍ��݁C�t�@�C���C���f�b�N�X�𐶐�����(map�̌��̓p�X�C�l�̓A�[�J�C�u�擪����̈ʒu)
std::map<fs::path, IndexItem> ConvertIndex(const fs::path& archivePath)
{
	assert(&archivePath);

	fs::ifstream in_stream(archivePath, std::ios_base::in | std::ios_base::binary);
	if(in_stream.fail())
	{
		throw std::runtime_error("�A�[�J�C�u�̓ǂݍ��݂Ɏ��s");
	}

	in_stream.seekg(0, fs::ifstream::beg);

	std::vector<char> readData(4);
	in_stream.read(&readData.front(), readData.size());

	if(memcmp(&readData.front(), "ARCH", readData.size()))
	{
		throw std::runtime_error("�A�[�J�C�u�̓Ǎ��݂Ɏ��s");
	}

	std::size_t indexSize;
	in_stream.read(reinterpret_cast<char*>(&indexSize), sizeof(indexSize));

	std::map<fs::path, IndexItem> destIndex;
	for(unsigned int readSize = 0; readSize < indexSize; )
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

		const fs::path filePath(pathBuffer);

		IndexItem item(filePath, fileBegin, fileSize);

		destIndex.insert(std::map<fs::path, IndexItem>::value_type(filePath, item));
	}	

	return destIndex;
}

//*****************************************************************************
// �p�X���A�[�J�C�u�����肷��
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
// �p�X�ȉ��̃t�@�C����T�����ăC���f�b�N�X�𐶐�����(�T�u�f�B���N�g�����T��)
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

/******************************************************************************
	IndexItem
******************************************************************************/

IndexItem::IndexItem(const fs::path& path, const std::size_t begin, const std::size_t filesize) :
	begin_(begin),
	fileSize_(filesize ? filesize : static_cast<std::size_t>(fs::file_size(path))),
	pathLength_(path.string().size())
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
		throw std::runtime_error("�t�@�C���̓ǂݍ��݂Ɏ��s");
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
		throw std::runtime_error("�t�@�C���̏����݂Ɏ��s");
	}

	/**************************************************************************
		�A�[�J�C�u�̍\��
		[SIGN][INDEX_SIZE][INDEX][DATA]

			SIGN		: "ARCH"
			INDEX_SIZE	: [INDEX]���̃T�C�Y
			INDEX		: �t�@�C���C���f�b�N�X(��q)
			DATA		: �f�[�^

		�C���f�b�N�X�̍\��
		[BEGIN][SIZE][PATH_LENGTH][PATH][BEGIN][SIZE]...

			BEGIN		: �A�[�J�C�u�擪����̃I�t�Z�b�g
			SIZE		: �t�@�C���T�C�Y
			PATH_LENGTH	: �t�@�C���p�X�̒���
			PATH		: �t�@�C���p�X
	**************************************************************************/

	out_stream.seekp(0, fs::ofstream::beg);

	//****************************************************************************
	// [SIGN]
	std::string sign("ARCH");
	out_stream.write(&sign.front(), sign.size());

	//****************************************************************************
	// [INDEX_SIZE]
	// issue: �C���f�b�N�X�T�C�Y��4�o�C�g����Ȃ�
	std::size_t indexSize = 0;
	for(const auto& record : index_)
	{
		indexSize += sizeof(record.second);
		indexSize += record.second.pathLength_;
	}
	out_stream.write(reinterpret_cast<const char*>(&indexSize), sizeof(indexSize));

	//****************************************************************************
	// [INDEX]
	std::size_t begin = sign.size() + indexSize;

	for(const auto& record : index_)
	{
		IndexItem buffer(record.first, begin);

		out_stream.write(reinterpret_cast<const char*>(&buffer), sizeof(buffer));
		out_stream.write(reinterpret_cast<const char*>(record.first.string().c_str()), record.first.string().size());

		begin += record.second.fileSize_;
	}

	//****************************************************************************
	// [DATA]
	for(const auto& record : index_)
	{
		std::size_t fileSize = record.second.fileSize_;

		fs::ifstream in_stream(record.first, std::ios_base::in | std::ios_base::binary);
		if(in_stream.fail())
		{
			throw std::runtime_error("�t�@�C���̏����݂Ɏ��s");
		}
		
		in_stream.seekg(0, fs::ifstream::beg);
		const auto inBeg = in_stream.tellg();

		// I/O�T�C�Y: 1024KB
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
		return index_.at(filePath.string()).fileSize_;
	}

	return static_cast<std::size_t>(fs::file_size(filePath));
}

} // namespace archiver
} // namespace hm