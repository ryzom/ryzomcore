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

#ifndef UTILS_H_
#define UTILS_H_


// DtName must be the 1st one
enum TDataCol { DtName, DtTitle, DtRMFamily, DtGroup, DtEcosystem, DtLevelZone, DtStatQuality, DtProp, DtCreature, DtCreaTitle, DtCraftSlotName, DtCraftCivSpec, DtColor, DtAverageEnergy, DtMaxLevel, DtCustomizedProperties, DtNbCols };
const char *DataColStr [DtNbCols] = { "Code", "Name", "Family", "Group", "Ecosystem", "LevelZone", "Stat Quality", "Properties", "Creature sheets", "Creatures", "Item parts", "Craft civ spec", "Color", "Average energy", "Max level", "Customized" };



/*
 * Multi-indexed array.
 * NC is the number of columns.
 */
template <uint32 NC>
class CSortableData
{
public:

	/// A row is made of fields, usually 1 per column but there may be more than one (each one is a key)
	struct TSortableItem
	{
		std::vector<std::string>		Fields [NC];

		///
		void			push( uint32 column, const std::string& f, bool allowDuplicates=false )
		{
			if ( (allowDuplicates) || (find( Fields[column].begin(), Fields[column].end(), f ) == Fields[column].end()) )
				Fields[column].push_back( f );
		}

		///
		void reset( uint32 column )
		{
			Fields[ column ].clear();
		}

		/**
		 * Display the item as a row of a HTML table.
		 * If (key!=previousKey) and (name==previousName), the row will not be displayed entirely to save space
		 *
		 * \param keyColumn If not std::numeric_limits<uint32>::max(), column used for sorting => this column displays only the field matching the key
		 * \param key The key used for sorting (see keyColumn)
		 * \param previousKey Previous key
		 * \param nameColumn If not std::numeric_limits<uint32>::max(), column used for the unique name (column must have exaclty one element)
		 * \param previousName Previous name
		 */
		std::string		toHTMLRow( uint32 keyColumn=std::numeric_limits<uint32>::max(), const string& key=string(), const string& previousKey=string(),
								   uint32 nameColumn=std::numeric_limits<uint32>::max(), const string& previousName=string() ) const
		{
			std::string s = "<tr>";
			bool lightMode = (nameColumn == std::numeric_limits<uint32>::max()) ? false : ((key != previousKey) && (Fields[nameColumn][0] == previousName));
			for ( uint32 c=0; c!=NC; ++c )
			{
				s += "<td>";
				if ( c == keyColumn )
					s += key; // key should be a substr of toString( c )
				else
				{
					if ( lightMode )
						s += "\"";
					else
						s += columnToString( c );
				}
				s += "</td>";
			}
			s += "</tr>\n";
			return s;
		}


		///
		std::string		toCSVLine( char columnSeparator=',', string internalSeparator=" - ", uint32 keyColumn=std::numeric_limits<uint32>::max(), const string& key=string(), const string& previousKey=string(),
								   uint32 nameColumn=std::numeric_limits<uint32>::max(), const string& previousName=string()  ) const
		{
			std::string s;
			bool lightMode = (nameColumn == std::numeric_limits<uint32>::max()) ? false : ((key != previousKey) && (Fields[nameColumn][0] == previousName));
			for ( uint32 c=0; c!=NC; ++c )
			{
				if ( c == keyColumn )
					s += key; // key should be a substr of columnToString( c )
				else
				{
					if ( lightMode )
						s += "\"";
					else
						s += columnToString( c, internalSeparator );
				}
				s += columnSeparator;
			}
			s += "\n";
			return s;
		}

		///
		std::string		columnToString( uint32 column, const std::string& internalSeparator=", " ) const
		{
			std::string s;
			std::vector<std::string>::const_iterator ivs;
			for ( ivs=Fields[column].begin(); ivs!=Fields[column].end(); ++ivs )
			{
				if ( ivs!=Fields[column].begin() )
					s += internalSeparator;
				s += (*ivs);
			}
			return s;
		}
	};

	typedef std::multimap< std::string, uint32 > CLookup; // key to index (not pt because reallocation invalidates pointers)
	typedef std::vector< TSortableItem > CItems;

	/// Init
	void	init( bool enabled )
	{
		_Enabled = enabled;
	}

	/// Add a row
	void	addItem( const TSortableItem& item )
	{
		if ( ! _Enabled )
			return;

		_Items.push_back( item );
		for ( uint32 c=0; c!=NC; ++c )
		{
			for ( std::vector<std::string>::const_iterator ik=item.Fields[c].begin(); ik!=item.Fields[c].end(); ++ik )
			{
				_Indices[c].insert( make_pair( *ik, (uint32)_Items.size()-1 ) );
			}
		}
	}

