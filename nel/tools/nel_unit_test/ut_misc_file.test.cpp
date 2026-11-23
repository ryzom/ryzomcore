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

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/displayer.h>

#ifdef NL_OS_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <thread>
#else
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

// The tests of a COFile that writes through a temporary file: the file takes the name of its target only in
// close(), in one step, and a target that cannot be replaced is never lost. They write to a directory of
// their own in the temporary directory of the system, which is removed after each test, passed or failed.
static const char *s_TempOld = "old content";
static const char *s_TempNew = "the new content, which is longer";

static void tempWriteFile(const string &name, const string &content)
{
	FILE *fp = NLMISC::nlfopen(name, "wb");
	ASSERT_TRUE(fp != NULL);
	if (!content.empty())
	{
		EXPECT_EQ(fwrite(content.data(), 1, content.size(), fp), content.size());
	}
	EXPECT_EQ(fclose(fp), 0);
}

// the content of a file, or a text no test writes when the file cannot be read
static string tempReadFile(const string &name)
{
	FILE *fp = NLMISC::nlfopen(name, "rb");
	if (fp == NULL)
		return "<cannot be opened>";
	string content;
	char buffer[256];
	size_t s;
	while ((s = fread(buffer, 1, sizeof(buffer), fp)) != 0)
		content.append(buffer, s);
	fclose(fp);
	return content;
}

static void tempSerial(NLMISC::COFile &out, const string &content)
{
	out.serialBuffer((uint8 *)content.data(), (uint)content.size());
}

// Counts the warnings that contain a text, while it exists
class CTempFileTestWarnings : public NLMISC::IDisplayer
{
public:
	explicit CTempFileTestWarnings(const string &text)
	    : m_Text(text)
	    , m_Count(0)
	{
		NLMISC::createDebug();
		NLMISC::WarningLog->addDisplayer(this);
	}

	~CTempFileTestWarnings() override
	{
		NLMISC::WarningLog->removeDisplayer(this);
	}

	uint count() const { return m_Count; }

protected:
	void doDisplay(const NLMISC::CLog::TDisplayInfo & /* args */, const char *message) override
	{
		if (strstr(message, m_Text.c_str()) != NULL)
			++m_Count;
	}

private:
	string m_Text;
	uint m_Count;
};

class CUTMiscTempFileTest : public testing::Test
{
protected:
	string m_Dir;
	// the file the tests write, and the name COFile gives its temporary file when there is none yet
	string m_Target;
	string m_Temp;
	// a directory the tests put in the place of a file
	string m_SubDir;

	void SetUp() override
	{
		ASSERT_TRUE(NLMISC::INelContext::getInstance().isContextInitialised());
#ifdef NL_OS_WINDOWS
		const uint processId = (uint)GetCurrentProcessId();
#else
		const uint processId = (uint)getpid();
#endif
		m_Dir = NLMISC::CPath::getTemporaryDirectory() + NLMISC::toString("nel_ut_temp_file_%u/", processId);
		m_Target = m_Dir + "target.foo";
		m_Temp = m_Target + ".tmp0";
		m_SubDir = m_Dir + "target_dir";
		// a run that was stopped may have left the directory
		removeDir();
		ASSERT_FALSE(NLMISC::CFile::isExists(m_Dir)) << m_Dir;
		ASSERT_TRUE(NLMISC::CFile::createDirectory(m_Dir)) << m_Dir;
	}

	void TearDown() override
	{
		removeDir();
		EXPECT_FALSE(NLMISC::CFile::isExists(m_Dir)) << m_Dir;
	}

	// the directory with all the tests put in it, read-only or not
	void removeDir()
	{
		if (m_Dir.empty() || !NLMISC::CFile::isDirectory(m_Dir))
			return;
		if (NLMISC::CFile::isDirectory(m_SubDir))
			NLMISC::CFile::deleteDirectory(m_SubDir);
		std::vector<string> content;
		NLMISC::CPath::getPathContent(m_Dir, false, false, true, content, nullptr, true);
		for (size_t i = 0; i < content.size(); ++i)
			NLMISC::CFile::deleteFile(content[i]);
		NLMISC::CFile::deleteDirectory(m_Dir);
	}

