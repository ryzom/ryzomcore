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



#ifndef RY_AGST_SHEETS_H
#define RY_AGST_SHEETS_H


// Nel Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/sheet_id.h"

///Nel Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"


namespace AGS_TEST
{



/**
 * Singleton containing database on information for actors
 * \author Sadge
 * \author Nevrax France
 * \date 2002
 */
class CSheets
{
public:
	class CSheet
	{
	public:
		CSheet(): WalkSpeed(1.3f), RunSpeed(6.0f), Radius(0.5f), Height(2.0f), BoundingRadius(0.5) {}

 		uint32	Level;				// Level of the creature
 
 		float	AttackThreshold;
 		float	FleeThreshold;
 		float	SurvivalThreshold;

		float	WalkSpeed;
		float	RunSpeed;
		float	Radius;				// pacs primitive's radius
		float	Height;				// pacs primitive's height
		float	BoundingRadius;		// fighting radius
		bool	isNpc;				// is it an Npc or is it a creature?
		bool	CanChatTP;			// true if can access chat teleport sub-menu
		bool	CanChatTrade;		// true if can access chat trade sub-menu
		bool	CanChatMission;		// true if can access chat mission sub-menu
//		std::string Name;

		void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
		{
			const NLGEORGES::UFormElm *elmt = 0;
			std::string s;
			uint i;

			// the form was found so read the true values from George
			form->getRootNode ().getValueByName (WalkSpeed, "Basics.MovementSpeeds.WalkSpeed");
			form->getRootNode ().getValueByName (RunSpeed, "Basics.MovementSpeeds.RunSpeed");
			form->getRootNode ().getValueByName (Radius, "Collision.CollisionRadius");
			form->getRootNode ().getValueByName (Height, "Collision.Height");
			form->getRootNode ().getValueByName (BoundingRadius, "Collision.BoundingRadius");

			form->getRootNode().getValueByName(Level, "Basics.Level");
 
 			form->getRootNode().getValueByName (AttackThreshold, "AI.IsA.combatif.seuil_attaque");
 			form->getRootNode().getValueByName (AttackThreshold, "AI.IsA.combatif.seuil_fuite");
 			form->getRootNode().getValueByName (SurvivalThreshold, "AI.IsA.combatif.poids_survie");

//			form->getRootNode ().getValueByName (Name, "Basics.First Name");
//			form->getRootNode ().getValueByName (s, "Basics.CharacterName");
//			if (!Name.empty())
//				Name+=' ';
//			Name+=s;

			// are we a creature or an NPC?
			form->getRootNode ().getValueByName (s, "Basics.Race");
			isNpc=	s=="Fyros"? true:
					s=="Tryker"? true:
					s=="Zorai"? true:
					s=="Matis"? true:
					false;

			// can we chat mission?
			if (form->getRootNode ().getValueByName (s, "Basics.Race"))
				CanChatMission= (s=="Kami");

			// can we chat trade?
			CanChatTrade=false;
			for (i=1;i<5;++i)
			{
				if (form->getRootNode ().getValueByName (s, (std::string("ShopKeeper infos.ObjectType")+char('0'+i)).c_str()))
					CanChatTrade|= (s!="UNDEFINED" && s!="Unknown" && !s.empty());
				if (form->getRootNode ().getValueByName (s, (std::string("ShopKeeper infos.ObjectType")+char('0'+i)).c_str()))
					CanChatTrade|= (s!="UNDEFINED" && s!="Unknown" && !s.empty());
			}

			if (form->getRootNode ().getNodeByName(&elmt,"ShopKeeper infos.Special Items") && elmt!=0)
			{
				uint arraySize;
				elmt->getArraySize(arraySize);
				CanChatTrade|= (arraySize!=0);
			}

			if (form->getRootNode ().getNodeByName(&elmt,"ShopKeeper infos.Special Mp") && elmt!=0)
			{
				uint arraySize;
				elmt->getArraySize(arraySize);
				CanChatTrade|= (arraySize!=0);
			}


			// can we chat TP?
			form->getRootNode ().getValueByName (s, "ShopKeeper infos.TeleportType");
			CanChatTP= (s!="NONE");

			if (CanChatMission)	nlinfo("bot can chat Mission: %s",sheetId.toString().c_str());
			if (CanChatTrade)	nlinfo("bot can chat Trade: %s",sheetId.toString().c_str());
			if (CanChatTP)		nlinfo("bot can chat TP: %s",sheetId.toString().c_str());
		}

		void serial (NLMISC::IStream &s)
		{
			s.serial (WalkSpeed, RunSpeed, Radius, Height);
			s.serial (BoundingRadius, isNpc/*, Name*/);
			s.serial (CanChatTP, CanChatTrade, CanChatMission);
 			s.serial (Level);
 			s.serial( AttackThreshold, FleeThreshold, SurvivalThreshold);
		}

		void removed() {}

		static uint getVersion () { return 6; }
	};

	// load the creature data from the george files
	static	void	init	();
	static	void	removeUnsuableSheets	();
		
	// display the creature data for all known creature types
	static void display(NLMISC::CLog *log = NLMISC::InfoLog);

	//
	static void release() {}

	// get a data record from the database
	static const CSheet *lookup( NLMISC::CSheetId id );

private:
	// prohibit cnstructor as this is a singleton
	CSheets();

	static std::map<NLMISC::CSheetId,CSheet> _Sheets;
	static bool _Initialised;
};


} // end of namespace AGS_TEST

#endif // RY_AGST_SHEETS_H
