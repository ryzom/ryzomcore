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

// The seek tests use a file larger than the buffer of a FILE, so that after a read the position of the
// stream is not the position of the file pointer of the system. 251 is prime: the pattern does not line
// up with a buffer size, and the values above 250 are free for markers.
static const sint32 s_SeekFileSize = 10000;

static uint8 seekPatternByte(sint64 pos)
{
	return uint8(pos % 251);
}

TEST_F(CUTMiscFileTest, nlfseek64Origins)
{
	FILE *fp = NLMISC::nlfopen(_SrcFile, "wb");
	ASSERT_TRUE(fp != NULL);
	for (sint32 i = 0; i < s_SeekFileSize; ++i)
	{
		uint8 c = seekPatternByte(i);
		EXPECT_EQ(fwrite(&c, 1, 1, fp), (size_t)1);
	}
	fclose(fp);

	fp = NLMISC::nlfopen(_SrcFile, "rb");
	ASSERT_TRUE(fp != NULL);
	uint8 buf[16];

	// from the current position after a read, forward
	EXPECT_EQ(fread(buf, 1, 10, fp), (size_t)10);
	EXPECT_EQ(NLMISC::nlftell64(fp), 10);
	EXPECT_EQ(NLMISC::nlfseek64(fp, 5, SEEK_CUR), 0);
	EXPECT_EQ(NLMISC::nlftell64(fp), 15);
	EXPECT_EQ(fread(buf, 1, 1, fp), (size_t)1);
	EXPECT_EQ(buf[0], seekPatternByte(15));

	// from the current position, backward
	EXPECT_EQ(NLMISC::nlfseek64(fp, -12, SEEK_CUR), 0);
	EXPECT_EQ(NLMISC::nlftell64(fp), 4);
	EXPECT_EQ(fread(buf, 1, 1, fp), (size_t)1);
	EXPECT_EQ(buf[0], seekPatternByte(4));

	// from the current position, nowhere, and then past what the stream has buffered
	EXPECT_EQ(NLMISC::nlfseek64(fp, 0, SEEK_CUR), 0);
	EXPECT_EQ(NLMISC::nlftell64(fp), 5);
	EXPECT_EQ(NLMISC::nlfseek64(fp, 6000, SEEK_CUR), 0);
	EXPECT_EQ(NLMISC::nlftell64(fp), 6005);
	EXPECT_EQ(fread(buf, 1, 1, fp), (size_t)1);
	EXPECT_EQ(buf[0], seekPatternByte(6005));

	// from the start
	EXPECT_EQ(NLMISC::nlfseek64(fp, 100, SEEK_SET), 0);
	EXPECT_EQ(NLMISC::nlftell64(fp), 100);
	EXPECT_EQ(fread(buf, 1, 1, fp), (size_t)1);
	EXPECT_EQ(buf[0], seekPatternByte(100));

	// from the end
	EXPECT_EQ(NLMISC::nlfseek64(fp, -4000, SEEK_END), 0);
	EXPECT_EQ(NLMISC::nlftell64(fp), s_SeekFileSize - 4000);
	EXPECT_EQ(fread(buf, 1, 1, fp), (size_t)1);
	EXPECT_EQ(buf[0], seekPatternByte(s_SeekFileSize - 4000));
	EXPECT_EQ(NLMISC::nlfseek64(fp, -1, SEEK_END), 0);
	EXPECT_EQ(NLMISC::nlftell64(fp), s_SeekFileSize - 1);
	EXPECT_EQ(fread(buf, 1, 1, fp), (size_t)1);
	EXPECT_EQ(buf[0], seekPatternByte(s_SeekFileSize - 1));
	EXPECT_EQ(NLMISC::nlfseek64(fp, 0, SEEK_END), 0);
	EXPECT_EQ(NLMISC::nlftell64(fp), s_SeekFileSize);
	EXPECT_EQ(fread(buf, 1, 1, fp), (size_t)0);

	fclose(fp);
	NLMISC::CFile::deleteFile(_SrcFile);
}

