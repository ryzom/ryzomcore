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

#ifndef NL_PATH_H
#define NL_PATH_H

#include "types_nl.h"
#include "time_nl.h"
#include "common.h"
#include "string_mapper.h"

#include <map>
#include <string>
#include <vector>

namespace NLMISC {

/// Exception throw when a find is not found in a lookup() call
struct EPathNotFound : public Exception
{
	EPathNotFound (const std::string& filename) : Exception ("Path not found for " + filename) { }
};

/** Utility to store a pre-built list of file, bnp and xml_pack
 *	Used by CPath to store the default application patch.
 *	Can be used by user to build a custom set of file.
 * \warning addSearchPath(), clearMap() and remapExtension() are not reentrant.
 * \warning all path and files are *case sensitive* on linux.
 */
class CFileContainer
{
	// no copy allowed
	CFileContainer(const CFileContainer &/* other */)
	{}

	CFileContainer &operator =(const CFileContainer &/* other */)
	{
		return *this;
	}

public:
	CFileContainer()
	{
		_MemoryCompressed = false;
		_AllFileNames = NULL;
	}

	~CFileContainer();


	void			addSearchPath (const std::string &path, bool recurse, bool alternative, class IProgressCallback *progressCallBack = NULL);

	/** Used only for compatibility with the old CPath. In this case, we don't use the map to have the same behavior as the old CPath */
	void			addSearchPath (const std::string &path) { addSearchPath (path, false, true, NULL); }

	/** Same as AddSearchPath but with a file "c:/autoexec.bat" this file only will included. wildwards *doesn't* work */
	void			addSearchFile (const std::string &file, bool remap = false, const std::string &virtual_ext = "", class NLMISC::IProgressCallback *progressCallBack = NULL);

	/** Same as AddSearchPath but with a path file "c:/test.pth" all files name contain in this file will be included (the extention is used to know that it's a path file) */
	void			addSearchListFile (const std::string &filename, bool recurse, bool alternative);

	/** Same as AddSearchPath but with a big file "c:/test.nbf" all files name contained in the big file will be included  (the extention (Nel Big File) is used to know that it's a big file) */
	void			addSearchBigFile (const std::string &filename, bool recurse, bool alternative, class NLMISC::IProgressCallback *progressCallBack = NULL);

	/** Same as AddSearchPath but with a xml pack file "c:/test.xml_pack" all files name contained in the xml pack will be included   */
	void			addSearchXmlpackFile (const std::string &sXmlpackFilename, bool recurse, bool alternative, class NLMISC::IProgressCallback *progressCallBack = NULL);

	/** Remove all search path contains in the alternative directories */
	void			removeAllAlternativeSearchPath ();

	// Remove a set of big file from the search paths (and also from CBigFile)
	void			removeBigFiles(const std::vector<std::string> &bnpFilenames);

	/** Returns the long name (path + filename) for the specified file.
	 * The directory separator is always '/'.
	 * First, the lookup() lookups in standard directories (Alternative=false).
	 * If not found, it lookups in the Alternative directories.
	 * If not found the lookup() returns empty string "" (and generate an exception if throwException is true)
	 *
	 * The filename is not case sensitive so if the real filename is "FooBAR.Jpg" and you call lookup("fOOBar.jPg"), it'll
	 * return the real filename "FooBAR.Jpg"
	 *
	 * \param filename the file name you are seeking. (ex: "test.txt")
	 * \param throwException used for backward compatibility, set to true to generate an EPathNotFound.
	 * \param displayWarning set to false if you don't want the function displays a warning if the file is not found
	 * \param lookupInLocalDirectory if true, the lookup() will first try to open the file without path.
	 * \return empty string if file is not found or the full path + file name (ex: "c:/temp/test.txt");
	 *
	 * ***********************************************
	 *	WARNING: This Method is NOT thread safe
	 *	user must ensure that no mutator is called on CPath while async loading
	 * ***********************************************
	 *
	 */
	std::string	lookup (const std::string &filename, bool throwException = true, bool displayWarning = true, bool lookupInLocalDirectory = true);

