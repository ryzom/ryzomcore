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

#include "egs_sheets/egs_static_deposit.h"

// Fix the stupid Visual 6 Warning
void foo_egs_static_deposit_cpp() {};


//--------------------
// USING
//--------------------
using namespace std;
using namespace NLGEORGES;
using namespace NLMISC;

///////////////////////////////////////////////////////////////////////////
/////////////////////////// Static Deposit ////////////////////////////////
//                            (OBSOLETE)                                 //
///////////////////////////////////////////////////////////////////////////

//-----------------------------------------------
// readGeorges for CStaticDeposit
//
//-----------------------------------------------
/*void CStaticDeposit::readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId )
{
	if( form )
	{
		UFormElm& root = form->getRootNode();
		uint NbDeposit;
		nlverify( root.getStructSize( NbDeposit ) );

		string sheetName = sheetId.toString();
		nlinfo("Read Deposit %s", sheetId.toString().c_str() );

		for( uint i = 0; i < NbDeposit; ++i )
		{
			string nodeName = string("MP");
			if (i >= 9) 
				nodeName += toString(i+1);
			else
				nodeName += toString(0) + toString(i+1);
			nodeName += string(".");

//			UFormElm* DepositElt = NULL;
//			if( ! ( root.getStructNode( i, &DepositElt ) && DepositE-+--t ) )
//			{
//				nlwarning("<CStaticDeposit::readGeorges> can't get array node %u of DepositElt in sheet %s", i, sheetId.toString().c_str() );
//			}
//			else
//			{

				CStaticDepositRawMaterial deposit;
				string value;
				string name;

				name = nodeName + string("AssociatedItem");
				if( ! root.getValueByName( value, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node AssociatedItem in sheet %s", sheetId.toString().c_str() );
					continue; // skip that mp
				}
				else if ( !value.empty())
				{
					deposit.AssociatedItem = CSheetId( value );
					if (deposit.AssociatedItem == CSheetId::Unknown )
					{
						nlwarning("<CStaticDeposit::readGeorges> can't fin item sheet %s in sheet_id.bin for deposit %s", deposit.AssociatedItem.toString().c_str(), sheetId.toString().c_str() );
						continue; // skip that mp
					}
				}
				else
				{
					// empty
					nlwarning("<CStaticDeposit::readGeorges> Deposit %s contained empty AssociatedItem item", sheetId.toString().c_str() );
					continue;
				}

				name = nodeName + string("MaxGetQuantity");
				if( ! root.getValueByName( deposit.MaxGetQuantity, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node MaxGetQuantity in sheet %s", sheetId.toString().c_str() );
				}

				name = nodeName + string("PresenceProbabilities");
				if( ! root.getValueByName( deposit.PresenceProbabilities, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node PresenceProbabilities in sheet %s", sheetId.toString().c_str() );
				}

				// Check if this element contained something
				if( deposit.MaxGetQuantity == 0 || deposit.PresenceProbabilities == 0 )
				{
					continue;
				}
				
				name = nodeName + string("MinGetQuantity");
				if( ! root.getValueByName( deposit.MinGetQuantity, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node MinGetQuantity in sheet %s", sheetId.toString().c_str() );
				}
				
				name = nodeName + string("MinGetQuality");
				if( ! root.getValueByName( deposit.MinGetQuality, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node MinGetQuality in sheet %s", sheetId.toString().c_str() );
				}

				name = nodeName + string("MaxGetQuality");
				if( ! root.getValueByName( deposit.MaxGetQuality, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node MaxGetQuality in sheet %s", sheetId.toString().c_str() );
				}

				// seasonal parameters
				// spring
				name = nodeName + string("Spring.MinQuantity");
				if( ! root.getValueByName( deposit.SpringParams.MinQuantity, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Spring.MinQuantity in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Spring.MaxQuantity");
				if( ! root.getValueByName( deposit.SpringParams.MaxQuantity, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Spring.MaxQuantity in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Spring.RegenRate");
				if( ! root.getValueByName( deposit.SpringParams.RegenRateByHours, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Spring.RegenRate in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Spring.AngryLevel");
				if( ! root.getValueByName( deposit.SpringParams.AngryLevel, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Spring.AngryLevel in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Spring.FuryLevel");
				if( ! root.getValueByName( deposit.SpringParams.FuryLevel, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Spring.FuryLevel in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Spring.BlackKamiLevel");
				if( ! root.getValueByName( deposit.SpringParams.BlackKamiLevel, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Spring.BlackKamiLevel in sheet %s", sheetId.toString().c_str() );
				}
				// summer
				name = nodeName + string("Summer.MinQuantity");
				if( ! root.getValueByName( deposit.SummerParams.MinQuantity, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Summer.MinQuantity in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Summer.MaxQuantity");
				if( ! root.getValueByName( deposit.SummerParams.MaxQuantity, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Summer.MaxQuantity in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Summer.RegenRate");
				if( ! root.getValueByName( deposit.SummerParams.RegenRateByHours, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Summer.RegenRate in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Summer.AngryLevel");
				if( ! root.getValueByName( deposit.SummerParams.AngryLevel, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Summer.AngryLevel in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Summer.FuryLevel");
				if( ! root.getValueByName( deposit.SummerParams.FuryLevel, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Summer.FuryLevel in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Summer.BlackKamiLevel");
				if( ! root.getValueByName( deposit.SummerParams.BlackKamiLevel, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Summer.BlackKamiLevel in sheet %s", sheetId.toString().c_str() );
				}
				// autumn
				name = nodeName + string("Autumn.MinQuantity");
				if( ! root.getValueByName( deposit.AutumnParams.MinQuantity, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Autumn.MinQuantity in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Autumn.MaxQuantity");
				if( ! root.getValueByName( deposit.AutumnParams.MaxQuantity, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Autumn.MaxQuantity in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Autumn.RegenRate");
				if( ! root.getValueByName( deposit.AutumnParams.RegenRateByHours, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Autumn.RegenRate in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Autumn.AngryLevel");
				if( ! root.getValueByName( deposit.AutumnParams.AngryLevel, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Autumn.AngryLevel in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Autumn.FuryLevel");
				if( ! root.getValueByName( deposit.AutumnParams.FuryLevel, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Autumn.FuryLevel in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Autumn.BlackKamiLevel");
				if( ! root.getValueByName( deposit.AutumnParams.BlackKamiLevel, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Autumn.BlackKamiLevel in sheet %s", sheetId.toString().c_str() );
				}
				// winter
				name = nodeName + string("Winter.MinQuantity");
				if( ! root.getValueByName( deposit.WinterParams.MinQuantity, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Winter.MinQuantity in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Winter.MaxQuantity");
				if( ! root.getValueByName( deposit.WinterParams.MaxQuantity, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Winter.MaxQuantity in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Winter.RegenRate");
				if( ! root.getValueByName( deposit.WinterParams.RegenRateByHours, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Winter.RegenRate in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Winter.AngryLevel");
				if( ! root.getValueByName( deposit.WinterParams.AngryLevel, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Winter.AngryLevel in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Winter.FuryLevel");
				if( ! root.getValueByName( deposit.WinterParams.FuryLevel, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Winter.FuryLevel in sheet %s", sheetId.toString().c_str() );
				}
				name = nodeName + string("Winter.BlackKamiLevel");
				if( ! root.getValueByName( deposit.WinterParams.BlackKamiLevel, name.c_str() ) )
				{
					nlwarning("<CStaticDeposit::readGeorges> can't get node Winter.BlackKamiLevel in sheet %s", sheetId.toString().c_str() );
				}
				Deposit.push_back( deposit );
//			}
		}
	}
}*/
