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

#include "nel/misc/file.h"
#include "nel/misc/config_file.h"

using namespace std;
using namespace NLMISC;

#ifdef NL_DEBUG
#define INFO nlinfo
#else // NL_DEBUG
#	if defined(NL_COMP_VC) && NL_COMP_VC_VERSION >= 71
#		define INFO __noop
#	else
#		define INFO 0&&
#	endif
#endif // NL_DEBUG

bool readTheFile (const char *filename, vector<char> &fileArray)
{
	// Open the file
	CIFile file;
	if (file.open (filename))
	{
		// Go to the end
		file.seek (0, NLMISC::IStream::end);

		// Get file size
		sint32 size = file.getPos ();

		// Resize the array
		fileArray.resize (size);

		// Go to the begin
		file.seek (0, NLMISC::IStream::begin);

		if (size)
		{
			// Read the file
			NLMISC::IStream *stream = &file;
			stream->serialBuffer ((uint8*)&fileArray[0], size);
		}

		return true;
	}
	else
	{
		// Error, can't open the file
		nlwarning ("Can't open the file %s for reading.", filename);

		return false;
	}
}

inline bool isASCII (uint8 c)
{
	return ( (c>=0x20 /*&& c<=0x7E*/) || (c == 0x09) || (c == 0x0A) || (c == 0x0D) );
}

bool isASCII(vector<char> &fileArray)
{
	// Array size
	uint size = (uint)fileArray.size();

	if (size)
	{
		// Array pointer
		const char *arrayPointer = &fileArray[0];

		// Line count
		uint line = 0;

		for (uint c=0; c<size; c++)
		{
			if (!isASCII (arrayPointer[c]))
			{
				INFO ("Binary character: 0x%02x (line %d)", (uint)(uint8)arrayPointer[c], line);
				return false;
			}
			if (arrayPointer[c] == '\n')
				line++;
		}
	}

	return true;
}

inline bool isStringChar (char c)
{
	return ( ( (c>='a') && (c<='z') ) || ( (c>='A') && (c<='Z') ) || ( (c>='0') && (c<='9') ) || (c=='-') || (c=='_') || (c=='.') );
}

void removeBeginEndSpaces (string &str)
{
	// Remove begin space
	sint index=0;
	while ( (index<(sint)str.length ()) && (str[index]==' ') )
		index++;
	if (index != 0)
		str = str.substr (index, str.length ()-index);

	// Remove end space
	index=(sint)str.length ()-1;
	while ( (index>0) && (str[index]==' ') )
		index--;
	str.resize (index+1);
}

void removeChar (string &str, char ch)
{
	for (uint c=0; c<str.size (); c++)
	{
		if (str[c] == ch)
			str.erase (c, 1);
	}
}

void removeDirectory (string &str)
{
	// Remove begin space
	string::size_type pos = str.rfind ('\\');
	string::size_type pos2 = str.rfind ('/');

	if (pos == string::npos)
		pos = pos2;

	if (pos != string::npos)
		str = str.substr (pos+1, str.size());
}

bool filterExtension (const char *str, const vector<string> &extensions)
{
	// String size
	uint size = (uint)strlen (str);

	// For each filter
	for (uint i=0; i<extensions.size(); i++)
	{
		if ( (str+(size - extensions[i].size())) == extensions[i])
			return true;
	}

	// Not found
	return false;
}

bool validateFilename (const char *str)
{
	// Look for space
	return (strchr (str, ' ') == NULL);
}

const char *getExtension (const char *filename)
{
	return strrchr (filename, '.');
}

void removeExtension (string &filename)
{
	string::size_type index = filename.rfind ('.');
	filename.resize (index);
}

void extractStringsFromBinary (const vector<char> &fileArray, set<string> &filenameStrings, const vector<string> &extensions)
{
	// Array size
	uint size = (uint)fileArray.size();

	// Array pointer
	const char *arrayPointer = &fileArray[0];

	// run through the data buffer looking for plausible looking strings
	uint i,j;
	for (i=0; (j=i+sizeof(uint))<size; i++)
	{
		// if we're pointing at a string the first 4 bytes ar the string length
		uint len=*(uint *)(arrayPointer+i);

		// if the string length could be valid
		if (len>0 && len+i<=size)
		{
			uint k;
			for (k=j; k<len+j && isASCII (arrayPointer[k]);k++);
			if (k==len+j)
			{
				// build a string
				string str;

				// Resize the string
				str.resize (len);

				// Copy the string
				for (k=0; k<len; k++)
					str[k] = arrayPointer[j+k];

				// Remove space
				removeBeginEndSpaces (str);

				// Not empty ?
				if (str != "")
				{
					// Lower case
					str = toLowerAscii (str);

					// Filter extensions
					if (filterExtension (str.c_str(), extensions))
					{
						// Remove directory
						removeDirectory (str);

						// Validate filename
						if (validateFilename (str.c_str()))
						{
							// Add a string
							filenameStrings.insert (str);

							// Information
							INFO ("Binary filename extracted: \"%s\" (0x%08x)", str.c_str(), j);
						}
						else
						{
							INFO ("Invalid filename: \"%s\" (0x%08x)", str.c_str(), j);
						}
					}
				}
			}
		}
	}
}

