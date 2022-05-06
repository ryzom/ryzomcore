// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "filesextractor.h"
#include "operation.h"
#include "utils.h"

#include "nel/misc/big_file.h"
#include "nel/misc/callback.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"

#include "7z.h"
#include "7zAlloc.h"
#include "7zBuf.h"
#include "7zCrc.h"

#include "qzipreader.h"

#include <sys/stat.h>

#ifndef FILE_ATTRIBUTE_READONLY
#define FILE_ATTRIBUTE_READONLY            0x1
#endif
#ifndef FILE_ATTRIBUTE_HIDDEN
#define FILE_ATTRIBUTE_HIDDEN              0x2
#endif

#ifndef FILE_ATTRIBUTE_SYSTEM
#define FILE_ATTRIBUTE_SYSTEM              0x4
#endif

#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY          0x10
#endif

#ifndef FILE_ATTRIBUTE_ARCHIVE
#define FILE_ATTRIBUTE_ARCHIVE            0x20
#endif

#ifndef FILE_ATTRIBUTE_DEVICE
#define FILE_ATTRIBUTE_DEVICE             0x40
#endif

#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_NORMAL             0x80
#endif

#ifndef FILE_ATTRIBUTE_TEMPORARY
#define FILE_ATTRIBUTE_TEMPORARY         0x100
#endif

#ifndef FILE_ATTRIBUTE_SPARSE_FILE
#define FILE_ATTRIBUTE_SPARSE_FILE       0x200
#endif

#ifndef FILE_ATTRIBUTE_REPARSE_POINT
#define FILE_ATTRIBUTE_REPARSE_POINT     0x400
#endif

#ifndef FILE_ATTRIBUTE_COMPRESSED
#define FILE_ATTRIBUTE_COMPRESSED        0x800
#endif

#ifndef FILE_ATTRIBUTE_OFFLINE
#define FILE_ATTRIBUTE_OFFLINE          0x1000
#endif

#ifndef FILE_ATTRIBUTE_ENCRYPTED
#define FILE_ATTRIBUTE_ENCRYPTED        0x4000
#endif

#define FILE_ATTRIBUTE_UNIX_EXTENSION   0x8000   /* trick for Unix */

#define FILE_ATTRIBUTE_WINDOWS          0x7fff
#define FILE_ATTRIBUTE_UNIX         0xffff0000

#define kInputBufSize ((size_t)1 << 18)