	/** Return if a file is present in the lookup map.
	 * The function changes filename into lower case and removes ended spaces before searching.
	 *
	 * \warning This function checks *only* in the map, not in local dir or alternative dir
	 *
	 * \param filename the file name you are seeking. (ex: "test.txt")
	 * \param lookupInLocalDirectory if true, the lookup() will first try to open the file without path.
	 * \return true if the filename exists in the map used by lookup to know where the file is, false otherwise
	 */
	bool			exists (const std::string &filename);


	/** Clears the map that contains all cached files (Use this function to take into account new files).
	 */
	void clearMap ();

	/** Add a remapping function to allow file extension substitution.
	 * - eg remapExtension("dds", "tga", true) Where the boolean indicates whether
	 * the "dds" should replace a "tga" if one exists - again - a warning should
	 * be generated if the two are present.
	 *
	 * ie: If you have a file called pic.dds and you call remapExtension("dds", "tga", true),
	 *     if you call lookup("pic.tga"), it'll return "pic.dds"
	 *
	 */
	void remapExtension (const std::string &ext1, const std::string &ext2, bool substitute);

	/** Add file remapping
	 * ie: If you have a file called pic.dds, and call remapFile("picture.dds", "pic.dds")
	 * calling lookup("picture.dds") will in fact call lookup("pic.dds")
	 */
	void remapFile (const std::string &file1, const std::string &file2);

	/** Load a file containing the remapped file (you must have done addsearchpath, this method use lookup)
	 * Format is remapped_name_file, real_file
	 * separators are , and \n
	 */
	void loadRemappedFiles (const std::string &file);

	void display ();

	/**	Take a path and put it in the portable format and add a terminated / if needed
	 * ie: "C:\\Game/dir1" will become "C:/Game/dir1/" or "C:/Game/dir1" if addFinalSlash is false
	 */
	std::string	standardizePath (const std::string &path, bool addFinalSlash = true);

	/**	Replace / with \ for dos process. Use only this function if can't do another way.
	 * For example, if you do a system("copy data/toto data/tata"); it'll not work because dos doesn't
	 * understand /.
	 * But in the majority of case, / working (it works for fopen for example)
	 */
	std::string	standardizeDosPath (const std::string &path);


	/** List all files in a directory.
	 *	\param path path where files are scanned. No-op if empty
	 *	\param recurse true if want to recurs directories
	 *	\param wantDir true if want to add directories in result
	 *	\param wantFile true if want to add files in result
	 *	\param result list of string where directories/files names are added.
	 *  \param progressCallBack is a progression callback interface pointer.
	 *  \param showEverything false skips *.log files and CVS directories
	 */
	void			getPathContent (const std::string &path, bool recurse, bool wantDir, bool wantFile, std::vector<std::string> &result, class IProgressCallback *progressCallBack = NULL, bool showEverything=false);

	/** Get the full path based on a file/path and the current directory. Example, imagine that the current path is c:\temp and toto is a directory
	 * getFullPath ("toto") returns "c:/temp/toto/"
	 * getFullPath ("../toto") returns "c:/temp/../toto/"
	 * getFullPath ("d:\dir\toto") returns "d:/dir/toto/"
	 * getFullPath ("\toto") returns "c:/toto/"
	 * getFullPath ("") returns "c:/temp/"
	 *
	 * \param path the path
	 * \return the full path
	 */
	std::string getFullPath (const std::string &path, bool addFinalSlash = true);

	/** Returns the current path of the application.
	 */
	std::string getCurrentPath ();

	/** Set the current path of the application.
	 */
	bool setCurrentPath (const std::string &path);

	/** Create a list of file having the requested extension.
	 */
	void getFileList(const std::string &extension, std::vector<std::string> &filenames);

	/** Create a list of file having the requested string in the filename and the requested extension.
	 */
	void getFileListByName(const std::string &extension, const std::string &name, std::vector<std::string> &filenames);