void extractStringsFromASCII (const vector<char> &fileArray, set<string> &filenameStrings, const vector<string> &extensions)
{
	// Array size
	uint size = (uint)fileArray.size();
	if (size)
	{
		// Array pointer
		const char *arrayPointer = &fileArray[0];

		// Temp string
		string temp;

		// Begin of a valid string
		const char *begin = arrayPointer;
		while ((begin<arrayPointer) && (!isStringChar (*begin)))
			begin++;
		const char *end = begin;
		while (end<arrayPointer+size)
		{
			// End char is ok ?
			if (isStringChar (*end))
			{
				end++;
			}
			else
			{
				// Found a string ?
				if (end != begin)
				{
					// String size
					uint size = (uint)(end-begin);
					temp.resize (size);

					// Copy the string
					for (uint c=0; c<size; c++)
						temp[c] = begin[c];

					// Lower case
					temp = toLowerAscii (temp);

					// Filter extensions
					if (filterExtension (temp.c_str(), extensions))
					{
						// Remove directory
						removeDirectory (temp);

						// Validate filename
						if (validateFilename (temp.c_str()))
						{
							// Add a string
							filenameStrings.insert (temp);

							// Information
							INFO ("ASCII filename extracted: \"%s\"", temp.c_str());
						}
						else
						{
							INFO ("Invalid filename: \"%s\"", temp.c_str());
						}
					}
				}
				end++;
				begin = end;
			}
		}
	}
}

bool loadConfigFiles (const char *ext, const char *input_files, const char *available_files, vector<string> &extensions, set<string> &inputFiles, map<string, string> &availableFiles)
{
	// Get a string
	string temp;
	string temp2;

	// Extensions files
	FILE *file = fopen (ext, "r");
	if (file)
	{
		bool cont = true;
		char name[512];
		while (fgets (name, 512, file))
		{
			// To string and lower
			temp = toLowerAscii (string(name));

			// Remove return
			removeChar (temp, '\n');

			// Valid extension ?
			if (temp.size() && temp[0] == '.')
				// Add the extension
				extensions.push_back (temp);
			else
			{
				nlwarning ("ERROR extension %s must begin with a '.' character.", temp.c_str());
				cont = false;
				break;
			}
		}

		// Close
		fclose (file);
		if (cont)
		{
			// Input files
			file = fopen (input_files, "r");
			if (file)
			{
				char name[512];
				while (fgets (name, 512, file))
				{
					// To string
					temp = name;

					// Remove return
					removeChar (temp, '\n');

					// Add the extension
					inputFiles.insert (temp);
				}

				// Close
				fclose (file);

				// Available files
				file = fopen (available_files, "r");
				if (file)
				{
					char name[512];
					while (fgets (name, 512, file))
					{
						// To lower
						temp = toLowerAscii (string(name));
						temp2 = toLowerAscii (string(name));

						// Remove space
						removeBeginEndSpaces (temp);
						removeBeginEndSpaces (temp2);

						// Remove return
						removeChar (temp, '\n');
						removeChar (temp2, '\n');

						// Remove directory
						removeDirectory (temp);

						// Good extension
						if (filterExtension (temp.c_str (), extensions))
						{
							if (validateFilename (temp.c_str ()))
							{
								// Add the extension
								if (!availableFiles.insert (map<string, string>::value_type (temp, temp2)).second)
								{
									fprintf (stderr, "DUBLING: %s %s\n", temp.c_str (), temp2.c_str());
								}
							}
							else
							{
								fprintf (stderr, "INVALIDE NAME: %s\n", temp.c_str ());
							}
						}
						else
						{
							fprintf (stderr, "INVALIDE EXT: %s\n", temp.c_str ());
						}
					}

					// Close
					fclose (file);

					// Ok
					return true;
				}
				else
				{
					nlwarning ("ERROR can't load available files %s", ext);
				}
			}
			else
			{
				nlwarning ("ERROR can't load input files %s", ext);
			}
		}
	}
	else
	{
		nlwarning ("ERROR can't load extension file %s", ext);
	}

	return false;
}


