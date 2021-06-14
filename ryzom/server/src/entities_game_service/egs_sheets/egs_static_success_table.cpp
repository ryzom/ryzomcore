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
#include "egs_static_success_table.h"
#include "egs_sheets.h"
//Nel georges
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"
//
#include "nel/misc/string_conversion.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;

namespace SUCCESS_TABLE_TYPE
{
	// The conversion table
	NL_BEGIN_STRING_CONVERSION_TABLE (TSuccessTableType)
	NL_STRING_CONVERSION_TABLE_ENTRY (FightPhrase)
	NL_STRING_CONVERSION_TABLE_ENTRY (FightDefense)
	NL_STRING_CONVERSION_TABLE_ENTRY (FightDefenseAI)
	NL_STRING_CONVERSION_TABLE_ENTRY (ShieldUse)
	NL_STRING_CONVERSION_TABLE_ENTRY (OffensiveMagicCast)
	NL_STRING_CONVERSION_TABLE_ENTRY (CurativeMagicCast)
	NL_STRING_CONVERSION_TABLE_ENTRY (MagicResistDirect)
	NL_STRING_CONVERSION_TABLE_ENTRY (MagicResistLink)
	NL_STRING_CONVERSION_TABLE_ENTRY (Craft)
	NL_STRING_CONVERSION_TABLE_ENTRY (ForageExtract)
	NL_STRING_CONVERSION_TABLE_ENTRY (BreakCastResist)

	NL_STRING_CONVERSION_TABLE_ENTRY (Unknown)
	NL_END_STRING_CONVERSION_TABLE(TSuccessTableType, Conversion, Unknown)


	const string &toString(TSuccessTableType type)
	{
		return Conversion.toString(type);
	}

	TSuccessTableType toSuccessTableType( const string &str)
	{
		return Conversion.fromString(str);
	}

	TSuccessTableType actionNatureToTableType( ACTNATURE::TActionNature actionNature)
	{
		switch(actionNature)
		{
		case ACTNATURE::FIGHT:
			return FightPhrase;

		case ACTNATURE::OFFENSIVE_MAGIC:
			return OffensiveMagicCast;

		case ACTNATURE::CURATIVE_MAGIC:
			return CurativeMagicCast;

		case ACTNATURE::CRAFT:
			return Craft;

		case ACTNATURE::HARVEST:
		case ACTNATURE::SEARCH_MP:
			return ForageExtract;

		case ACTNATURE::DODGE:
		case ACTNATURE::PARRY:
			return FightDefense;

		case ACTNATURE::SHIELD_USE:
			return ShieldUse;

		default:
			return Unknown;
		}
	}
};

bool						CStaticSuccessTable::_Init = false;
float						CStaticSuccessTable::_AverageDodgeFactor = 0.0f;
const CStaticSuccessTable*	CStaticSuccessTable::_Tables[SUCCESS_TABLE_TYPE::NB_TABLE_TYPES];

//--------------------------------------------------------------
// initTables
//--------------------------------------------------------------
void CStaticSuccessTable::initTables()
{
	for (uint i = 0 ; i < SUCCESS_TABLE_TYPE::NB_TABLE_TYPES ; ++i)
		_Tables[i] = NULL;
	
	_Tables[SUCCESS_TABLE_TYPE::FightPhrase]	= CSheets::getSuccessTableForm( CSheetId("fight_phrase.succes_chances_table") );
	_Tables[SUCCESS_TABLE_TYPE::FightDefense]	= CSheets::getSuccessTableForm( CSheetId("dodge_parry.succes_chances_table") );
	_Tables[SUCCESS_TABLE_TYPE::FightDefenseAI]	= CSheets::getSuccessTableForm( CSheetId("dodge_parry_ai.succes_chances_table") );
	_Tables[SUCCESS_TABLE_TYPE::ShieldUse]		= CSheets::getSuccessTableForm( CSheetId("shield_use.succes_chances_table") );
	_Tables[SUCCESS_TABLE_TYPE::OffensiveMagicCast] = CSheets::getSuccessTableForm( CSheetId("offensive_magic.succes_chances_table") );
	_Tables[SUCCESS_TABLE_TYPE::CurativeMagicCast] = CSheets::getSuccessTableForm( CSheetId("curative_magic.succes_chances_table") );
	_Tables[SUCCESS_TABLE_TYPE::MagicResistDirect]	= CSheets::getSuccessTableForm( CSheetId("magic_resist.succes_chances_table") );
	_Tables[SUCCESS_TABLE_TYPE::MagicResistLink]	= CSheets::getSuccessTableForm( CSheetId("magic_resist_link.succes_chances_table") );
	_Tables[SUCCESS_TABLE_TYPE::Craft]			= CSheets::getSuccessTableForm( CSheetId("craft.succes_chances_table") ) ;
	_Tables[SUCCESS_TABLE_TYPE::ForageExtract]	= CSheets::getSuccessTableForm( CSheetId("extracting.succes_chances_table") );
	_Tables[SUCCESS_TABLE_TYPE::BreakCastResist] = CSheets::getSuccessTableForm( CSheetId("break_cast_resist.succes_chances_table") );

	nlassert(_Tables[SUCCESS_TABLE_TYPE::FightDefense]);
	nlassert(_Tables[SUCCESS_TABLE_TYPE::FightDefenseAI]);
	nlassert(_Tables[SUCCESS_TABLE_TYPE::FightPhrase]);
	nlassert(_Tables[SUCCESS_TABLE_TYPE::ShieldUse]);
	nlassert(_Tables[SUCCESS_TABLE_TYPE::OffensiveMagicCast]);
	nlassert(_Tables[SUCCESS_TABLE_TYPE::CurativeMagicCast]);
	nlassert(_Tables[SUCCESS_TABLE_TYPE::MagicResistDirect]);
	nlassert(_Tables[SUCCESS_TABLE_TYPE::MagicResistLink]);
	nlassert(_Tables[SUCCESS_TABLE_TYPE::Craft]);
	nlassert(_Tables[SUCCESS_TABLE_TYPE::ForageExtract]);
	nlassert(_Tables[SUCCESS_TABLE_TYPE::BreakCastResist]);

	_Init = true;
};


