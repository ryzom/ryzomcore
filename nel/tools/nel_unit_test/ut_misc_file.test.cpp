// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/common.h>

#ifndef NL_OS_WINDOWS
#include <sys/stat.h>
#include <unistd.h>
#endif

using std::string;

// Test suite for NLMISC::CFile behavior
class CUTMiscFileTest : public testing::Test
{
protected:
	string _SrcFile;
	string _DstFile;

	void SetUp() override
	{
		ASSERT_TRUE(NLMISC::INelContext::getInstance().isContextInitialised());
		_SrcFile = "__copy_file_src.foo";
		_DstFile = "__copy_file_dst.foo";
	}

	void TearDown() override
	{
	}

	void copyFileSize(uint fileSize)
	{
		// create a source file (using standard c code)
		FILE *fp = NLMISC::nlfopen(_SrcFile, "wb");
		nlverify(fp != NULL);

		for (uint i = 0; i < fileSize; ++i)
		{
			uint8 c = uint8(i & 0xff);
			nlverify(fwrite(&c, 1, 1, fp) == 1);
		}
		fclose(fp);
		fp = nullptr;

		NLMISC::CFile::copyFile(_DstFile, _SrcFile, false);

		// verify the resulting file
		fp = NLMISC::nlfopen(_DstFile, "rb");
		ASSERT_TRUE(fp != NULL);
		if (fp)
		{
			for (uint i = 0; i < fileSize; ++i)
			{
				uint8 c;
				size_t nbRead = fread(&c, 1, 1, fp);
				ASSERT_TRUE(nbRead == 1);
				if (nbRead != 1)
					break;
				ASSERT_TRUE(c == uint8(i & 0xff)) << "File content changed during copy";
				if (c != uint8(i & 0xff))
					break;
			}
			fclose(fp);
		}
	}

	void moveFileSize(size_t fileSize)
	{
		// remove the destination if any
		FILE *fp = NLMISC::nlfopen(_DstFile, "rb");
		if (fp != nullptr)
		{
			fclose(fp);
			NLMISC::CFile::deleteFile(_DstFile);
		}

		// create a source file (using standard c code)
		fp = NLMISC::nlfopen(_SrcFile, "wb");
		nlverify(fp != NULL);

		for (uint i = 0; i < fileSize; ++i)
		{
			uint8 c = uint8(i & 0xff);
			nlverify(fwrite(&c, 1, 1, fp) == 1);
		}
		fclose(fp);
		fp = nullptr;

		NLMISC::CFile::moveFile(_DstFile, _SrcFile);

		// verify the resulting file
		fp = NLMISC::nlfopen(_SrcFile, "rb");
		ASSERT_EQ(fp, nullptr) << "The source file is not removed";
		if (fp)
			fclose(fp);

		fp = NLMISC::nlfopen(_DstFile, "rb");
		ASSERT_NE(fp, nullptr);
		if (fp)
		{
			for (uint i = 0; i < fileSize; ++i)
			{
				uint8 c;
				size_t nbRead = fread(&c, 1, 1, fp);
				ASSERT_TRUE(nbRead == 1);
				if (nbRead != 1)
					break;
				ASSERT_TRUE(c == uint8(i & 0xff)) << "File content changed during move";
				if (c != uint8(i & 0xff))
					break;
			}
			fclose(fp);
		}
	}
};

TEST_F(CUTMiscFileTest, copyOneBigFile)
{
	// check for a big file
	copyFileSize(1024 * 1024);
}

TEST_F(CUTMiscFileTest, copyDifferentFileSize)
{
	// check for a series of size
	for (uint i = 0; i < 10; ++i)
	{
		copyFileSize(i);
	}

	srand(1234);
	for (uint i = 0; i < 1024; ++i)
	{
		i += rand() % 10;
		copyFileSize(i);
	}
}

TEST_F(CUTMiscFileTest, moveOneBigFile)
{
	// check for a big file
	moveFileSize(1024 * 1024);
}

TEST_F(CUTMiscFileTest, moveDifferentFileSize)
{
	// check for a series of size
	for (uint i = 0; i < 10; ++i)
	{
		moveFileSize(i);
	}

	srand(1234);
	for (uint i = 0; i < 1024; ++i)
	{
		i += rand() % 10;
		moveFileSize(i);
	}
}

TEST_F(CUTMiscFileTest, isDirectoryWritable)
{
	const string dir = "__writable_test_dir";
	const string kept = dir + "/kept.txt";
	std::vector<string> content;

	// start without the directory, a run that was stopped may have left it
	if (NLMISC::CFile::isDirectory(dir))
	{
		NLMISC::CPath::getPathContent(dir, false, false, true, content, nullptr, true);
		for (size_t i = 0; i < content.size(); ++i)
			NLMISC::CFile::deleteFile(content[i]);
		content.clear();
		NLMISC::CFile::deleteDirectory(dir);
	}
	ASSERT_FALSE(NLMISC::CFile::isExists(dir));

	// what is not a directory is not writable
	EXPECT_FALSE(NLMISC::CFile::isDirectoryWritable(""));
	EXPECT_FALSE(NLMISC::CFile::isDirectoryWritable(dir));
	ASSERT_TRUE(NLMISC::CFile::createEmptyFile(_SrcFile));
	EXPECT_FALSE(NLMISC::CFile::isDirectoryWritable(_SrcFile));
	NLMISC::CFile::deleteFile(_SrcFile);

	// a fresh directory is writable, with and without the final slash, and the test leaves nothing in it
	ASSERT_TRUE(NLMISC::CFile::createDirectory(dir));
	EXPECT_TRUE(NLMISC::CFile::isDirectoryWritable(dir));
	EXPECT_TRUE(NLMISC::CFile::isDirectoryWritable(dir + "/"));
	NLMISC::CPath::getPathContent(dir, false, true, true, content, nullptr, true);
	EXPECT_TRUE(content.empty());

	// the current directory, where the other tests write, as a relative and as an absolute path
	EXPECT_TRUE(NLMISC::CFile::isDirectoryWritable("."));
	EXPECT_TRUE(NLMISC::CFile::isDirectoryWritable(NLMISC::CPath::getCurrentPath()));

	// a file in the directory is left alone, and stays the only one
	FILE *fp = NLMISC::nlfopen(kept, "wb");
	ASSERT_NE(fp, nullptr);
	ASSERT_EQ(fwrite("keep", 1, 4, fp), (size_t)4);
	fclose(fp);
	EXPECT_TRUE(NLMISC::CFile::isDirectoryWritable(dir));
	EXPECT_EQ(NLMISC::CFile::getFileSize(kept), 4u);
	content.clear();
	NLMISC::CPath::getPathContent(dir, false, true, true, content, nullptr, true);
	ASSERT_EQ(content.size(), (size_t)1);
	EXPECT_EQ(NLMISC::CFile::getFilename(content[0]), "kept.txt");
	NLMISC::CFile::deleteFile(kept);

#ifndef NL_OS_WINDOWS
	// a directory without the write permission. The permission is not enforced for root,
	// and some filesystems ignore the change: the test only runs where the system refuses the write.
	if (chmod(dir.c_str(), S_IRUSR | S_IXUSR) == 0 && access(dir.c_str(), W_OK) != 0)
	{
		EXPECT_FALSE(NLMISC::CFile::isDirectoryWritable(dir));
	}
	chmod(dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
	EXPECT_TRUE(NLMISC::CFile::isDirectoryWritable(dir));
#endif

	EXPECT_TRUE(NLMISC::CFile::deleteDirectory(dir));
}