	/** Create a list of file having the requested string in the path and the requested extension.
	*/
	void getFileListByPath(const std::string &extension, const std::string &path, std::vector<std::string> &filenames);

	/** Make a path relative to another if possible, else doesn't change it.
	 * \param basePath is the base path to be relative to.
	 * \param relativePath is the path to make relative to basePath.
	 * return true if relativePath as been done relative to basePath, false is relativePath has not been changed.
	 */
	bool makePathRelative (const char *basePath, std::string &relativePath);

	/** If File in this list is added more than one in an addSearchPath, it doesn't launch a warning.
	 */
	void addIgnoredDoubleFile(const std::string &ignoredFile);

	/** For the moment after memoryCompress you cant addsearchpath anymore
	*/
	void memoryCompress();

	void memoryUncompress();

	bool isMemoryCompressed()	{ return _MemoryCompressed; }

	/** Get the ms windows directory (in standardized way with end slash), or returns an empty string on other os
	*/
	std::string getWindowsDirectory();

	/** Get application directory.
	* \return directory where applications should write files.
	*/
	std::string getApplicationDirectory(const std::string &appName = "");

	/** Get a temporary directory.
	* \return temporary directory where applications should write files.
	*/
	std::string getTemporaryDirectory();

private:

	// All path in this vector must have a terminated '/'
	std::vector<std::string> _AlternativePaths;

	std::vector<std::string> IgnoredFiles;

	std::map<std::string, std::string> _RemappedFiles;

	// ----------------------------------------------
	// MEMORY WISE
	// ----------------------------------------------

	bool _MemoryCompressed;
	CStaticStringMapper	SSMext;
	CStaticStringMapper	SSMpath;

	// If NOT memory compressed use this
	// ---------------------------------

	struct CFileEntry
	{
		std::string	Name;		// Normal case
		uint32	idPath	 : 16;
		uint32	idExt	 : 15;
		uint32	Remapped : 1;
	};

	typedef std::map<std::string, CFileEntry>	TFiles;
	TFiles	 _Files; // first is the filename in lowercase (can be with a remapped extension)



	// If memory compressed use this
	// -----------------------------

	struct CMCFileEntry
	{
		char *Name;				// Normal case (the search is done by using nlstricmp)
		uint32	idPath	 : 16;	// Path (not with file at the end) - look in the SSMpath (65536 different path allowed)
		uint32	idExt	 : 15;	// real extension of the file if remapped - look in the SSMext (32768 different extension allowed)
		uint32	Remapped : 1;	// true if the file is remapped
	};

	char *_AllFileNames;

	// first is the filename that can be with a remapped extension
	std::vector<CMCFileEntry> _MCFiles;

	// Compare a MCFileEntry with a lowered string (useful for MCfind)
	class CMCFileComp
	{
	public:
		sint specialCompare(const CMCFileEntry &fe, const char *rhs)
		{
			char *lhs = fe.Name;

			uint8 lchar, rchar;
			while (*lhs != '\0' && *rhs != '\0')
			{
				// lower case compare because name is in normal case
				lchar = uint8(::tolower(*lhs));
				rchar = uint8(::tolower(*rhs));
				if (lchar != rchar) return ((sint)lchar) - ((sint)rchar);
				++lhs;
				++rhs;
			}
			if (*lhs != 0) return 1;
			if (*rhs != 0) return -1;
			return 0;
		}

		bool operator()( const CMCFileEntry &fe, const CMCFileEntry &rhs )
		{
			return specialCompare( fe, rhs.Name ) < 0;
		}
	};

	/// first ext1, second ext2 (ext1 could replace ext2)
	std::vector<std::pair<std::string, std::string> > _Extensions;