	// the names of the files in the directory, sorted and separated by spaces
	string dirContent()
	{
		std::vector<string> content;
		NLMISC::CPath::getPathContent(m_Dir, false, false, true, content, nullptr, true);
		for (size_t i = 0; i < content.size(); ++i)
			content[i] = NLMISC::CFile::getFilename(content[i]);
		std::sort(content.begin(), content.end());
		string names;
		for (size_t i = 0; i < content.size(); ++i)
			names += (i == 0 ? "" : " ") + content[i];
		return names;
	}
};

TEST_F(CUTMiscTempFileTest, replacesTarget)
{
	tempWriteFile(m_Target, s_TempOld);

	// a target that exists: while the new file is written the target is the old file
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, false, false, true));
		EXPECT_TRUE(NLMISC::CFile::isExists(m_Temp));
		tempSerial(out, s_TempNew);
		out.flush();
		EXPECT_EQ(tempReadFile(m_Target), s_TempOld);
		EXPECT_EQ(tempReadFile(m_Temp), s_TempNew);
		bool closed = false;
		EXPECT_NO_THROW(closed = out.close());
		EXPECT_TRUE(closed);
		EXPECT_FALSE(out.isOpen());
		EXPECT_EQ(tempReadFile(m_Target), s_TempNew);
		EXPECT_EQ(dirContent(), "target.foo");
	}
	// the destructor of a closed file changes nothing
	EXPECT_EQ(tempReadFile(m_Target), s_TempNew);
	EXPECT_EQ(dirContent(), "target.foo");

	// appending starts from a copy of the target
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, true, false, true));
		tempSerial(out, " and more");
		EXPECT_NO_THROW(out.close());
	}
	EXPECT_EQ(tempReadFile(m_Target), string(s_TempNew) + " and more");
	EXPECT_EQ(dirContent(), "target.foo");

	// a target that does not exist yet
	ASSERT_TRUE(NLMISC::CFile::deleteFile(m_Target));
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, false, false, true));
		EXPECT_FALSE(NLMISC::CFile::isExists(m_Target));
		tempSerial(out, s_TempNew);
		EXPECT_NO_THROW(out.close());
	}
	EXPECT_EQ(tempReadFile(m_Target), s_TempNew);
	EXPECT_EQ(dirContent(), "target.foo");
}

TEST_F(CUTMiscTempFileTest, givenUpKeepsTarget)
{
	tempWriteFile(m_Target, s_TempOld);

	// destroyed without close(): the writing is given up
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, false, false, true));
		tempSerial(out, s_TempNew);
		EXPECT_TRUE(NLMISC::CFile::isExists(m_Temp));
	}
	EXPECT_EQ(tempReadFile(m_Target), s_TempOld);
	EXPECT_EQ(dirContent(), "target.foo");

	// the same for a target that does not exist: nothing is left
	ASSERT_TRUE(NLMISC::CFile::deleteFile(m_Target));
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, false, false, true));
		tempSerial(out, s_TempNew);
	}
	EXPECT_EQ(dirContent(), "");
}

TEST_F(CUTMiscTempFileTest, replacesReadOnlyTarget)
{
	tempWriteFile(m_Target, s_TempOld);

#ifdef NL_OS_WINDOWS
	const std::wstring wideTarget = NLMISC::utf8ToWide(m_Target);
	ASSERT_TRUE(SetFileAttributesW(wideTarget.c_str(), FILE_ATTRIBUTE_READONLY) != 0);
	// the file is refused for writing now
	FILE *fp = NLMISC::nlfopen(m_Target, "r+b");
	EXPECT_TRUE(fp == NULL);
	if (fp)
		fclose(fp);
#else
	ASSERT_EQ(chmod(m_Target.c_str(), S_IRUSR | S_IRGRP | S_IROTH), 0);
#endif

	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, false, false, true));
		tempSerial(out, s_TempNew);
		EXPECT_NO_THROW(out.close());
	}
	EXPECT_EQ(tempReadFile(m_Target), s_TempNew);
	EXPECT_EQ(dirContent(), "target.foo");

#ifdef NL_OS_WINDOWS
	// the new file comes with its own attributes, as it did when the target was deleted first
	const DWORD attributes = GetFileAttributesW(wideTarget.c_str());
	ASSERT_NE(attributes, INVALID_FILE_ATTRIBUTES);
	EXPECT_EQ(attributes & FILE_ATTRIBUTE_READONLY, 0u);
#endif
}

