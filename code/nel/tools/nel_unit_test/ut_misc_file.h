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

#ifndef UT_MISC_FILE
#define UT_MISC_FILE

#include <nel/misc/file.h>
#include <nel/misc/path.h>

// Test suite for NLMISC::CFile behavior
struct CUTMiscFile : public Test::Suite
{
	CUTMiscFile()
	{
		TEST_ADD(CUTMiscFile::copyOneBigFile);
		TEST_ADD(CUTMiscFile::copyDifferentFileSize);
		TEST_ADD(CUTMiscFile::moveOneBigFile);
		TEST_ADD(CUTMiscFile::moveDifferentFileSize);
		// Add a line here when adding a new test METHOD
	}

private:
	string	_SrcFile;
	string	_DstFile;

	void setup()
	{
		_SrcFile = "__copy_file_src.foo";
		_DstFile = "__copy_file_dst.foo";
	}

	void tear_down()
	{
	}

	void copyFileSize(uint fileSize)
	{
		// create a source file (using standard c code)
		FILE *fp = fopen(_SrcFile.c_str(), "wb");
		nlverify(fp != NULL);

		for (uint i=0; i<fileSize; ++i)
		{
			uint8 c = uint8(i & 0xff);
			nlverify(fwrite(&c, 1, 1, fp) == 1);
		}
		fclose(fp);
		fp = NULL;

		NLMISC::CFile::copyFile(_DstFile, _SrcFile, false);

		// verify the resulting file
		fp = fopen(_DstFile.c_str(), "rb");
		TEST_ASSERT(fp != NULL);
		if (fp)
		{
			for (uint i=0; i<fileSize; ++i)
			{
				uint8 c;
				size_t nbRead = fread(&c, 1,1, fp);
				TEST_ASSERT(nbRead == 1);
				if (nbRead != 1)
					break;
				TEST_ASSERT_MSG(c == uint8(i & 0xff), "File content changed during copy");
				if (c != uint8(i & 0xff))
					break;
			}
			fclose(fp);
		}
	}


	void copyOneBigFile()
	{
		// check for a big file
		copyFileSize(1024*1024);
	}

	void copyDifferentFileSize()
	{
		// check for a series of size
		for (uint i=0; i<10; ++i)
		{
			copyFileSize(i);
		}

		srand(1234);
		for (uint i=0; i<1024; ++i)
		{
			i += rand()%10;
			copyFileSize(i);
		}
	}

	void moveFileSize(size_t fileSize)
	{
		// remove the destination if any
		FILE *fp = fopen(_DstFile.c_str(), "rb");
		if (fp != NULL)
		{
			fclose(fp);
			NLMISC::CFile::deleteFile(_DstFile);
		}

		// create a source file (using standard c code)
		fp = fopen(_SrcFile.c_str(), "wb");
		nlverify(fp != NULL);

		for (uint i=0; i<fileSize; ++i)
		{
			uint8 c = uint8(i & 0xff);
			nlverify(fwrite(&c, 1, 1, fp) == 1);
		}
		fclose(fp);
		fp = NULL;

		NLMISC::CFile::moveFile(_DstFile.c_str(), _SrcFile.c_str());

		// verify the resulting file
		fp = fopen(_SrcFile.c_str(), "rb");
		TEST_ASSERT_MSG(fp == NULL, "The source file is not removed");
		if (fp)
			fclose(fp);

		fp = fopen(_DstFile.c_str(), "rb");
		TEST_ASSERT(fp != NULL);
		if (fp)
		{
			for (uint i=0; i<fileSize; ++i)
			{
				uint8 c;
				size_t nbRead = fread(&c, 1,1, fp);
				TEST_ASSERT(nbRead == 1);
				if (nbRead != 1)
					break;
				TEST_ASSERT_MSG(c == uint8(i & 0xff), "File content changed during move");
				if (c != uint8(i & 0xff))
					break;
			}
			fclose(fp);
		}
	}

	void moveOneBigFile()
	{
		// check for a big file
		moveFileSize(1024*1024);
	}

	void moveDifferentFileSize()
	{
		// check for a series of size
		for (uint i=0; i<10; ++i)
		{
			moveFileSize(i);
		}

		srand(1234);
		for (uint i=0; i<1024; ++i)
		{
			i += rand()%10;
			moveFileSize(i);
		}
	}

};

#endif