	CMCFileEntry	*MCfind (const std::string &filename);
	sint			findExtension (const std::string &ext1, const std::string &ext2);
	void			insertFileInMap (const std::string &filename, const std::string &filepath, bool remap, const std::string &extension);
};


/**
 * Utility class for searching files in different paths.
 *
 * Change in jun 2007 : now the implementation code is in CFileContainer, the
 * CPath class is just a wrapper class that contains one instance of CFileContainer.
 *
 * \warning addSearchPath(), clearMap() and remapExtension() are not reentrant.
 * \warning all path and files are *case sensitive* on linux.
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CPath
{
	NLMISC_SAFE_SINGLETON_DECL_PTR(CPath);
public:
	/** Adds a search path.
     * The path is a directory "c:/temp" all files in the directory will be included (and recursively if asked)
	 *
	 * Alternative directories are not pre-cached (instead of non Alternative files) and will be used when a file is not found in the standard directories.
	 * For example, local data will be in the cached directories and server repository files will be in the Alternative files. If a new file is not
	 * found in the local data, we'll try to find it on the repository.
	 *
	 * When Alternative is false, all added file names must be unique or a warning will be display. In the Alternative directories, it could have
	 * more than one file with the same name.
	 *
	 * \warning the path you provide is case sensitive, you must be sure that the path name is exactly the same
	 *
	 * \param path the path name.k The separator for directories could be '/' or '\' (bit '\' will be translate into '/' in the function).
	 * \param recurse true if you want the function recurse in sub-directories.
	 * \param Alternative true if you want to add the path in the Alternative directories.
	 * \param progressCallBack is a progression callback interface pointer.
	 */
	static void			addSearchPath (const std::string &path, bool recurse, bool alternative, class IProgressCallback *progressCallBack = NULL);

	/** Used only for compatibility with the old CPath. In this case, we don't use the map to have the same behavior as the old CPath */
	static void			addSearchPath (const std::string &path) { addSearchPath (path, false, true, NULL); }

	/** Same as AddSearchPath but with a file "c:/autoexec.bat" this file only will included. wildwards *doesn't* work */
	static void			addSearchFile (const std::string &file, bool remap = false, const std::string &virtual_ext = "", class NLMISC::IProgressCallback *progressCallBack = NULL);

	/** Same as AddSearchPath but with a path file "c:/test.pth" all files name contain in this file will be included (the extention is used to know that it's a path file) */
	static void			addSearchListFile (const std::string &filename, bool recurse, bool alternative);

	/** Same as AddSearchPath but with a big file "c:/test.nbf" all files name contained in the big file will be included  (the extention (Nel Big File) is used to know that it's a big file) */
	static void			addSearchBigFile (const std::string &filename, bool recurse, bool alternative, class NLMISC::IProgressCallback *progressCallBack = NULL);

	/** Same as AddSearchPath but with a xml pack file "c:/test.xml_pack" all files name contained in the xml pack will be included   */
	static void			addSearchXmlpackFile (const std::string &sXmlpackFilename, bool recurse, bool alternative, class NLMISC::IProgressCallback *progressCallBack = NULL);

	/** Remove all search path contains in the alternative directories */
	static void			removeAllAlternativeSearchPath ();

	// Remove a set of big file from the search paths (and also from CBigFile)
	static void			removeBigFiles(const std::vector<std::string> &bnpFilenames);

	/** Returns the long name (path + filename) for the specified file.
	 * The directory separator is always '/'.
	 * First, the lookup() lookups in standard directories (Alternative=false).
	 * If not found, it lookups in the Alternative directories.
	 * If not found the lookup() returns empty string "" (and generate an exception if throwException is true)
	 *
	 * The filename is not case sensitive so if the real filename is "FooBAR.Jpg" and you call lookup("fOOBar.jPg"), it'll
	 * return the real filename "FooBAR.Jpg"
	 *
	 * \param filename the file name you are seeking. (ex: "test.txt")
	 * \param throwException used for backward compatibility, set to true to generate an EPathNotFound.
	 * \param displayWarning set to false if you don't want the function displays a warning if the file is not found
	 * \param lookupInLocalDirectory if true, the lookup() will first try to open the file without path.
	 * \return empty string if file is not found or the full path + file name (ex: "c:/temp/test.txt");
	 *
	 * ***********************************************
	 *	WARNING: This Method is NOT thread safe
	 *	user must ensure that no mutator is called on CPath while async loading
	 * ***********************************************
	 *
	 */
	static std::string	lookup (const std::string &filename, bool throwException = true, bool displayWarning = true, bool lookupInLocalDirectory = true);

