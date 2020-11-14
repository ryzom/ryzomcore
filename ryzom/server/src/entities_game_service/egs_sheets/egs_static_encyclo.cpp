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
#include "egs_sheets/egs_static_encyclo.h"

#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"

///////////////////////////////////////////////////////////////////////////////////////////

using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;

extern CVariable<bool> EGSLight;

NL_INSTANCE_COUNTER_IMPL(CStaticEncyclo);
///////////////////////////////////////////////////////////////////////////////////////////

void CStaticEncycloAlbum::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	UFormElm &root = form->getRootNode();

	// Read the album number
	nlverify (root.getValueByName (AlbumNumber, "AlbumNumber"));

	// Read the title
	nlverify (root.getValueByName (Title, "Title"));

	// Read the reward brick
	string sRewardBrick;
	RewardBrick = CSheetId::Unknown;
	nlverify (root.getValueByName (sRewardBrick, "RewardBrick"));
	if (!sRewardBrick.empty())
	{
		RewardBrick = CSheetId(sRewardBrick);
		if ( RewardBrick == CSheetId::Unknown )
			nlerror("CStaticEncycloAlbum::RewardBrick sheet '%s' is invalid.", sRewardBrick.c_str() );
	}

	// Read all the themas
	const UFormElm *pElt;
	uint size;
	root.getNodeByName(&pElt, "Themas");
	nlverify (pElt->getArraySize (size));
	string sThmFilename;
	Themas.reserve(size);
	for (uint32 i = 0; i < size; ++i)
	{
		// Get the thema filename
		if (pElt->getArrayValue(sThmFilename, i))
		{
			CSheetId sheet( sThmFilename );
			if ( sheet == CSheetId::Unknown )
				nlerror("CStaticEncycloAlbum::Themas %d of sheet '%s' is invalid. ", i, sThmFilename.c_str() );
			Themas.push_back(sheet);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////

void CStaticEncycloThema::readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
{
	UFormElm &root = form->getRootNode();
	
	// Read the thema number
	nlverify (root.getValueByName (ThemaNumber, "ThemaNumber"));
	
	// Read the title
	nlverify (root.getValueByName (Title, "Title"));

	// Read the reward text
	nlverify (root.getValueByName (RewardText, "RewardText"));
	
	// Read the reward sheet (brick, item, phrase)
	string sRewardSheet;
	RewardSheet = CSheetId::Unknown;
	nlverify (root.getValueByName (sRewardSheet, "RewardSheet"));
	if (!sRewardSheet.empty())
	{
		RewardSheet = CSheetId(sRewardSheet);
		if ( RewardSheet == CSheetId::Unknown )
			nlerror("CStaticEncycloThema::RewardSheet sheet '%s' is invalid.", sRewardSheet.c_str() );
	}

	// Read all the tasks
	const UFormElm *pElt;
	uint size;
	root.getNodeByName(&pElt, "Tasks");
	if (pElt != NULL) // May be null if no task
	{
		nlverify (pElt->getArraySize (size));
		string sTaskSymbolicName;
		Tasks.reserve(size);
		for (uint32 i = 0; i < size; ++i)
			if (pElt->getArrayValue(sTaskSymbolicName, i))
				Tasks.push_back(sTaskSymbolicName);
	}
	
	// Read the rite
	nlverify (root.getValueByName (Rite, "Rite"));
}

///////////////////////////////////////////////////////////////////////////////////////////

void CStaticEncyclo::init()
{
	_OrderedAlbums.clear();

	if (EGSLight)
		return;

	// Get the higher album number
	sint32 nMaxAlbumNb = -1;
	map<CSheetId, CStaticEncycloAlbum>::iterator itAlbumForm = _AlbumsFromSheet.begin();
	while (itAlbumForm != _AlbumsFromSheet.end())
	{
		const CStaticEncycloAlbum &rAlbum = itAlbumForm->second;
		if ((sint32)rAlbum.AlbumNumber > nMaxAlbumNb)
			nMaxAlbumNb = rAlbum.AlbumNumber;
		++itAlbumForm;
	}

	// The size of the album vector is nMaxAlbumNb+1 because album number begins at 0
	// Album number directly reference an album in the _OrderedAlbums vector
	if (nMaxAlbumNb == -1)
		nlerror("CStaticEncyclo::init no album found");
	_OrderedAlbums.resize(nMaxAlbumNb+1);

	// Setup the accelerator table
	itAlbumForm = _AlbumsFromSheet.begin();
	while (itAlbumForm != _AlbumsFromSheet.end())
	{
		const CStaticEncycloAlbum &rStatAlbum = itAlbumForm->second;
		_OrderedAlbums[rStatAlbum.AlbumNumber].AlbumSheet = &(itAlbumForm->second);
		CAlbum &rOrdAlbum = _OrderedAlbums[rStatAlbum.AlbumNumber];

		// Count the number of thema in the album
		sint32 nMaxThemaNb = -1;
		uint32 i;
		for (i = 0; i < rStatAlbum.Themas.size(); ++i)
		{
			const CStaticEncycloThema *pThm = getThemaForm(rStatAlbum.Themas[i]);
			if ((sint32)pThm->ThemaNumber > nMaxThemaNb)
				nMaxThemaNb = pThm->ThemaNumber;
		}

		if (nMaxThemaNb == -1)
			nlerror("CStaticEncyclo::init no thema found");

		// the size of the thema vector is just the same as the max thema number because thema number begins at 1
		rOrdAlbum.ThemaSheets.resize(nMaxThemaNb, NULL);

		for (i = 0; i < rStatAlbum.Themas.size(); ++i)
		{
			map<CSheetId, CStaticEncycloThema>::iterator itThmForm = _ThemasFromSheet.find( rStatAlbum.Themas[i] );
			if( itThmForm == _ThemasFromSheet.end() )
				nlerror( "CStaticEncyclo::init The static form for sheet %s (%d) is unknown", rStatAlbum.Themas[i].toString().c_str(), rStatAlbum.Themas[i].asInt() );
			

			CStaticEncycloThema *pThm = &(itThmForm->second);
			if (pThm->ThemaNumber == 0)
				nlerror("CStaticEncyclo::init no thema number is zero !");
			rOrdAlbum.ThemaSheets[pThm->ThemaNumber - 1] = pThm;
		}
		
		++itAlbumForm;
	}

	// Check integrity !
	for (uint32 i = 0; i < _OrderedAlbums.size(); ++i)
	{
		if (_OrderedAlbums[i].AlbumSheet == NULL)
			nlwarning("CStaticEncyclo::init Albums number not continuous missing album %d", i);
		for (uint32 j = 0; j < _OrderedAlbums[i].ThemaSheets.size(); ++j)
		{
			//if (_OrderedAlbums[i].ThemaSheets[j] == NULL)
				//nlwarning("CStaticEncyclo::init Themas number not continuous missing for album %d thema %d", i, j);
			if ((_OrderedAlbums[i].ThemaSheets[j] != NULL) && 
				(_OrderedAlbums[i].ThemaSheets[j]->Tasks.size() > 7))
				nlwarning("CStaticEncyclo::init Tasks number is too big (%d, max is 7) for album %d thema %d",_OrderedAlbums[i].ThemaSheets[j]->Tasks.size(), i, j);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////

const CStaticEncycloAlbum * CStaticEncyclo::getAlbumForm( const NLMISC::CSheetId& sheetId ) const
{
	map<CSheetId, CStaticEncycloAlbum>::const_iterator itForm = _AlbumsFromSheet.find( sheetId );
	if( itForm == _AlbumsFromSheet.end() )
	{
		nlwarning( "<CStaticEncyclo::getAlbumForm> The static form for sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}

///////////////////////////////////////////////////////////////////////////////////////////

const CStaticEncycloThema * CStaticEncyclo::getThemaForm( const NLMISC::CSheetId& sheetId ) const
{
	map<CSheetId, CStaticEncycloThema>::const_iterator itForm = _ThemasFromSheet.find( sheetId );
	if( itForm == _ThemasFromSheet.end() )
	{
		nlwarning( "<CStaticEncyclo::getThemaForm> The static form for sheet %s (%d) is unknown", sheetId.toString().c_str(), sheetId.asInt() );
		return 0;
	}
	return &(*itForm).second;
}

///////////////////////////////////////////////////////////////////////////////////////////

uint32 CStaticEncyclo::getNbAlbum () const
{
	return (uint32)_OrderedAlbums.size();
}

///////////////////////////////////////////////////////////////////////////////////////////

const CStaticEncycloAlbum * CStaticEncyclo::getAlbum ( uint32 nAlbumNb ) const
{
	nlassert(nAlbumNb < _OrderedAlbums.size());
	return _OrderedAlbums[nAlbumNb].AlbumSheet;
}

///////////////////////////////////////////////////////////////////////////////////////////

uint32 CStaticEncyclo::getNbThema ( uint32 nAlbumNb ) const
{
	nlassert(nAlbumNb < _OrderedAlbums.size());
	return (uint32)_OrderedAlbums[nAlbumNb].ThemaSheets.size();
}

///////////////////////////////////////////////////////////////////////////////////////////

const CStaticEncycloThema * CStaticEncyclo::getThema ( uint32 nAlbumNb, uint32 nThema ) const
{
	nlassert(nAlbumNb < _OrderedAlbums.size());
	nlassert(nThema > 0); // Must start at 1 !
	nThema = nThema - 1;
	nlassert(nThema < _OrderedAlbums[nAlbumNb].ThemaSheets.size());
	return _OrderedAlbums[nAlbumNb].ThemaSheets[nThema];
}

///////////////////////////////////////////////////////////////////////////////////////////

bool CStaticEncyclo::isMissionPresent(const string &sMissionSymbolicName,	sint32 &nOutAlb, 
																			sint32 &nOutThm, 
																			sint32 &nOutTask) const
{
	// This search is linear so its slow. May be optimized.

	nOutAlb = nOutThm = nOutTask = -1;

	for (uint32 i = 0; i < _OrderedAlbums.size(); ++i)
	{
		if (_OrderedAlbums[i].AlbumSheet != NULL)
		{
			for (uint32 j = 0; j < _OrderedAlbums[i].ThemaSheets.size(); ++j)
			{
				CStaticEncycloThema *pThm = _OrderedAlbums[i].ThemaSheets[j];
				if (pThm != NULL)
				{
					if (pThm->Rite == sMissionSymbolicName)
					{
						nOutAlb = i;
						nOutThm = j+1;
						nOutTask = 0;
						return true;
					}
					for (uint32 k = 0; k < pThm->Tasks.size(); ++k)
					{
						if (pThm->Tasks[k] == sMissionSymbolicName)
						{
							nOutAlb = i;
							nOutThm = j+1;
							nOutTask = k+1;
							return true;
						}
					}
				}
			}
		}
	}

	// Not found
	return false;
}



///////////////////////////////////////////////////////////////////////////////////////////

void CStaticEncyclo::getRiteInfos( string& rite, uint32& nAlbum, uint32& nThema, uint32& taskCount ) const
{
	// find thema infos
	CSheetId themaSheet;
	map<CSheetId, CStaticEncycloThema>::const_iterator itTh;
	for( itTh = _ThemasFromSheet.begin(); itTh != _ThemasFromSheet.end(); ++itTh )
	{
		if( rite == (*itTh).second.Rite )
		{
			nThema = (*itTh).second.ThemaNumber;
			taskCount = (uint32)(*itTh).second.Tasks.size();
			themaSheet = (*itTh).first;
			break;
		}
	}
	
	// find album infos
	map<CSheetId, CStaticEncycloAlbum>::const_iterator itAl;
	for( itAl = _AlbumsFromSheet.begin(); itAl != _AlbumsFromSheet.end(); ++itAl )
	{
		uint i;
		for( i=0; i<(*itAl).second.Themas.size(); ++i )
		{
			if( (*itAl).second.Themas[i] == themaSheet )
			{
				nAlbum = (*itAl).second.AlbumNumber;
				break;
			}
		}
	}
}