// A target that cannot be replaced on any system: a directory. The directory stays, and so does the
// temporary file, the only copy of what was written.
TEST_F(CUTMiscTempFileTest, overDirectoryKeepsBoth)
{
	const string temp = m_SubDir + ".tmp0";
	ASSERT_TRUE(NLMISC::CFile::createDirectory(m_SubDir));

	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_SubDir, false, false, true));
		tempSerial(out, s_TempNew);
		bool thrown = false;
		try
		{
			out.close();
		}
		catch (const NLMISC::ERenameError &e)
		{
			thrown = true;
			EXPECT_EQ(e.Filename, m_SubDir);
			EXPECT_EQ(e.TempFilename, temp);
			const string what = e.what();
			EXPECT_NE(what.find("'" + m_SubDir + "'"), string::npos) << what;
			EXPECT_NE(what.find("'" + temp + "'"), string::npos) << what;
		}
		EXPECT_TRUE(thrown);
		EXPECT_FALSE(out.isOpen());
		// nothing is left to close
		EXPECT_NO_THROW(out.close());
	}
	// nor for the destructor to delete
	EXPECT_TRUE(NLMISC::CFile::isDirectory(m_SubDir));
	EXPECT_EQ(tempReadFile(temp), s_TempNew);
}

// A copy that cannot open its destination does not keep its source open: on Windows a file that is open
// cannot be deleted, nor replaced
TEST_F(CUTMiscTempFileTest, failedCopyReleasesSource)
{
	tempWriteFile(m_Target, s_TempOld);
	ASSERT_TRUE(NLMISC::CFile::createDirectory(m_SubDir));

	EXPECT_FALSE(NLMISC::CFile::copyFile(m_SubDir, m_Target));
	EXPECT_TRUE(NLMISC::CFile::deleteFile(m_Target));
	EXPECT_EQ(dirContent(), "");
}

TEST_F(CUTMiscTempFileTest, isTemporaryOutputFilename)
{
	string temp;
	NLMISC::CFile::getTemporaryOutputFilename(m_Target, temp);
	EXPECT_EQ(temp, m_Temp);
	EXPECT_TRUE(NLMISC::CFile::isTemporaryOutputFilename(temp));

	EXPECT_TRUE(NLMISC::CFile::isTemporaryOutputFilename("a.tmp0"));
	EXPECT_TRUE(NLMISC::CFile::isTemporaryOutputFilename("account_1_2_pdr.bin.tmp12"));
	EXPECT_TRUE(NLMISC::CFile::isTemporaryOutputFilename("characters/001/account_1_2_pdr.bin.tmp0"));
	EXPECT_TRUE(NLMISC::CFile::isTemporaryOutputFilename("characters\\001\\a.tmp3"));

	EXPECT_FALSE(NLMISC::CFile::isTemporaryOutputFilename(""));
	EXPECT_FALSE(NLMISC::CFile::isTemporaryOutputFilename("account_1_2_pdr.bin"));
	EXPECT_FALSE(NLMISC::CFile::isTemporaryOutputFilename("a.tmp"));
	EXPECT_FALSE(NLMISC::CFile::isTemporaryOutputFilename("a.tmpx"));
	EXPECT_FALSE(NLMISC::CFile::isTemporaryOutputFilename("a.tmp0.backup"));
	EXPECT_FALSE(NLMISC::CFile::isTemporaryOutputFilename("atmp0"));
	EXPECT_FALSE(NLMISC::CFile::isTemporaryOutputFilename("tmp0"));
	// a name that is nothing but the suffix is not the temporary file of a file
	EXPECT_FALSE(NLMISC::CFile::isTemporaryOutputFilename(".tmp0"));
	EXPECT_FALSE(NLMISC::CFile::isTemporaryOutputFilename("characters/.tmp0"));
	EXPECT_FALSE(NLMISC::CFile::isTemporaryOutputFilename("characters\\.tmp0"));
}