	/** Return if a file is present in the lookup map.
	 * The function changes filename into lower case and removes ended spaces before searching.
	 *
	 * \warning This function checks *only* in the map, not in local dir or alternative dir
	 *
	 * \param filename the file name you are seeking. (ex: "test.txt")
	 * \param lookupInLocalDirectory if true, the lookup() will first try to open the file without path.
	 * \return true if the filename exists in the map used by lookup to know where the file is, false otherwise
	 */
	static bool			exists (const std::string &filename);


	/** Clears the map that contains all cached files (Use this function to take into account new files).
	 */
	static void clearMap ();

	/** Add a remapping function to allow file extension substitution.
	 * - eg remapExtension("dds", "tga", true) Where the boolean indicates whether
	 * the "dds" should replace a "tga" if one exists - again - a warning should
	 * be generated if the two are present.
	 *
	 * ie: If you have a file called pic.dds and you call remapExtension("dds", "tga", true),
	 *     if you call lookup("pic.tga"), it'll return "pic.dds"
	 *
	 */
	static void remapExtension (const std::string &ext1, const std::string &ext2, bool substitute);

	/** Add file remapping
	 * ie: If you have a file called pic.dds, and call remapFile("picture.dds", "pic.dds")
	 * calling lookup("picture.dds") will in fact call lookup("pic.dds")
	 */
	static void remapFile (const std::string &file1, const std::string &file2);

	/** Load a file containing the remapped file (you must have done addsearchpath, this method use lookup)
	 * Format is remapped_name_file, real_file
	 * separators are , and \n
	 */
	static void loadRemappedFiles (const std::string &file);

	static void display ();

	/**	Take a path and put it in the portable format and add a terminated / if needed
	 * ie: "C:\\Game/dir1" will become "C:/Game/dir1/" or "C:/Game/dir1" if addFinalSlash is false
	 */
	static std::string	standardizePath (const std::string &path, bool addFinalSlash = true);

	/**	Replace / with \ for dos process. Use only this function if can't do another way.
	 * For example, if you do a system("copy data/toto data/tata"); it'll not work because dos doesn't
	 * understand /.
	 * But in the majority of case, / working (it works for fopen for example)
	 */
	static std::string	standardizeDosPath (const std::string &path);


	/** List all files in a directory.
	 *	\param path path where files are scanned. No-op if empty
	 *	\param recurse true if want to recurs directories
	 *	\param wantDir true if want to add directories in result
	 *	\param wantFile true if want to add files in result
	 *	\param result list of string where directories/files names are added.
	 *  \param progressCallBack is a progression callback interface pointer.
	 *  \param showEverything false skips *.log files and CVS directories
	 */
	static void			getPathContent (const std::string &path, bool recurse, bool wantDir, bool wantFile, std::vector<std::string> &result, class IProgressCallback *progressCallBack = NULL, bool showEverything=false);

	/** Get the full path based on a file/path and the current directory. Example, imagine that the current path is c:\temp and toto is a directory
	 * getFullPath ("toto") returns "c:/temp/toto/"
	 * getFullPath ("../toto") returns "c:/temp/../toto/"
	 * getFullPath ("d:\dir\toto") returns "d:/dir/toto/"
	 * getFullPath ("\toto") returns "c:/toto/"
	 * getFullPath ("") returns "c:/temp/"
	 *
	 * \param path the path
	 * \return the full path
	 */
	static std::string getFullPath (const std::string &path, bool addFinalSlash = true);

	/** Returns the current path of the application.
	 */
	static std::string getCurrentPath ();

