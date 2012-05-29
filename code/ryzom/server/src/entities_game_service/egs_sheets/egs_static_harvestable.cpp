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

#include "egs_sheets/egs_static_harvestable.h"
#include "egs_sheets/egs_sheets.h"
#include "../egs_variables.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;

const uint8 NbRawMaterials = 10;


const float QuarteringForcedQuantities [6] = { 0, 1.0f, 2.0f, 3.0f, 4.0f, 0.5f };

const float *QuarteringQuantityByVariable [NBRMQuantityVariables] =
{
	&QuarteringQuantityAverageForCraftHerbivore.get(),
	&QuarteringQuantityAverageForCraftCarnivore.get(),
	&QuarteringQuantityAverageForBoss5.get(),
	&QuarteringQuantityAverageForBoss7.get(),
	&QuarteringQuantityForInvasion5.get(),
	&QuarteringQuantityForInvasion7.get(),
	&QuarteringForcedQuantities[0],
	&QuarteringForcedQuantities[1],
	&QuarteringForcedQuantities[2],
	&QuarteringForcedQuantities[3],
	&QuarteringForcedQuantities[4],
	&QuarteringForcedQuantities[5]
};


CVariable<bool> VerboseQuartering( "egs", "VerboseQuartering", "", false, 0, true );

//--------------------------------------------------------------
//						Constructor
//--------------------------------------------------------------
CStaticHarvestable::CStaticHarvestable()
{
	_HarvestSkill = SKILLS::unknown;
} 


//--------------------------------------------------------------
//						Destructor
//--------------------------------------------------------------
CStaticHarvestable::~CStaticHarvestable()
{
} 



static	void	getVarListFromParents	(const NLGEORGES::UForm	*form, std::set<NLGEORGES::UFormElm*>	&varList,	const std::string	&varName)
{
	uint32	nbParents=form->getNumParent ();
	while	(nbParents>0)
	{
		nbParents--;
		const NLGEORGES::UForm	*parentForm=form->getParentForm (nbParents);
		if (parentForm)
		{
			getVarListFromParents	(parentForm, varList,	varName);
		}
		const NLGEORGES::UFormElm &item=form->getRootNode();

		NLGEORGES::UFormElm	*elem=NULL;
		const_cast<NLGEORGES::UFormElm*>(&item)->getNodeByName(&elem, varName.c_str());
		if	(elem!=NULL)	//	item.getValueByName(temp, varName.c_str()))
		{
			varList.insert	(elem);
		}
		
	}

}

