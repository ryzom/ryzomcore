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

#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/common.h>

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
		fp = NULL;

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
		if (fp != NULL)
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
		fp = NULL;

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
