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

#ifndef R2_TYPES_H
#define R2_TYPES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/string_conversion.h"

#include "game_share/far_position.h"

#include "r2_basic_types.h"

namespace R2
{
	///  Users that are allowed to connect (but not connected yet), and their role

	// context in which a tp occurs while in r2
	enum TTeleportContext
	{
		TPContext_Mainland = 0, // a player in a live scenario
		TPContext_Edit, 		// a pionneer testing its own scenario while in the editor
		TPContext_IslandOwner,	// outland owner in a live scenario
		TPContext_Unknown
	};

	struct TR2TpInfos
	{
		bool UseTpMessage;
		std::string TpReasonId;
		std::vector<std::string> TpReasonParams;
		std::string TpCancelTextId;
		R2::TTeleportContext TpContext;

		TR2TpInfos():UseTpMessage(false){}

		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{

			f.serial(UseTpMessage);
			if (!UseTpMessage){ return; }
			f.serial(TpReasonId);
			f.serialCont(TpReasonParams);
			f.serial(TpCancelTextId);
			f.serialEnum(TpContext);
		}
	};

	struct TMissionItem
	{
		NLMISC::CSheetId	SheetId;
		ucstring			Name;
		ucstring			Description;
		ucstring			Comment;

		/// serial
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial( SheetId );
			f.serial( Name );
			f.serial( Description );
			f.serial( Comment );
		}
	};

	struct TItemAndQuantity
	{
		NLMISC::CSheetId	SheetId;
		uint32				Quantity;

		/// serial
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial( SheetId );
			f.serial( Quantity );
		}

		TItemAndQuantity() {}

		TItemAndQuantity( NLMISC::CSheetId sheetId, uint32 quantity )
		{
			SheetId = sheetId;
			Quantity = quantity;
		}
	};

	struct TEasterEggInfo
	{
		uint32							EasterEggId;
		std::vector< NLMISC::CSheetId > Item;
		uint32							ScenarioId;
		uint32							InstanceId;
		NLMISC::CEntityId				LastOwnerCharacterId;
		NLMISC::CEntityId				CreatureIdAsEgg;
		CFarPosition					Position;
	};

	// This 'smart enum' class was generated from r2_share_itf.xml
	// but I moved it here to prevent from having to include r2_share_itf.h
	// in character.h. I commented out getConversionTable() to prevent from
	// including nel/misc/string_conversion.h
	struct TUserRole
	{
		enum TValues
		{
			ur_player,
			ur_editor,
			ur_animator,
			ur_outland_owner,

			invalid
		};

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(ur_player)
				NL_STRING_CONVERSION_TABLE_ENTRY(ur_editor)
				NL_STRING_CONVERSION_TABLE_ENTRY(ur_animator)
				NL_STRING_CONVERSION_TABLE_ENTRY(ur_outland_owner)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid);

			return conversionTable;
		}



		TValues _Value;

	public:
		TUserRole()
			: _Value(invalid)
		{
		}
		TUserRole(TValues value)
			: _Value(value)
		{
		}

		TUserRole(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TUserRole &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TUserRole &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TUserRole &other) const
		{
			return _Value < other._Value;
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}
	};


	struct TActPositionDescription
	{
		std::string Name;
		std::string Island;
		uint8 Season;
		uint32 LocationId;

		/// serial
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial( Name );
			f.serial( Island );
			f.serial( Season );
			f.serial( LocationId );
		}

		TActPositionDescription() {}

		TActPositionDescription(const std::string &name, const std::string& island, uint8 season, uint32 locationId)
			:Name(name), Island(island), Season(season), LocationId(locationId)
		{
		}

	};

	typedef std::vector<R2::TActPositionDescription> TActPositionDescriptions;

	struct TUserTriggerDescription
	{

		std::string Name;
		uint32 Act;
		uint32 Id;

		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial( Name );
			f.serial( Act);
			f.serial( Id );
		}

		TUserTriggerDescription(){}

		TUserTriggerDescription(const std::string& name, uint32 act, uint32 id)
			:Name(name), Act(act), Id(id)
		{
		}
	};

	typedef std::vector<R2::TUserTriggerDescription> TUserTriggerDescriptions;


	#define R2_ENUM_FROM_STRING(X)	{if ( str == #X ) { _Value = X; return; } }
	#define R2_ENUM_TO_STRING(X)	{if ( _Value == X ) { return  #X;} }

	// Mode of a player if (player or test) npc speed == NORMAL, if ( dm or editor) then character speed == Quicker
	struct TCharMode
	{
		enum TValues
		{
			Player,
			Tester,
			Editer,
			Dm,
			Invalid
		};

		TValues _Value;

	public:
		TCharMode()
			: _Value(Invalid)
		{
		}
		TCharMode(TValues value)
			: _Value(value)
		{
		}

		TCharMode(const std::string &str)
		{
			R2_ENUM_FROM_STRING(Player);
			R2_ENUM_FROM_STRING(Tester);
			R2_ENUM_FROM_STRING(Editer);
			R2_ENUM_FROM_STRING(Dm);
			_Value = Invalid;

		}

		std::string toString() const
		{
			R2_ENUM_TO_STRING(Player);
			R2_ENUM_TO_STRING(Tester);
			R2_ENUM_TO_STRING(Editer);
			R2_ENUM_TO_STRING(Dm);

			return "Invalid";
		}
		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TCharMode &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TCharMode &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TCharMode &other) const
		{
			return _Value < other._Value;
		}

		TValues getValue() const
		{
			return _Value;
		}
	};

	#undef R2_ENUM_FROM_STRING
	#undef R2_ENUM_TO_STRING

	struct TPioneersSessionsAllowed
	{
	public:
//		typedef uint32 TSessionId;

	public:
		TSessionId SessionId;
		TUserRole Role;
		std::string RingAccess;
		bool	EgsReloadPos;

	bool	Newcomer;

	public:
		TPioneersSessionsAllowed():SessionId(uint32(-1)), Role(TUserRole::invalid){EgsReloadPos = false;}
		TPioneersSessionsAllowed( TSessionId sessionId, TUserRole role, bool newcomer):
		SessionId(sessionId), Role(role), Newcomer(newcomer){}

	};

	enum TScenarioSessionType
	{
		st_edit,
		st_anim,

		invalid
	};

	struct TScenarioHeaderSerializer
	{
	public:
		typedef std::vector< std::pair< std::string ,std::string > >  TValueType;
	public:
		TScenarioHeaderSerializer(TValueType value = TValueType() ):Value(value) {}
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			uint32 size = 0;
			if (!f.isReading() )
			{
				size = (uint32)Value.size();
				f.serial(size);
			}
			else
			{
				f.serial(size);
				Value.resize(size);
			}

			uint32 first = 0;
			for ( ; first != size; ++first)
			{
				f.serial( Value[first].first );
				f.serial( Value[first].second );
			}
		}
	public:
		TValueType Value;

	};
} // namespace R2

#endif