// A write of no bytes writes nothing and is no failed write, with or without a temporary file: a file that
// only gets such writes is written without content
TEST_F(CUTMiscTempFileTest, zeroLengthWrite)
{
	const string logged = "The file '" + m_Target + "' is not written completely";
	uint8 unused = 0;

	// without a temporary file: the file is created, and is complete
	{
		CTempFileTestWarnings warnings(logged);
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target));
		EXPECT_NO_THROW(out.serialBuffer(&unused, 0));
		EXPECT_NO_THROW(out.serialBuffer(NULL, 0));
		bool closed = false;
		EXPECT_NO_THROW(closed = out.close());
		EXPECT_TRUE(closed);
		EXPECT_EQ(warnings.count(), 0u);
	}
	EXPECT_TRUE(NLMISC::CFile::isExists(m_Target));
	EXPECT_EQ(NLMISC::CFile::getFileSize(m_Target), 0u);

	// between other writes it changes nothing
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target));
		EXPECT_NO_THROW(tempSerial(out, s_TempOld));
		EXPECT_NO_THROW(out.serialBuffer(&unused, 0));
		EXPECT_NO_THROW(tempSerial(out, " and more"));
		EXPECT_TRUE(out.close());
	}
	EXPECT_EQ(tempReadFile(m_Target), string(s_TempOld) + " and more");

	// with a temporary file: the file without content takes the place of the target
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, false, false, true));
		EXPECT_NO_THROW(out.serialBuffer(&unused, 0));
		EXPECT_EQ(tempReadFile(m_Target), string(s_TempOld) + " and more");
		bool closed = false;
		EXPECT_NO_THROW(closed = out.close());
		EXPECT_TRUE(closed);
	}
	EXPECT_TRUE(NLMISC::CFile::isExists(m_Target));
	EXPECT_EQ(NLMISC::CFile::getFileSize(m_Target), 0u);
	EXPECT_EQ(dirContent(), "target.foo");

	// appending nothing through a temporary file leaves the content as it was
	tempWriteFile(m_Target, s_TempOld);
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, true, false, true));
		EXPECT_NO_THROW(out.serialBuffer(&unused, 0));
		EXPECT_NO_THROW(out.close());
	}
	EXPECT_EQ(tempReadFile(m_Target), s_TempOld);
	EXPECT_EQ(dirContent(), "target.foo");

	// a file that is not open is still an error
	{
		NLMISC::COFile out;
		EXPECT_THROW(out.serialBuffer(&unused, 0), NLMISC::EFileNotOpened);
	}
}

#ifdef NL_OS_WINDOWS

// A handle on a file, closed when the test leaves its scope, passed or failed
class CTempFileTestHandle
{
public:
	CTempFileTestHandle()
	    : m_Handle(INVALID_HANDLE_VALUE)
	{
	}

	~CTempFileTestHandle()
	{
		close();
	}