#define BAR_LENGTH 21

const char *progressbar[BAR_LENGTH]=
{
	"[                    ]",
	"[.                   ]",
	"[..                  ]",
	"[...                 ]",
	"[....                ]",
	"[.....               ]",
	"[......              ]",
	"[.......             ]",
	"[........            ]",
	"[.........           ]",
	"[..........          ]",
	"[...........         ]",
	"[............        ]",
	"[.............       ]",
	"[..............      ]",
	"[...............     ]",
	"[................    ]",
	"[.................   ]",
	"[..................  ]",
	"[................... ]",
	"[....................]"
};

void progress (const char *message, float progress)
{
	// Progress bar
	char msg[512];
	uint	pgId= (uint)(progress*(float)BAR_LENGTH);
	pgId= min(pgId, (uint)(BAR_LENGTH-1));
	sprintf (msg, "\r%s %s", progressbar[pgId], message );
	uint i;
	for (i=(uint)strlen(msg); i<79; i++)
		msg[i]=' ';
	msg[i]=0;
	printf ("%s", msg);
	printf ("\r");
}

int main(int argc, char* argv[])
{
	if (argc==4)
	{
		// Load the config file
		printf ("Loading config files...");

		// Extensions
		vector<string> extensions;

		// Extensions
		map<string, string> alternateExtensions;
		alternateExtensions.insert (map<string, string>::value_type (".tga", ".dds"));

		// String array
		set<string> inputFiles;

		// String array
		set<string> usedFiles;

		// String array
		map<string, string> availableFiles;

		if (loadConfigFiles (argv[1], argv[2], argv[3], extensions, inputFiles, availableFiles))
		{
			// Load the config file
			printf ("\rScaning files...                 \n");

			uint size = 0;
			uint maxSize = (uint)inputFiles.size();
			set<string>::iterator ite = inputFiles.begin();
			while (ite != inputFiles.end())
			{
				// Update
				string label = ite->c_str();

				// Remove directory
				removeDirectory (label);

				progress (label.c_str(), (float)size/(float)maxSize);

				// String array
				set<string> outputFilename;

				// File array
				vector<char> fileArray;

				// Read the file
				INFO ("Open file: %s", ite->c_str());
				if (readTheFile (ite->c_str(), fileArray))
				{
					// Is it a ASCII file ?
					if (isASCII (fileArray))
					{
						INFO ("ASCII mode");

						// Extract strings
						extractStringsFromASCII (fileArray, outputFilename, extensions);
					}
					else
					{
						INFO ("BINARY mode");

						// Extract strings
						extractStringsFromBinary (fileArray, outputFilename, extensions);
					}
				}

				// Check this files exists
				set<string>::iterator found = outputFilename.begin();
				while (found != outputFilename.end())
				{
					// Look for this file
					if (availableFiles.find (*found) == availableFiles.end())
					{
						// Found ?
						bool foundIt = false;

						// Try alternate extension
						string ext = getExtension ((*found).c_str());
						std::map<string, string>::iterator alternIte = alternateExtensions.find (ext);
						if (alternIte != alternateExtensions.end ())
						{
							string name = *found;
							removeExtension (name);
							name += alternIte->second;
							if (availableFiles.find (name) != availableFiles.end())
								foundIt = true;
						}

						// Not found ?
						if (!foundIt)
						{
							fprintf (stderr, "MISSING: %s (needed by file %s)\n", found->c_str(), ite->c_str());
						}
					}
					else
					{
						// Add to used files
						usedFiles.insert (*found);
					}

					// Next file found
					found++;
				}

				size++;

				// Next file
				ite++;
			}

			// Look for unused files
			map<string, string>::iterator available = availableFiles.begin();
			while (available != availableFiles.end())
			{
				// It is used ?
				if (usedFiles.find (available->first) == usedFiles.end())
				{
					string temp = toLowerAscii (available->second);
					fprintf (stderr, "UNUSED: %s\n", temp.c_str());
				}

				// Next
				available++;
			}

			// Ok
			return 1;
		}
	}
	else
	{
		printf ("extract_filename [extensions.txt] [inputFiles.txt] [availableFiles.txt]\n");
	}
	return 0;
}