	/** Set the current path of the application.
	 */
	static bool setCurrentPath (const std::string &path);

	/** Create a list of file having the requested extension.
	 */
	static void getFileList(const std::string &extension, std::vector<std::string> &filenames);

	/** Create a list of file having the requested string in the filename and the requested extension.
	 */
	static void getFileListByName(const std::string &extension, const std::string &name, std::vector<std::string> &filenames);

	/** Create a list of file having the requested string in the path and the requested extension
	*/
	static void getFileListByPath(const std::string &extension, const std::string &path, std::vector<std::string> &filenames);

	/** Make a path relative to another if possible, else doesn't change it.
	 * \param basePath is the base path to be relative to.
	 * \param relativePath is the path to make relative to basePath.
	 * return true if relativePath as been done relative to basePath, false is relativePath has not been changed.
	 */
	static bool makePathRelative (const char *basePath, std::string &relativePath);

	/** If File in this list is added more than one in an addSearchPath, it doesn't launch a warning.
	 */
	static void addIgnoredDoubleFile(const std::string &ignoredFile);

	/** For the moment after memoryCompress you cant addsearchpath anymore
	*/
	static void memoryCompress();

	static void memoryUncompress();

	static bool isMemoryCompressed()	{ return getInstance()->_FileContainer.isMemoryCompressed(); }

	/** Get the ms windows directory (in standardized way with end slash), or returns an empty string on other os
	*/
	static std::string getWindowsDirectory();

	/** Get application directory.
	* \return directory where applications should write files.
	*/
	static std::string getApplicationDirectory(const std::string &appName = "");

	/** Get a temporary directory.
	* \return temporary directory where applications should write files.
	*/
	static std::string getTemporaryDirectory();

	// release singleton
	static void releaseInstance();

private:

	CPath()
	{
	}

	/// The container used by the standard CPath
	CFileContainer		_FileContainer;
};