	bool open(const string &name, DWORD access, DWORD share)
	{
		close();
		m_Handle = CreateFileW(NLMISC::utf8ToWide(name).c_str(), access, share, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		return m_Handle != INVALID_HANDLE_VALUE;
	}

	// as a process does that reads a file and lets the others read and write it, but not delete, rename
	// or replace it
	bool hold(const string &name)
	{
		return open(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE);
	}

	// as scanners, indexers and sync agents do: the others may do anything with the file
	bool holdSharingDelete(const string &name)
	{
		return open(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
	}

	// Lock the start of a file that a stream of this process writes or reads through a handle of its own:
	// its writes and reads there fail. This is what a full disk does to a small file, whose data is first
	// written when the stream is flushed or closed.
	bool lockStart(const string &name)
	{
		if (!open(name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE))
			return false;
		if (LockFile(m_Handle, 0, 0, 1024 * 1024, 0))
			return true;
		close();
		return false;
	}

	void close()
	{
		if (m_Handle != INVALID_HANDLE_VALUE)
			CloseHandle(m_Handle);
		m_Handle = INVALID_HANDLE_VALUE;
	}

	// what the file of the handle holds, read through the handle
	string content()
	{
		string content;
		char buffer[256];
		DWORD s = 0;
		SetFilePointer(m_Handle, 0, NULL, FILE_BEGIN);
		while (ReadFile(m_Handle, buffer, sizeof(buffer), &s, NULL) && s != 0)
			content.append(buffer, s);
		return content;
	}

private:
	CTempFileTestHandle(const CTempFileTestHandle &);
	CTempFileTestHandle &operator=(const CTempFileTestHandle &);

	HANDLE m_Handle;
};

// Tell if the system and the file system of a directory know the rename with POSIX semantics, which replaces
// a file that other handles hold open, by trying it. Where they do not, such a file cannot be replaced.
static bool tempPosixRenameKnown(const string &dir)
{
	struct CRenameInfo
	{
		DWORD Flags;
		HANDLE RootDirectory;
		DWORD FileNameLength;
		WCHAR FileName[1];
	};
	const string from = dir + "probe_from";
	const string to = dir + "probe_to";
	FILE *fp = NLMISC::nlfopen(from, "wb");
	if (fp == NULL)
		return false;
	fclose(fp);

	bool known = false;
	WCHAR fullTo[MAX_PATH];
	const DWORD length = GetFullPathNameW(NLMISC::utf8ToWide(to).c_str(), MAX_PATH, fullTo, NULL);
	HANDLE file = CreateFileW(NLMISC::utf8ToWide(from).c_str(), DELETE | SYNCHRONIZE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file != INVALID_HANDLE_VALUE)
	{
		if (length != 0 && length < MAX_PATH)
		{
			std::vector<uint8> buffer(sizeof(CRenameInfo) + length * sizeof(WCHAR), 0);
			CRenameInfo *info = (CRenameInfo *)&buffer[0];
			info->Flags = 0x1 | 0x2; // FILE_RENAME_FLAG_REPLACE_IF_EXISTS, FILE_RENAME_FLAG_POSIX_SEMANTICS
			info->FileNameLength = length * sizeof(WCHAR);
			memcpy(info->FileName, fullTo, length * sizeof(WCHAR));
			// 22: FileRenameInfoEx
			known = SetFileInformationByHandle(file, (FILE_INFO_BY_HANDLE_CLASS)22, info, (DWORD)buffer.size()) != 0;
		}
		CloseHandle(file);
	}
	NLMISC::CFile::deleteFile(from);
	NLMISC::CFile::deleteFile(to);
	return known;
}

// A target that other processes hold open while they let it be deleted, as scanners, indexers and sync agents
// do, is replaced: they keep the old file through their handle, and under the name there is the new one
TEST_F(CUTMiscTempFileTest, heldSharingDeleteIsReplaced)
{
	if (!tempPosixRenameKnown(m_Dir))
	{
		GTEST_SKIP() << "the file system of " << m_Dir << " has no rename with POSIX semantics";
	}
	const std::wstring wideTarget = NLMISC::utf8ToWide(m_Target);

	// once with a plain target, once with a read-only one
	for (uint pass = 0; pass < 2; ++pass)
	{
		tempWriteFile(m_Target, s_TempOld);
		if (pass == 1)
		{
			ASSERT_TRUE(SetFileAttributesW(wideTarget.c_str(), FILE_ATTRIBUTE_READONLY) != 0);
		}

		CTempFileTestHandle held;
		ASSERT_TRUE(held.holdSharingDelete(m_Target));
		{
			NLMISC::COFile out;
			ASSERT_TRUE(out.open(m_Target, false, false, true));
			tempSerial(out, s_TempNew);
			EXPECT_NO_THROW(out.close());
		}
		EXPECT_EQ(tempReadFile(m_Target), s_TempNew);
		EXPECT_EQ(dirContent(), "target.foo");
		EXPECT_EQ(held.content(), s_TempOld);
		held.close();
		EXPECT_EQ(tempReadFile(m_Target), s_TempNew);
		EXPECT_EQ(dirContent(), "target.foo");
		const DWORD attributes = GetFileAttributesW(wideTarget.c_str());
		ASSERT_NE(attributes, INVALID_FILE_ATTRIBUTES);
		EXPECT_EQ(attributes & FILE_ATTRIBUTE_READONLY, 0u);
	}
}

// A target that a process holds open without letting it be deleted cannot be replaced: it keeps its content
// and its attributes, the temporary file is kept, and ERenameError names both
TEST_F(CUTMiscTempFileTest, heldTargetKeepsBoth)
{
	const std::wstring wideTarget = NLMISC::utf8ToWide(m_Target);
	tempWriteFile(m_Target, s_TempOld);

	// once with a plain target, once with a read-only one
	for (uint pass = 0; pass < 2; ++pass)
	{
		if (pass == 1)
		{
			ASSERT_TRUE(SetFileAttributesW(wideTarget.c_str(), FILE_ATTRIBUTE_READONLY) != 0);
		}
		const DWORD attributesBefore = GetFileAttributesW(wideTarget.c_str());

		CTempFileTestHandle held;
		ASSERT_TRUE(held.hold(m_Target));
		{
			NLMISC::COFile out;
			ASSERT_TRUE(out.open(m_Target, false, false, true));
			tempSerial(out, s_TempNew);
			bool thrown = false;
			const NLMISC::TTime before = NLMISC::CTime::getLocalTime();
			try
			{
				out.close();
			}
			catch (const NLMISC::ERenameError &e)
			{
				thrown = true;
				EXPECT_EQ(e.Filename, m_Target);
				EXPECT_EQ(e.TempFilename, m_Temp);
				const string what = e.what();
				EXPECT_NE(what.find("'" + m_Target + "'"), string::npos) << what;
				EXPECT_NE(what.find("'" + m_Temp + "'"), string::npos) << what;
			}
			const NLMISC::TTime elapsed = NLMISC::CTime::getLocalTime() - before;
			EXPECT_TRUE(thrown);
			// the tries end: after 100 ms, a bound far above it allows for a loaded machine
			EXPECT_LT(elapsed, 2000);
			EXPECT_FALSE(out.isOpen());
			EXPECT_NO_THROW(out.close());
		}
		// the target is as it was, and the destructor has left the temporary file
		EXPECT_EQ(tempReadFile(m_Target), s_TempOld);
		EXPECT_EQ(GetFileAttributesW(wideTarget.c_str()), attributesBefore);
		EXPECT_EQ(tempReadFile(m_Temp), s_TempNew);
		EXPECT_EQ(dirContent(), "target.foo target.foo.tmp0");
		held.close();

		ASSERT_TRUE(NLMISC::CFile::deleteFile(m_Temp));
	}
}

// A file that is only held for a moment, the target or the temporary file (as a scanner holds the file just
// written): the move is tried again and succeeds. The holder lets go after 5 ms and the tries last 100 ms:
// on a machine that is busy enough to miss that, the case is run again, three times at most.
TEST_F(CUTMiscTempFileTest, heldForAMoment)
{
	for (uint pass = 0; pass < 2; ++pass)
	{
		bool replaced = false;
		for (uint attempt = 0; attempt < 3 && !replaced; ++attempt)
		{
			tempWriteFile(m_Target, s_TempOld);
			NLMISC::CFile::deleteFile(m_Temp);
			{
				NLMISC::COFile out;
				ASSERT_TRUE(out.open(m_Target, false, false, true));
				tempSerial(out, s_TempNew);
				CTempFileTestHandle held;
				ASSERT_TRUE(held.hold(pass == 0 ? m_Target : m_Temp));
				std::thread release([&held]() {
					Sleep(5);
					held.close();
				});
				try
				{
					out.close();
					replaced = true;
				}
				catch (const NLMISC::ERenameError &)
				{
				}
				release.join();
			}
			if (replaced)
			{
				EXPECT_EQ(tempReadFile(m_Target), s_TempNew);
				EXPECT_EQ(dirContent(), "target.foo");
			}
		}
		EXPECT_TRUE(replaced) << (pass == 0 ? "target held" : "temporary file held");
	}
}

// A temporary file that stays held cannot be moved: it is kept, and the target is as it was
TEST_F(CUTMiscTempFileTest, heldTempKeepsBoth)
{
	tempWriteFile(m_Target, s_TempOld);

	CTempFileTestHandle held;
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, false, false, true));
		tempSerial(out, s_TempNew);
		ASSERT_TRUE(held.hold(m_Temp));
		EXPECT_THROW(out.close(), NLMISC::ERenameError);
		EXPECT_FALSE(out.isOpen());
	}
	EXPECT_EQ(tempReadFile(m_Target), s_TempOld);
	EXPECT_EQ(tempReadFile(m_Temp), s_TempNew);
	held.close();

	// the next writing of the target leaves the kept file alone, and takes the next name
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, false, false, true));
		EXPECT_TRUE(NLMISC::CFile::isExists(m_Target + ".tmp1"));
		tempSerial(out, "third content");
		EXPECT_NO_THROW(out.close());
	}
	EXPECT_EQ(tempReadFile(m_Target), "third content");
	EXPECT_EQ(tempReadFile(m_Temp), s_TempNew);
	EXPECT_EQ(dirContent(), "target.foo target.foo.tmp0");
}

