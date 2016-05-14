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
#include "archive.h"
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

#include <Windows.h>

#define FILE_ATTRIBUTE_READONLY            0x1
#define FILE_ATTRIBUTE_HIDDEN              0x2
#define FILE_ATTRIBUTE_SYSTEM              0x4
#define FILE_ATTRIBUTE_DIRECTORY          0x10
#define FILE_ATTRIBUTE_ARCHIVE            0x20
#define FILE_ATTRIBUTE_DEVICE             0x40
#define FILE_ATTRIBUTE_NORMAL             0x80
#define FILE_ATTRIBUTE_TEMPORARY         0x100
#define FILE_ATTRIBUTE_SPARSE_FILE       0x200
#define FILE_ATTRIBUTE_REPARSE_POINT     0x400
#define FILE_ATTRIBUTE_COMPRESSED        0x800
#define FILE_ATTRIBUTE_OFFLINE          0x1000
#define FILE_ATTRIBUTE_ENCRYPTED        0x4000
#define FILE_ATTRIBUTE_UNIX_EXTENSION   0x8000   /* trick for Unix */

#define FILE_ATTRIBUTE_WINDOWS          0x5fff
#define FILE_ATTRIBUTE_UNIX         0xffff0000

bool Set7zFileAttrib(const QString &filename, uint32 fileAttributes)
{
	bool attrReadOnly = (fileAttributes & FILE_ATTRIBUTE_READONLY != 0);
	bool attrHidden = (fileAttributes & FILE_ATTRIBUTE_HIDDEN != 0);
	bool attrSystem = (fileAttributes & FILE_ATTRIBUTE_SYSTEM != 0);
	bool attrDir = (fileAttributes & FILE_ATTRIBUTE_DIRECTORY != 0);
	bool attrArchive = (fileAttributes & FILE_ATTRIBUTE_ARCHIVE != 0);
	bool attrDevice = (fileAttributes & FILE_ATTRIBUTE_DEVICE != 0);
	bool attrNormal = (fileAttributes & FILE_ATTRIBUTE_NORMAL != 0);
	bool attrTemp = (fileAttributes & FILE_ATTRIBUTE_TEMPORARY != 0);
	bool attrSparceFile = (fileAttributes & FILE_ATTRIBUTE_SPARSE_FILE != 0);
	bool attrReparsePoint = (fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT != 0);
	bool attrCompressed = (fileAttributes & FILE_ATTRIBUTE_COMPRESSED != 0);
	bool attrOffline = (fileAttributes & FILE_ATTRIBUTE_OFFLINE != 0);
	bool attrEncrypted = (fileAttributes & FILE_ATTRIBUTE_ENCRYPTED != 0);
	bool attrUnix = (fileAttributes & FILE_ATTRIBUTE_UNIX_EXTENSION != 0);

	uint32 unixAttributes = (fileAttributes & FILE_ATTRIBUTE_UNIX) >> 16;
	uint32 windowsAttributes = fileAttributes & FILE_ATTRIBUTE_WINDOWS;

	qDebug() << "attribs" << QByteArray::fromRawData((const char*)&fileAttributes, 4).toHex();

#ifdef Q_OS_WIN
	SetFileAttributesW((wchar_t*)filename.utf16(), windowsAttributes);
#else
	const char *name = filename.toUtf8().constData();

	struct stat stat_info;
	if (lstat(name, &stat_info)!=0)
	{
		nlwarning("SetFileAttrib(%s,%d) : false-2-1", (const char *)name, fileAttributes);
		return false;
	}

	if (attrUnix)
	{
		stat_info.st_mode = unixAttributes;

		if (S_ISLNK(stat_info.st_mode))
		{
			if (convert_to_symlink(name) != 0)
			{
				nlwarning("SetFileAttrib(%s,%d) : false-3",(const char *)name,fileAttributes);
				return false;
			}
		}
		else if (S_ISREG(stat_info.st_mode))
		{
			nlwarning("##DBG chmod-2(%s,%o)", (const char *)name, (unsigned)stat_info.st_mode & gbl_umask.mask);
			chmod(name, stat_info.st_mode & gbl_umask.mask);
		}
		else if (S_ISDIR(stat_info.st_mode))
		{
			// user/7za must be able to create files in this directory
			stat_info.st_mode |= (S_IRUSR | S_IWUSR | S_IXUSR);
			nlwarning("##DBG chmod-3(%s,%o)", (const char *)name, (unsigned)stat_info.st_mode & gbl_umask.mask);
			chmod(name, stat_info.st_mode & gbl_umask.mask);
		}
	}
	else if (!S_ISLNK(stat_info.st_mode))
	{
		// do not use chmod on a link

		// Only Windows Attributes
		if( S_ISDIR(stat_info.st_mode))
		{
			// Remark : FILE_ATTRIBUTE_READONLY ignored for directory.
			nlwarning("##DBG chmod-4(%s,%o)", (const char *)name, (unsigned)stat_info.st_mode & gbl_umask.mask);
			chmod(name,stat_info.st_mode & gbl_umask.mask);
		}
		else
		{
			// octal!, clear write permission bits
			if (fileAttributes & FILE_ATTRIBUTE_READONLY) stat_info.st_mode &= ~0222;
			nlwarning("##DBG chmod-5(%s,%o)", (const char *)name, (unsigned)stat_info.st_mode & gbl_umask.mask);
			chmod(name,stat_info.st_mode & gbl_umask.mask);
		}
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
	static SRes readFunc(void *object, void *buffer, size_t *size)
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
	static SRes seekFunc(void *object, Int64 *pos, ESzSeek origin)
	{
		Q7zFile *me = (Q7zFile*)object;
		qint64 newPos;

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

CArchive::CArchive(QObject *parent):QObject(parent), m_mustStop(false)
{
}

CArchive::~CArchive()
{
}

bool CArchive::extract(const QString &filename, const QString &dest)
{
	m_filename = filename;
	m_dest = dest;

	QFile file(m_filename);

	// open archive file to check format
	if (!file.open(QFile::ReadOnly)) return false;

	// read 2 first bytes
	QByteArray header = file.read(2);

	// close file
	file.close();

	// create destination directory
	QDir dir;
	dir.mkpath(dest);

	QFuture<bool> future;

	// compare to supported formats and call the appropriate decompressor
	if (header == "7z")
	{
		future = QtConcurrent::run(this, &CArchive::extract7z);
	}
	else if (header == "PK")
	{
		future = QtConcurrent::run(this, &CArchive::extractZip);
	}
	else if (QFileInfo(filename).suffix().toLower() == "bnp")
	{
		future = QtConcurrent::run(this, &CArchive::extractBnp);
	}
	else
	{
		qDebug() << "Unsupported format";
		return false;
	}

	return true;
}

void CArchive::getFilesList(const QString &srcDir, const QString &dstDir, const QStringList &filter, FilesToCopy &files)
{
	QDir dir(srcDir);

	QFileInfoList entries = dir.entryInfoList(filter);

	foreach(const QFileInfo &entry, entries)
	{
		QString fullPath = entry.absoluteFilePath();

		QString dstPath = dstDir + "/" + dir.relativeFilePath(fullPath);

		if (entry.isDir())
		{
			QDir().mkpath(dstPath);

			QDir subDir(fullPath);

			QDirIterator it(subDir, QDirIterator::Subdirectories);

			while (it.hasNext())
			{
				fullPath = it.next();

				if (it.fileName().startsWith('.')) continue;

				QFileInfo fileInfo = it.fileInfo();

				dstPath = dstDir + "/" + dir.relativeFilePath(fullPath);

				if (fileInfo.isDir())
				{
					QDir().mkpath(dstPath);
				}
				else
				{
					FileToCopy file;
					file.filename = it.fileName();
					file.src = it.filePath();
					file.dst = dstPath;
					file.size = it.fileInfo().size();
					file.date = it.fileInfo().lastModified();

					files << file;
				}
			}
		}
		else
		{
			FileToCopy file;
			file.filename = entry.fileName();
			file.src = entry.filePath();
			file.dst = dstPath;
			file.size = entry.size();
			file.date = entry.lastModified();

			files << file;
		}
	}
}

bool CArchive::copyServerFiles()
{
	emit extractPrepare();

	FilesToCopy files;

	QStringList serverFiles;
	serverFiles << "cfg";
	serverFiles << "data";
	serverFiles << "examples";
	serverFiles << "patch";
	serverFiles << "unpack";
	serverFiles << "client_default.cfg";

	CArchive::getFilesList(m_filename, m_dest, serverFiles, files);

	return copyFiles(files);
}

bool CArchive::copyProfileFiles()
{
	emit extractPrepare();

	FilesToCopy files;

	QStringList configFiles;
	configFiles << "cache";
	configFiles << "save";
	configFiles << "user";
	configFiles << "screenshots";
	configFiles << "client.cfg";
	configFiles << "*.log";

	CArchive::getFilesList(m_filename, m_dest, configFiles, files);

	return copyFiles(files);
}

bool CArchive::cleanServerFiles(const QString &directory)
{
	QDir dir(directory);

	// directory doesn't exist
	if (!dir.exists()) return false;

	if (!dir.cd("data") && dir.exists()) return false;

	// temporary files
	QStringList files = dir.entryList(QStringList() << "*.string_cache" << "*.packed_sheets" << "*.packed" << "*.pem", QDir::Files);

	foreach(const QString &file, files)
	{
		dir.remove(file);
	}

	// fonts directory is not needed anymore
	if (dir.cd("fonts") && dir.exists())
	{
		dir.removeRecursively();
	}

	emit done();

	return true;
}

bool CArchive::copyServerFiles(const QString &src, const QString &dst)
{
	if (src.isEmpty() || dst.isEmpty()) return false;

	m_filename = src;
	m_dest = dst;

	// create destination directory
	QDir().mkpath(dst);

	QFuture<bool> future = QtConcurrent::run(this, &CArchive::copyServerFiles);

	return true;
}

bool CArchive::copyProfileFiles(const QString &src, const QString &dst)
{
	if (src.isEmpty() || dst.isEmpty()) return false;

	m_filename = src;
	m_dest = dst;

	// create destination directory
	QDir().mkpath(dst);

	QFuture<bool> future = QtConcurrent::run(this, &CArchive::copyProfileFiles);

	return true;
}

bool CArchive::copyFiles(const FilesToCopy &files)
{
	qint64 totalSize = 0;

	foreach(const FileToCopy &file, files)
	{
		totalSize += file.size;

		qDebug() << file.filename;
	}

	emit extractInit(0, totalSize);

	emit extractStart();

	qint64 processedSize = 0;

	foreach(const FileToCopy &file, files)
	{
		if (mustStop())
		{
			emit extractStop();
			return true;
		}

		emit extractProgress(processedSize, file.filename);

		QFileInfo dstFileInfo(file.dst);

		if (dstFileInfo.size() != file.size || dstFileInfo.lastModified() != file.date)
		{
			if (!QFile::copy(file.src, file.dst))
			{
				emit extractFail(tr("Unable to copy file %1").arg(file.src));
				return false;
			}

			if (!NLMISC::CFile::setFileModificationDate(qToUtf8(file.dst), file.date.toTime_t()))
			{
				qDebug() << "Unable to change date";
			}
		}

		processedSize += file.size;
	}

	emit extractSuccess(totalSize);

	return true;
}

bool CArchive::extract7z()
{
	Q7zFile inFile(m_filename);

	if (!inFile.open())
	{
		emit extractFail(tr("Unable to open %1").arg(m_filename));
		return false;
	}

	emit extractPrepare();

	UInt16 *temp = NULL;
	size_t tempSize = 0;

	// register the files read handlers
	CLookToRead lookStream;
	lookStream.realStream = &inFile;
	LookToRead_CreateVTable(&lookStream, False);
	LookToRead_Init(&lookStream);

	// init CRC table
	CrcGenerateTable();

	// init 7z
	CSzArEx db;
	SzArEx_Init(&db);

	// register allocators
	ISzAlloc allocImp;
	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	ISzAlloc allocTempImp;
	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	qint64 total = 0, totalUncompressed = 0;
	QString error;

	// open 7z acrhive
	SRes res = SzArEx_Open(&db, &lookStream.s, &allocImp, &allocTempImp);

	if (res == SZ_OK)
	{
		// process each file in archive
		for (UInt32 i = 0; i < db.NumFiles; ++i)
		{
			bool isDir = SzArEx_IsDir(&db, i) != 0;

			if (!isDir) total += SzArEx_GetFileSize(&db, i);
		}

		emit extractInit(0, total);

		emit extractStart();

		// variables used for decompression
		UInt32 blockIndex = 0xFFFFFFFF;
		Byte *outBuffer = NULL;
		size_t outBufferSize = 0;

		// process each file in archive
		for (UInt32 i = 0; i < db.NumFiles; ++i)
		{
			if (mustStop())
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

			if (!isDir)
			{
				emit extractProgress(totalUncompressed, filename);

				res = SzArEx_Extract(&db, &lookStream.s, i, &blockIndex, &outBuffer, &outBufferSize,
					&offset, &outSizeProcessed, &allocImp, &allocTempImp);

				if (res != SZ_OK) break;
			}

			QString destPath = m_dest + '/' + path;

			QDir dir;

			if (isDir)
			{
				dir.mkpath(destPath);
				continue;
			}

			dir.mkpath(QFileInfo(destPath).absolutePath());

			QFile outFile(destPath);

			if (!outFile.open(QFile::WriteOnly))
			{
				error = tr("Unable to open output file");
				res = SZ_ERROR_FAIL;
				break;
			}

			size_t processedSize = outFile.write((const char*)(outBuffer + offset), outSizeProcessed);

			if (processedSize != outSizeProcessed)
			{
				error = tr("Unable to write output file");
				res = SZ_ERROR_FAIL;
				break;
			}

			outFile.close();

			totalUncompressed += SzArEx_GetFileSize(&db, i);

			emit extractProgress(totalUncompressed, filename);

			if (SzBitWithVals_Check(&db.Attribs, i))
				Set7zFileAttrib(destPath, db.Attribs.Vals[i]);
		}

		IAlloc_Free(&allocImp, outBuffer);
	}

	SzArEx_Free(&db, &allocImp);
	SzFree(NULL, temp);

	switch(res)
	{
		case SZ_OK:
		emit extractSuccess(totalUncompressed);
		return true;

		case SZ_ERROR_INTERRUPTED:
		emit extractStop();
		return true;

		case SZ_ERROR_UNSUPPORTED:
		error = tr("7zip decoder doesn't support this archive");
		break;

		case SZ_ERROR_MEM:
		error = tr("Unable to allocate memory");
		break;

		case SZ_ERROR_CRC:
		error = tr("7zip decoder doesn't support this archive");
		break;

		case SZ_ERROR_FAIL:
		// error already defined
		break;

		default:
		error = tr("Error %1").arg(res);
	}

	emit extractFail(error);

	return false;
}

bool CArchive::extractZip()
{
	emit extractPrepare();

	QZipReader reader(m_filename);

	QDir baseDir(m_dest);

	// create directories first
	QList<QZipReader::FileInfo> allFiles = reader.fileInfoList();

	qint64 totalSize = 0, currentSize = 0;

	foreach (const QZipReader::FileInfo &fi, allFiles)
	{
		if (fi.isDir)
		{
			const QString absPath = m_dest + QDir::separator() + fi.filePath;

			if (!baseDir.mkpath(fi.filePath))
			{
				emit extractFail(tr("Unable to create directory %1").arg(absPath));
				return false;
			}

			if (!QFile::setPermissions(absPath, fi.permissions))
			{
				emit extractFail(tr("Unable to set permissions of %1").arg(absPath));
				return false;
			}
		}

		totalSize += fi.size;
	}

	emit extractInit(0, totalSize);
	emit extractStart();

	// client won't use symbolic links so don't process them

	foreach (const QZipReader::FileInfo &fi, allFiles)
	{
		const QString absPath = m_dest + QDir::separator() + fi.filePath;

		if (fi.isFile)
		{
			if (mustStop())
			{
				emit extractStop();
				return true;
			}

			QFile f(absPath);

			if (!f.open(QIODevice::WriteOnly))
			{
				emit extractFail(tr("Unable to open %1").arg(absPath));
				return false;
			}

			currentSize += f.write(reader.fileData(fi.filePath));

			f.setPermissions(fi.permissions);
			f.close();

			emit extractProgress(currentSize, QFileInfo(absPath).fileName());
		}
	}

	emit extractSuccess(totalSize);

	return true;
}

bool CArchive::progress(const std::string &filename, uint32 currentSize, uint32 totalSize)
{
	if (mustStop())
	{
		emit extractStop();
	
		return false;
	}

	if (currentSize == 0)
	{
		emit extractInit(0, (qint64)totalSize);
		emit extractStart();
	}

	emit extractProgress((qint64)currentSize, qFromUtf8(filename));
	
	if (currentSize == totalSize)
	{
		emit extractSuccess((qint64)totalSize);
	}

	return true;
}

bool CArchive::extractBnp()
{
	QString error;

	emit extractPrepare();

	NLMISC::CBigFile::TUnpackProgressCallback cbMethod = NLMISC::CBigFile::TUnpackProgressCallback(this, &CArchive::progress);

	try
	{
		if (NLMISC::CBigFile::unpack(qToUtf8(m_filename), qToUtf8(m_dest), &cbMethod))
		{
			return true;
		}

		if (mustStop())
		{
			// stopped

			return true;
		}

		error.clear();
	}
	catch(const NLMISC::EDiskFullError &e)
	{
		error = tr("disk full");
	}
	catch(const NLMISC::EWriteError &e)
	{
		error = tr("unable to write %1").arg(qFromUtf8(e.Filename));
	}
	catch(const NLMISC::EReadError &e)
	{
		error = tr("unable to read %1").arg(qFromUtf8(e.Filename));
	}
	catch(const std::exception &e)
	{
		error = tr("failed (%1)").arg(qFromUtf8(e.what()));
	}

	emit extractFail(tr("Unable to unpack %1 to %2: %3").arg(m_filename).arg(m_dest).arg(error));

	return false;
}

void CArchive::stop()
{
	QMutexLocker locker(&m_mutex);

	m_mustStop = true;
}

bool CArchive::mustStop()
{
	QMutexLocker locker(&m_mutex);

	return m_mustStop;
}

