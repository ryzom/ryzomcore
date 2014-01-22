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



#ifndef CST_LOADER_H
#define CST_LOADER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"

#include <map>
#include <vector>
#include <string>
#include <fstream>


/**
 * CSTLoader
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CSTLoader
{
public:

	/// data type
	enum TDataType
	{
		UINT8,
		SINT8,
		UINT16,
		SINT16,
		UINT32,
		SINT32,
		FLOAT,
		STRING,
		BOOL
	};

private:

	/// cst file
	std::ifstream * _File;

	/// name of the cst file (used for debug information)
	std::string		_FileName;

	/// separators
	std::string _Seps;

	/// current line in the file
	uint _LineCount;

	/// columns label of the actually readen data file
	std::vector<std::string> _Columns;

	/// file format expected
	std::map<std::string,TDataType> _FileFormat;

	/// tokens for each expected columns
	std::map<std::string, std::string> _Tokens;

public:

	/// constructor
	CSTLoader()
	{
		_Seps = "\t;";
		_LineCount = 0;
	}

	/**
	 * Set the separators to extract tokens
	 * \param seps all characters than can be a separator
	 */
	void setSeparators( std::string seps )
	{
		_Seps = seps;
	}


	/**
	 * (For files respecting the dynamical column label format : name<type>)
	 * Open a file, and init the file format. Each column has a label and a data type
	 *
	 * \param fileName the name of the file
	 * \param fileFormat the name of the columns and their data type
	 */
	void buildTableFormat( std::string fileName, std::list<std::pair< std::string,TDataType> >& tableFormat );


	/**
	 * (For files respecting the dynamical column label format : name<type>)
	 * Read all the data of the file. For each object it fills a list of value(string)
	 *
	 * \param data the list of the name of the columns and their data type
	 */
	void readData( std::list<std::list<std::string> >& data );

	/**
	 * Open a file, and init the file format
	 * \param fileName the name of the file
	 * \param fileFormat the name of the columns and their data type
	 */
	void init( std::string fileName, const std::map<std::string,TDataType>& fileFormat);


	/**
	 * read a line in the file and check validity of token data type
	 * \return false if the ned of file has been reached, true else.
	 */
	bool readLine();


	/**
	 * return the value corresponding to the given column in the current file line
	 * \param value  a value in the table
	 */
	template<class T> void getValue( const std::string& columnLabel, T& value )
	{
		std::map<std::string,std::string>::const_iterator itt = _Tokens.find( columnLabel );
		if( itt == _Tokens.end() )
		{
			nlerror("The value for the column %s has not been read in the line",columnLabel.c_str());
		}

		std::map<std::string,TDataType>::const_iterator itf = _FileFormat.find( columnLabel );
		if( itf == _FileFormat.end() )
		{
			nlerror("The column %s is not in the file format",columnLabel.c_str());
		}

		nlassert( (*itf).second != STRING && (*itf).second != BOOL );

		switch( (*itf).second )
		{
			case UINT8 :
			case SINT8 :
			case UINT16 :
			case SINT16 :
			case UINT32 :
			case SINT32 :
			case FLOAT :
				fromString((*itt).second, value);
				break;
			default:
				break;
		}
	}

	/**
	 * return the string value corresponding to the given column in the current file line
	 * \param value a string value in the table
	 */
	void getStringValue( const std::string& columnLabel, std::string& value )
	{
		std::map<std::string,std::string>::const_iterator itt = _Tokens.find( columnLabel );
		if( itt == _Tokens.end() )
		{
			nlerror("The value for the column %s has not been read in the line",columnLabel.c_str());
		}

		std::map<std::string,TDataType>::const_iterator itf = _FileFormat.find( columnLabel );
		if( itf == _FileFormat.end() )
		{
			nlerror("The column %s is not in the file format",columnLabel.c_str());
		}

		nlassert( (*itf).second == STRING );

		value = (*itt).second;
	}

	/**
	 * return the bool value corresponding to the given column in the current file line
	 * \param value a bool value in the table
	 */
	void getBoolValue( const std::string& columnLabel, bool& value )
	{
		std::map<std::string,std::string>::const_iterator itt = _Tokens.find( columnLabel );
		if( itt == _Tokens.end() )
		{
			nlerror("The value for the column %s has not been read in the line",columnLabel.c_str());
		}

		std::map<std::string,TDataType>::const_iterator itf = _FileFormat.find( columnLabel );
		if( itf == _FileFormat.end() )
		{
			nlerror("The column %s is not in the file format",columnLabel.c_str());
		}

		nlassert( (*itf).second == BOOL );

		NLMISC::fromString((*itt).second, value);
	}


	/// close file
	void close()
	{
		_File->close();
		delete _File;
	}


	void Load(std::string fileName,std::ofstream &script_file)
	{
		// Generates the base class
		std::list< std::pair<std::string,TDataType> > format;
		buildTableFormat( fileName, format );
		generateBaseClass( script_file, format);

		// Generates a derived class for each type of object
		std::list< std::list<std::string> > data;
		readData( data );
		generateDerivedClasses( script_file, format, data );
	}

	void generateBaseClass(std::ofstream &file, std::list< std::pair<std::string,TDataType> > &/* format */)
	{
		file << "From Agent : Define Item" << std::endl;
		file << "{" << std::endl;
/*		file << "\tComponent:" << std::endl;

		std::list< std::pair<std::string,TDataType> >::iterator it_obj = format.begin();
		it_obj++;
		while ( it_obj != format.end() )
		{
			file << "\t\t";
			switch ( (*it_obj).second )
			{
				case UINT8:
					file << "uint8";
					break;
				case SINT8:
					file << "sint8";
					break;
				case UINT16:
					file << "uint16";
					break;
				case SINT16:
					file << "sint16";
					break;
				case UINT32:
					file << "uint32";
					break;
				case SINT32:
					file << "sint32";
					break;
				case FLOAT:
					file << "Float";
					break;
				case STRING:
					file << "String";
					break;
				case BOOL:
					file << "Bool";
					break;
			}
			file << "<'" << (*it_obj).first << "', Static>;" << std::endl;
			it_obj++;
		}

		file << "\tEnd" << std::endl;*/
		file << "}" << std::endl;
		file << std::endl;
	}

	void generateDerivedClasses(std::ofstream &, std::list< std::pair<std::string, TDataType> > &, std::list< std::list< std::string> > &);

	TDataType convertType(std::string type_str)
	{
		if ( type_str == "UINT8")
			return UINT8;
		if ( type_str == "SINT8")
			return SINT8;
		if ( type_str == "UINT16")
			return UINT16;
		if ( type_str == "SINT16")
			return SINT16;
		if ( type_str == "UINT32")
			return UINT32;
		if ( type_str == "SINT32")
			return SINT32;
		if ( type_str == "FLOAT")
			return FLOAT;
		if ( type_str == "STRING")
			return STRING;
		if ( type_str == "BOOL")
			return BOOL;
		return (TDataType)0;
	}

	std::string convertName(std::string &name)
	{
		int i = 0;
		char buffer[1024];
		std::string::iterator it_c = name.begin();
		while ( it_c != name.end() )
		{
			char c = *it_c;
			switch ( c )
			{
				case ' ':
				case '.':
					buffer[i] = '_';
					break;

				case ',':
					buffer[i] = '.';
					break;

				default:
					buffer[i] = *it_c;
			}
			i++;
			it_c++;
		}
		buffer[i] = 0;
		return std::string( buffer );
	}
};

#endif // CST_LOADER_H