// With a temporary file, a file that is not written completely never takes the place of its target, and
// close() says so: whether the write fails in close(), before it, in a flush or in the flush of a seek
TEST_F(CUTMiscTempFileTest, failedWriteKeepsTarget)
{
	tempWriteFile(m_Target, s_TempOld);

	// the write of what the stream has buffered fails in close()
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, false, false, true));
		CTempFileTestHandle lock;
		ASSERT_TRUE(lock.lockStart(m_Temp));
		// less than the buffer of the stream: nothing is written yet, and nothing fails yet
		EXPECT_NO_THROW(tempSerial(out, s_TempNew));
		EXPECT_THROW(out.close(), NLMISC::EWriteError);
		EXPECT_FALSE(out.isOpen());
		EXPECT_NO_THROW(out.close());
	}
	EXPECT_EQ(tempReadFile(m_Target), s_TempOld);
	EXPECT_EQ(dirContent(), "target.foo");

	// the write fails before close(), and the caller closes all the same
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, false, false, true));
		CTempFileTestHandle lock;
		ASSERT_TRUE(lock.lockStart(m_Temp));
		const string big(256 * 1024, 'x');
		EXPECT_THROW(tempSerial(out, big), NLMISC::EWriteError);
		lock.close();
		EXPECT_THROW(out.close(), NLMISC::EWriteError);
		EXPECT_FALSE(out.isOpen());
	}
	EXPECT_EQ(tempReadFile(m_Target), s_TempOld);
	EXPECT_EQ(dirContent(), "target.foo");

	// a flush fails, which tells its caller nothing, and the close after it succeeds
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, false, false, true));
		CTempFileTestHandle lock;
		ASSERT_TRUE(lock.lockStart(m_Temp));
		EXPECT_NO_THROW(tempSerial(out, s_TempNew));
		EXPECT_NO_THROW(out.flush());
		lock.close();
		EXPECT_THROW(out.close(), NLMISC::EWriteError);
	}
	EXPECT_EQ(tempReadFile(m_Target), s_TempOld);
	EXPECT_EQ(dirContent(), "target.foo");

	// a seek writes what is buffered first: as the writers of the packed sheets do, who go back to fill in
	// a size and then to the end again
	{
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target, false, false, true));
		CTempFileTestHandle lock;
		ASSERT_TRUE(lock.lockStart(m_Temp));
		EXPECT_NO_THROW(tempSerial(out, s_TempNew));
		out.seek(4, NLMISC::IStream::begin);
		lock.close();
		EXPECT_NO_THROW(tempSerial(out, "size"));
		out.seek(0, NLMISC::IStream::end);
		EXPECT_THROW(out.close(), NLMISC::EWriteError);
	}
	EXPECT_EQ(tempReadFile(m_Target), s_TempOld);
	EXPECT_EQ(dirContent(), "target.foo");
}