bool Set7zFileAttrib(const QString &filename, uint32 fileAttributes)
{
	if (filename.isEmpty()) return false;

	bool attrReadOnly = (fileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
	bool attrHidden = (fileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
	bool attrSystem = (fileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0;
	bool attrDir = (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	bool attrArchive = (fileAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0;
	bool attrDevice = (fileAttributes & FILE_ATTRIBUTE_DEVICE) != 0;
	bool attrNormal = (fileAttributes & FILE_ATTRIBUTE_NORMAL) != 0;
	bool attrTemp = (fileAttributes & FILE_ATTRIBUTE_TEMPORARY) != 0;
	bool attrSparceFile = (fileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) != 0;
	bool attrReparsePoint = (fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
	bool attrCompressed = (fileAttributes & FILE_ATTRIBUTE_COMPRESSED) != 0;
	bool attrOffline = (fileAttributes & FILE_ATTRIBUTE_OFFLINE) != 0;
	bool attrEncrypted = (fileAttributes & FILE_ATTRIBUTE_ENCRYPTED) != 0;
	bool attrUnix = (fileAttributes & FILE_ATTRIBUTE_UNIX_EXTENSION) != 0;

	uint32 unixAttributes = (fileAttributes & FILE_ATTRIBUTE_UNIX) >> 16;
	uint32 windowsAttributes = fileAttributes & FILE_ATTRIBUTE_WINDOWS;

//	qDebug() << "attribs" << QByteArray::fromRawData((const char*)&fileAttributes, 4).toHex();

#ifdef Q_OS_WIN
	SetFileAttributesW((wchar_t*)filename.utf16(), windowsAttributes);
#else
	std::string name = filename.toUtf8().constData();

	mode_t current_umask = umask(0); // get and set the umask
	umask(current_umask); // restore the umask
	mode_t mask = 0777 & (~current_umask);

	struct stat stat_info;

	if (lstat(name.c_str(), &stat_info) != 0)
	{
		nlwarning("Unable to get file attributes for %s", name.c_str());
		return false;
	}

	if (attrUnix)
	{
		stat_info.st_mode = unixAttributes;

		// ignore symbolic links
		if (!S_ISLNK(stat_info.st_mode))
		{
			if (S_ISREG(stat_info.st_mode))
			{
				chmod(name.c_str(), stat_info.st_mode & mask);
			}
			else if (S_ISDIR(stat_info.st_mode))
			{
				// user/7za must be able to create files in this directory
				stat_info.st_mode |= (S_IRUSR | S_IWUSR | S_IXUSR);
				chmod(name.c_str(), stat_info.st_mode & mask);
			}
		}
	}
	else if (!S_ISLNK(stat_info.st_mode) && !S_ISDIR(stat_info.st_mode) && attrReadOnly)
	{
		// do not use chmod on a link
		// Remark : FILE_ATTRIBUTE_READONLY ignored for directory.
		// Only Windows Attributes

		// octal!, clear write permission bits
		stat_info.st_mode &= ~0222;

		chmod(name.c_str(), stat_info.st_mode & mask);
	}
#endif

	return true;
}

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

#define SZ_ERROR_INTERRUPTED 18

class Q7zFile : public ISeekInStream
{
	QFile m_file;

public:
	Q7zFile(const QString &filename):m_file(filename)
	{
		Read = readFunc;
		Seek = seekFunc;
	}

	~Q7zFile()
	{
	}

	bool open()
	{
		return m_file.open(QFile::ReadOnly);
	}

	// the read function called by 7zip to read data
	static SRes readFunc(const ISeekInStream *object, void *buffer, size_t *size)
	{
		Q7zFile *me = (Q7zFile*)object;
		qint64 len = *size;
		len = me->m_file.read((char*)buffer, len);

		if (len == *size)
		{
			*size = len;
			return SZ_OK;
		}
		else
		{
			return SZ_ERROR_READ;
		}
	}

	// the seek function called by seven zip to seek inside stream
	static SRes seekFunc(const ISeekInStream *object, Int64 *pos, ESzSeek origin)
	{
		Q7zFile *me = (Q7zFile*)object;
		qint64 newPos = 0;

		switch(origin)
		{
			case SZ_SEEK_SET: newPos = *pos; break;
			case SZ_SEEK_CUR: newPos = me->m_file.pos() + *pos; break;
			case SZ_SEEK_END: newPos = me->m_file.size() - *pos; break;
		}

		if (me->m_file.seek(newPos))
		{
			*pos = newPos;
			return SZ_OK;
		}
		else
		{
			return SZ_ERROR_READ;
		}
	}
};

CFilesExtractor::CFilesExtractor(IOperationProgressListener *listener):m_listener(listener)
{
}

CFilesExtractor::~CFilesExtractor()
{
}

void CFilesExtractor::setSourceFile(const QString &src)
{
	m_sourceFile = src;
}

void CFilesExtractor::setDestinationDirectory(const QString &dst)
{
	m_destinationDirectory = dst;
}

bool CFilesExtractor::exec()
{
	if (m_sourceFile.isEmpty() || m_destinationDirectory.isEmpty()) return false;

	if (m_listener) m_listener->operationPrepare();

	QFile file(m_sourceFile);

	// open archive file to check format
	if (!file.open(QFile::ReadOnly)) return false;

	// read 2 first bytes
	QByteArray header = file.read(2);

	// close file
	file.close();

	// create destination directory
	QDir dir;
	dir.mkpath(m_destinationDirectory);

	// compare to supported formats and call the appropriate decompressor
	if (header == "7z")
	{
		return extract7z();
	}

	if (header == "PK")
	{
		return extractZip();
	}

	if (QFileInfo(m_sourceFile).suffix().toLower() == "bnp")
	{
		return extractBnp();
	}

	nlwarning("Unsupported format for file %s", Q2C(m_sourceFile));
	return false;
}

static uint32 convertWindowsFileTimeToUnixTimestamp(const CNtfsFileTime &nt)
{
	// first, convert it into second since jan1, 1601
	uint64 t = nt.Low | (uint64(nt.High) << 32);

	// offset to convert Windows file times to UNIX timestamp
	uint64 offset = UINT64_CONSTANT(116444736000000000);

	// adjust time base to unix epoch base
	t -= offset;

	// convert the resulting time into seconds
	t /= 10;	// microsec
	t /= 1000;	// millisec
	t /= 1000;	// sec

	// return the resulting time
	return uint32(t);
}

bool CFilesExtractor::extract7z()
{
	Q7zFile inFile(m_sourceFile);

	if (!inFile.open())
	{
		nlwarning("Unable to open %s", Q2C(m_sourceFile));

		if (m_listener) m_listener->operationFail(QApplication::tr("Unable to open %1").arg(m_sourceFile));
		return false;
	}

	UInt16 *temp = NULL;
	size_t tempSize = 0;

	// register allocators
	ISzAlloc allocImp;
	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	ISzAlloc allocTempImp;
	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	// register the files read handlers
	CLookToRead2 lookStream;
	LookToRead2_CreateVTable(&lookStream, False);
	lookStream.buf = (Byte*)ISzAlloc_Alloc(&allocImp, kInputBufSize);

	if (!lookStream.buf)
	{
		nlwarning("Unable to allocate %u bytes", (uint32)kInputBufSize);

		if (m_listener) m_listener->operationFail(QApplication::tr("Unable to allocate %1 bytes").arg(kInputBufSize));
		return false;
	}

	LookToRead2_Init(&lookStream);

	lookStream.bufSize = kInputBufSize;
	lookStream.realStream = &inFile;
	LookToRead2_Init(&lookStream);

	// init CRC table
	CrcGenerateTable();

	// init 7z
	CSzArEx db;
	SzArEx_Init(&db);

	qint64 total = 0, totalUncompressed = 0;
	QString error;

	// open 7z archive
	SRes res = SzArEx_Open(&db, &lookStream.vt, &allocImp, &allocTempImp);

	if (res == SZ_OK)
	{
		// process each file in archive
		for (UInt32 i = 0; i < db.NumFiles; ++i)
		{
			bool isDir = SzArEx_IsDir(&db, i) != 0;

			if (!isDir) total += SzArEx_GetFileSize(&db, i);
		}

		if (m_listener)
		{
			m_listener->operationInit(0, total);
			m_listener->operationStart();
		}

		// variables used for decompression
		UInt32 blockIndex = 0xFFFFFFFF;
		Byte *outBuffer = NULL;
		size_t outBufferSize = 0;

		// process each file in archive
		for (UInt32 i = 0; i < db.NumFiles; ++i)
		{
			if (m_listener && m_listener->operationShouldStop())
			{
				res = SZ_ERROR_INTERRUPTED;
				break;
			}

			size_t offset = 0;
			size_t outSizeProcessed = 0;

			bool isDir = SzArEx_IsDir(&db, i) != 0;

			size_t len = SzArEx_GetFileNameUtf16(&db, i, NULL);

			if (len > tempSize)
			{
				SzFree(NULL, temp);
				tempSize = len;
				temp = (UInt16 *)SzAlloc(NULL, tempSize * sizeof(temp[0]));
				if (!temp)
				{
					res = SZ_ERROR_MEM;
					break;
				}
			}

			SzArEx_GetFileNameUtf16(&db, i, temp);

			QString path = QString::fromUtf16(temp);
			QString filename = QFileInfo(path).fileName();

			QString destPath = m_destinationDirectory + '/' + path;

			// get uncompressed size
			quint64 uncompressedSize = SzArEx_GetFileSize(&db, i);

			// get modification time
			quint32 modificationTime = 0, creationTime = 0;

			if (SzBitWithVals_Check(&db.MTime, i))
			{
				modificationTime = convertWindowsFileTimeToUnixTimestamp(db.MTime.Vals[i]);
			}

			if (SzBitWithVals_Check(&db.CTime, i))
			{
				creationTime = convertWindowsFileTimeToUnixTimestamp(db.CTime.Vals[i]);
			}

			if (isDir)
			{
				QDir().mkpath(destPath);
				continue;
			}

			// check if file exists
			if (QFile::exists(destPath))
			{
				QFileInfo currentFileInfo(destPath);

				// skip file if same size and same modification date
				if (currentFileInfo.lastModified().toTime_t() == modificationTime && currentFileInfo.size() == uncompressedSize)
				{
					// update progress
					totalUncompressed += uncompressedSize;

					if (m_listener) m_listener->operationProgress(totalUncompressed, filename);

					continue;
				}
			}

			if (m_listener) m_listener->operationProgress(totalUncompressed, filename);

			res = SzArEx_Extract(&db, &lookStream.vt, i, &blockIndex, &outBuffer, &outBufferSize,
				&offset, &outSizeProcessed, &allocImp, &allocTempImp);

			if (res != SZ_OK) break;

			QString destSubPath = QFileInfo(destPath).absolutePath();

			// create file directory
			if (!QDir().mkpath(destSubPath))
			{
				nlwarning("Unable to create directory %s", Q2C(destSubPath));
			}

			// create file
			QSaveFile outFile(destPath);

			if (!outFile.open(QFile::WriteOnly))
			{
				nlwarning("Unable to open file %s", Q2C(destPath));

				error = QApplication::tr("Unable to open output file %1").arg(destPath);
				res = SZ_ERROR_FAIL;
				break;
			}

			qint64 currentSizeToProcess = outSizeProcessed;
			
			do
			{
				qint64 currentProcessedSize = outFile.write((const char*)(outBuffer + offset), currentSizeToProcess);

				// errors only occur when returned size is -1
				if (currentProcessedSize < 0) break;

				offset += currentProcessedSize;
				currentSizeToProcess -= currentProcessedSize;
			}
			while (currentSizeToProcess > 0);

			if (offset != outSizeProcessed)
			{
				nlwarning("Unable to write output file %s (%u bytes written but expecting %u bytes)", Q2C(destPath), (uint32)offset, (uint32)outSizeProcessed);

				error = QApplication::tr("Unable to write output file %1 (%2 bytes written but expecting %3 bytes)").arg(destPath).arg(offset).arg(outSizeProcessed);
				res = SZ_ERROR_FAIL;
				break;
			}

			outFile.commit();

			totalUncompressed += uncompressedSize;

			if (m_listener) m_listener->operationProgress(totalUncompressed, filename);

			// set attributes
			if (SzBitWithVals_Check(&db.Attribs, i))
			{
				Set7zFileAttrib(destPath, db.Attribs.Vals[i]);
			}

			// set modification time
			if (!NLMISC::CFile::setFileModificationDate(qToUtf8(destPath), modificationTime))
			{
				nlwarning("Unable to change date of %s", Q2C(destPath));
			}
		}

		IAlloc_Free(&allocImp, outBuffer);
	}

	SzFree(NULL, temp);
	SzArEx_Free(&db, &allocImp);
	ISzAlloc_Free(&allocImp, lookStream.buf);

	switch(res)
	{
		case SZ_OK:
		if (m_listener)
		{
			m_listener->operationSuccess(totalUncompressed);
		}
		return true;

		case SZ_ERROR_INTERRUPTED:
		if (m_listener) m_listener->operationStop();
		return true;

		case SZ_ERROR_UNSUPPORTED:
		error = QApplication::tr("7zip decoder doesn't support this archive");
		break;

		case SZ_ERROR_MEM:
		error = QApplication::tr("Unable to allocate memory");
		break;

		case SZ_ERROR_CRC:
		error = QApplication::tr("7zip decoder doesn't support this archive");
		break;

		case SZ_ERROR_INPUT_EOF:
		error = QApplication::tr("File %1 is corrupted, unable to uncompress it").arg(m_sourceFile);
		break;

		case SZ_ERROR_FAIL:
		// error already defined
		break;

		default:
		error = QApplication::tr("Error %1").arg(res);
	}

	if (m_listener) m_listener->operationFail(error);

	return false;
}

bool CFilesExtractor::extractZip()
{
	QZipReader reader(m_sourceFile);

	QDir baseDir(m_destinationDirectory);

	// create directories first
	QList<QZipReader::FileInfo> allFiles = reader.fileInfoList();

	qint64 totalSize = 0, currentSize = 0;

	foreach (const QZipReader::FileInfo &fi, allFiles)
	{
		if (fi.isDir)
		{
			const QString absPath = m_destinationDirectory + QDir::separator() + fi.filePath;

			if (!baseDir.mkpath(fi.filePath))
			{
				nlwarning("Unable to create directory %s", Q2C(fi.filePath));

				if (m_listener) m_listener->operationFail(QApplication::tr("Unable to create directory %1").arg(fi.filePath));
				return false;
			}

			if (!QFile::setPermissions(absPath, fi.permissions))
			{
				nlwarning("Unable to change permissions of %s", Q2C(absPath));

				if (m_listener) m_listener->operationFail(QApplication::tr("Unable to set permissions of %1").arg(absPath));
				return false;
			}
		}

		totalSize += fi.size;
	}

	if (m_listener)
	{
		m_listener->operationInit(0, totalSize);
		m_listener->operationStart();
	}

	// client won't use symbolic links so don't process them

	foreach (const QZipReader::FileInfo &fi, allFiles)
	{
		const QString absPath = m_destinationDirectory + QDir::separator() + fi.filePath;

		if (fi.isFile)
		{
			if (m_listener && m_listener->operationShouldStop())
			{
				m_listener->operationStop();
				return true;
			}

			QSaveFile f(absPath);

			if (!f.open(QIODevice::WriteOnly))
			{
				nlwarning("Unable to open %s", Q2C(absPath));

				if (m_listener) m_listener->operationFail(QApplication::tr("Unable to open %1").arg(absPath));
				return false;
			}

			currentSize += f.write(reader.fileData(fi.filePath));

			if (!f.setPermissions(fi.permissions))
			{
				nlwarning("Unable to change permissions of %s", Q2C(absPath));
			}

			f.commit();

			// set the right modification date
			if (!NLMISC::CFile::setFileModificationDate(qToUtf8(absPath), fi.lastModified.toTime_t()))
			{
				nlwarning("Unable to change date of %s", Q2C(absPath));
			}

			if (m_listener) m_listener->operationProgress(currentSize, QFileInfo(absPath).fileName());
		}
	}

	if (m_listener)
	{
		m_listener->operationSuccess(totalSize);
	}

	return true;
}

bool CFilesExtractor::progress(const std::string &filename, uint32 currentSize, uint32 totalSize)
{
	if (m_listener && m_listener->operationShouldStop())
	{
		m_listener->operationStop();
	
		return false;
	}

	if (currentSize == 0)
	{
		if (m_listener)
		{
			m_listener->operationInit(0, (qint64)totalSize);
			m_listener->operationStart();
		}
	}

	if (m_listener) m_listener->operationProgress((qint64)currentSize, qFromUtf8(filename));
	
	if (currentSize == totalSize)
	{
		if (m_listener)
		{
			m_listener->operationSuccess((qint64)totalSize);
		}
	}

	return true;
}

bool CFilesExtractor::extractBnp()
{
	QString error;

	NLMISC::CBigFile::TUnpackProgressCallback cbMethod = NLMISC::CBigFile::TUnpackProgressCallback(this, &CFilesExtractor::progress);

	try
	{
		if (NLMISC::CBigFile::unpack(qToUtf8(m_sourceFile), qToUtf8(m_destinationDirectory), &cbMethod))
		{
			return true;
		}

		if (m_listener && m_listener->operationShouldStop())
		{
			// stopped
			m_listener->operationStop();

			return true;
		}

		error.clear();
	}
	catch(const NLMISC::EDiskFullError &e)
	{
		nlwarning("Disk full when extracting %s to %s", Q2C(m_sourceFile), Q2C(m_destinationDirectory));

		error = QApplication::tr("disk full");
	}
	catch(const NLMISC::EWriteError &e)
	{
		nlwarning("Write error when extracting %s to %s", Q2C(m_sourceFile), Q2C(m_destinationDirectory));

		error = QApplication::tr("unable to write %1").arg(qFromUtf8(e.Filename));
	}
	catch(const NLMISC::EReadError &e)
	{
		nlwarning("Read error when extracting %s to %s", Q2C(m_sourceFile), Q2C(m_destinationDirectory));

		error = QApplication::tr("unable to read %1").arg(qFromUtf8(e.Filename));
	}
	catch(const std::exception &e)
	{
		nlwarning("Unknown exception when extracting %s to %s", Q2C(m_sourceFile), Q2C(m_destinationDirectory));

		error = QApplication::tr("failed (%1)").arg(qFromUtf8(e.what()));
	}

	if (m_listener) m_listener->operationFail(QApplication::tr("Unable to unpack %1 to %2: %3").arg(m_sourceFile).arg(m_destinationDirectory).arg(error));

	return false;
}