	/**
	 * Update a row (found by the first column, which must have exactly one element).
	 * Returns true if it existed before, false if it's being created.
	 * If it existed before:
	 * - Does not remove elements that already exist and are not in the new item
	 * - Adds the new elements found in the new item at the specified columns, and updates lookup map
	 */
	bool	updateItemAppend( const TSortableItem& item, uint32 column )
	{
		if ( ! _Enabled )
			return true; // quiet

		uint32 nameColumn = 0;
		CLookup::iterator ilk = _Indices[nameColumn].find( item.Fields[nameColumn][0] );
		if ( ilk != _Indices[nameColumn].end() )
		{
			uint32& index = (*ilk).second;

			// Update map for the specified column
			// and update item column
			for ( std::vector<std::string>::const_iterator ivs=item.Fields[column].begin(); ivs!=item.Fields[column].end(); ++ivs )
			{
				ilk = _Indices[column].find( *ivs );
				if ( (ilk == _Indices[column].end()) || (  getRow( (*ilk).second ).Fields[nameColumn][0] != item.Fields[nameColumn][0]) )
				{
					_Indices[column].insert( make_pair( *ivs, index ) );
					_Items[index].Fields[column].push_back( *ivs );
				}
			}

			return true;
		}
		else
		{
			addItem( item );
			return false;
		}
	}

	/**
	 * Update a row (found by the first column, which must have exactly one element).
	 * Returns true if it existed before, false if it's being created.
	 * If it existed before:
	 * - Does not update lookup maps or item for columns that were already present.
	 * - Adds entries in lookup maps and updates item for new columns (fields that were empty).
	 */
	/*bool	updateItemAppend( const TSortableItem& item )
	{
		if ( ! _Enabled )
			return true; // quiet

		CLookup::iterator ilk = _Indices[0].find( item.Fields[0][0] );
		if ( ilk != _Indices[0].end() )
		{
			uint32& index = (*ilk).second;

			for ( uint32 c=1; c!=NC; ++c )
			{
				// Update maps for previously empty columns
				if ( _Items[index].Fields[c].empty() )
				{
					for ( std::vector<std::string>::iterator ivs=item.Fields[c].begin(); ivs!=item.Fields[c].end(); ++ivs )
						_Indices[c].insert( make_pair( *ivs, index ) );
				}

				// Update item column
				_Items[index].Fields[c] = item.Fields[c];
			}

			return true;
		}
		else
		{
			addItem( item );
			return false;
		}
	}*/

	/// Find or browse by key
	CLookup& lookup( uint32 column )
	{
		return _Indices[column];
	}

	/// Browse by adding order
	CItems& items()
	{
		return _Items;
	}

	/// Get a row by index
	TSortableItem&	getRow( uint32 index )
	{
		return _Items[index];
	}

private:

	CLookup							_Indices [NC];

	CItems							_Items;

	bool							_Enabled;
};


typedef CSortableData<DtNbCols> CRMData;
typedef CRMData::TSortableItem TRMItem;


/**
 *
 */
class CProducedDocHtml
{
public:

	///
	CProducedDocHtml() : _File(NULL), _Enabled(false) {}

	///
	void	open( const std::string& filename, const std::string& title, bool enableFlag )
	{
		_Enabled = enableFlag;
		if ( ! _Enabled )
			return;

		_File = fopen( filename.c_str(), "wt" );
		if(!_File)
		{
			throw Exception("Could not open html: %s", filename.c_str());
		}
		fprintf( _File, "<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n<title>%s</title>\n</head><body>\n", title.c_str() );
	}

	///
	void	write( const std::string& htmlCode )
	{
		if ( ! _Enabled )
			return;

		fprintf( _File, "%s", htmlCode.c_str() );
	}

	///
	void	writeln( const std::string& htmlCode )
	{
		write( htmlCode + "\n" );
	}

	///
	void	writebln( const std::string& htmlCode )
	{
		write( htmlCode + "<br>\n" );
	}

	///
	void	writepln( const std::string& htmlCode )
	{
		write( "<p>" + htmlCode + "</p>\n" );
	}

	///
	void	save()
	{
		if ( ! _Enabled )
			return;

		fprintf( _File, "</body></html>\n" );
		fclose( _File );
	}

private:

	FILE	*_File;
	bool	_Enabled;
};


#endif