//--------------------------------------------------------------
//						loadFromGeorges()  
//--------------------------------------------------------------
void CStaticHarvestable::loadFromGeorges( const UForm &form, const NLMISC::CSheetId &sheetId )
{	
	_Mps.clear();
/*
//	getVarListFromParents	(&form, arrayList,	"Harvest");

	std::set<NLGEORGES::UFormElm*>	arrayList;
	{
		NLGEORGES::UFormElm	*tmpelem=NULL;
		const_cast<NLGEORGES::UFormElm*>(&form.getRootNode())->getNodeByName(&tmpelem, "Harvest");
		if (tmpelem)
		{
			arrayList.insert(tmpelem);
		}
	}

	if (arrayList.empty())
	{
		return;
	}

	for	(std::set<NLGEORGES::UFormElm*>::iterator	it=arrayList.begin(), itEnd=arrayList.end(); it!=itEnd;++it)
	{
		NLGEORGES::UFormElm	*elem=*it;
*/
		{
			string value;
			if	(	form.getRootNode().getValueByName( value, (string("Harvest.")+"Skill").c_str() )
				&&	!value.empty()	)
			{
				_HarvestSkill = SKILLS::toSkill( value );
			}

		}

		// in Georges sheet MP starts with MP1 and ends with MP10
		if (VerboseQuartering)
			nldebug("QRTR: Loading RMs of creatures %s", sheetId.toString().c_str());
		for (uint i = 1 ; i <= NbRawMaterials ; ++i)
		{
			const string mpName=NLMISC::toString("MP%u",i);
			CStaticCreatureRawMaterial	mp;
			
			if( form.getRootNode().getValueByName( mp.MpCommon.AssociatedItemName, ("Harvest." +mpName+".AssociatedItem").c_str())
				&& !mp.MpCommon.AssociatedItemName.empty() )
			{
				if (VerboseQuartering)
					nldebug("QRTR: %s=%s", mpName.c_str(), mp.MpCommon.AssociatedItemName.c_str());

				form.getRootNode().getValueByName( mp.MpCommon.Name,		("Harvest." +mpName+".Name").c_str()			);
				uint16 sheetQuantity;
				form.getRootNode().getValueByName( sheetQuantity,			("Harvest." +mpName+".Quantity").c_str()		);
				if ( sheetQuantity != 0 )
				{
					nlwarning( "Quantity set to %hu in %s", sheetQuantity, sheetId.toString().c_str() );
				}
				form.getRootNode().getValueByName( mp.MpCommon.MinQuality,	("Harvest." +mpName+".MinQuality").c_str()		);
				form.getRootNode().getValueByName( mp.MpCommon.MaxQuality,	("Harvest." +mpName+".MaxQuality").c_str()		);
	//			harvest->getValueByName( mp.PresenceProbabilities,	(mpName+".PresenceProbabilities").c_str()	);

				mp.ItemId = mp.MpCommon.AssociatedItemName;
				if ( mp.MpCommon.MinQuality == 0)
					mp.MpCommon.MinQuality = 1;
				if ( mp.MpCommon.MaxQuality == 0)
					mp.MpCommon.MaxQuality = 5;
				/// end temp hack

				const CStaticItem *staticItem = CSheets::getForm( mp.ItemId );
				if ( staticItem )
				{
					string itemSheetCode = mp.ItemId.toString();

					// Identify the usage of the RM (if it is a raw material for mission or craft) or fixed quantity (for invasion reward)
					if ( ( itemSheetCode.find( "ixx" ) != string::npos) ||
						 ( itemSheetCode.find( "cxx" ) != string::npos ) )
					{
						if (VerboseQuartering)
							nldebug("QRTR: For invasions");

						if( itemSheetCode == "m0077ixxcc01.sitem" )
						{
							mp.UsageAndQuantity.Usage = (uint16)RMUTotalQuantity;
							mp.UsageAndQuantity.QuantityVar = (TRMQuantityVariable)(RMQVForceBase+5);
						}
						else
						{
							// Invasion/goo raw material: fixed quantity, depending on creature local level
							//nldebug( "%s/%s: fixed quantity", sheetId.toString().c_str(), itemSheetCode.c_str() );
							mp.UsageAndQuantity.Usage = (uint16)RMUFixedQuantity;
							string creatureCode = sheetId.toString();
							uint16 creatureLocalLevel;
							form.getRootNode().getValueByName( creatureLocalLevel, "Basics.LocalCode" );
							if ( itemSheetCode.size() >= 12 )
							{
								uint itemVariant = (uint)(itemSheetCode[11] - '0');
								switch ( itemVariant )
								{
								case 1:
									if ( creatureLocalLevel < 5 )
										mp.UsageAndQuantity.QuantityVar = (TRMQuantityVariable)(RMQVForceBase+(uint)creatureLocalLevel) ;
									else if ( creatureLocalLevel < 7 )
										mp.UsageAndQuantity.QuantityVar = RMQVInvasion5;
									else if ( creatureLocalLevel < 9 )
										mp.UsageAndQuantity.QuantityVar = RMQVInvasion7;
									else
									{
										nlwarning( "Invalid creature local level in %s", creatureCode.c_str() );
										mp.UsageAndQuantity.QuantityVar = RMQVForceBase; // 0
									}
									break;
								case 2:
									if ( creatureLocalLevel < 5 )
										mp.UsageAndQuantity.QuantityVar = RMQVForceBase; // 0
									else if ( creatureLocalLevel < 7 )
										mp.UsageAndQuantity.QuantityVar = (TRMQuantityVariable)(RMQVForceBase+1) ;
									else if ( creatureLocalLevel < 9 )
										mp.UsageAndQuantity.QuantityVar = (TRMQuantityVariable)(RMQVForceBase+3) ;
									else
									{
										nlwarning( "Invalid creature local level in %s", creatureCode.c_str() );
										mp.UsageAndQuantity.QuantityVar = RMQVForceBase; // 0
									}
									break;
								default:;
								}
							}
						}
						//nldebug( "Creature: %s, RM: %s, Quantity: %hu", creatureCode.c_str(), itemSheetCode.c_str(), mp.UsageAndQuantity.Quantity );

						// Add item only if it has not a 0 fixed-quantity
						if ( mp.quantityVariable() >= NBRMQuantityVariables )
							nlwarning( "Invalid quantity variable" );
						else if ( mp.quantityVariable() != RMQVForceBase )
							_Mps.push_back( mp );
						else
						{
							if (VerboseQuartering)
								nldebug("QRTR: Qtty=0, not added");
						}

					}
					else if ( staticItem->Mp && (! staticItem->Mp->MpFaberParameters.empty()) )
					{
						if (VerboseQuartering)
							nldebug("QRTR: For craft");

						// Raw material for craft
						mp.UsageAndQuantity.Usage = (uint16)RMUTotalQuantity;
						uint16 creatureLocalLevel;
						form.getRootNode().getValueByName( creatureLocalLevel, "Basics.LocalCode" );
						if ( creatureLocalLevel < 5 )
						{
							// Identify the type of creature
							if ( (itemSheetCode.size() > 7) &&
								((itemSheetCode[6]=='h') || (itemSheetCode[6]=='b') || (itemSheetCode[6]=='p')) )
							{
								mp.UsageAndQuantity.QuantityVar = RMQVHerbivore;
							}
							else
							{
								mp.UsageAndQuantity.QuantityVar = RMQVCarnivore;
							}
						}
						else
						{
							switch ( creatureLocalLevel )
							{
							case 5:
							case 8:
								mp.UsageAndQuantity.QuantityVar = RMQVBoss5;
								break;
							case 7:
								mp.UsageAndQuantity.QuantityVar = RMQVBoss7;
								break;
							default: // 6 and non-nomenclatured sheets
								mp.UsageAndQuantity.QuantityVar = RMQVForceBase;
							}
						}

						// Add item only if it has not a 0 fixed-quantity
						if ( mp.quantityVariable() >= NBRMQuantityVariables )
							nlwarning( "Invalid quantity variable" );
						else if ( mp.quantityVariable() != RMQVForceBase )
							_Mps.push_back( mp );
						else
						{
							if (VerboseQuartering)
								nldebug("QRTR: Qtty=0, not added");
						}
					}
					else
					{
						if (VerboseQuartering)
							nldebug("QRTR: For missions");

						// Raw material or item for mission
						_ItemsForMissions.push_back( mp.ItemId );
					}
				}
				else
				{
					// Cancel adding if item not found
					if (VerboseQuartering)
						nldebug("QRTR: %s not found", mp.MpCommon.AssociatedItemName.c_str());
				}
			}

		}

//	}

} // loadFromGeorges //

