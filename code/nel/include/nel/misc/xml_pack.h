
#ifndef XML_PACK_H
#define XML_PACK_H

#include "types_nl.h"
#include "string_mapper.h"
#include "sstring.h"

namespace NLMISC
{

/** The xml pack is a data format to store a great number of XML file
 *	in a simple pseudo XML format.
 *	The primary goal was to support the greate number of XML file
 *	that we handle for Ryzom in a convenient and efficient way (mainly
 *	in the CVS point of view).
 *
 *	In a xml pack file, all xml files are concatenated inside
 *	XML elements.
 *	In the header of each subfile, there is only the file name.
 *	So, it is possible to easily edit the file by hand, compare it
 *	and add or remove file.
 *
 *	NB : we use xml pack file as an intermediate format between the
 *	level design team and the runtime data (that are ganarated by
 *	the loadForm function of george) that are a binary extraction of
 *	the content of the xml files.
 */
	class CXMLPack
	{
		NLMISC_SAFE_SINGLETON_DECL(CXMLPack);

		CXMLPack(){}
		~CXMLPack() {}

	public:
		// Add an xml pack to the manager
		bool add (const std::string &xmlPackFileName);

		// List all files in an xml_pack file
		void list (const std::string &xmlPackFileName, std::vector<std::string> &allFiles);

		// Used by CIFile to get information about the files within the xml pack
		FILE* getFile (const std::string &sFileName, uint32 &rFileSize, uint32 &rFileOffset,
						bool &rCacheFileOnOpen, bool &rAlwaysOpened);

	private:

		///@name parser functions
		///@{
		/// Consume space and tab characters (but NOT newlines)
		void skipWS(std::string::iterator &it, std::string::iterator end);
		/** Try to match the specified text at current position. Return false if no match
		 *	return true and advance the it iterator if match is found.
		 */
		bool matchString(std::string::iterator &it, std::string::iterator end, const char *text);
		/// Advance up to the begining of the next line, incrementing the in/out param lineCount
		void skipLine(std::string::iterator &it, std::string::iterator end, uint32 &lineCount);
		///@}


		/// the name of the xml_pack file
		std::string		XMLPackFileName;

		/// A descriptor for one file inside the pack
		struct TXMLFileInfo
		{
			/// The name of the file, mapped in the string mapper
			TStringId		FileName;
			/// The start position of the data for this file
			uint32			FileOffset;
			/// The size of this file
			uint32			FileSize;
		};

		/// A descriptor for the content of an xml pack file
		struct TXMLPackInfo
		{
			typedef std::map<TStringId, TXMLFileInfo>	TFileList;
			/// the content of the xml pack
			TFileList		_XMLFiles;
			/// The xml pack file handler (we keep it open)
//			FILE			*FileHandler;

//			TXMLPackInfo()
//				:	 FileHandler(NULL)
//			{
//			}
//
//
//			~TXMLPackInfo()
//			{
//				if (FileHandler != NULL)
//					fclose(FileHandler);
//			}
		};


		typedef std::map<TStringId, TXMLPackInfo>	TPackList;
		/// The list of xml pack file already parsed
		TPackList				_XMLPacks;
	};

} // namespace NLMISC



#endif // XML_PACK_H