TEST_F(CUTMiscFileTest, fileSeekOrigins)
{
	std::vector<uint8> expected(s_SeekFileSize);
	for (sint32 i = 0; i < s_SeekFileSize; ++i)
		expected[i] = seekPatternByte(i);

	// write the pattern, then markers at positions reached from the current position, the start and the end
	NLMISC::COFile out;
	ASSERT_TRUE(out.open(_SrcFile));
	out.serialBuffer(&expected[0], (uint)expected.size());
	EXPECT_EQ(out.getPos(), s_SeekFileSize);

	uint8 marker = 0xff;
	EXPECT_TRUE(out.seek(-10, NLMISC::IStream::current));
	EXPECT_EQ(out.getPos(), s_SeekFileSize - 10);
	out.serialBuffer(&marker, 1);
	expected[s_SeekFileSize - 10] = marker;

	marker = 0xfe;
	EXPECT_TRUE(out.seek(4, NLMISC::IStream::current));
	EXPECT_EQ(out.getPos(), s_SeekFileSize - 5);
	out.serialBuffer(&marker, 1);
	expected[s_SeekFileSize - 5] = marker;

	marker = 0xfd;
	EXPECT_TRUE(out.seek(100, NLMISC::IStream::begin));
	EXPECT_EQ(out.getPos(), 100);
	out.serialBuffer(&marker, 1);
	expected[100] = marker;

	marker = 0xfc;
	EXPECT_TRUE(out.seek(-1, NLMISC::IStream::end));
	EXPECT_EQ(out.getPos(), s_SeekFileSize - 1);
	out.serialBuffer(&marker, 1);
	expected[s_SeekFileSize - 1] = marker;

	EXPECT_TRUE(out.seek(0, NLMISC::IStream::end));
	EXPECT_EQ(out.getPos(), s_SeekFileSize);
	out.close();

	// read the markers back the same way, then the whole file
	NLMISC::CIFile in;
	ASSERT_TRUE(in.open(_SrcFile));
	EXPECT_EQ(in.getFileSize(), (uint32)s_SeekFileSize);
	uint8 buf[16];

	in.serialBuffer(buf, 10);
	EXPECT_EQ(in.getPos(), 10);
	EXPECT_TRUE(in.seek(s_SeekFileSize - 20, NLMISC::IStream::current));
	EXPECT_EQ(in.getPos(), s_SeekFileSize - 10);
	in.serialBuffer(buf, 1);
	EXPECT_EQ(buf[0], 0xff);

	EXPECT_TRUE(in.seek(4, NLMISC::IStream::current));
	EXPECT_EQ(in.getPos(), s_SeekFileSize - 5);
	in.serialBuffer(buf, 1);
	EXPECT_EQ(buf[0], 0xfe);

	EXPECT_TRUE(in.seek(100 - (s_SeekFileSize - 4), NLMISC::IStream::current));
	EXPECT_EQ(in.getPos(), 100);
	in.serialBuffer(buf, 1);
	EXPECT_EQ(buf[0], 0xfd);

	EXPECT_TRUE(in.seek(-1, NLMISC::IStream::end));
	EXPECT_EQ(in.getPos(), s_SeekFileSize - 1);
	in.serialBuffer(buf, 1);
	EXPECT_EQ(buf[0], 0xfc);

	EXPECT_TRUE(in.seek(0, NLMISC::IStream::begin));
	EXPECT_EQ(in.getPos(), 0);
	std::vector<uint8> content(s_SeekFileSize);
	in.serialBuffer(&content[0], (uint)content.size());
	EXPECT_TRUE(content == expected);
	in.close();

	NLMISC::CFile::deleteFile(_SrcFile);
}

// The end of a file that is being written includes what the stream has not flushed yet: each case
// writes and then seeks from the end at once, without a seek or a flush in between.
TEST_F(CUTMiscFileTest, seekEndUnflushed)
{
	std::vector<uint8> pattern(s_SeekFileSize);
	for (sint32 i = 0; i < s_SeekFileSize; ++i)
		pattern[i] = seekPatternByte(i);
	uint8 marker = 0xff;

	// FILE, to the end: less than a buffer written, nothing has reached the file yet
	FILE *fp = NLMISC::nlfopen(_SrcFile, "wb");
	ASSERT_TRUE(fp != NULL);
	EXPECT_EQ(fwrite(&pattern[0], 1, 100, fp), (size_t)100);
	EXPECT_EQ(NLMISC::nlfseek64(fp, 0, SEEK_END), 0);
	EXPECT_EQ(NLMISC::nlftell64(fp), 100);
	EXPECT_EQ(fwrite(&marker, 1, 1, fp), (size_t)1);
	fclose(fp);
	EXPECT_EQ(NLMISC::CFile::getFileSize(_SrcFile), 101u);

	// FILE, back from the end: more than a buffer written, a part of it has reached the file
	fp = NLMISC::nlfopen(_SrcFile, "wb");
	ASSERT_TRUE(fp != NULL);
	EXPECT_EQ(fwrite(&pattern[0], 1, pattern.size(), fp), pattern.size());
	EXPECT_EQ(NLMISC::nlfseek64(fp, -10, SEEK_END), 0);
	EXPECT_EQ(NLMISC::nlftell64(fp), s_SeekFileSize - 10);
	EXPECT_EQ(fwrite(&marker, 1, 1, fp), (size_t)1);
	fclose(fp);
	EXPECT_EQ(NLMISC::CFile::getFileSize(_SrcFile), (uint32)s_SeekFileSize);

	// the marker is where it was written, and nothing else changed
	std::vector<uint8> expected = pattern;
	expected[s_SeekFileSize - 10] = marker;
	std::vector<uint8> content(s_SeekFileSize);
	fp = NLMISC::nlfopen(_SrcFile, "rb");
	ASSERT_TRUE(fp != NULL);
	EXPECT_EQ(fread(&content[0], 1, content.size(), fp), content.size());
	fclose(fp);
	EXPECT_TRUE(content == expected);

	// COFile, to the end
	NLMISC::COFile out;
	ASSERT_TRUE(out.open(_SrcFile));
	out.serialBuffer(&pattern[0], 100);
	EXPECT_TRUE(out.seek(0, NLMISC::IStream::end));
	EXPECT_EQ(out.getPos(), 100);
	out.serialBuffer(&marker, 1);
	out.close();
	EXPECT_EQ(NLMISC::CFile::getFileSize(_SrcFile), 101u);

	// COFile, back from the end
	ASSERT_TRUE(out.open(_SrcFile));
	out.serialBuffer(&pattern[0], (uint)pattern.size());
	EXPECT_TRUE(out.seek(-10, NLMISC::IStream::end));
	EXPECT_EQ(out.getPos(), s_SeekFileSize - 10);
	out.serialBuffer(&marker, 1);
	out.close();
	EXPECT_EQ(NLMISC::CFile::getFileSize(_SrcFile), (uint32)s_SeekFileSize);

	NLMISC::CIFile in;
	ASSERT_TRUE(in.open(_SrcFile));
	ASSERT_EQ(in.getFileSize(), (uint32)s_SeekFileSize);
	content.assign(s_SeekFileSize, 0);
	in.serialBuffer(&content[0], (uint)content.size());
	in.close();
	EXPECT_TRUE(content == expected);

	NLMISC::CFile::deleteFile(_SrcFile);
}
