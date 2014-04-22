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

#include "cst_loader.h"

using namespace std;
using namespace NLMISC;

/****************************************************************\
						buildTableFormat()
\****************************************************************/
void CSTLoader::buildTableFormat( string fileName, list<pair<string,TDataType> >& tableFormat )
{
	_File = new ifstream(fileName.c_str(), ios::in);

	if( !_File->is_open() )
	{
		nlerror("can't open file : %s\n", fileName.c_str());
	}


	// read first line
	//================
	char readBuffer[4096];
	char * token;
	_File->getline(readBuffer, 4096);

	// extract first token
	//====================
	token = strtok(readBuffer, _Seps.c_str());

	// extract type from token
	string stoken = string( token );
	string::size_type typeStart = stoken.find_last_of("<");
	nlassert(typeStart != string::npos);
	string::size_type typeEnd = stoken.find_first_of(">",typeStart);
	nlassert(typeEnd != string::npos);
	string type_str = stoken.substr( typeStart+1, (typeEnd-typeStart-1) );
	TDataType type_enum = convertType(type_str);

	// extract column label from token
	string toConvert(stoken.substr(0,typeStart));
	string label = convertName( toConvert );
//	string label = stoken.substr(0,typeStart);

	// insert pair (label,type)
	tableFormat.push_back( make_pair( label, type_enum ) );


	// extract next tokens
	//====================
	while( token != NULL )
	{
		token = strtok(NULL, _Seps.c_str());
		if(token != NULL)
		{
			stoken = string( token );

			// extract type from token
			typeStart = stoken.find_last_of("<");
			nlassert(typeStart != string::npos);
			typeEnd = stoken.find_first_of(">",typeStart);
			nlassert(typeEnd != string::npos);
			type_str = stoken.substr( typeStart+1, (typeEnd-typeStart-1) );
			type_enum = convertType(type_str);

			// extract column label from token
			string toConvert(stoken.substr(0,typeStart));
			label = convertName( toConvert );
//			label = stoken.substr(0,typeStart);

			// insert pair (label,type)
			tableFormat.push_back( make_pair( label, type_enum ) );
		}
	}
}


/****************************************************************\
							readData()
\****************************************************************/
void CSTLoader::readData( list<list<string> >& data )
{
	char readBuffer[4096];
	char * token;
	bool firstToken = true;

	while( !_File->eof() )
	{
		// list of current object values
		list<string> lineData;

		// read a line
		_File->getline(readBuffer, 4096);

		// check all tokens of the current line
		do
		{
			if( firstToken )
			{
				// extract first token
				token = strtok(readBuffer, _Seps.c_str());
				firstToken = false;
			}
			else
			{
				// extract next token
				token = strtok(NULL, _Seps.c_str());
			}
			if( token != NULL )
			{
				string stoken( token );
				lineData.push_back( stoken );
			}
		}
		while( token != NULL );

		firstToken = true;

		if( lineData.size() )
		{
			data.push_back( lineData );
		}
	}
}



/****************************************************************\
					generateDerivedClasses()
\****************************************************************/
void CSTLoader::generateDerivedClasses(ofstream &file, std::list< std::pair<std::string, TDataType> > &format, std::list< std::list< std::string> > &data )
{


	std::list< std::list< std::string> >::iterator it_dl = data.begin();

	while ( it_dl != data.end() )
	{
		std::list< std::pair<std::string, TDataType> >::iterator it_def = format.begin();
		std::list<std::string>::iterator it_val = (*it_dl).begin();
//		sint32 size = data.size();
//		sint32 size2 = (*it_dl).size();

//			std::string name = convertName( *it_val );

//			std::string test = *it_val;

		if ( (*it_dl).size() )
		{
			file << "From Item : Define " << convertName( *it_val ) << endl;
			it_val++;
			it_def++;
			file << "{" << endl;
			file << "\tComponent:" << endl;
		}

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
			file << "<'" << (*it_obj).first << "', Static>;" << endl;
			it_obj++;
		}

		file << "\tEnd" << endl << endl;

		file << "\t StaticInit()" << endl;
		while ( it_def != format.end() && it_val != (*it_dl).end() )
		{

#ifdef NL_DEBUG
			std::string test1 = *it_val;
			std::string test2 = (*it_def).first;
#endif

			file << "\t\t" << (*it_def).first << " = ";
			switch ( (*it_def).second )
			{
				case UINT8:
					file << "new uint8(" << convertName(*it_val);
					break;
				case SINT8:
					file << "new sint8(" << convertName(*it_val);
					break;
				case UINT16:
					file << "new uint16(" << convertName(*it_val);
					break;
				case SINT16:
					file << "new sint16(" << convertName(*it_val);
					break;
				case UINT32:
					file << "new uint32(" << convertName(*it_val);
					break;
				case SINT32:
					file << "new sint32(" << convertName(*it_val);
					break;
				case FLOAT:
					file << "new Float(" <<convertName(*it_val);
					break;
				case STRING:
					file << "'" << (*it_val) << "'";
					break;
				case BOOL:
					file << "new Bool(" << (*it_val);
					break;
				default:
					file << "ERROR: unsuported type " << (*it_def).second << std::endl;
					break;
			}
			file << ");" << endl;

			it_def++;
			it_val++;
		}
		file << "\tEnd" << endl;
		file << "}" << endl;

		it_dl++;
	}
}