// Without a temporary file nothing can be kept from being lost, and close() is called where no exception is
// expected: neither close() nor the destructor throws, both log one warning that names the file, and close()
// returns false
TEST_F(CUTMiscTempFileTest, plainFileFailedWriteIsLogged)
{
	const string logged = "The file '" + m_Target + "' is not written completely";

	// the write of what the stream has buffered fails in close()
	{
		CTempFileTestWarnings warnings(logged);
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target));
		CTempFileTestHandle lock;
		ASSERT_TRUE(lock.lockStart(m_Target));
		EXPECT_NO_THROW(tempSerial(out, s_TempNew));
		bool closed = true;
		EXPECT_NO_THROW(closed = out.close());
		EXPECT_FALSE(closed);
		EXPECT_FALSE(out.isOpen());
		EXPECT_EQ(warnings.count(), 1u);
		// a second close has nothing to close, and nothing to say
		EXPECT_NO_THROW(closed = out.close());
		EXPECT_TRUE(closed);
		EXPECT_EQ(warnings.count(), 1u);
	}
	EXPECT_EQ(tempReadFile(m_Target), "");

	// the destructor meets the same failure
	{
		CTempFileTestWarnings warnings(logged);
		CTempFileTestHandle lock;
		{
			NLMISC::COFile out;
			ASSERT_TRUE(out.open(m_Target));
			ASSERT_TRUE(lock.lockStart(m_Target));
			EXPECT_NO_THROW(tempSerial(out, s_TempNew));
		}
		EXPECT_EQ(warnings.count(), 1u);
	}
	EXPECT_EQ(tempReadFile(m_Target), "");

	// a flush fails, and the close after it succeeds
	{
		CTempFileTestWarnings warnings(logged);
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target));
		CTempFileTestHandle lock;
		ASSERT_TRUE(lock.lockStart(m_Target));
		EXPECT_NO_THROW(tempSerial(out, s_TempNew));
		EXPECT_NO_THROW(out.flush());
		lock.close();
		EXPECT_EQ(warnings.count(), 0u);
		bool closed = true;
		EXPECT_NO_THROW(closed = out.close());
		EXPECT_FALSE(closed);
		EXPECT_EQ(warnings.count(), 1u);
	}

	// a write that failed has thrown, and the close logs it once more, without throwing
	{
		CTempFileTestWarnings warnings(logged);
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target));
		CTempFileTestHandle lock;
		ASSERT_TRUE(lock.lockStart(m_Target));
		const string big(256 * 1024, 'x');
		EXPECT_THROW(tempSerial(out, big), NLMISC::EWriteError);
		lock.close();
		bool closed = true;
		EXPECT_NO_THROW(closed = out.close());
		EXPECT_FALSE(closed);
		EXPECT_EQ(warnings.count(), 1u);
	}

	// a file that is written completely logs nothing
	{
		CTempFileTestWarnings warnings(logged);
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(m_Target));
		EXPECT_NO_THROW(tempSerial(out, s_TempNew));
		out.flush();
		bool closed = false;
		EXPECT_NO_THROW(closed = out.close());
		EXPECT_TRUE(closed);
		EXPECT_EQ(warnings.count(), 0u);
	}
	EXPECT_EQ(tempReadFile(m_Target), s_TempNew);
}

