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



#ifndef EGS_STATIC_ENCYCLO_H
#define EGS_STATIC_ENCYCLO_H

//Nel georges
#include "nel/georges/u_form.h"

/*
 *	It seems that on server side there are no problem with serialization of SheetId. So we use it.
 *  At the packing time we convert the string inside georges sheet into SheetId.
 */


/**
 * 
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2004
 */
class CStaticEncycloAlbum
{
	friend class CStaticEncyclo;
public:
	/// ctor
	CStaticEncycloAlbum(){}

	/// Read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);
	/// Return the version of this class, increments this value when the content of this class has changed
	inline static uint getVersion () { return 1; }
	/// Serial
	void serial(class NLMISC::IStream &f)
	{
		f.serial( AlbumNumber );
		f.serial( Title );
		f.serial( RewardBrick );
		f.serialCont( Themas );
	}
	/// Removed
	void removed() {}

	/// same as the one used in title (used to classify album read in random order)
	uint32							AlbumNumber;
	/// title something like ENCY_ALB_00 (in phrases\encyclopedia\phrase_ency_alb_00_en.txt for instance)
	std::string						Title;
	/// a brick that can unlock a honorific title for the player
	NLMISC::CSheetId				RewardBrick;

	// Never use the themas list directly (this is not ordered)
private:

	/// a list of themas that composed the album (themas can then be accessed through a common CSheetId ctor)
	std::vector< NLMISC::CSheetId > Themas;
};

/**
 * 
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2004
 */
class CStaticEncycloThema
{
public:
	/// ctor
	CStaticEncycloThema(){}

	/// Read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);
	/// Return the version of this class, increments this value when the content of this class has changed
	inline static uint getVersion () { return 1; }
	/// Serial
	void serial(class NLMISC::IStream &f)
	{
		f.serial( ThemaNumber );
		f.serial( Title );
		f.serial( RewardText );
		f.serial( RewardSheet );
		f.serialCont( Tasks );
		f.serial( Rite );
	}
	/// Removed
	void removed() {}

	/// number of the thema in the album (needed for ordering) (begins at 1 not 0)
	uint32							ThemaNumber;
	/// title something like ENCY_THM_00_01 (in phrases\encyclopedia\phrase_ency_alb_00_en.txt for instance)
	std::string						Title;
	/// something like ENCY_THM_TXT_00_01, text explaining game background
	std::string						RewardText;
	/// a brick, phrase or item that can be a special aura or increase the number of landmarks etc...
	NLMISC::CSheetId				RewardSheet;
	/// all the missions that unlock the rite mission (the name is the 'symbolic name' task, aka id for leveldesign team)
	std::vector<std::string>		Tasks;
	/// this name is also a 'symbolic name'
	std::string						Rite;

};

/**
 * Static part of the encyclopedia stored in the singleton CSheets::_StaticSheets
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2004
 */
class CStaticEncyclo
{
	NL_INSTANCE_COUNTER_DECL(CStaticEncyclo);
public:

	// Init is called after the 2 maps are loaded and is used to create accelerators and thema by numbers
	void init();

	// Get album/thema from the sheet id
	const CStaticEncycloAlbum * getAlbumForm( const NLMISC::CSheetId& sheetId ) const;
	const CStaticEncycloThema * getThemaForm( const NLMISC::CSheetId& sheetId ) const;

	// Get album/thema from index in the structure (given by the AlbumNumber and ThemaNumber in the georges sheets)
	uint32						getNbAlbum () const;
	const CStaticEncycloAlbum * getAlbum ( uint32 nAlbumNb ) const;
	uint32						getNbThema ( uint32 nAlbumNb ) const;
	const CStaticEncycloThema * getThema ( uint32 nAlbumNb, uint32 nThema ) const;

	// Return the position in the static structure of a mission (given its symbolic name)
	// for the nOutTask 0 is the rite 1->7 are tasks prerequesite
	bool isMissionPresent(const std::string &sMissionSymbolicName, sint32 &nOutAlb, sint32 &nOutThm, sint32 &nOutTask) const;

	// get album and thema number, and task count, from rite name
	void getRiteInfos( std::string& rite, uint32& nAlbum, uint32& nThema, uint32& taskCount ) const;

private:
	// These 2 maps are initialized at CSheets::init time
	std::map<NLMISC::CSheetId, CStaticEncycloAlbum>	_AlbumsFromSheet;
	std::map<NLMISC::CSheetId, CStaticEncycloThema>	_ThemasFromSheet;
	friend class CSheets;

	// Accelerator Table
	struct CAlbum
	{
		CStaticEncycloAlbum					*AlbumSheet;
		std::vector<CStaticEncycloThema*>	ThemaSheets;
		// ---------------------------------------------
		CAlbum() 
		{
			AlbumSheet = NULL;
		}
	};
	std::vector<CAlbum>	_OrderedAlbums;
};

#endif // EGS_STATIC_WORLD_H

/* End of egs_static_world.h */

