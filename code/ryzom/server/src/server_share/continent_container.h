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



#ifndef NL_CONTINENT_CONTAINER_H
#define NL_CONTINENT_CONTAINER_H

// Nel Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/vectord.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/log.h"
#include "nel/misc/entity_id.h"

// Nel Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"

// Nel Pacs
#include "nel/pacs/u_move_container.h"
#include "nel/pacs/u_move_primitive.h"
#include "nel/pacs/u_retriever_bank.h"
#include "nel/pacs/u_global_retriever.h"
#include "nel/pacs/u_global_position.h"
#include "nel/pacs/u_primitive_block.h"

/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CContinentContainer
{
protected:
	/// Container of continent
	class CContinentMoveContainer
	{
	public:
		CContinentMoveContainer() : GlobalRetriever(NULL), RetrieverBank(NULL), MoveContainer(NULL), AllowAutoSpawn(false) {}
		/// Then name of the continent
		std::string								Name;
		/// The PACS global retriever for this continent
		NLPACS::UGlobalRetriever				*GlobalRetriever;		// Global retriever retrieved surfaces of all territory
		/// The PACS retriever bank for this continent
		NLPACS::URetrieverBank					*RetrieverBank;			// Bank of global retriever
		/// The PACS move container for this continent
		NLPACS::UMoveContainer					*MoveContainer;			// Move container
		/// True if entities are allowed to try spawning in this continent if no continent index is provided
		bool									AllowAutoSpawn;
	};

	///
	class CSheet
	{
	public:
		CSheet() {}

		std::string					Name;
		std::string					PacsRBank;
		std::string					PacsGR;
		std::string					LandscapeIG;
		std::vector<std::string>	ListIG;

		void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
		{
			Name = sheetId.toString();
			// the form was found so read the true values from George
			form->getRootNode ().getValueByName (PacsRBank, "PacsRBank");
			form->getRootNode ().getValueByName (PacsGR, "PacsGR");
			form->getRootNode ().getValueByName (LandscapeIG, "LandscapeIG");


			NLGEORGES::UFormElm	*node;
			uint				numVillages, numIgs;

			if (form->getRootNode().getNodeByName(&node, "Villages") && node &&
				node->getArraySize(numVillages))
			{
				for (uint village=0; village<numVillages; ++village)
				{
					if (form->getRootNode ().getNodeByName(&node, NLMISC::toString("Villages[%d].IgList", village).c_str()) && node &&
						node->getArraySize(numIgs))
					{
						for (uint ig=0; ig<numIgs; ++ig)
						{
							std::string	igName;
							if (form->getRootNode ().getValueByName(igName, NLMISC::toString("Villages[%d].IgList[%d].IgName", village, ig).c_str()))
								ListIG.push_back(igName);
						}						
					}
				}
			}
			
		}

		void serial (NLMISC::IStream &s)
		{
			s.serial(Name);
			s.serial(PacsRBank, PacsGR, LandscapeIG);
			s.serialCont(ListIG);
		}

		static uint getVersion () { return 1; }
		
		void removed() {}
	};

	/// The global continent container
	typedef std::vector<CContinentMoveContainer>	TContinentContainer;

	///
	typedef std::map<std::string, NLPACS::UPrimitiveBlock*>	TPacsPrimMap;

	///
	typedef std::map<uint, NLMISC::CVectorD>	TTriggerMap;

	/// the continents
	TContinentContainer							_Continents;

	/// pacs primitives
	TPacsPrimMap								_PacsPrimMap;

	/// Sheet map type
	typedef std::map<NLMISC::CSheetId, CSheet>	TSheetMap;

	///
	TSheetMap									_SheetMap;

	///
	TTriggerMap									_TriggerMap;

	///
	uint										_GridWidth;
	uint										_GridHeight;
	double										_CellSize;
	double										_PrimitiveMaxSize;
	uint										_NbWorldImages;

	/// Should load dynamic collision (e.g. trees)
	bool										_LoadPacsPrims;

public:

	/// Constructor
	CContinentContainer();

	/// Init whole continent container
	void	init(uint gridWidth, uint gridHeight, double primitiveMaxSize, uint nbWorldImages, const std::string packedSheetsDirectory, double cellSize=0.0, bool loadPacsPrims = true);

	/// Init pacs prims
	void	initPacsPrim(const std::string &path = std::string("landscape_col_prim_pacs_list.txt"));


	/// Load continent
	void	loadContinent(std::string name, std::string file, sint index, bool allowAutoSpawn = true);

	/// Remove continent
	void	removeContinent(sint index);


	/// Get move container for continent
	NLPACS::UMoveContainer	*getMoveContainer(sint index)
	{
		if (index < 0 || index >= (sint)_Continents.size())
		{
			nlwarning("getMoveContainer(): invalid index %d", index);
			return NULL;
		}
		return _Continents[index].MoveContainer;
	}

	/// Get move container for continent
	NLPACS::UGlobalRetriever	*getRetriever(sint index)
	{
		if (index < 0 || index >= (sint)_Continents.size())
		{
			nlwarning("getRetriever(): invalid index %d", index);
			return NULL;
		}
		return _Continents[index].GlobalRetriever;
	}

	/// Get move container for continent
	NLPACS::URetrieverBank	*getRetrieverBank(sint index)
	{
		if (index < 0 || index >= (sint)_Continents.size())
		{
			nlwarning("getRetriverBank(): invalid index %d", index);
			return NULL;
		}
		return _Continents[index].RetrieverBank;
	}


	/// Find continent to spawn in
	sint	findContinent(const NLMISC::CVectorD &worldPosition, const NLMISC::CEntityId& id = NLMISC::CEntityId::Unknown)
	{
		sint	continent = -1;
		// finds the continent that fits given position
		uint	i;
		for (i=0; i<_Continents.size(); ++i)
		{
			if (_Continents[i].GlobalRetriever == NULL ||
				!_Continents[i].AllowAutoSpawn)
				continue;

			NLPACS::UGlobalPosition		globalPosition = _Continents[i].GlobalRetriever->retrievePosition(worldPosition);
			if (globalPosition.InstanceId != -1 && globalPosition.LocalPosition.Surface != -1)
			{
				if (continent != -1)
				{
					nlwarning("findContinent%s: continent not provided, and sole position (%.3f, %.3f, %.3f) refers to several continents (%d, %d) -- use first found", 
						id.toString().c_str(),
						worldPosition.x, worldPosition.y, worldPosition.z,
						i, continent);
					continue;
				}

				continent = i;
			}
		}

		if (continent == -1)
		{
			if (worldPosition.x != 0.0 || worldPosition.y != 0.0 || worldPosition.z != 0.0)
				nlwarning("findContinent%s: can't find best fitting continent for position (%.3f, %.3f, %.3f) -- use first available continent", id.toString().c_str(), worldPosition.x, worldPosition.y, worldPosition.z);

			for (uint i=0; i<_Continents.size(); ++i)
			{
				if (_Continents[i].MoveContainer != NULL)
				{
					continent = i;
					break;
				}
			}
		}

		return continent;
	}

	/// Get container size
	sint	size() const { return (sint)_Continents.size(); }

	/// clear the container -- WARNING 
	void	clear()
	{ 
		uint	i;
		for (i=0; i<_Continents.size(); ++i)
			removeContinent(i);
		_Continents.clear();
	}

	/// get trigger position
	NLMISC::CVectorD	getTriggerPosition(uint i) const
	{
		TTriggerMap::const_iterator	it = _TriggerMap.find(i);
		return  (it == _TriggerMap.end()) ? NLMISC::CVectorD::Null : (*it).second;
	}

	/// Display triggers
	void				displayTriggers(NLMISC::CLog *log = NLMISC::InfoLog) const
	{
		TTriggerMap::const_iterator	it;
		for (it=_TriggerMap.begin(); it!=_TriggerMap.end(); ++it)
			log->displayNL("Trigger %d: (%.3f, %.3f, %.3f)", (*it).first, (*it).second.x, (*it).second.y, (*it).second.z);
	}

protected:

	void	loadPacsPrims(const CSheet &sheet, NLPACS::UMoveContainer *moveContainer);
};


#endif // NL_CONTINENT_CONTAINER_H

/* End of continent_container.h */