// Appending through a temporary file starts from a copy of the target. A target that cannot be read to its
// end gives no copy to append to: the file is not opened, and no part of a copy is left.
TEST_F(CUTMiscTempFileTest, appendToUnreadableTarget)
{
	tempWriteFile(m_Target, s_TempOld);

	{
		CTempFileTestHandle lock;
		ASSERT_TRUE(lock.lockStart(m_Target));
		NLMISC::COFile out;
		EXPECT_FALSE(out.open(m_Target, true, false, true));
		EXPECT_FALSE(out.isOpen());
		EXPECT_EQ(dirContent(), "target.foo");
	}
	EXPECT_EQ(tempReadFile(m_Target), s_TempOld);
	EXPECT_EQ(dirContent(), "target.foo");
}

#else // NL_OS_WINDOWS

// /dev/full takes no data: every write to it fails as on a full disk
TEST_F(CUTMiscTempFileTest, diskFull)
{
	const string full = "/dev/full";
	if (access(full.c_str(), W_OK) != 0)
	{
		GTEST_SKIP() << "no " << full << " on this system";
	}
	const string logged = "The file '" + full + "' is not written completely";

	// more than the buffer of the stream: the write itself meets the full disk
	{
		CTempFileTestWarnings warnings(logged);
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(full));
		const string big(256 * 1024, 'x');
		EXPECT_THROW(tempSerial(out, big), NLMISC::EDiskFullError);
		bool closed = true;
		EXPECT_NO_THROW(closed = out.close());
		EXPECT_FALSE(closed);
		EXPECT_EQ(warnings.count(), 1u);
	}

	// less than the buffer: the close meets it. Without a temporary file it does not throw: it logs, and
	// returns that the file is not written.
	{
		CTempFileTestWarnings warnings(logged);
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(full));
		EXPECT_NO_THROW(tempSerial(out, s_TempNew));
		bool closed = true;
		EXPECT_NO_THROW(closed = out.close());
		EXPECT_FALSE(closed);
		EXPECT_EQ(warnings.count(), 1u);
	}

	// a flush meets it, and the close after it has nothing left to write
	{
		CTempFileTestWarnings warnings(logged);
		NLMISC::COFile out;
		ASSERT_TRUE(out.open(full));
		EXPECT_NO_THROW(tempSerial(out, s_TempNew));
		EXPECT_NO_THROW(out.flush());
		EXPECT_EQ(warnings.count(), 0u);
		bool closed = true;
		EXPECT_NO_THROW(closed = out.close());
		EXPECT_FALSE(closed);
		EXPECT_EQ(warnings.count(), 1u);
	}

	// a copy whose data is first written when the copy is closed is not complete either
	tempWriteFile(m_Target, s_TempOld);
	EXPECT_FALSE(NLMISC::CFile::copyFile(full, m_Target));
}

#endif // NL_OS_WINDOWS