//--------------------------------------------------------------
// serial
//--------------------------------------------------------------
void CStaticSuccessTable::serial(class NLMISC::IStream &f)
{
	f.serial(_MaxSuccessFactor);
	f.serial(_MaxPartialSuccessFactor);
	f.serial(_MinPartialSuccessFactor);
	f.serial(_FullSuccessRoll);
	f.serial(_MinSuccessRoll);

	f.serial(_AverageDodgeFactor);
	
	f.serialCont(_SuccessXpTable);
} // serial //


//--------------------------------------------------------------
// readGeorges
//--------------------------------------------------------------
void CStaticSuccessTable::readGeorges (const CSmartPtr<UForm> &form, const CSheetId &sheetId)
{
	if (!form)
	{
		nlwarning("Error reading sheet %s, form == NULL", sheetId.toString().c_str());
		return;
	}

	UFormElm& root = form->getRootNode();		
	
	const UFormElm *array = NULL;
	if (root.getNodeByName (&array, "Chances") && array)
    {
		// Get array size
        uint size;
		array->getArraySize (size);
		
		nlassertex( size == NB_DELTA_LVL, ( "Success table %s must be have %d raw of delta level, but nb raw reading is %d", sheetId.toString().c_str(), NB_DELTA_LVL, size ) );
		
		_SuccessXpTable.resize( size );
		
        // Get a array value
        for (uint i=0; i<size; ++i)
        {
			const UFormElm *line = NULL;
			if ( array->getArrayNode( &line, i) && line)
			{
				line->getValueByName( _SuccessXpTable[ i ].RelativeLevel, "RelativeLevel" );
				line->getValueByName( _SuccessXpTable[ i ].SuccessProbability, "SuccessChances" );
				line->getValueByName( _SuccessXpTable[ i ].PartialSuccessMaxDraw, "PartialSuccessMaxDraw" );
				line->getValueByName( _SuccessXpTable[ i ].XpGain, "XpGain" );
			}
        }
	}
	
	if( ! root.getValueByName( _MaxSuccessFactor, "Max Success Factor" ) )
	{
		nlwarning("<CStaticSuccessTable::readGeorges> Can't find field 'Max Success' in form %s", sheetId.toString().c_str() );
	}
	if( ! root.getValueByName( _MaxPartialSuccessFactor, "Max Partial Success Factor" ) )
	{
		nlwarning("<CStaticSuccessTable::readGeorges> Can't find field 'Max Partial Success factor ' in form %s", sheetId.toString().c_str() );
	}
	if( ! root.getValueByName( _MinPartialSuccessFactor, "Min Partial Success Factor" ) )
	{
		nlwarning("<CStaticSuccessTable::readGeorges> Can't find field 'Min Partial Success factor ' in form %s", sheetId.toString().c_str() );
	}

	// if dodge table, change AverageDodgeFactor
	if ( sheetId == CSheetId("dodge_parry.succes_chances_table"))
	{
		float x1 = getSuccessChance(0);
		float x2 = getPartialSuccessChance(0);

		_AverageDodgeFactor = float((x1 + (x2-x1) * (_MinPartialSuccessFactor + _MaxPartialSuccessFactor) / 2.0f) / 100.0f);

		nldebug("_MinPartialSuccessFactor %f, _MaxPartialSuccessFactor %f, x1 %f, x2 %f, AverageDodgeFactor = %f",
			_MinPartialSuccessFactor, _MaxPartialSuccessFactor,x1,x2, _AverageDodgeFactor
 			);
	}

} // readGeorges //


//--------------------------------------------------------------
// reloadSheet
//--------------------------------------------------------------
/// called to copy from another sheet (operator= + care ptrs)
void CStaticSuccessTable::reloadSheet(const CStaticSuccessTable &o)
{
	// nothing special, but check correct size
	nlassert(_SuccessXpTable.size()==o._SuccessXpTable.size());
	this->operator=(o);
}