/**
 * Utility class for file manipulation
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
struct CFile
{
	/**
	 * Retrieve the associated file name.
	 * An empty string is returned if the path is invalid
	 */
	static std::string getFilename (const std::string &filename);

	/**
	 * Retrieve the associated file path with the trailing slash.
	 * Returns an empty string if the path is invalid
	 */
	static std::string getPath (const std::string &filename);

	/**
	 * Just to know if it is a directory.
	 * _FileName empty and path not !!!
	 */
	static bool isDirectory (const std::string &filename);

	/**
	 * Return true if the file exists.
	 * Warning: this test will also tell that the file does not
	 * exist if you don't have the rights to read it (Unix).
	 */
	static bool fileExists (const std::string &filename);

	/**
	 * Return true if the file OR directory exists.
	 * Warning: this test will also tell that the file does not
	 * exist if you don't have the rights to read it (Unix).
	 */
	static bool isExists (const std::string& filename);

	/**
	 * Create an empty file.
	 * Return true if the file has been correctly created.
	 */
	static bool createEmptyFile (const std::string& filename);

	/**
	 * Return a new filename that doesn't exist. It's used for screenshot filename for example.
	 * example: findNewFile("foobar.tga");
	 * will try foobar001.tga, if the file exists, try foobar002.tga and so on until it finds an unexistant file.
	 */
	static std::string findNewFile (const std::string &filename);

	/**
	 * Return the position between [begin,end[ of the last separator between path and filename ('/' or '\').
	 * If there's no separator, it returns string::npos.
	 */
	static int getLastSeparator (const std::string &filename);

	static std::string getFilenameWithoutExtension (const std::string &filename);
	static std::string getExtension (const std::string &filename);

	/**
	 * Return the size of the file (in bytes).
	 *
	 * You have to provide the full path of the file (the function doesn't lookup)
	 */
	static uint32	getFileSize (const std::string &filename);

	/**
	 * Return the size of the file (in bytes).
	 */
	static uint32	getFileSize (FILE *f);

	/**
	 * Return Time of last modification of file. 0 if not found.
	 *
	 * You have to provide the full path of the file (the function doesn't lookup)
	 * The time is measured in second since 01-01-1970 0:0:0 UTC
	 */
	static uint32	getFileModificationDate(const std::string &filename);

	/**
	 * Set the time of last modification of file.
	 *
	 * You have to provide the full path of the file (the function doesn't lookup)
	 * The time is measured in second since 01-01-1970 0:0:0 UTC
	 * Return 'true' if the file date has been changed or false in case of error.
	 */
	static bool		setFileModificationDate(const std::string &filename, uint32 modTime);

	/**
	 * Return creation Time of the file. 0 if not found.
	 *
	 * You have to provide the full path of the file (the function doesn't lookup)
	 */
	static uint32	getFileCreationDate(const std::string &filename);

	/**
	 * Add a callback that will be call when the content file, named filename, changed.
	 * The system use the file modification date. To work, you need to call evenly the
	 * function checkFileChange(), this function only checks every 1s by default (you can
	 * change the default time)
	 *
	 * ie:
	 * void cb (const std::string &filename) { nlinfo ("the file %s changed", filename.c_str()); }
	 * CFile::addFileChangeCallback ("myfile.txt", cb);
	 *
	 */
	static void addFileChangeCallback (const std::string &filename, void (*)(const std::string &filename));

	/**
	 * Remove a file that was previously added by addFileChangeCallback
	 */
	static void removeFileChangeCallback (const std::string &filename);

	/**
	 * You have to call this function evenly (each frame for example) to enable the file change callback system.
	 * If the file not exists and is created in the run time, the callback will be called.
	 * If the file exists and is removed in the run time, the callback will be called.
	 *
	 * \param frequency the time in millisecond that we wait before check another time (1s by default).
	 */
	static void checkFileChange (TTime frequency = 1000);

	/** Copy a file
	  * NB this keeps file attributes
	  * \param failIfExists If the destination file exists, nothing is done, and it returns false.
	  * \return true if the copy succeeded
	  */
	static bool copyFile(const std::string &dest, const std::string &src, bool failIfExists = false, class IProgressCallback *progress = NULL);

	/** Compare 2 files
	  * \return true if both files exist and the files have same timestamp and size
	  */
	static bool quickFileCompare(const std::string &fileName0, const std::string &fileName1);

	/** Compare 2 files
	  * \param maxBufSize fixes max memory space to use for the pair buffers used for data comparison (eg 16 would allow 8 bytes per buffer for the 2 buffers)
	  * \return true if both files exist and the files have same contents (timestamp is ignored)
	  */
	static bool thoroughFileCompare(const std::string &fileName0, const std::string &fileName1,uint32 maxBufSize=1024*1024*2);

	/** Move a file
	  * NB this keeps file attributes
	  */
	static bool moveFile(const char *dest, const char *src);

	/** Create a directory
	  *	\return true if success
	  */
	static bool	createDirectory(const std::string &dirname);

	/** Create a directory and any missing parent directories
	  *	\return true if success
	  */
	static bool	createDirectoryTree(const std::string &dirname);

	/** Try to set the file access to read/write if not already set.
	 * return true if the file doesn't exist or if the file already have RW access.
	 * Work actually only on Windows and returns always true on other platforms.
	 * \return true if RW access is granted
	 */
	static bool	setRWAccess(const std::string &filename);

	/** Delete a file if possible (change the write access if possible)
	* \return true if the delete occurs.
	*/
	static bool deleteFile(const std::string &filename);

	/** Delete a directory if possible (change the write access if possible)
	* \return true if the delete occurs.
	*/
	static bool deleteDirectory(const std::string &filename);

	/** Get temporary output filename.
	*	Call this method to get a temporary output filename. If you have successfully saved your data, delete the old filename and move the new one.
	*/
	static void getTemporaryOutputFilename (const std::string &originalFilename, std::string &tempFilename);
};

} // NLMISC

#endif // NL_PATH_H

/* End of path.h */