/****************************************************************\
							init()
\****************************************************************/
void CSTLoader::init(string fileName, const map<string,TDataType>& fileFormat)
{
	_FileFormat = fileFormat;

	_FileName = fileName;

	_File = new ifstream(fileName.c_str(), ios::in);

	if( !_File->is_open() )
	{
		nlerror("can't open file : %s\n", fileName.c_str());
	}


	// read first line
	char readBuffer[4096];
	char * token;
	_File->getline(readBuffer, 4096);

	// extract first token
	token = strtok(readBuffer, _Seps.c_str());
	_Columns.push_back( string(token) );

	// extract next tokens
	while( token != NULL )
	{
		token = strtok(NULL, _Seps.c_str());
		if(token != NULL)
		{
			_Columns.push_back( string(token) );
		}
	}
}




/****************************************************************\
							readLine()
\****************************************************************/
bool CSTLoader::readLine()
{
	if( _File->eof() )
	{
		return false;
	}

	const uint nbColumn = (uint)_Columns.size();
	char readBuffer[4096];
	char * token;
	bool firstToken = true;
	uint j = 0;
	string columnLabel;
	map<string,TDataType>::iterator itf;

	// erase all previous values
	_Tokens.clear();

	// read a line
	_File->getline(readBuffer, 4096);

	// if the line is empty we consider we are at end of file
	if( strlen(readBuffer) == 0)
	{
		//nlinfo("CSTLoader: empty line in file '%s'", _FileName.c_str());
		return false;
	}

	// check all tokens of the current line
	do
	{
		if( firstToken )
		{
			// extract first token
			token = strtok(readBuffer, _Seps.c_str());
			firstToken = false;
		}
		else
		{
			// extract next token
			token = strtok(NULL, _Seps.c_str());
		}
		if( token != NULL )
		{
			// label of token column
			nlassert( j < nbColumn);
			columnLabel = _Columns[j];

			// look for column data type
			itf = _FileFormat.find( columnLabel );
			if( itf != _FileFormat.end() )
			{
				// check data type validity
				switch( (*itf).second )
				{
					case UINT8 :
					case SINT8 :
					case UINT16 :
					case SINT16 :
					case UINT32 :
					case SINT32 :
						{
							sint32 tmp;
							fromString((const char*)token, tmp);
							if( tmp == 0 )
							{
								if( token[0] != '0' )
								{
									nlerror("Data type error at column %s, at line %d : not an integer",columnLabel.c_str(),_LineCount);
								}
							}
						}
						break;
					case FLOAT :
						{
							double tmp;
							fromString((const char*)token, tmp);
							if( tmp == 0.0 )
							{
								if( token[0] != '0' )
								{
									nlerror("Data type error at column %s, at line %d : not a float",columnLabel.c_str(),_LineCount);
								}
							}
						}
						break;
					case BOOL :
						if( strcmp(token,"0") &&  strcmp(token,"1") )
						{
							nlerror("Data type error at column %s, at line %d : not a bool",columnLabel.c_str(),_LineCount);
						}
						break;
					default:
						break;
				}
				// we keep the pair <column,token>
				_Tokens.erase(columnLabel); // do not forget to delete the old value (insert does not assure proper remplacement)

				_Tokens.insert( make_pair(columnLabel,string(token)) );
			}
			// if this column is not expected by the user we go on to next token
			else
			{
				nlinfo("%s : this colum is not in the file format",columnLabel.c_str());
			}
		}
		++j;
	}
	while( token != NULL );

	++_LineCount;

	return true;
}

