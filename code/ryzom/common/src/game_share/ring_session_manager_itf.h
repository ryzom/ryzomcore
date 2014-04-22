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

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef RING_SESSION_MANAGER_ITF
#define RING_SESSION_MANAGER_ITF
#include "nel/misc/types_nl.h"
#include <memory>
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "game_share/callback_adaptor.h"

#include "nel/misc/entity_id.h"

#include "game_share/r2_basic_types.h"

#include "game_share/r2_share_itf.h"

#include "nel/net/login_cookie.h"

#include "game_share/welcome_service_itf.h"

#include "game_share/character_sync_itf.h"

#include "game_share/security_check.h"

namespace RSMGR
{

	class TRunningSessionInfo;

	class TSessionDesc;

	class TCharDesc;



	struct TSessionPartStatus
	{
		enum TValues
		{
			sps_play_subscribed = 1,
			sps_play_invited,
			sps_edit_invited,
			sps_anim_invited,
			sps_playing,
			sps_editing,
			sps_animating,
			/// the highest valid value in the enum
			last_enum_item = sps_animating,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 7
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(sps_play_subscribed, 0));
				indexTable.insert(std::make_pair(sps_play_invited, 1));
				indexTable.insert(std::make_pair(sps_edit_invited, 2));
				indexTable.insert(std::make_pair(sps_anim_invited, 3));
				indexTable.insert(std::make_pair(sps_playing, 4));
				indexTable.insert(std::make_pair(sps_editing, 5));
				indexTable.insert(std::make_pair(sps_animating, 6));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(sps_play_subscribed)
				NL_STRING_CONVERSION_TABLE_ENTRY(sps_play_invited)
				NL_STRING_CONVERSION_TABLE_ENTRY(sps_edit_invited)
				NL_STRING_CONVERSION_TABLE_ENTRY(sps_anim_invited)
				NL_STRING_CONVERSION_TABLE_ENTRY(sps_playing)
				NL_STRING_CONVERSION_TABLE_ENTRY(sps_editing)
				NL_STRING_CONVERSION_TABLE_ENTRY(sps_animating)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TSessionPartStatus()
			: _Value(invalid_val)
		{
		}
		TSessionPartStatus(TValues value)
			: _Value(value)
		{
		}

		TSessionPartStatus(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TSessionPartStatus &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TSessionPartStatus &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TSessionPartStatus &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TSessionPartStatus &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TSessionPartStatus &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TSessionPartStatus &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TSessionType
	{
		enum TValues
		{
			st_edit,
			st_anim,
			st_outland,
			st_mainland,
			/// the highest valid value in the enum
			last_enum_item = st_mainland,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 4
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(st_edit, 0));
				indexTable.insert(std::make_pair(st_anim, 1));
				indexTable.insert(std::make_pair(st_outland, 2));
				indexTable.insert(std::make_pair(st_mainland, 3));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(st_edit)
				NL_STRING_CONVERSION_TABLE_ENTRY(st_anim)
				NL_STRING_CONVERSION_TABLE_ENTRY(st_outland)
				NL_STRING_CONVERSION_TABLE_ENTRY(st_mainland)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TSessionType()
			: _Value(invalid_val)
		{
		}
		TSessionType(TValues value)
			: _Value(value)
		{
		}

		TSessionType(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TSessionType &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TSessionType &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TSessionType &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TSessionType &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TSessionType &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TSessionType &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TSessionOrientation
	{
		enum TValues
		{
			so_newbie_training = 1,
			so_story_telling,
			so_mistery,
			so_hack_slash,
			so_guild_training,
			so_other,
			/// the highest valid value in the enum
			last_enum_item = so_other,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 6
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(so_newbie_training, 0));
				indexTable.insert(std::make_pair(so_story_telling, 1));
				indexTable.insert(std::make_pair(so_mistery, 2));
				indexTable.insert(std::make_pair(so_hack_slash, 3));
				indexTable.insert(std::make_pair(so_guild_training, 4));
				indexTable.insert(std::make_pair(so_other, 5));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(so_newbie_training)
				NL_STRING_CONVERSION_TABLE_ENTRY(so_story_telling)
				NL_STRING_CONVERSION_TABLE_ENTRY(so_mistery)
				NL_STRING_CONVERSION_TABLE_ENTRY(so_hack_slash)
				NL_STRING_CONVERSION_TABLE_ENTRY(so_guild_training)
				NL_STRING_CONVERSION_TABLE_ENTRY(so_other)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TSessionOrientation()
			: _Value(invalid_val)
		{
		}
		TSessionOrientation(TValues value)
			: _Value(value)
		{
		}

		TSessionOrientation(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TSessionOrientation &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TSessionOrientation &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TSessionOrientation &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TSessionOrientation &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TSessionOrientation &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TSessionOrientation &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TSessionState
	{
		enum TValues
		{
			ss_planned = 1,
			ss_open,
			ss_locked,
			ss_closed,
			/// the highest valid value in the enum
			last_enum_item = ss_closed,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 4
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(ss_planned, 0));
				indexTable.insert(std::make_pair(ss_open, 1));
				indexTable.insert(std::make_pair(ss_locked, 2));
				indexTable.insert(std::make_pair(ss_closed, 3));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(ss_planned)
				NL_STRING_CONVERSION_TABLE_ENTRY(ss_open)
				NL_STRING_CONVERSION_TABLE_ENTRY(ss_locked)
				NL_STRING_CONVERSION_TABLE_ENTRY(ss_closed)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TSessionState()
			: _Value(invalid_val)
		{
		}
		TSessionState(TValues value)
			: _Value(value)
		{
		}

		TSessionState(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TSessionState &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TSessionState &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TSessionState &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TSessionState &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TSessionState &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TSessionState &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TAnimMode
	{
		enum TValues
		{
			am_dm = 1,
			am_autonomous,
			/// the highest valid value in the enum
			last_enum_item = am_autonomous,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 2
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(am_dm, 0));
				indexTable.insert(std::make_pair(am_autonomous, 1));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(am_dm)
				NL_STRING_CONVERSION_TABLE_ENTRY(am_autonomous)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TAnimMode()
			: _Value(invalid_val)
		{
		}
		TAnimMode(TValues value)
			: _Value(value)
		{
		}

		TAnimMode(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TAnimMode &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TAnimMode &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TAnimMode &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TAnimMode &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TAnimMode &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TAnimMode &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TAccessType
	{
		enum TValues
		{
			at_public = 1,
			at_private,
			/// the highest valid value in the enum
			last_enum_item = at_private,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 2
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(at_public, 0));
				indexTable.insert(std::make_pair(at_private, 1));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(at_public)
				NL_STRING_CONVERSION_TABLE_ENTRY(at_private)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TAccessType()
			: _Value(invalid_val)
		{
		}
		TAccessType(TValues value)
			: _Value(value)
		{
		}

		TAccessType(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TAccessType &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TAccessType &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TAccessType &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TAccessType &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TAccessType &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TAccessType &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TRuleType
	{
		enum TValues
		{
			rt_strict = 1,
			rt_liberal,
			/// the highest valid value in the enum
			last_enum_item = rt_liberal,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 2
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(rt_strict, 0));
				indexTable.insert(std::make_pair(rt_liberal, 1));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(rt_strict)
				NL_STRING_CONVERSION_TABLE_ENTRY(rt_liberal)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TRuleType()
			: _Value(invalid_val)
		{
		}
		TRuleType(TValues value)
			: _Value(value)
		{
		}

		TRuleType(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TRuleType &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TRuleType &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TRuleType &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TRuleType &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TRuleType &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TRuleType &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TLevelFilterEnum
	{
		enum TValues
		{
			lf_a = 1,
			lf_b = 2,
			lf_c = 4,
			lf_d = 8,
			lf_e = 16,
			lf_f = 32,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 6
		};


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(lf_a)
				NL_STRING_CONVERSION_TABLE_ENTRY(lf_b)
				NL_STRING_CONVERSION_TABLE_ENTRY(lf_c)
				NL_STRING_CONVERSION_TABLE_ENTRY(lf_d)
				NL_STRING_CONVERSION_TABLE_ENTRY(lf_e)
				NL_STRING_CONVERSION_TABLE_ENTRY(lf_f)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TLevelFilterEnum()
			: _Value(invalid_val)
		{
		}
		TLevelFilterEnum(TValues value)
			: _Value(value)
		{
		}

		TLevelFilterEnum(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TLevelFilterEnum &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TLevelFilterEnum &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TLevelFilterEnum &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TLevelFilterEnum &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TLevelFilterEnum &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TLevelFilterEnum &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


	};

	typedef NLMISC::CEnumBitset < TLevelFilterEnum, uint32, TLevelFilterEnum::invalid_val, ',', NLMISC::TContainedEnum < TLevelFilterEnum, uint32 >, TLevelFilterEnum::TValues > TLevelFilter;


	struct TEstimatedDuration
	{
		enum TValues
		{
			et_short = 1,
			et_medium,
			et_long,
			/// the highest valid value in the enum
			last_enum_item = et_long,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 3
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(et_short, 0));
				indexTable.insert(std::make_pair(et_medium, 1));
				indexTable.insert(std::make_pair(et_long, 2));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(et_short)
				NL_STRING_CONVERSION_TABLE_ENTRY(et_medium)
				NL_STRING_CONVERSION_TABLE_ENTRY(et_long)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TEstimatedDuration()
			: _Value(invalid_val)
		{
		}
		TEstimatedDuration(TValues value)
			: _Value(value)
		{
		}

		TEstimatedDuration(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TEstimatedDuration &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TEstimatedDuration &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TEstimatedDuration &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TEstimatedDuration &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TEstimatedDuration &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TEstimatedDuration &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TRaceFilterEnum
	{
		enum TValues
		{
			rf_fyros = 1,
			rf_matis = 2,
			rf_tryker = 4,
			rf_zorai = 8,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 4
		};


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(rf_fyros)
				NL_STRING_CONVERSION_TABLE_ENTRY(rf_matis)
				NL_STRING_CONVERSION_TABLE_ENTRY(rf_tryker)
				NL_STRING_CONVERSION_TABLE_ENTRY(rf_zorai)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TRaceFilterEnum()
			: _Value(invalid_val)
		{
		}
		TRaceFilterEnum(TValues value)
			: _Value(value)
		{
		}

		TRaceFilterEnum(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TRaceFilterEnum &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TRaceFilterEnum &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TRaceFilterEnum &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TRaceFilterEnum &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TRaceFilterEnum &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TRaceFilterEnum &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


	};

	typedef NLMISC::CEnumBitset < TRaceFilterEnum, uint32, TRaceFilterEnum::invalid_val, ',', NLMISC::TContainedEnum < TRaceFilterEnum, uint32 >, TRaceFilterEnum::TValues > TRaceFilter;


	struct TReligionFilterEnum
	{
		enum TValues
		{
			rf_kami = 1,
			rf_karavan = 2,
			rf_neutral = 4,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 3
		};


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(rf_kami)
				NL_STRING_CONVERSION_TABLE_ENTRY(rf_karavan)
				NL_STRING_CONVERSION_TABLE_ENTRY(rf_neutral)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TReligionFilterEnum()
			: _Value(invalid_val)
		{
		}
		TReligionFilterEnum(TValues value)
			: _Value(value)
		{
		}

		TReligionFilterEnum(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TReligionFilterEnum &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TReligionFilterEnum &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TReligionFilterEnum &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TReligionFilterEnum &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TReligionFilterEnum &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TReligionFilterEnum &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


	};

	typedef NLMISC::CEnumBitset < TReligionFilterEnum, uint32, TReligionFilterEnum::invalid_val, ',', NLMISC::TContainedEnum < TReligionFilterEnum, uint32 >, TReligionFilterEnum::TValues > TReligionFilter;


	struct TGuildFilter
	{
		enum TValues
		{
			gf_only_my_guild,
			gf_any_player,
			/// the highest valid value in the enum
			last_enum_item = gf_any_player,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 2
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(gf_only_my_guild, 0));
				indexTable.insert(std::make_pair(gf_any_player, 1));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(gf_only_my_guild)
				NL_STRING_CONVERSION_TABLE_ENTRY(gf_any_player)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TGuildFilter()
			: _Value(invalid_val)
		{
		}
		TGuildFilter(TValues value)
			: _Value(value)
		{
		}

		TGuildFilter(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TGuildFilter &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TGuildFilter &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TGuildFilter &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TGuildFilter &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TGuildFilter &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TGuildFilter &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};


	struct TShardFilterEnum
	{
		enum TValues
		{
			sf_shard00 = 1<<0,
			sf_shard01 = 1<<1,
			sf_shard02 = 1<<2,
			sf_shard03 = 1<<3,
			sf_shard04 = 1<<4,
			sf_shard05 = 1<<5,
			sf_shard06 = 1<<6,
			sf_shard07 = 1<<7,
			sf_shard08 = 1<<8,
			sf_shard09 = 1<<9,
			sf_shard10 = 1<<10,
			sf_shard11 = 1<<11,
			sf_shard12 = 1<<12,
			sf_shard13 = 1<<13,
			sf_shard14 = 1<<14,
			sf_shard15 = 1<<15,
			sf_shard16 = 1<<16,
			sf_shard17 = 1<<17,
			sf_shard18 = 1<<18,
			sf_shard19 = 1<<19,
			sf_shard20 = 1<<20,
			sf_shard21 = 1<<21,
			sf_shard22 = 1<<22,
			sf_shard23 = 1<<23,
			sf_shard24 = 1<<24,
			sf_shard25 = 1<<25,
			sf_shard26 = 1<<26,
			sf_shard27 = 1<<27,
			sf_shard28 = 1<<28,
			sf_shard29 = 1<<29,
			sf_shard30 = 1<<30,
			sf_shard31 = 1<<31,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 32
		};


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard00)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard01)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard02)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard03)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard04)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard05)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard06)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard07)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard08)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard09)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard10)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard11)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard12)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard13)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard14)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard15)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard16)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard17)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard18)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard19)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard20)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard21)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard22)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard23)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard24)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard25)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard26)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard27)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard28)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard29)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard30)
				NL_STRING_CONVERSION_TABLE_ENTRY(sf_shard31)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TShardFilterEnum()
			: _Value(invalid_val)
		{
		}
		TShardFilterEnum(TValues value)
			: _Value(value)
		{
		}

		TShardFilterEnum(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TShardFilterEnum &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TShardFilterEnum &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TShardFilterEnum &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TShardFilterEnum &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TShardFilterEnum &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TShardFilterEnum &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


	};

	typedef NLMISC::CEnumBitset < TShardFilterEnum, uint32, TShardFilterEnum::invalid_val, ',', NLMISC::TContainedEnum < TShardFilterEnum, uint32 >, TShardFilterEnum::TValues > TShardFilter;
	// Info about a running session in a DSS
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TRunningSessionInfo
	{
	protected:
		// The session Id
		TSessionId	_SessionId;
		// The type of the session
		TSessionType	_SessionType;
		// Instance ID that host this session
		uint32	_InstanceId;
		// Number of characters currently playing in the session (player and pioneer)
		uint32	_NbPlayingChars;
	public:
		// The session Id
		TSessionId getSessionId() const
		{
			return _SessionId;
		}

		void setSessionId(TSessionId value)
		{

				_SessionId = value;

		}
			// The type of the session
		TSessionType getSessionType() const
		{
			return _SessionType;
		}

		void setSessionType(TSessionType value)
		{

				_SessionType = value;

		}
			// Instance ID that host this session
		uint32 getInstanceId() const
		{
			return _InstanceId;
		}

		void setInstanceId(uint32 value)
		{

				_InstanceId = value;

		}
			// Number of characters currently playing in the session (player and pioneer)
		uint32 getNbPlayingChars() const
		{
			return _NbPlayingChars;
		}

		void setNbPlayingChars(uint32 value)
		{

				_NbPlayingChars = value;

		}

		bool operator == (const TRunningSessionInfo &other) const
		{
			return _SessionId == other._SessionId
				&& _SessionType == other._SessionType
				&& _InstanceId == other._InstanceId
				&& _NbPlayingChars == other._NbPlayingChars;
		}


		// constructor
		TRunningSessionInfo()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_SessionId);
			s.serial(_SessionType);
			s.serial(_InstanceId);
			s.serial(_NbPlayingChars);

		}


	private:


	};


		// A character enter the session
	// A character leave the session
	// The session is almost closed


	struct TSessionEvent
	{
		enum TValues
		{
			se_char_enter,
			se_char_leave,
			se_session_closing,
			/// the highest valid value in the enum
			last_enum_item = se_session_closing,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,

			/// Number of enumerated values
			nb_enum_items = 3
		};

		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(se_char_enter, 0));
				indexTable.insert(std::make_pair(se_char_leave, 1));
				indexTable.insert(std::make_pair(se_session_closing, 2));

				init = true;
			}

			return indexTable;
		}


		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(se_char_enter)
				NL_STRING_CONVERSION_TABLE_ENTRY(se_char_leave)
				NL_STRING_CONVERSION_TABLE_ENTRY(se_session_closing)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion<TValues>
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TSessionEvent()
			: _Value(invalid_val)
		{
		}
		TSessionEvent(TValues value)
			: _Value(value)
		{
		}

		TSessionEvent(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TSessionEvent &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TSessionEvent &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TSessionEvent &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TSessionEvent &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TSessionEvent &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TSessionEvent &other) const
		{
			return !(_Value < other._Value);
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

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}


		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}

	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CRingSessionManagerSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CRingSessionManagerSkel>	TInterceptor;
	protected:
		CRingSessionManagerSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CRingSessionManagerSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy * /* moduleProxy */)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy * /* moduleProxy */) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy * /* moduleProxy */) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CRingSessionManagerSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void registerDSS_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void sessionCreated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void reportSessionEvent_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void scenarioStarted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void reportCharacterKicked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void scenarioEnded_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CRingSessionManagerSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// A edition or animation server module register in the session manager
		// It send the list of session hosted in the server
		virtual void registerDSS(NLNET::IModuleProxy *sender, uint32 shardId, const std::vector < TRunningSessionInfo > &runningSessions) =0;
		// The session server report a session creation.
		virtual void sessionCreated(NLNET::IModuleProxy *sender, const RSMGR::TRunningSessionInfo &sessionInfo) =0;
		// The session report an event.
		// charId is used only when the event is about a character.
		virtual void reportSessionEvent(NLNET::IModuleProxy *sender, RSMGR::TSessionEvent event, TSessionId sessionId, uint32 charId) =0;
		// The DSS report that an animation scenario has just started
		// this allow SU to create the session log and scenario info record if needed.
		virtual void scenarioStarted(NLNET::IModuleProxy *sender, TSessionId sessionId, const R2::TRunningScenarioInfo &scenarioInfo) =0;
		// The session report that a DM has kicked a character from a session.
		virtual void reportCharacterKicked(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 charId) =0;
		// The DSS report the end of an animation session and
		// provides a bunch of data about the session life.
		virtual void scenarioEnded(NLNET::IModuleProxy *sender, TSessionId sessionId, const R2::TRunningScenarioInfo &scenarioInfo, uint32 rrpScored, uint32 scenarioPointScored, uint32 timeTaken, const std::vector < uint32 > &participants) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CRingSessionManagerProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CRingSessionManagerSkel	*_LocalModuleSkel;


	public:
		CRingSessionManagerProxy(NLNET::IModuleProxy *proxy)
		{
			nlassert(proxy->getModuleClassName() == "RingSessionManager");
			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CRingSessionManagerSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CRingSessionManagerProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// A edition or animation server module register in the session manager
		// It send the list of session hosted in the server
		void registerDSS(NLNET::IModule *sender, uint32 shardId, const std::vector < TRunningSessionInfo > &runningSessions);
		// The session server report a session creation.
		void sessionCreated(NLNET::IModule *sender, const RSMGR::TRunningSessionInfo &sessionInfo);
		// The session report an event.
		// charId is used only when the event is about a character.
		void reportSessionEvent(NLNET::IModule *sender, RSMGR::TSessionEvent event, TSessionId sessionId, uint32 charId);
		// The DSS report that an animation scenario has just started
		// this allow SU to create the session log and scenario info record if needed.
		void scenarioStarted(NLNET::IModule *sender, TSessionId sessionId, const R2::TRunningScenarioInfo &scenarioInfo);
		// The session report that a DM has kicked a character from a session.
		void reportCharacterKicked(NLNET::IModule *sender, TSessionId sessionId, uint32 charId);
		// The DSS report the end of an animation session and
		// provides a bunch of data about the session life.
		void scenarioEnded(NLNET::IModule *sender, TSessionId sessionId, const R2::TRunningScenarioInfo &scenarioInfo, uint32 rrpScored, uint32 scenarioPointScored, uint32 timeTaken, const std::vector < uint32 > &participants);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_registerDSS(NLNET::CMessage &__message, uint32 shardId, const std::vector < TRunningSessionInfo > &runningSessions);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_sessionCreated(NLNET::CMessage &__message, const RSMGR::TRunningSessionInfo &sessionInfo);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_reportSessionEvent(NLNET::CMessage &__message, RSMGR::TSessionEvent event, TSessionId sessionId, uint32 charId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_scenarioStarted(NLNET::CMessage &__message, TSessionId sessionId, const R2::TRunningScenarioInfo &scenarioInfo);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_reportCharacterKicked(NLNET::CMessage &__message, TSessionId sessionId, uint32 charId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_scenarioEnded(NLNET::CMessage &__message, TSessionId sessionId, const R2::TRunningScenarioInfo &scenarioInfo, uint32 rrpScored, uint32 scenarioPointScored, uint32 timeTaken, const std::vector < uint32 > &participants);




	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CRingSessionManagerClientSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CRingSessionManagerClientSkel>	TInterceptor;
	protected:
		CRingSessionManagerClientSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CRingSessionManagerClientSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy * /* moduleProxy */)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy * /* moduleProxy */) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy * /* moduleProxy */) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CRingSessionManagerClientSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;


		void createSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void addCharacterInSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void closeSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void stopHibernation_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void characterKicked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void characterUnkicked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void teleportOneCharacterToAnother_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void hibernateSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void setSessionStartParams_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CRingSessionManagerClientSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// Ask the client to create a new session modules
		virtual void createSession(NLNET::IModuleProxy *sender, uint32 ownerCharId, TSessionId sessionId, const RSMGR::TSessionType &type) =0;
		// Ask the client to allow a character in the session
		virtual void addCharacterInSession(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 charId, const WS::TUserRole &enterAs, const std::string &ringAccess, bool newcomer) =0;
		// Ask the client to close a running session
		virtual void closeSession(NLNET::IModuleProxy *sender, TSessionId sessionId) =0;
		// Ask the client stop hibernation for the
		// specified session. This mean to remove any
		// hibernated scenario file from the backup.
		virtual void stopHibernation(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 ownerId) =0;
		// Session mananger report that a character has been kicked by the web
		virtual void characterKicked(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 charId) =0;
		// Session mananger report that a character has been unkicked by the web
		virtual void characterUnkicked(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 charId) =0;
		// Ask to teleport on character to another (the 2 characters must be in the same season)
		// The character must arrived in the season
		virtual void teleportOneCharacterToAnother(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 sourceCharId, uint32 destCharId) =0;
		// Ask to hibernate a session
		virtual void hibernateSession(NLNET::IModuleProxy *sender, TSessionId sessionId) =0;
		// Set the start position of a session (eg while your are uploading an animation session)
		virtual void setSessionStartParams(NLNET::IModuleProxy *sender, uint32 charId, TSessionId sessionId, const std::string &initialIslandLocation, const std::string &initialEntryPointLocation, const std::string &initialSeason) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CRingSessionManagerClientProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CRingSessionManagerClientSkel	*_LocalModuleSkel;


	public:
		CRingSessionManagerClientProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CRingSessionManagerClientSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CRingSessionManagerClientProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// Ask the client to create a new session modules
		void createSession(NLNET::IModule *sender, uint32 ownerCharId, TSessionId sessionId, const RSMGR::TSessionType &type);
		// Ask the client to allow a character in the session
		void addCharacterInSession(NLNET::IModule *sender, TSessionId sessionId, uint32 charId, const WS::TUserRole &enterAs, const std::string &ringAccess, bool newcomer);
		// Ask the client to close a running session
		void closeSession(NLNET::IModule *sender, TSessionId sessionId);
		// Ask the client stop hibernation for the
		// specified session. This mean to remove any
		// hibernated scenario file from the backup.
		void stopHibernation(NLNET::IModule *sender, TSessionId sessionId, uint32 ownerId);
		// Session mananger report that a character has been kicked by the web
		void characterKicked(NLNET::IModule *sender, TSessionId sessionId, uint32 charId);
		// Session mananger report that a character has been unkicked by the web
		void characterUnkicked(NLNET::IModule *sender, TSessionId sessionId, uint32 charId);
		// Ask to teleport on character to another (the 2 characters must be in the same season)
		// The character must arrived in the season
		void teleportOneCharacterToAnother(NLNET::IModule *sender, TSessionId sessionId, uint32 sourceCharId, uint32 destCharId);
		// Ask to hibernate a session
		void hibernateSession(NLNET::IModule *sender, TSessionId sessionId);
		// Set the start position of a session (eg while your are uploading an animation session)
		void setSessionStartParams(NLNET::IModule *sender, uint32 charId, TSessionId sessionId, const std::string &initialIslandLocation, const std::string &initialEntryPointLocation, const std::string &initialSeason);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_createSession(NLNET::CMessage &__message, uint32 ownerCharId, TSessionId sessionId, const RSMGR::TSessionType &type);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_addCharacterInSession(NLNET::CMessage &__message, TSessionId sessionId, uint32 charId, const WS::TUserRole &enterAs, const std::string &ringAccess, bool newcomer);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_closeSession(NLNET::CMessage &__message, TSessionId sessionId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_stopHibernation(NLNET::CMessage &__message, TSessionId sessionId, uint32 ownerId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_characterKicked(NLNET::CMessage &__message, TSessionId sessionId, uint32 charId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_characterUnkicked(NLNET::CMessage &__message, TSessionId sessionId, uint32 charId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_teleportOneCharacterToAnother(NLNET::CMessage &__message, TSessionId sessionId, uint32 sourceCharId, uint32 destCharId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_hibernateSession(NLNET::CMessage &__message, TSessionId sessionId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_setSessionStartParams(NLNET::CMessage &__message, uint32 charId, TSessionId sessionId, const std::string &initialIslandLocation, const std::string &initialEntryPointLocation, const std::string &initialSeason);




	};
	// Callback interface used by web server during 'outgame' operation

	class CRingSessionManagerWebItf
	{
	protected:

		/// the callback server adaptor
		std::auto_ptr<ICallbackServerAdaptor>	_CallbackServer;

		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{
			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"SSS",	CRingSessionManagerWebItf::cb_scheduleSession	},
				{	"SSSP",	CRingSessionManagerWebItf::cb_setSessionStartParams	},
				{	"GSI",	CRingSessionManagerWebItf::cb_getSessionInfo	},
				{	"USS",	CRingSessionManagerWebItf::cb_updateSessionInfo	},
				{	"CANSS",	CRingSessionManagerWebItf::cb_cancelSession	},
				{	"STSS",	CRingSessionManagerWebItf::cb_startSession	},
				{	"CLSS",	CRingSessionManagerWebItf::cb_closeSession	},
				{	"CLESS",	CRingSessionManagerWebItf::cb_closeEditSession	},
				{	"AFC",	CRingSessionManagerWebItf::cb_addFriendCharacter	},
				{	"RFC",	CRingSessionManagerWebItf::cb_removeFriendCharacter	},
				{	"ABC",	CRingSessionManagerWebItf::cb_addBannedCharacter	},
				{	"RBC",	CRingSessionManagerWebItf::cb_removeBannedCharacter	},
				{	"AFDC",	CRingSessionManagerWebItf::cb_addFriendDMCharacter	},
				{	"RFDC",	CRingSessionManagerWebItf::cb_removeFriendDMCharacter	},
				{	"SKCC",	CRingSessionManagerWebItf::cb_setKnownCharacterComments	},
				{	"IC",	CRingSessionManagerWebItf::cb_inviteCharacter	},
				{	"RIC",	CRingSessionManagerWebItf::cb_removeInvitedCharacter	},
				{	"SBS",	CRingSessionManagerWebItf::cb_subscribeSession	},
				{	"USBS",	CRingSessionManagerWebItf::cb_unsubscribeSession	},
				{	"JSS",	CRingSessionManagerWebItf::cb_joinSession	},
				{	"JML",	CRingSessionManagerWebItf::cb_joinMainland	},
				{	"JES",	CRingSessionManagerWebItf::cb_joinEditSession	},
				{	"HES",	CRingSessionManagerWebItf::cb_hibernateEditSession	},
				{	"GSH",	CRingSessionManagerWebItf::cb_getShards	},
				{	"KC",	CRingSessionManagerWebItf::cb_kickCharacter	},
				{	"UKC",	CRingSessionManagerWebItf::cb_unkickCharacter	},
				{	"IG",	CRingSessionManagerWebItf::cb_inviteGuild	},
				{	"RIG",	CRingSessionManagerWebItf::cb_removeInvitedGuild	},
				{	"SSCI",	CRingSessionManagerWebItf::cb_setScenarioInfo	},
				{	"AJE",	CRingSessionManagerWebItf::cb_addJournalEntry	},
				{	"SPR",	CRingSessionManagerWebItf::cb_setPlayerRating	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);
		}

		static void _cbConnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CRingSessionManagerWeb__cbConnection);
			CRingSessionManagerWebItf *_this = reinterpret_cast<CRingSessionManagerWebItf *>(arg);

			_this->on_CRingSessionManagerWeb_Connection(from);
		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CRingSessionManagerWeb__cbDisconnection);
			CRingSessionManagerWebItf *_this = reinterpret_cast<CRingSessionManagerWebItf *>(arg);

			_this->on_CRingSessionManagerWeb_Disconnection(from);
		}


	public:
		/** Constructor, if you specify a replacement adaptor, then the object
		 *	become owner of the adaptor (and it will be released with the
		 *	interface).
		 */
		CRingSessionManagerWebItf(ICallbackServerAdaptor *replacementAdaptor = NULL)
		{
			if (replacementAdaptor == NULL)
			{
				// use default callback server
				_CallbackServer = std::auto_ptr<ICallbackServerAdaptor>(new CNelCallbackServerAdaptor(this));
			}
			else
			{
				// use the replacement one
				_CallbackServer = std::auto_ptr<ICallbackServerAdaptor>(replacementAdaptor);
			}
		}

		virtual ~CRingSessionManagerWebItf()
		{
		}

		/// Open the interface socket in the specified port
		void openItf(uint16 port)
		{
			NLNET::TCallbackItem *arrayPtr;
			uint32 arraySize;



			getCallbakArray(arrayPtr, arraySize);
			_CallbackServer->addCallbackArray(arrayPtr, arraySize);

			_CallbackServer->setConnectionCallback (_cbConnection, this);
			_CallbackServer->setDisconnectionCallback (_cbDisconnection, this);

			_CallbackServer->init(port);
		}

		/** Must be called evenly, update the network subclass to receive message
		 *	and dispatch method invokation.
		 */
		void update()
		{
			H_AUTO(CRingSessionManagerWeb_update);

			try
			{
				_CallbackServer->update();
			}
			catch (...)
			{
				nlwarning("CRingSessionManagerWeb : Exception launch in callback server update");
			}
		}

		// Generic response to invoke.
		// result contains 0 if no error, more than 0 in case of error

		void invokeResult(NLNET::TSockId dest, uint32 userId, uint32 resultCode, const std::string &resultString)
		{
			H_AUTO(invokeResult_invokeResult);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::invokeResult called");
#endif
			NLNET::CMessage message("RET");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, resultCode);
			nlWrite(message, serial, const_cast < std::string& > (resultString));

			_CallbackServer->send(message, dest);
		}
		// result is : 0 : session have been created fine
		//             1 : invalid session type
		//             2 : invalid level
		//             3 : unknown character
		//             4 : not used
		//             5 : invalid access type
		//             6 : invalid rule type
		//             7 : invalid duration
		//             8 : invalid user
		//             9 : free trial account can't create anim session
		//             10 : user is ban from ring anim session

		void scheduleSessionResult(NLNET::TSockId dest, uint32 charId, TSessionId sessionId, uint8 result, const std::string &resultString)
		{
			H_AUTO(scheduleSessionResult_scheduleSessionResult);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::scheduleSessionResult called");
#endif
			NLNET::CMessage message("SSSR");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, result);
			nlWrite(message, serial, const_cast < std::string& > (resultString));

			_CallbackServer->send(message, dest);
		}
		// session info result (anim)

		void sessionInfoResult(NLNET::TSockId dest, uint32 charId, TSessionId sessionId, const TRaceFilter &raceFilter, const TReligionFilter &religionFilter, const TGuildFilter &guildFilter, const TShardFilter &shardFilter, const TLevelFilter &levelFilter, bool subscriptionClosed, bool autoInvite, const std::string &language, const TSessionOrientation &orientation, const std::string &description)
		{
			H_AUTO(sessionInfoResult_sessionInfoResult);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::sessionInfoResult called");
#endif
			NLNET::CMessage message("SIR");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, raceFilter);
			nlWrite(message, serial, religionFilter);
			nlWrite(message, serial, guildFilter);
			nlWrite(message, serial, shardFilter);
			nlWrite(message, serial, levelFilter);
			nlWrite(message, serial, subscriptionClosed);
			nlWrite(message, serial, autoInvite);
			nlWrite(message, serial, const_cast < std::string& > (language));
			nlWrite(message, serial, orientation);
			nlWrite(message, serial, const_cast < std::string& > (description));

			_CallbackServer->send(message, dest);
		}
		// Return the result of the session joining attempt
		// If join is ok, the shardAddr contain <ip:port> of the
		// Front end that waits for the player to come in and the.
		// participation mode for the character (editor, animator or player).
		// If ok, the web must return a page with a lua script.
		// that trigger the action handler 'on_connect_to_shard' :
		// <lua>runAH(nul, "on_connect_to_shard", "cookie=cookieValue|fsAddr=shardAddr|mode=participantStatus");<lua>
		// result : 0 : ok the client can join the session
		//          1 : char not found
		//          2 : session not found
		//          3 : no session participant for this character (not used for a mainland shard)
		//          4 : can't find session server (not used for a mainland shard)
		//          5 : shard hosting session is not reachable
		//          6 : nel user info not found
		//          7 : ring user not found
		//          8 : welcome service rejected connection request
		//          9 : session service shutdown (not used for a mainland shard)
		//         10 : no mainland shard found (joinMainland only)
		//         11 : internal error
		//         12 : failed to request for access permission
		//         13 : can't find access permission for user and domain
		//         14 : Welcome service is closed for you
		//         15 : Session is not open
		//         16 : User banned from ring
		//         17 : Newcomer flag missmatch
		//         18 : Can't find session log to validate session access
		//         19 : Can't find scenario info to validate session access
		//         20 : Scenario is not allowed to free trial players

		void joinSessionResult(NLNET::TSockId dest, uint32 userId, TSessionId sessionId, uint32 result, const std::string &shardAddr, const TSessionPartStatus &participantStatus)
		{
			H_AUTO(joinSessionResult_joinSessionResult);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::joinSessionResult called");
#endif
			NLNET::CMessage message("JSSR");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, result);
			nlWrite(message, serial, const_cast < std::string& > (shardAddr));
			nlWrite(message, serial, const_cast < TSessionPartStatus& > (participantStatus));

			_CallbackServer->send(message, dest);
		}
		// See joinSessionResult.
		// Adds a security code.

		void joinSessionResultExt(NLNET::TSockId dest, uint32 userId, TSessionId sessionId, uint32 result, const std::string &shardAddr, const TSessionPartStatus &participantStatus, const CSecurityCode &securityCheckForFastDisconnection)
		{
			H_AUTO(joinSessionResultExt_joinSessionResultExt);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::joinSessionResultExt called");
#endif
			NLNET::CMessage message("JSSRE");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, result);
			nlWrite(message, serial, const_cast < std::string& > (shardAddr));
			nlWrite(message, serial, const_cast < TSessionPartStatus& > (participantStatus));
			nlWrite(message, serial, const_cast < CSecurityCode& > (securityCheckForFastDisconnection));

			_CallbackServer->send(message, dest);
		}
		// Return the list of online shards on which the user is allowed to connect,
		// and their current dynamic attributes. Other attributes (e.g. names)
		// can be queried from the database. Offline shards are the ones in the database
		// of the same domain but not listed in the result.
		// Then the client will have to call joinShard to connect on an online shard.

		void getShardsResult(NLNET::TSockId dest, uint32 userId, const std::string &result)
		{
			H_AUTO(getShardsResult_getShardsResult);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::getShardsResult called");
#endif
			NLNET::CMessage message("GSHR");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, const_cast < std::string& > (result));

			_CallbackServer->send(message, dest);
		}

		static void cb_scheduleSession (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(scheduleSession_on_scheduleSession);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_scheduleSession received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionType	sessionType;
			std::string	sessionTitle;
			std::string	sessionDesc;
			R2::TSessionLevel	sessionLevel;
			TRuleType	ruleType;
			TEstimatedDuration	estimatedDuration;
			uint32	subscriptionSlot;
			TAnimMode	animMode;
			TRaceFilter	raceFilter;
			TReligionFilter	religionFilter;
			TGuildFilter	guildFilter;
			TShardFilter	shardFilter;
			TLevelFilter	levelFilter;
			std::string	language;
			TSessionOrientation	orientation;
			bool	subscriptionClosed;
			bool	autoInvite;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionType);
			nlRead(message, serial, sessionTitle);
			nlRead(message, serial, sessionDesc);
			nlRead(message, serial, sessionLevel);
			nlRead(message, serial, ruleType);
			nlRead(message, serial, estimatedDuration);
			nlRead(message, serial, subscriptionSlot);
			nlRead(message, serial, animMode);
			nlRead(message, serial, raceFilter);
			nlRead(message, serial, religionFilter);
			nlRead(message, serial, guildFilter);
			nlRead(message, serial, shardFilter);
			nlRead(message, serial, levelFilter);
			nlRead(message, serial, language);
			nlRead(message, serial, orientation);
			nlRead(message, serial, subscriptionClosed);
			nlRead(message, serial, autoInvite);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_scheduleSession : calling on_scheduleSession");
#endif


			callback->on_scheduleSession(from, charId, sessionType, sessionTitle, sessionDesc, sessionLevel, ruleType, estimatedDuration, subscriptionSlot, animMode, raceFilter, religionFilter, guildFilter, shardFilter, levelFilter, language, orientation, subscriptionClosed, autoInvite);

		}

		static void cb_setSessionStartParams (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(setSessionStartParams_on_setSessionStartParams);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_setSessionStartParams received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			std::string	initialIslandLocation;
			std::string	initialEntryPointLocation;
			std::string	initialSeason;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, initialIslandLocation);
			nlRead(message, serial, initialEntryPointLocation);
			nlRead(message, serial, initialSeason);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_setSessionStartParams : calling on_setSessionStartParams");
#endif


			callback->on_setSessionStartParams(from, charId, sessionId, initialIslandLocation, initialEntryPointLocation, initialSeason);

		}

		static void cb_getSessionInfo (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(getSessionInfo_on_getSessionInfo);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_getSessionInfo received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_getSessionInfo : calling on_getSessionInfo");
#endif


			callback->on_getSessionInfo(from, charId, sessionId);

		}

		static void cb_updateSessionInfo (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(updateSessionInfo_on_updateSessionInfo);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_updateSessionInfo received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			std::string	sessionTitle;
			uint32	plannedDate;
			std::string	sessionDesc;
			R2::TSessionLevel	sessionLevel;
			TEstimatedDuration	estimatedDuration;
			uint32	subscriptionSlot;
			TRaceFilter	raceFilter;
			TReligionFilter	religionFilter;
			TGuildFilter	guildFilter;
			TShardFilter	shardFilter;
			TLevelFilter	levelFilter;
			bool	subscriptionClosed;
			bool	autoInvite;
			std::string	language;
			TSessionOrientation	orientation;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, sessionTitle);
			nlRead(message, serial, plannedDate);
			nlRead(message, serial, sessionDesc);
			nlRead(message, serial, sessionLevel);
			nlRead(message, serial, estimatedDuration);
			nlRead(message, serial, subscriptionSlot);
			nlRead(message, serial, raceFilter);
			nlRead(message, serial, religionFilter);
			nlRead(message, serial, guildFilter);
			nlRead(message, serial, shardFilter);
			nlRead(message, serial, levelFilter);
			nlRead(message, serial, subscriptionClosed);
			nlRead(message, serial, autoInvite);
			nlRead(message, serial, language);
			nlRead(message, serial, orientation);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_updateSessionInfo : calling on_updateSessionInfo");
#endif


			callback->on_updateSessionInfo(from, charId, sessionId, sessionTitle, plannedDate, sessionDesc, sessionLevel, estimatedDuration, subscriptionSlot, raceFilter, religionFilter, guildFilter, shardFilter, levelFilter, subscriptionClosed, autoInvite, language, orientation);

		}

		static void cb_cancelSession (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(cancelSession_on_cancelSession);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_cancelSession received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_cancelSession : calling on_cancelSession");
#endif


			callback->on_cancelSession(from, charId, sessionId);

		}

		static void cb_startSession (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(startSession_on_startSession);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_startSession received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_startSession : calling on_startSession");
#endif


			callback->on_startSession(from, charId, sessionId);

		}

		static void cb_closeSession (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(closeSession_on_closeSession);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_closeSession received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_closeSession : calling on_closeSession");
#endif


			callback->on_closeSession(from, charId, sessionId);

		}

		static void cb_closeEditSession (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(closeEditSession_on_closeEditSession);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_closeEditSession received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			nlRead(message, serial, charId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_closeEditSession : calling on_closeEditSession");
#endif


			callback->on_closeEditSession(from, charId);

		}

		static void cb_addFriendCharacter (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(addFriendCharacter_on_addFriendCharacter);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_addFriendCharacter received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			uint32	friendCharId;
			nlRead(message, serial, userId);
			nlRead(message, serial, friendCharId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_addFriendCharacter : calling on_addFriendCharacter");
#endif


			callback->on_addFriendCharacter(from, userId, friendCharId);

		}

		static void cb_removeFriendCharacter (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(removeFriendCharacter_on_removeFriendCharacter);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_removeFriendCharacter received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			uint32	friendCharId;
			nlRead(message, serial, userId);
			nlRead(message, serial, friendCharId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_removeFriendCharacter : calling on_removeFriendCharacter");
#endif


			callback->on_removeFriendCharacter(from, userId, friendCharId);

		}

		static void cb_addBannedCharacter (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(addBannedCharacter_on_addBannedCharacter);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_addBannedCharacter received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			uint32	bannedCharId;
			nlRead(message, serial, userId);
			nlRead(message, serial, bannedCharId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_addBannedCharacter : calling on_addBannedCharacter");
#endif


			callback->on_addBannedCharacter(from, userId, bannedCharId);

		}

		static void cb_removeBannedCharacter (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(removeBannedCharacter_on_removeBannedCharacter);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_removeBannedCharacter received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			uint32	bannedCharId;
			nlRead(message, serial, userId);
			nlRead(message, serial, bannedCharId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_removeBannedCharacter : calling on_removeBannedCharacter");
#endif


			callback->on_removeBannedCharacter(from, userId, bannedCharId);

		}

		static void cb_addFriendDMCharacter (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(addFriendDMCharacter_on_addFriendDMCharacter);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_addFriendDMCharacter received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			uint32	friendDMCharId;
			nlRead(message, serial, userId);
			nlRead(message, serial, friendDMCharId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_addFriendDMCharacter : calling on_addFriendDMCharacter");
#endif


			callback->on_addFriendDMCharacter(from, userId, friendDMCharId);

		}

		static void cb_removeFriendDMCharacter (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(removeFriendDMCharacter_on_removeFriendDMCharacter);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_removeFriendDMCharacter received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			uint32	friendDMCharId;
			nlRead(message, serial, userId);
			nlRead(message, serial, friendDMCharId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_removeFriendDMCharacter : calling on_removeFriendDMCharacter");
#endif


			callback->on_removeFriendDMCharacter(from, userId, friendDMCharId);

		}

		static void cb_setKnownCharacterComments (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(setKnownCharacterComments_on_setKnownCharacterComments);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_setKnownCharacterComments received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			uint32	charId;
			std::string	relation;
			std::string	comments;
			nlRead(message, serial, userId);
			nlRead(message, serial, charId);
			nlRead(message, serial, relation);
			nlRead(message, serial, comments);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_setKnownCharacterComments : calling on_setKnownCharacterComments");
#endif


			callback->on_setKnownCharacterComments(from, userId, charId, relation, comments);

		}

		static void cb_inviteCharacter (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(inviteCharacter_on_inviteCharacter);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_inviteCharacter received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	ownerCharId;
			TSessionId	sessionId;
			uint32	invitedCharId;
			TSessionPartStatus	charRole;
			nlRead(message, serial, ownerCharId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, invitedCharId);
			nlRead(message, serial, charRole);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_inviteCharacter : calling on_inviteCharacter");
#endif


			callback->on_inviteCharacter(from, ownerCharId, sessionId, invitedCharId, charRole);

		}

		static void cb_removeInvitedCharacter (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(removeInvitedCharacter_on_removeInvitedCharacter);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_removeInvitedCharacter received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	ownerCharId;
			TSessionId	sessionId;
			uint32	removedCharId;
			nlRead(message, serial, ownerCharId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, removedCharId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_removeInvitedCharacter : calling on_removeInvitedCharacter");
#endif


			callback->on_removeInvitedCharacter(from, ownerCharId, sessionId, removedCharId);

		}

		static void cb_subscribeSession (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(subscribeSession_on_subscribeSession);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_subscribeSession received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_subscribeSession : calling on_subscribeSession");
#endif


			callback->on_subscribeSession(from, charId, sessionId);

		}

		static void cb_unsubscribeSession (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(unsubscribeSession_on_unsubscribeSession);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_unsubscribeSession received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_unsubscribeSession : calling on_unsubscribeSession");
#endif


			callback->on_unsubscribeSession(from, charId, sessionId);

		}

		static void cb_joinSession (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(joinSession_on_joinSession);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_joinSession received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			std::string	clientApplication;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, clientApplication);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_joinSession : calling on_joinSession");
#endif


			callback->on_joinSession(from, charId, sessionId, clientApplication);

		}

		static void cb_joinMainland (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(joinMainland_on_joinMainland);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_joinMainland received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			std::string	clientApplication;
			nlRead(message, serial, charId);
			nlRead(message, serial, clientApplication);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_joinMainland : calling on_joinMainland");
#endif


			callback->on_joinMainland(from, charId, clientApplication);

		}

		static void cb_joinEditSession (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(joinEditSession_on_joinEditSession);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_joinEditSession received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			std::string	clientApplication;
			nlRead(message, serial, charId);
			nlRead(message, serial, clientApplication);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_joinEditSession : calling on_joinEditSession");
#endif


			callback->on_joinEditSession(from, charId, clientApplication);

		}

		static void cb_hibernateEditSession (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(hibernateEditSession_on_hibernateEditSession);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_hibernateEditSession received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			nlRead(message, serial, charId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_hibernateEditSession : calling on_hibernateEditSession");
#endif


			callback->on_hibernateEditSession(from, charId);

		}

		static void cb_getShards (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(getShards_on_getShards);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_getShards received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			nlRead(message, serial, charId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_getShards : calling on_getShards");
#endif


			callback->on_getShards(from, charId);

		}

		static void cb_kickCharacter (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(kickCharacter_on_kickCharacter);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_kickCharacter received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	ownerCharId;
			TSessionId	sessionId;
			uint32	kickedCharId;
			nlRead(message, serial, ownerCharId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, kickedCharId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_kickCharacter : calling on_kickCharacter");
#endif


			callback->on_kickCharacter(from, ownerCharId, sessionId, kickedCharId);

		}

		static void cb_unkickCharacter (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(unkickCharacter_on_unkickCharacter);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_unkickCharacter received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	ownerCharId;
			TSessionId	sessionId;
			uint32	unkickedCharId;
			nlRead(message, serial, ownerCharId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, unkickedCharId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_unkickCharacter : calling on_unkickCharacter");
#endif


			callback->on_unkickCharacter(from, ownerCharId, sessionId, unkickedCharId);

		}

		static void cb_inviteGuild (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(inviteGuild_on_inviteGuild);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_inviteGuild received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			uint32	guildId;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, guildId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_inviteGuild : calling on_inviteGuild");
#endif


			callback->on_inviteGuild(from, charId, sessionId, guildId);

		}

		static void cb_removeInvitedGuild (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(removeInvitedGuild_on_removeInvitedGuild);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_removeInvitedGuild received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			uint32	guildId;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, guildId);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_removeInvitedGuild : calling on_removeInvitedGuild");
#endif


			callback->on_removeInvitedGuild(from, charId, sessionId, guildId);

		}

		static void cb_setScenarioInfo (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(setScenarioInfo_on_setScenarioInfo);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_setScenarioInfo received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			std::string	title;
			uint32	numPlayer;
			std::string	playType;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, title);
			nlRead(message, serial, numPlayer);
			nlRead(message, serial, playType);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_setScenarioInfo : calling on_setScenarioInfo");
#endif


			callback->on_setScenarioInfo(from, charId, sessionId, title, numPlayer, playType);

		}

		static void cb_addJournalEntry (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(addJournalEntry_on_addJournalEntry);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_addJournalEntry received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			std::string	entryType;
			std::string	text;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, entryType);
			nlRead(message, serial, text);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_addJournalEntry : calling on_addJournalEntry");
#endif


			callback->on_addJournalEntry(from, charId, sessionId, entryType, text);

		}

		static void cb_setPlayerRating (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(setPlayerRating_on_setPlayerRating);
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_setPlayerRating received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebItf *callback = (CRingSessionManagerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			uint32	rateFun;
			uint32	rateDifficulty;
			uint32	rateAccessibility;
			uint32	rateOriginality;
			uint32	rateDirection;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, rateFun);
			nlRead(message, serial, rateDifficulty);
			nlRead(message, serial, rateAccessibility);
			nlRead(message, serial, rateOriginality);
			nlRead(message, serial, rateDirection);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWeb::cb_setPlayerRating : calling on_setPlayerRating");
#endif


			callback->on_setPlayerRating(from, charId, sessionId, rateFun, rateDifficulty, rateAccessibility, rateOriginality, rateDirection);

		}


		/// Connection callback : a new interface client connect
		virtual void on_CRingSessionManagerWeb_Connection(NLNET::TSockId from) =0;
		/// Disconnection callback : one of the interface client disconnect
		virtual void on_CRingSessionManagerWeb_Disconnection(NLNET::TSockId from) =0;


		// Create or schedule a new session (edit or anim)
		virtual void on_scheduleSession(NLNET::TSockId from, uint32 charId, const TSessionType &sessionType, const std::string &sessionTitle, const std::string &sessionDesc, const R2::TSessionLevel &sessionLevel, const TRuleType &ruleType, const TEstimatedDuration &estimatedDuration, uint32 subscriptionSlot, const TAnimMode &animMode, const TRaceFilter &raceFilter, const TReligionFilter &religionFilter, const TGuildFilter &guildFilter, const TShardFilter &shardFilter, const TLevelFilter &levelFilter, const std::string &language, const TSessionOrientation &orientation, bool subscriptionClosed, bool autoInvite) =0;

		// Set the start position of a session (eg while your are uploading an animation session)
		virtual void on_setSessionStartParams(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::string &initialIslandLocation, const std::string &initialEntryPointLocation, const std::string &initialSeason) =0;

		// get session info (anim)
		virtual void on_getSessionInfo(NLNET::TSockId from, uint32 charId, TSessionId sessionId) =0;

		// Update the information of a planned or running session
		// Return 'invokeResult' : 0 : ok, session updated
		//                         1 : unknown character
		//                         2 : unknown session
		//                         3 : char don't own the session
		//                         4 : session is closed, no update allowed
		virtual void on_updateSessionInfo(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::string &sessionTitle, uint32 plannedDate, const std::string &sessionDesc, const R2::TSessionLevel &sessionLevel, const TEstimatedDuration &estimatedDuration, uint32 subscriptionSlot, const TRaceFilter &raceFilter, const TReligionFilter &religionFilter, const TGuildFilter &guildFilter, const TShardFilter &shardFilter, const TLevelFilter &levelFilter, bool subscriptionClosed, bool autoInvite, const std::string &language, const TSessionOrientation &orientation) =0;

		// Cancel a plannified session
		// Return 'invokeResult' : 0 : ok, session canceled
		//                         1 : unknown char
		//                         2 : unknown session
		//                         3 : char don't own the session
		//                         4 : session not in planned state
		virtual void on_cancelSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId) =0;

		// start a planned session
		// Return 'invokeResult' : 0 : ok, session started
		//                         1 : char not found
		//                         2 : session not found
		//                         3 : session not own by user
		//                         4 : user is already have a running session of this type
		//                         5 : session server failure
		//                         6 : Action fordidden for free trial
		//                         7 : User not found
		virtual void on_startSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId) =0;

		// Close a running session
		// Return 'invokeResult' : 0 : ok, session closed (or about to close)
		//                         1 : session not found
		//                         2 : char don't own the session
		//                         3 : session not open
		//                         4 : failed to close the session, internal error
		virtual void on_closeSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId) =0;

		// Close the current edit session of the character
		// Return 'invokeResult' : 0 : ok, session closed (or about to close)
		//                         1 : char not found
		//                         2 : failed to close the session, internal error
		//                         3 : no session server to close the session
		//                         4 : no edit session found
		//                         5 : internal error
		virtual void on_closeEditSession(NLNET::TSockId from, uint32 charId) =0;

		// Add a character in a user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char already friend
		virtual void on_addFriendCharacter(NLNET::TSockId from, uint32 userId, uint32 friendCharId) =0;

		// Repove a character from a user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char not friend
		virtual void on_removeFriendCharacter(NLNET::TSockId from, uint32 userId, uint32 friendCharId) =0;

		// Add a character to a user ban list. This ban the user that own the character
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : banned char not found
		//                         3 : char already banned by user
		virtual void on_addBannedCharacter(NLNET::TSockId from, uint32 userId, uint32 bannedCharId) =0;

		// Remove a character from a user ban list.
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : banned char not found
		//                         3 : char not banned by user
		virtual void on_removeBannedCharacter(NLNET::TSockId from, uint32 userId, uint32 bannedCharId) =0;

		// Add a character in a DM user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char already DM friend
		virtual void on_addFriendDMCharacter(NLNET::TSockId from, uint32 userId, uint32 friendDMCharId) =0;

		// Remove a character from a DM user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char not friend
		virtual void on_removeFriendDMCharacter(NLNET::TSockId from, uint32 userId, uint32 friendDMCharId) =0;

		// Set the comment associated to a known character entry
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : known character entry not found
		//                         3 : character relation don't match the set comments relation
		//                         4 : internal error
		virtual void on_setKnownCharacterComments(NLNET::TSockId from, uint32 userId, uint32 charId, const std::string &relation, const std::string &comments) =0;

		// A user invite a character to help or play in his session
		// charRole is from enum TSessionPartStatus
		// invokeReturn : 0 : ok, character invited
		//                1 : char not found
		//                2 : session not found
		//                3 : invited char not found
		//                4 : char not own the session
		//                5 : char already invited
		//                6 : char role and session type don't match (edit/editor, anim/animator)
		//                7 : charRole is invalid (must be sps_play_invited, sps_edit_invited or sps_anim_invited)
		//                8 : database failure
		//               11 : owner char is not animator in the session
		//               12 : newcomer flag missmatch
		//               13 : scenario not started, can't validate invitation now
		//               14 : free trial character are not allowed in user scenario
		virtual void on_inviteCharacter(NLNET::TSockId from, uint32 ownerCharId, TSessionId sessionId, uint32 invitedCharId, const TSessionPartStatus &charRole) =0;

		// A user remove an invitation in a session
		// invokeReturn : 0 : ok, character invited
		//                1 : removed char not found
		//                2 : session not found
		//                3 : character already entered in session
		//                4 : invitation not found
		//                5 : owner char don't own the session
		virtual void on_removeInvitedCharacter(NLNET::TSockId from, uint32 ownerCharId, TSessionId sessionId, uint32 removedCharId) =0;

		// A character subscribe to a public animation session
		// invokeReturn : 0 : ok, subscription accepted
		//                1 : char not found
		//                2 : session not found
		//                3 : character already subscribed to or invited in the session
		//                4 : session not public
		//                5 : character banned
		//                6 : no place left, session is full
		//                7 : session owner not found
		//                8 : internal error
		virtual void on_subscribeSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId) =0;

		// A character unsubscribe to a public animation session
		// The character must not join the session in order to unsubscribe
		// invokeReturn : 0 : ok, unsubscription accepted
		//                1 : char not found
		//                2 : session not found
		//                3 : character has not subscribed in the session
		virtual void on_unsubscribeSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId) =0;

		// A character asks to join (or enter) a running session.
		// It must have been subscribed or invited to the session to be allowed
		virtual void on_joinSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::string &clientApplication) =0;

		// A character asks to join a shard.
		// charId: userId << 4 & charSlot
		// If ingame, charSlot in [0..14], otherwise 15 if outgame
		// The actual shard will be chosen according to current load.
		// Will return a joinSessionResult.
		virtual void on_joinMainland(NLNET::TSockId from, uint32 charId, const std::string &clientApplication) =0;

		// Ask to join the edit session for the specified character.
		// If the edit session do not exist, then the SU
		// create it before internally calling the join session request
		// the SU.
		// Return joinSessionResult.
		virtual void on_joinEditSession(NLNET::TSockId from, uint32 charId, const std::string &clientApplication) =0;

		// hibernate an editing session
		// Return 'invokeResult' : 0 : ok, session hibernating(or no session)
		//                         1 : undefined error
		virtual void on_hibernateEditSession(NLNET::TSockId from, uint32 charId) =0;

		// Request to have the list of accessible shards with their attributes.
		// This is a dev feature only.
		virtual void on_getShards(NLNET::TSockId from, uint32 charId) =0;

		// Kick a character from a session
		// charId must be the owner of a DM in the session
		// invokeReturn : 0 : ok, character kicked
		//                1 : char not found
		//                2 : session not found
		//                3 : kicked character has no participation in the session
		//                4 : internal error
		//                5 : owner char don't own the session
		virtual void on_kickCharacter(NLNET::TSockId from, uint32 ownerCharId, TSessionId sessionId, uint32 kickedCharId) =0;

		// Unkick a character from a session
		// charId must be the owner of a DM in the session
		// invokeReturn : 0 : ok, character kicked
		//                1 : char not found
		//                2 : session not found
		//                3 : kicked character has no participation in the session
		//                4 : internal error
		//                5 : owner char don't own the session
		virtual void on_unkickCharacter(NLNET::TSockId from, uint32 ownerCharId, TSessionId sessionId, uint32 unkickedCharId) =0;

		// A user invite a guild to play in a session
		// invokeReturn : 0 : ok, guild invited
		//                1 : guild not found
		//                2 : char not found
		//                3 : session not found
		//                4 : guild already invited
		//                5 : char don't own the session
		virtual void on_inviteGuild(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint32 guildId) =0;

		// Remove a guild invitation in a session
		// invokeReturn : 0 : ok, guild invited
		//                1 : guild not found
		//                2 : char not found
		//                3 : session not found
		//                4 : guild not invited
		//                5 : char don't own the session
		virtual void on_removeInvitedGuild(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint32 guildId) =0;

		// Set the additionnal scenario info
		// playType is the enumerated type TPlayType
		// invokeReturn : 0 : ok, info setted
		//                1 : scenario not found
		//                2 : char not owner of session
		//                3 : char not found
		virtual void on_setScenarioInfo(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::string &title, uint32 numPlayer, const std::string &playType) =0;

		// Add an entry in the session journal
		// invokeReturn : 0 : ok, entry added
		//                1 : scenario not found
		//                2 : user can't post in this journal
		virtual void on_addJournalEntry(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::string &entryType, const std::string &text) =0;

		// Set the rating for a scenario
		// invokeReturn : 0 : ok, rating set
		//                1 : scenario not found
		//                2 : char is not found
		//                3 : char is not a participant of session
		//                4 : no session log for the session
		//                5 : char is banned from session
		//                6 : session not found
		//                7 : scenario not found
		//                8 : internal error
		virtual void on_setPlayerRating(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection) =0;

	};

		// Callback interface used by web server during 'outgame' operation

	/** This is the client side of the interface
	 *	Derive from this class to invoke method on the callback server
	 */

	class CRingSessionManagerWebClientItf
	{
	protected:

		/// the callback client adaptor
		std::auto_ptr < ICallbackClientAdaptor >	_CallbackClient;


		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{

			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"RET",	CRingSessionManagerWebClientItf::cb_invokeResult	},
				{	"SSSR",	CRingSessionManagerWebClientItf::cb_scheduleSessionResult	},
				{	"SIR",	CRingSessionManagerWebClientItf::cb_sessionInfoResult	},
				{	"JSSR",	CRingSessionManagerWebClientItf::cb_joinSessionResult	},
				{	"JSSRE",	CRingSessionManagerWebClientItf::cb_joinSessionResultExt	},
				{	"GSHR",	CRingSessionManagerWebClientItf::cb_getShardsResult	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);

		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			CRingSessionManagerWebClientItf *_this = reinterpret_cast<CRingSessionManagerWebClientItf *>(arg);

			_this->on_CRingSessionManagerWebClient_Disconnection(from);
		}


	public:
		/// Retreive the message name for a given callback name
		static const std::string &getMessageName(const std::string &methodName)
		{
			static std::map<std::string, std::string> messageNames;
			static bool initialized = false;
			if (!initialized)
			{
			messageNames.insert(std::make_pair(std::string("on_invokeResult"), std::string("RET")));
			messageNames.insert(std::make_pair(std::string("on_scheduleSessionResult"), std::string("SSSR")));
			messageNames.insert(std::make_pair(std::string("on_sessionInfoResult"), std::string("SIR")));
			messageNames.insert(std::make_pair(std::string("on_joinSessionResult"), std::string("JSSR")));
			messageNames.insert(std::make_pair(std::string("on_joinSessionResultExt"), std::string("JSSRE")));
			messageNames.insert(std::make_pair(std::string("on_getShardsResult"), std::string("GSHR")));

				initialized = true;
			}

			std::map < std::string, std::string>::const_iterator it(messageNames.find(methodName));
			if (it != messageNames.end())
				return it->second;


			static std::string emptyString;

			return emptyString;

		}

		CRingSessionManagerWebClientItf(ICallbackClientAdaptor *adaptorReplacement = NULL)
		{
			if (adaptorReplacement == NULL)
			{
				// use the default Nel adaptor
				_CallbackClient = std::auto_ptr < ICallbackClientAdaptor >(new CNelCallbackClientAdaptor(this));
			}
			else
			{
				// use the replacement one
				_CallbackClient = std::auto_ptr < ICallbackClientAdaptor >(adaptorReplacement);
			}
		}

		/// Connect the interface client to the callback server at the specified address and port
		virtual void connectItf(NLNET::CInetAddress address)
		{
			NLNET::TCallbackItem *arrayPtr;
			uint32 arraySize;

			static bool callbackAdded = false;
			if (!callbackAdded)
			{

				getCallbakArray(arrayPtr, arraySize);
				_CallbackClient->addCallbackArray(arrayPtr, arraySize);
			}

			_CallbackClient->setDisconnectionCallback (_cbDisconnection, this);

			_CallbackClient->connect(address);
		}

		/** Must be called evenly, update the network subclass to receive message
		 *	and dispatch invokation returns.
		 */
		virtual void update()
		{
			H_AUTO(CRingSessionManagerWeb_update);

			try
			{
				_CallbackClient->update();
			}
			catch (...)
			{
				nlwarning("CRingSessionManagerWeb : Exception launch in callback client update");
			}
		}

		// Create or schedule a new session (edit or anim)

		void scheduleSession(uint32 charId, const TSessionType &sessionType, const std::string &sessionTitle, const std::string &sessionDesc, const R2::TSessionLevel &sessionLevel, const TRuleType &ruleType, const TEstimatedDuration &estimatedDuration, uint32 subscriptionSlot, const TAnimMode &animMode, const TRaceFilter &raceFilter, const TReligionFilter &religionFilter, const TGuildFilter &guildFilter, const TShardFilter &shardFilter, const TLevelFilter &levelFilter, const std::string &language, const TSessionOrientation &orientation, bool subscriptionClosed, bool autoInvite)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::scheduleSession called");
#endif
			NLNET::CMessage message("SSS");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionType);
			nlWrite(message, serial, const_cast < std::string& > (sessionTitle));
			nlWrite(message, serial, const_cast < std::string& > (sessionDesc));
			nlWrite(message, serial, sessionLevel);
			nlWrite(message, serial, ruleType);
			nlWrite(message, serial, estimatedDuration);
			nlWrite(message, serial, subscriptionSlot);
			nlWrite(message, serial, animMode);
			nlWrite(message, serial, raceFilter);
			nlWrite(message, serial, religionFilter);
			nlWrite(message, serial, guildFilter);
			nlWrite(message, serial, shardFilter);
			nlWrite(message, serial, levelFilter);
			nlWrite(message, serial, const_cast < std::string& > (language));
			nlWrite(message, serial, orientation);
			nlWrite(message, serial, subscriptionClosed);
			nlWrite(message, serial, autoInvite);

			_CallbackClient->send(message);
		}
		// Set the start position of a session (eg while your are uploading an animation session)

		void setSessionStartParams(uint32 charId, TSessionId sessionId, const std::string &initialIslandLocation, const std::string &initialEntryPointLocation, const std::string &initialSeason)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::setSessionStartParams called");
#endif
			NLNET::CMessage message("SSSP");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, const_cast < std::string& > (initialIslandLocation));
			nlWrite(message, serial, const_cast < std::string& > (initialEntryPointLocation));
			nlWrite(message, serial, const_cast < std::string& > (initialSeason));

			_CallbackClient->send(message);
		}
		// get session info (anim)

		void getSessionInfo(uint32 charId, TSessionId sessionId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::getSessionInfo called");
#endif
			NLNET::CMessage message("GSI");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);

			_CallbackClient->send(message);
		}
		// Update the information of a planned or running session
		// Return 'invokeResult' : 0 : ok, session updated
		//                         1 : unknown character
		//                         2 : unknown session
		//                         3 : char don't own the session
		//                         4 : session is closed, no update allowed

		void updateSessionInfo(uint32 charId, TSessionId sessionId, const std::string &sessionTitle, uint32 plannedDate, const std::string &sessionDesc, const R2::TSessionLevel &sessionLevel, const TEstimatedDuration &estimatedDuration, uint32 subscriptionSlot, const TRaceFilter &raceFilter, const TReligionFilter &religionFilter, const TGuildFilter &guildFilter, const TShardFilter &shardFilter, const TLevelFilter &levelFilter, bool subscriptionClosed, bool autoInvite, const std::string &language, const TSessionOrientation &orientation)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::updateSessionInfo called");
#endif
			NLNET::CMessage message("USS");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, const_cast < std::string& > (sessionTitle));
			nlWrite(message, serial, plannedDate);
			nlWrite(message, serial, const_cast < std::string& > (sessionDesc));
			nlWrite(message, serial, sessionLevel);
			nlWrite(message, serial, estimatedDuration);
			nlWrite(message, serial, subscriptionSlot);
			nlWrite(message, serial, raceFilter);
			nlWrite(message, serial, religionFilter);
			nlWrite(message, serial, guildFilter);
			nlWrite(message, serial, shardFilter);
			nlWrite(message, serial, levelFilter);
			nlWrite(message, serial, subscriptionClosed);
			nlWrite(message, serial, autoInvite);
			nlWrite(message, serial, const_cast < std::string& > (language));
			nlWrite(message, serial, orientation);

			_CallbackClient->send(message);
		}
		// Cancel a plannified session
		// Return 'invokeResult' : 0 : ok, session canceled
		//                         1 : unknown char
		//                         2 : unknown session
		//                         3 : char don't own the session
		//                         4 : session not in planned state

		void cancelSession(uint32 charId, TSessionId sessionId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::cancelSession called");
#endif
			NLNET::CMessage message("CANSS");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);

			_CallbackClient->send(message);
		}
		// start a planned session
		// Return 'invokeResult' : 0 : ok, session started
		//                         1 : char not found
		//                         2 : session not found
		//                         3 : session not own by user
		//                         4 : user is already have a running session of this type
		//                         5 : session server failure
		//                         6 : Action fordidden for free trial
		//                         7 : User not found

		void startSession(uint32 charId, TSessionId sessionId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::startSession called");
#endif
			NLNET::CMessage message("STSS");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);

			_CallbackClient->send(message);
		}
		// Close a running session
		// Return 'invokeResult' : 0 : ok, session closed (or about to close)
		//                         1 : session not found
		//                         2 : char don't own the session
		//                         3 : session not open
		//                         4 : failed to close the session, internal error

		void closeSession(uint32 charId, TSessionId sessionId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::closeSession called");
#endif
			NLNET::CMessage message("CLSS");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);

			_CallbackClient->send(message);
		}
		// Close the current edit session of the character
		// Return 'invokeResult' : 0 : ok, session closed (or about to close)
		//                         1 : char not found
		//                         2 : failed to close the session, internal error
		//                         3 : no session server to close the session
		//                         4 : no edit session found
		//                         5 : internal error

		void closeEditSession(uint32 charId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::closeEditSession called");
#endif
			NLNET::CMessage message("CLESS");
			nlWrite(message, serial, charId);

			_CallbackClient->send(message);
		}
		// Add a character in a user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char already friend

		void addFriendCharacter(uint32 userId, uint32 friendCharId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::addFriendCharacter called");
#endif
			NLNET::CMessage message("AFC");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, friendCharId);

			_CallbackClient->send(message);
		}
		// Repove a character from a user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char not friend

		void removeFriendCharacter(uint32 userId, uint32 friendCharId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::removeFriendCharacter called");
#endif
			NLNET::CMessage message("RFC");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, friendCharId);

			_CallbackClient->send(message);
		}
		// Add a character to a user ban list. This ban the user that own the character
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : banned char not found
		//                         3 : char already banned by user

		void addBannedCharacter(uint32 userId, uint32 bannedCharId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::addBannedCharacter called");
#endif
			NLNET::CMessage message("ABC");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, bannedCharId);

			_CallbackClient->send(message);
		}
		// Remove a character from a user ban list.
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : banned char not found
		//                         3 : char not banned by user

		void removeBannedCharacter(uint32 userId, uint32 bannedCharId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::removeBannedCharacter called");
#endif
			NLNET::CMessage message("RBC");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, bannedCharId);

			_CallbackClient->send(message);
		}
		// Add a character in a DM user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char already DM friend

		void addFriendDMCharacter(uint32 userId, uint32 friendDMCharId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::addFriendDMCharacter called");
#endif
			NLNET::CMessage message("AFDC");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, friendDMCharId);

			_CallbackClient->send(message);
		}
		// Remove a character from a DM user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char not friend

		void removeFriendDMCharacter(uint32 userId, uint32 friendDMCharId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::removeFriendDMCharacter called");
#endif
			NLNET::CMessage message("RFDC");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, friendDMCharId);

			_CallbackClient->send(message);
		}
		// Set the comment associated to a known character entry
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : known character entry not found
		//                         3 : character relation don't match the set comments relation
		//                         4 : internal error

		void setKnownCharacterComments(uint32 userId, uint32 charId, const std::string &relation, const std::string &comments)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::setKnownCharacterComments called");
#endif
			NLNET::CMessage message("SKCC");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, charId);
			nlWrite(message, serial, const_cast < std::string& > (relation));
			nlWrite(message, serial, const_cast < std::string& > (comments));

			_CallbackClient->send(message);
		}
		// A user invite a character to help or play in his session
		// charRole is from enum TSessionPartStatus
		// invokeReturn : 0 : ok, character invited
		//                1 : char not found
		//                2 : session not found
		//                3 : invited char not found
		//                4 : char not own the session
		//                5 : char already invited
		//                6 : char role and session type don't match (edit/editor, anim/animator)
		//                7 : charRole is invalid (must be sps_play_invited, sps_edit_invited or sps_anim_invited)
		//                8 : database failure
		//               11 : owner char is not animator in the session
		//               12 : newcomer flag missmatch
		//               13 : scenario not started, can't validate invitation now
		//               14 : free trial character are not allowed in user scenario

		void inviteCharacter(uint32 ownerCharId, TSessionId sessionId, uint32 invitedCharId, const TSessionPartStatus &charRole)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::inviteCharacter called");
#endif
			NLNET::CMessage message("IC");
			nlWrite(message, serial, ownerCharId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, invitedCharId);
			nlWrite(message, serial, charRole);

			_CallbackClient->send(message);
		}
		// A user remove an invitation in a session
		// invokeReturn : 0 : ok, character invited
		//                1 : removed char not found
		//                2 : session not found
		//                3 : character already entered in session
		//                4 : invitation not found
		//                5 : owner char don't own the session

		void removeInvitedCharacter(uint32 ownerCharId, TSessionId sessionId, uint32 removedCharId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::removeInvitedCharacter called");
#endif
			NLNET::CMessage message("RIC");
			nlWrite(message, serial, ownerCharId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, removedCharId);

			_CallbackClient->send(message);
		}
		// A character subscribe to a public animation session
		// invokeReturn : 0 : ok, subscription accepted
		//                1 : char not found
		//                2 : session not found
		//                3 : character already subscribed to or invited in the session
		//                4 : session not public
		//                5 : character banned
		//                6 : no place left, session is full
		//                7 : session owner not found
		//                8 : internal error

		void subscribeSession(uint32 charId, TSessionId sessionId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::subscribeSession called");
#endif
			NLNET::CMessage message("SBS");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);

			_CallbackClient->send(message);
		}
		// A character unsubscribe to a public animation session
		// The character must not join the session in order to unsubscribe
		// invokeReturn : 0 : ok, unsubscription accepted
		//                1 : char not found
		//                2 : session not found
		//                3 : character has not subscribed in the session

		void unsubscribeSession(uint32 charId, TSessionId sessionId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::unsubscribeSession called");
#endif
			NLNET::CMessage message("USBS");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);

			_CallbackClient->send(message);
		}
		// A character asks to join (or enter) a running session.
		// It must have been subscribed or invited to the session to be allowed

		void joinSession(uint32 charId, TSessionId sessionId, const std::string &clientApplication)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::joinSession called");
#endif
			NLNET::CMessage message("JSS");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, const_cast < std::string& > (clientApplication));

			_CallbackClient->send(message);
		}
		// A character asks to join a shard.
		// charId: userId << 4 & charSlot
		// If ingame, charSlot in [0..14], otherwise 15 if outgame
		// The actual shard will be chosen according to current load.
		// Will return a joinSessionResult.

		void joinMainland(uint32 charId, const std::string &clientApplication)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::joinMainland called");
#endif
			NLNET::CMessage message("JML");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, const_cast < std::string& > (clientApplication));

			_CallbackClient->send(message);
		}
		// Ask to join the edit session for the specified character.
		// If the edit session do not exist, then the SU
		// create it before internally calling the join session request
		// the SU.
		// Return joinSessionResult.

		void joinEditSession(uint32 charId, const std::string &clientApplication)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::joinEditSession called");
#endif
			NLNET::CMessage message("JES");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, const_cast < std::string& > (clientApplication));

			_CallbackClient->send(message);
		}
		// hibernate an editing session
		// Return 'invokeResult' : 0 : ok, session hibernating(or no session)
		//                         1 : undefined error

		void hibernateEditSession(uint32 charId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::hibernateEditSession called");
#endif
			NLNET::CMessage message("HES");
			nlWrite(message, serial, charId);

			_CallbackClient->send(message);
		}
		// Request to have the list of accessible shards with their attributes.
		// This is a dev feature only.

		void getShards(uint32 charId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::getShards called");
#endif
			NLNET::CMessage message("GSH");
			nlWrite(message, serial, charId);

			_CallbackClient->send(message);
		}
		// Kick a character from a session
		// charId must be the owner of a DM in the session
		// invokeReturn : 0 : ok, character kicked
		//                1 : char not found
		//                2 : session not found
		//                3 : kicked character has no participation in the session
		//                4 : internal error
		//                5 : owner char don't own the session

		void kickCharacter(uint32 ownerCharId, TSessionId sessionId, uint32 kickedCharId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::kickCharacter called");
#endif
			NLNET::CMessage message("KC");
			nlWrite(message, serial, ownerCharId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, kickedCharId);

			_CallbackClient->send(message);
		}
		// Unkick a character from a session
		// charId must be the owner of a DM in the session
		// invokeReturn : 0 : ok, character kicked
		//                1 : char not found
		//                2 : session not found
		//                3 : kicked character has no participation in the session
		//                4 : internal error
		//                5 : owner char don't own the session

		void unkickCharacter(uint32 ownerCharId, TSessionId sessionId, uint32 unkickedCharId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::unkickCharacter called");
#endif
			NLNET::CMessage message("UKC");
			nlWrite(message, serial, ownerCharId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, unkickedCharId);

			_CallbackClient->send(message);
		}
		// A user invite a guild to play in a session
		// invokeReturn : 0 : ok, guild invited
		//                1 : guild not found
		//                2 : char not found
		//                3 : session not found
		//                4 : guild already invited
		//                5 : char don't own the session

		void inviteGuild(uint32 charId, TSessionId sessionId, uint32 guildId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::inviteGuild called");
#endif
			NLNET::CMessage message("IG");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, guildId);

			_CallbackClient->send(message);
		}
		// Remove a guild invitation in a session
		// invokeReturn : 0 : ok, guild invited
		//                1 : guild not found
		//                2 : char not found
		//                3 : session not found
		//                4 : guild not invited
		//                5 : char don't own the session

		void removeInvitedGuild(uint32 charId, TSessionId sessionId, uint32 guildId)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::removeInvitedGuild called");
#endif
			NLNET::CMessage message("RIG");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, guildId);

			_CallbackClient->send(message);
		}
		// Set the additionnal scenario info
		// playType is the enumerated type TPlayType
		// invokeReturn : 0 : ok, info setted
		//                1 : scenario not found
		//                2 : char not owner of session
		//                3 : char not found

		void setScenarioInfo(uint32 charId, TSessionId sessionId, const std::string &title, uint32 numPlayer, const std::string &playType)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::setScenarioInfo called");
#endif
			NLNET::CMessage message("SSCI");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, const_cast < std::string& > (title));
			nlWrite(message, serial, numPlayer);
			nlWrite(message, serial, const_cast < std::string& > (playType));

			_CallbackClient->send(message);
		}
		// Add an entry in the session journal
		// invokeReturn : 0 : ok, entry added
		//                1 : scenario not found
		//                2 : user can't post in this journal

		void addJournalEntry(uint32 charId, TSessionId sessionId, const std::string &entryType, const std::string &text)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::addJournalEntry called");
#endif
			NLNET::CMessage message("AJE");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, const_cast < std::string& > (entryType));
			nlWrite(message, serial, const_cast < std::string& > (text));

			_CallbackClient->send(message);
		}
		// Set the rating for a scenario
		// invokeReturn : 0 : ok, rating set
		//                1 : scenario not found
		//                2 : char is not found
		//                3 : char is not a participant of session
		//                4 : no session log for the session
		//                5 : char is banned from session
		//                6 : session not found
		//                7 : scenario not found
		//                8 : internal error

		void setPlayerRating(uint32 charId, TSessionId sessionId, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::setPlayerRating called");
#endif
			NLNET::CMessage message("SPR");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serial, rateFun);
			nlWrite(message, serial, rateDifficulty);
			nlWrite(message, serial, rateAccessibility);
			nlWrite(message, serial, rateOriginality);
			nlWrite(message, serial, rateDirection);

			_CallbackClient->send(message);
		}

		static void cb_invokeResult (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::cb_invokeResult received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebClientItf *callback = (CRingSessionManagerWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			uint32	resultCode;
			std::string	resultString;
			nlRead(message, serial, userId);
			nlRead(message, serial, resultCode);
			nlRead(message, serial, resultString);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::cb_invokeResult : calling on_invokeResult");
#endif

			callback->on_invokeResult(from, userId, resultCode, resultString);
		}

		static void cb_scheduleSessionResult (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::cb_scheduleSessionResult received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebClientItf *callback = (CRingSessionManagerWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			uint8	result;
			std::string	resultString;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, result);
			nlRead(message, serial, resultString);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::cb_scheduleSessionResult : calling on_scheduleSessionResult");
#endif

			callback->on_scheduleSessionResult(from, charId, sessionId, result, resultString);
		}

		static void cb_sessionInfoResult (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::cb_sessionInfoResult received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebClientItf *callback = (CRingSessionManagerWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			TRaceFilter	raceFilter;
			TReligionFilter	religionFilter;
			TGuildFilter	guildFilter;
			TShardFilter	shardFilter;
			TLevelFilter	levelFilter;
			bool	subscriptionClosed;
			bool	autoInvite;
			std::string	language;
			TSessionOrientation	orientation;
			std::string	description;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, raceFilter);
			nlRead(message, serial, religionFilter);
			nlRead(message, serial, guildFilter);
			nlRead(message, serial, shardFilter);
			nlRead(message, serial, levelFilter);
			nlRead(message, serial, subscriptionClosed);
			nlRead(message, serial, autoInvite);
			nlRead(message, serial, language);
			nlRead(message, serial, orientation);
			nlRead(message, serial, description);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::cb_sessionInfoResult : calling on_sessionInfoResult");
#endif

			callback->on_sessionInfoResult(from, charId, sessionId, raceFilter, religionFilter, guildFilter, shardFilter, levelFilter, subscriptionClosed, autoInvite, language, orientation, description);
		}

		static void cb_joinSessionResult (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::cb_joinSessionResult received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebClientItf *callback = (CRingSessionManagerWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			TSessionId	sessionId;
			uint32	result;
			std::string	shardAddr;
			TSessionPartStatus	participantStatus;
			nlRead(message, serial, userId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, result);
			nlRead(message, serial, shardAddr);
			nlRead(message, serial, participantStatus);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::cb_joinSessionResult : calling on_joinSessionResult");
#endif

			callback->on_joinSessionResult(from, userId, sessionId, result, shardAddr, participantStatus);
		}

		static void cb_joinSessionResultExt (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::cb_joinSessionResultExt received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebClientItf *callback = (CRingSessionManagerWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			TSessionId	sessionId;
			uint32	result;
			std::string	shardAddr;
			TSessionPartStatus	participantStatus;
			CSecurityCode	securityCheckForFastDisconnection;
			nlRead(message, serial, userId);
			nlRead(message, serial, sessionId);
			nlRead(message, serial, result);
			nlRead(message, serial, shardAddr);
			nlRead(message, serial, participantStatus);
			nlRead(message, serial, securityCheckForFastDisconnection);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::cb_joinSessionResultExt : calling on_joinSessionResultExt");
#endif

			callback->on_joinSessionResultExt(from, userId, sessionId, result, shardAddr, participantStatus, securityCheckForFastDisconnection);
		}

		static void cb_getShardsResult (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::cb_getShardsResult received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CRingSessionManagerWebClientItf *callback = (CRingSessionManagerWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			std::string	result;
			nlRead(message, serial, userId);
			nlRead(message, serial, result);


#ifdef NL_DEBUG
			nldebug("CRingSessionManagerWebClient::cb_getShardsResult : calling on_getShardsResult");
#endif

			callback->on_getShardsResult(from, userId, result);
		}


		/// Disconnection callback : the connection to the server is lost
		virtual void on_CRingSessionManagerWebClient_Disconnection(NLNET::TSockId from) =0;


		// Generic response to invoke.
		// result contains 0 if no error, more than 0 in case of error
		virtual void on_invokeResult(NLNET::TSockId from, uint32 userId, uint32 resultCode, const std::string &resultString) =0;

		// result is : 0 : session have been created fine
		//             1 : invalid session type
		//             2 : invalid level
		//             3 : unknown character
		//             4 : not used
		//             5 : invalid access type
		//             6 : invalid rule type
		//             7 : invalid duration
		//             8 : invalid user
		//             9 : free trial account can't create anim session
		//             10 : user is ban from ring anim session
		virtual void on_scheduleSessionResult(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint8 result, const std::string &resultString) =0;

		// session info result (anim)
		virtual void on_sessionInfoResult(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const TRaceFilter &raceFilter, const TReligionFilter &religionFilter, const TGuildFilter &guildFilter, const TShardFilter &shardFilter, const TLevelFilter &levelFilter, bool subscriptionClosed, bool autoInvite, const std::string &language, const TSessionOrientation &orientation, const std::string &description) =0;

		// Return the result of the session joining attempt
		// If join is ok, the shardAddr contain <ip:port> of the
		// Front end that waits for the player to come in and the.
		// participation mode for the character (editor, animator or player).
		// If ok, the web must return a page with a lua script.
		// that trigger the action handler 'on_connect_to_shard' :
		// <lua>runAH(nul, "on_connect_to_shard", "cookie=cookieValue|fsAddr=shardAddr|mode=participantStatus");<lua>
		// result : 0 : ok the client can join the session
		//          1 : char not found
		//          2 : session not found
		//          3 : no session participant for this character (not used for a mainland shard)
		//          4 : can't find session server (not used for a mainland shard)
		//          5 : shard hosting session is not reachable
		//          6 : nel user info not found
		//          7 : ring user not found
		//          8 : welcome service rejected connection request
		//          9 : session service shutdown (not used for a mainland shard)
		//         10 : no mainland shard found (joinMainland only)
		//         11 : internal error
		//         12 : failed to request for access permission
		//         13 : can't find access permission for user and domain
		//         14 : Welcome service is closed for you
		//         15 : Session is not open
		//         16 : User banned from ring
		//         17 : Newcomer flag missmatch
		//         18 : Can't find session log to validate session access
		//         19 : Can't find scenario info to validate session access
		//         20 : Scenario is not allowed to free trial players
		virtual void on_joinSessionResult(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 result, const std::string &shardAddr, const TSessionPartStatus &participantStatus) =0;

		// See joinSessionResult.
		// Adds a security code.
		virtual void on_joinSessionResultExt(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 result, const std::string &shardAddr, const TSessionPartStatus &participantStatus, const CSecurityCode &securityCheckForFastDisconnection) =0;

		// Return the list of online shards on which the user is allowed to connect,
		// and their current dynamic attributes. Other attributes (e.g. names)
		// can be queried from the database. Offline shards are the ones in the database
		// of the same domain but not listed in the result.
		// Then the client will have to call joinShard to connect on an online shard.
		virtual void on_getShardsResult(NLNET::TSockId from, uint32 userId, const std::string &result) =0;

	};
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TSessionDesc
	{
	protected:
		//
		TSessionId	_SessionId;
		//
		bool	_RequesterCharInvited;
		//
		bool	_RequesterCharKicked;
		//
		std::string	_OwnerName;
		//
		std::string	_Title;
		//
		std::string	_Description;
		//
		TAnimMode	_AnimMode;
		//
		R2::TSessionLevel	_SessionLevel;
		//
		bool	_AllowFreeTrial;
		//
		uint32	_LaunchDate;
		//
		uint32	_NbConnectedPlayer;
		//
		std::string	_Language;
		//
		TSessionOrientation	_Orientation;
		//
		uint32	_NbRating;
		//
		uint32	_RateFun;
		//
		uint32	_RateDifficulty;
		//
		uint32	_RateAccessibility;
		//
		uint32	_RateOriginality;
		//
		uint32	_RateDirection;
		//
		uint32	_ScenarioRRPTotal;
	public:
		//
		TSessionId getSessionId() const
		{
			return _SessionId;
		}

		void setSessionId(TSessionId value)
		{

				_SessionId = value;

		}
			//
		bool getRequesterCharInvited() const
		{
			return _RequesterCharInvited;
		}

		void setRequesterCharInvited(bool value)
		{

				_RequesterCharInvited = value;

		}
			//
		bool getRequesterCharKicked() const
		{
			return _RequesterCharKicked;
		}

		void setRequesterCharKicked(bool value)
		{

				_RequesterCharKicked = value;

		}
			//
		const std::string &getOwnerName() const
		{
			return _OwnerName;
		}

		std::string &getOwnerName()
		{
			return _OwnerName;
		}


		void setOwnerName(const std::string &value)
		{


				_OwnerName = value;


		}
			//
		const std::string &getTitle() const
		{
			return _Title;
		}

		std::string &getTitle()
		{
			return _Title;
		}


		void setTitle(const std::string &value)
		{


				_Title = value;


		}
			//
		const std::string &getDescription() const
		{
			return _Description;
		}

		std::string &getDescription()
		{
			return _Description;
		}


		void setDescription(const std::string &value)
		{


				_Description = value;


		}
			//
		TAnimMode getAnimMode() const
		{
			return _AnimMode;
		}

		void setAnimMode(TAnimMode value)
		{

				_AnimMode = value;

		}
			//
		R2::TSessionLevel getSessionLevel() const
		{
			return _SessionLevel;
		}

		void setSessionLevel(R2::TSessionLevel value)
		{

				_SessionLevel = value;

		}
			//
		bool getAllowFreeTrial() const
		{
			return _AllowFreeTrial;
		}

		void setAllowFreeTrial(bool value)
		{

				_AllowFreeTrial = value;

		}
			//
		uint32 getLaunchDate() const
		{
			return _LaunchDate;
		}

		void setLaunchDate(uint32 value)
		{

				_LaunchDate = value;

		}
			//
		uint32 getNbConnectedPlayer() const
		{
			return _NbConnectedPlayer;
		}

		void setNbConnectedPlayer(uint32 value)
		{

				_NbConnectedPlayer = value;

		}
			//
		const std::string &getLanguage() const
		{
			return _Language;
		}

		std::string &getLanguage()
		{
			return _Language;
		}


		void setLanguage(const std::string &value)
		{


				_Language = value;


		}
			//
		TSessionOrientation getOrientation() const
		{
			return _Orientation;
		}

		void setOrientation(TSessionOrientation value)
		{

				_Orientation = value;

		}
			//
		uint32 getNbRating() const
		{
			return _NbRating;
		}

		void setNbRating(uint32 value)
		{

				_NbRating = value;

		}
			//
		uint32 getRateFun() const
		{
			return _RateFun;
		}

		void setRateFun(uint32 value)
		{

				_RateFun = value;

		}
			//
		uint32 getRateDifficulty() const
		{
			return _RateDifficulty;
		}

		void setRateDifficulty(uint32 value)
		{

				_RateDifficulty = value;

		}
			//
		uint32 getRateAccessibility() const
		{
			return _RateAccessibility;
		}

		void setRateAccessibility(uint32 value)
		{

				_RateAccessibility = value;

		}
			//
		uint32 getRateOriginality() const
		{
			return _RateOriginality;
		}

		void setRateOriginality(uint32 value)
		{

				_RateOriginality = value;

		}
			//
		uint32 getRateDirection() const
		{
			return _RateDirection;
		}

		void setRateDirection(uint32 value)
		{

				_RateDirection = value;

		}
			//
		uint32 getScenarioRRPTotal() const
		{
			return _ScenarioRRPTotal;
		}

		void setScenarioRRPTotal(uint32 value)
		{

				_ScenarioRRPTotal = value;

		}

		bool operator == (const TSessionDesc &other) const
		{
			return _SessionId == other._SessionId
				&& _RequesterCharInvited == other._RequesterCharInvited
				&& _RequesterCharKicked == other._RequesterCharKicked
				&& _OwnerName == other._OwnerName
				&& _Title == other._Title
				&& _Description == other._Description
				&& _AnimMode == other._AnimMode
				&& _SessionLevel == other._SessionLevel
				&& _AllowFreeTrial == other._AllowFreeTrial
				&& _LaunchDate == other._LaunchDate
				&& _NbConnectedPlayer == other._NbConnectedPlayer
				&& _Language == other._Language
				&& _Orientation == other._Orientation
				&& _NbRating == other._NbRating
				&& _RateFun == other._RateFun
				&& _RateDifficulty == other._RateDifficulty
				&& _RateAccessibility == other._RateAccessibility
				&& _RateOriginality == other._RateOriginality
				&& _RateDirection == other._RateDirection
				&& _ScenarioRRPTotal == other._ScenarioRRPTotal;
		}


		// constructor
		TSessionDesc()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_SessionId);
			s.serial(_RequesterCharInvited);
			s.serial(_RequesterCharKicked);
			s.serial(_OwnerName);
			s.serial(_Title);
			s.serial(_Description);
			s.serial(_AnimMode);
			s.serial(_SessionLevel);
			s.serial(_AllowFreeTrial);
			s.serial(_LaunchDate);
			s.serial(_NbConnectedPlayer);
			s.serial(_Language);
			s.serial(_Orientation);
			s.serial(_NbRating);
			s.serial(_RateFun);
			s.serial(_RateDifficulty);
			s.serial(_RateAccessibility);
			s.serial(_RateOriginality);
			s.serial(_RateDirection);
			s.serial(_ScenarioRRPTotal);

		}


	private:


	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TCharDesc
	{
	protected:
		//
		uint32	_CharId;
		//
		bool	_Connected;
		//
		bool	_Kicked;
		//
		std::string	_CharName;
		//
		std::string	_GuildName;
		//
		uint32	_ShardId;
		//
		R2::TSessionLevel	_Level;
		//
		CHARSYNC::TRace	_Race;
		//
		CHARSYNC::TCivilisation	_Civilisation;
		//
		CHARSYNC::TCult	_Cult;
		//
		TSessionPartStatus	_PartStatus;
	public:
		//
		uint32 getCharId() const
		{
			return _CharId;
		}

		void setCharId(uint32 value)
		{

				_CharId = value;

		}
			//
		bool getConnected() const
		{
			return _Connected;
		}

		void setConnected(bool value)
		{

				_Connected = value;

		}
			//
		bool getKicked() const
		{
			return _Kicked;
		}

		void setKicked(bool value)
		{

				_Kicked = value;

		}
			//
		const std::string &getCharName() const
		{
			return _CharName;
		}

		std::string &getCharName()
		{
			return _CharName;
		}


		void setCharName(const std::string &value)
		{


				_CharName = value;


		}
			//
		const std::string &getGuildName() const
		{
			return _GuildName;
		}

		std::string &getGuildName()
		{
			return _GuildName;
		}


		void setGuildName(const std::string &value)
		{


				_GuildName = value;


		}
			//
		uint32 getShardId() const
		{
			return _ShardId;
		}

		void setShardId(uint32 value)
		{

				_ShardId = value;

		}
			//
		R2::TSessionLevel getLevel() const
		{
			return _Level;
		}

		void setLevel(R2::TSessionLevel value)
		{

				_Level = value;

		}
			//
		CHARSYNC::TRace getRace() const
		{
			return _Race;
		}

		void setRace(CHARSYNC::TRace value)
		{

				_Race = value;

		}
			//
		CHARSYNC::TCivilisation getCivilisation() const
		{
			return _Civilisation;
		}

		void setCivilisation(CHARSYNC::TCivilisation value)
		{

				_Civilisation = value;

		}
			//
		CHARSYNC::TCult getCult() const
		{
			return _Cult;
		}

		void setCult(CHARSYNC::TCult value)
		{

				_Cult = value;

		}
			//
		TSessionPartStatus getPartStatus() const
		{
			return _PartStatus;
		}

		void setPartStatus(TSessionPartStatus value)
		{

				_PartStatus = value;

		}

		bool operator == (const TCharDesc &other) const
		{
			return _CharId == other._CharId
				&& _Connected == other._Connected
				&& _Kicked == other._Kicked
				&& _CharName == other._CharName
				&& _GuildName == other._GuildName
				&& _ShardId == other._ShardId
				&& _Level == other._Level
				&& _Race == other._Race
				&& _Civilisation == other._Civilisation
				&& _Cult == other._Cult
				&& _PartStatus == other._PartStatus;
		}


		// constructor
		TCharDesc()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_CharId);
			s.serial(_Connected);
			s.serial(_Kicked);
			s.serial(_CharName);
			s.serial(_GuildName);
			s.serial(_ShardId);
			s.serial(_Level);
			s.serial(_Race);
			s.serial(_Civilisation);
			s.serial(_Cult);
			s.serial(_PartStatus);

		}


	private:


	};


		// Callback interface used by client to request session info

	class CSessionBrowserServerWebItf : public CRingSessionManagerWebItf

	{
	protected:

		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{
			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"AUTH",	CSessionBrowserServerWebItf::cb_authenticate	},
				{	"GSL",	CSessionBrowserServerWebItf::cb_getSessionList	},
				{	"GCL",	CSessionBrowserServerWebItf::cb_getCharList	},
				{	"ICBN",	CSessionBrowserServerWebItf::cb_inviteCharacterByName	},
				{	"GMSR",	CSessionBrowserServerWebItf::cb_getMyRatings	},
				{	"GSAS",	CSessionBrowserServerWebItf::cb_getSessionAverageScores	},
				{	"GSCAS",	CSessionBrowserServerWebItf::cb_getScenarioAverageScores	},
				{	"GRR",	CSessionBrowserServerWebItf::cb_getRingRatings	},
				{	"GRP",	CSessionBrowserServerWebItf::cb_getRingPoints	},
				{	"DSS_FW",	CSessionBrowserServerWebItf::cb_forwardToDss	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);
		}

		static void _cbConnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CSessionBrowserServerWeb__cbConnection);
			CSessionBrowserServerWebItf *_this = reinterpret_cast<CSessionBrowserServerWebItf *>(arg);

			_this->on_CSessionBrowserServerWeb_Connection(from);
		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(CSessionBrowserServerWeb__cbDisconnection);
			CSessionBrowserServerWebItf *_this = reinterpret_cast<CSessionBrowserServerWebItf *>(arg);

			_this->on_CSessionBrowserServerWeb_Disconnection(from);
		}


	public:
		/** Constructor, if you specify a replacement adaptor, then the object
		 *	become owner of the adaptor (and it will be released with the
		 *	interface).
		 */
		CSessionBrowserServerWebItf(ICallbackServerAdaptor *replacementAdaptor = NULL)
			:	CRingSessionManagerWebItf(replacementAdaptor)
		{}

		virtual ~CSessionBrowserServerWebItf()
		{
		}

		/// Open the interface socket in the specified port
		void openItf(uint16 port)
		{
			NLNET::TCallbackItem *arrayPtr;
			uint32 arraySize;


			// add callback array of the base interface class
			CRingSessionManagerWebItf::getCallbakArray(arrayPtr, arraySize);
			_CallbackServer->addCallbackArray(arrayPtr, arraySize);


			getCallbakArray(arrayPtr, arraySize);
			_CallbackServer->addCallbackArray(arrayPtr, arraySize);

			_CallbackServer->setConnectionCallback (_cbConnection, this);
			_CallbackServer->setDisconnectionCallback (_cbDisconnection, this);

			_CallbackServer->init(port);
		}

		/** Must be called evenly, update the network subclass to receive message
		 *	and dispatch method invokation.
		 */
		void update()
		{
			H_AUTO(CSessionBrowserServerWeb_update);

			try
			{
				_CallbackServer->update();
			}
			catch (...)
			{
				nlwarning("CSessionBrowserServerWeb : Exception launch in callback server update");
			}
		}

		// Return the list of available session

		void sessionList(NLNET::TSockId dest, uint32 charId, const std::vector < TSessionDesc > &sessions)
		{
			H_AUTO(sessionList_sessionList);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::sessionList called");
#endif
			NLNET::CMessage message("SL");
			nlWrite(message, serial, charId);
			nlWrite(message, serialCont, const_cast < std::vector < TSessionDesc >& > (sessions));

			_CallbackServer->send(message, dest);
		}
		// Return the list of player characters in the session

		void charList(NLNET::TSockId dest, uint32 charId, TSessionId sessionId, const std::vector < TCharDesc > &characters)
		{
			H_AUTO(charList_charList);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::charList called");
#endif
			NLNET::CMessage message("CL");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);
			nlWrite(message, serialCont, const_cast < std::vector < TCharDesc >& > (characters));

			_CallbackServer->send(message, dest);
		}
		// Return current player rating of the current session scenario

		void playerRatings(NLNET::TSockId dest, uint32 charId, bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection)
		{
			H_AUTO(playerRatings_playerRatings);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::playerRatings called");
#endif
			NLNET::CMessage message("PR");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, scenarioRated);
			nlWrite(message, serial, rateFun);
			nlWrite(message, serial, rateDifficulty);
			nlWrite(message, serial, rateAccessibility);
			nlWrite(message, serial, rateOriginality);
			nlWrite(message, serial, rateDirection);

			_CallbackServer->send(message, dest);
		}
		// Return average scores of a session

		void sessionAverageScores(NLNET::TSockId dest, bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection, uint32 rrpTotal)
		{
			H_AUTO(sessionAverageScores_sessionAverageScores);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::sessionAverageScores called");
#endif
			NLNET::CMessage message("SAS");
			nlWrite(message, serial, scenarioRated);
			nlWrite(message, serial, rateFun);
			nlWrite(message, serial, rateDifficulty);
			nlWrite(message, serial, rateAccessibility);
			nlWrite(message, serial, rateOriginality);
			nlWrite(message, serial, rateDirection);
			nlWrite(message, serial, rrpTotal);

			_CallbackServer->send(message, dest);
		}
		// Return average scores of a scenario

		void scenarioAverageScores(NLNET::TSockId dest, bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection, uint32 rrpTotal)
		{
			H_AUTO(scenarioAverageScores_scenarioAverageScores);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::scenarioAverageScores called");
#endif
			NLNET::CMessage message("SCAS");
			nlWrite(message, serial, scenarioRated);
			nlWrite(message, serial, rateFun);
			nlWrite(message, serial, rateDifficulty);
			nlWrite(message, serial, rateAccessibility);
			nlWrite(message, serial, rateOriginality);
			nlWrite(message, serial, rateDirection);
			nlWrite(message, serial, rrpTotal);

			_CallbackServer->send(message, dest);
		}
		// Return the author rating, the AM rating and the Masterless rating

		void ringRatings(NLNET::TSockId dest, uint32 charId, uint32 authorRating, uint32 AMRating, uint32 masterlessRating)
		{
			H_AUTO(ringRatings_ringRatings);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::ringRatings called");
#endif
			NLNET::CMessage message("RR");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, authorRating);
			nlWrite(message, serial, AMRating);
			nlWrite(message, serial, masterlessRating);

			_CallbackServer->send(message, dest);
		}
		// Return the ring points of the character

		void ringPoints(NLNET::TSockId dest, uint32 charId, const std::string &ringPoints, const std::string &maxRingPoints)
		{
			H_AUTO(ringPoints_ringPoints);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::ringPoints called");
#endif
			NLNET::CMessage message("RP");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, const_cast < std::string& > (ringPoints));
			nlWrite(message, serial, const_cast < std::string& > (maxRingPoints));

			_CallbackServer->send(message, dest);
		}

		static void cb_authenticate (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(authenticate_on_authenticate);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_authenticate received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebItf *callback = (CSessionBrowserServerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	userId;
			NLNET::CLoginCookie	cookie;
			nlRead(message, serial, userId);
			nlRead(message, serial, cookie);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_authenticate : calling on_authenticate");
#endif


			callback->on_authenticate(from, userId, cookie);

		}

		static void cb_getSessionList (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(getSessionList_on_getSessionList);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getSessionList received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebItf *callback = (CSessionBrowserServerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			nlRead(message, serial, charId);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getSessionList : calling on_getSessionList");
#endif


			callback->on_getSessionList(from, charId);

		}

		static void cb_getCharList (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(getCharList_on_getCharList);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getCharList received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebItf *callback = (CSessionBrowserServerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getCharList : calling on_getCharList");
#endif


			callback->on_getCharList(from, charId, sessionId);

		}

		static void cb_inviteCharacterByName (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(inviteCharacterByName_on_inviteCharacterByName);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_inviteCharacterByName received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebItf *callback = (CSessionBrowserServerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			std::string	invitedCharName;
			nlRead(message, serial, charId);
			nlRead(message, serial, invitedCharName);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_inviteCharacterByName : calling on_inviteCharacterByName");
#endif


			callback->on_inviteCharacterByName(from, charId, invitedCharName);

		}

		static void cb_getMyRatings (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(getMyRatings_on_getMyRatings);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getMyRatings received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebItf *callback = (CSessionBrowserServerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			uint32	sessionId;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getMyRatings : calling on_getMyRatings");
#endif


			callback->on_getMyRatings(from, charId, sessionId);

		}

		static void cb_getSessionAverageScores (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(getSessionAverageScores_on_getSessionAverageScores);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getSessionAverageScores received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebItf *callback = (CSessionBrowserServerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	sessionId;
			nlRead(message, serial, sessionId);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getSessionAverageScores : calling on_getSessionAverageScores");
#endif


			callback->on_getSessionAverageScores(from, sessionId);

		}

		static void cb_getScenarioAverageScores (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(getScenarioAverageScores_on_getScenarioAverageScores);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getScenarioAverageScores received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebItf *callback = (CSessionBrowserServerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			std::string	md5;
			nlRead(message, serial, md5);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getScenarioAverageScores : calling on_getScenarioAverageScores");
#endif


			callback->on_getScenarioAverageScores(from, md5);

		}

		static void cb_getRingRatings (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(getRingRatings_on_getRingRatings);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getRingRatings received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebItf *callback = (CSessionBrowserServerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			nlRead(message, serial, charId);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getRingRatings : calling on_getRingRatings");
#endif


			callback->on_getRingRatings(from, charId);

		}

		static void cb_getRingPoints (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(getRingPoints_on_getRingPoints);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getRingPoints received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebItf *callback = (CSessionBrowserServerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			nlRead(message, serial, charId);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_getRingPoints : calling on_getRingPoints");
#endif


			callback->on_getRingPoints(from, charId);

		}

		static void cb_forwardToDss (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
			H_AUTO(forwardToDss_on_forwardToDss);
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_forwardToDss received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast< ICallbackServerAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebItf *callback = (CSessionBrowserServerWebItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			NLNET::CMessage	msg;
			nlRead(message, serial, charId);
			nlRead(message, serialMessage, msg);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWeb::cb_forwardToDss : calling on_forwardToDss");
#endif


			callback->on_forwardToDss(from, charId, msg);

		}


		/// Connection callback : a new interface client connect
		virtual void on_CSessionBrowserServerWeb_Connection(NLNET::TSockId from) =0;
		/// Disconnection callback : one of the interface client disconnect
		virtual void on_CSessionBrowserServerWeb_Disconnection(NLNET::TSockId from) =0;


		// The client send it's cookie information to
		// authenticate himself.
		// The cookie value is checked against the value stored.
		// in the database.
		// Furthermore, the server will check
		// evenly the database to see if the user is still
		// online and still have the same cookie
		virtual void on_authenticate(NLNET::TSockId from, uint32 userId, const NLNET::CLoginCookie &cookie) =0;

		// Ask for the list of session that are available
		// for the requesting character.
		virtual void on_getSessionList(NLNET::TSockId from, uint32 charId) =0;

		// Ask for the list of player characters that are available
		// for the requesting session.
		virtual void on_getCharList(NLNET::TSockId from, uint32 charId, TSessionId sessionId) =0;

		// Invite a player in a session given it's name
		// This method make a certain number of asumption :
		// The sessionId is deducted from the current session id of
		// the requester character.
		// The invited char id is deducted from the name by using.
		// the full name rules for shard resolution.
		// Return invoke_result with the following error codes :
		//   0   : no error
		//   100 : unknown onwer char
		//   101 : player already invited
		//   102 : no current session
		//   103 : internal error
		//   104 : invited char not found
		//   plus all the error code from inviteCharacter in the ring session manager interface
		virtual void on_inviteCharacterByName(NLNET::TSockId from, uint32 charId, std::string invitedCharName) =0;

		// Ask for character existing rating for the current session scenario
		// return playerRatings.
		virtual void on_getMyRatings(NLNET::TSockId from, uint32 charId, uint32 sessionId) =0;

		// Ask for average scores of a session
		// return scessionAverageScores.
		virtual void on_getSessionAverageScores(NLNET::TSockId from, uint32 sessionId) =0;

		// Ask for average scores of a scenario
		// return scenarioAverageScores.
		virtual void on_getScenarioAverageScores(NLNET::TSockId from, const std::string &md5) =0;

		// Ask for the author rating, the AM rating and the Masterless rating
		// for the requesting character.
		virtual void on_getRingRatings(NLNET::TSockId from, uint32 charId) =0;

		// Ask for ring points of the character
		virtual void on_getRingPoints(NLNET::TSockId from, uint32 charId) =0;

		// simulate the forward of a message (to dss)
		virtual void on_forwardToDss(NLNET::TSockId from, uint32 charId, const NLNET::CMessage &msg) =0;

	};

		// Callback interface used by client to request session info

	/** This is the client side of the interface
	 *	Derive from this class to invoke method on the callback server
	 */

	class CSessionBrowserServerWebClientItf : public CRingSessionManagerWebClientItf

	{
	protected:


		void getCallbakArray(NLNET::TCallbackItem *&arrayPtr, uint32 &arraySize)
		{

			static NLNET::TCallbackItem callbackArray[] =
			{
				{	"SL",	CSessionBrowserServerWebClientItf::cb_sessionList	},
				{	"CL",	CSessionBrowserServerWebClientItf::cb_charList	},
				{	"PR",	CSessionBrowserServerWebClientItf::cb_playerRatings	},
				{	"SAS",	CSessionBrowserServerWebClientItf::cb_sessionAverageScores	},
				{	"SCAS",	CSessionBrowserServerWebClientItf::cb_scenarioAverageScores	},
				{	"RR",	CSessionBrowserServerWebClientItf::cb_ringRatings	},
				{	"RP",	CSessionBrowserServerWebClientItf::cb_ringPoints	},

			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);

		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			CSessionBrowserServerWebClientItf *_this = reinterpret_cast<CSessionBrowserServerWebClientItf *>(arg);

			_this->on_CSessionBrowserServerWebClient_Disconnection(from);
		}


	public:
		/// Retreive the message name for a given callback name
		static const std::string &getMessageName(const std::string &methodName)
		{
			static std::map<std::string, std::string> messageNames;
			static bool initialized = false;
			if (!initialized)
			{
			messageNames.insert(std::make_pair(std::string("on_sessionList"), std::string("SL")));
			messageNames.insert(std::make_pair(std::string("on_charList"), std::string("CL")));
			messageNames.insert(std::make_pair(std::string("on_playerRatings"), std::string("PR")));
			messageNames.insert(std::make_pair(std::string("on_sessionAverageScores"), std::string("SAS")));
			messageNames.insert(std::make_pair(std::string("on_scenarioAverageScores"), std::string("SCAS")));
			messageNames.insert(std::make_pair(std::string("on_ringRatings"), std::string("RR")));
			messageNames.insert(std::make_pair(std::string("on_ringPoints"), std::string("RP")));

				initialized = true;
			}

			std::map < std::string, std::string>::const_iterator it(messageNames.find(methodName));
			if (it != messageNames.end())
				return it->second;


			// try with the base class
			return CRingSessionManagerWebClientItf::getMessageName(methodName);

		}

		CSessionBrowserServerWebClientItf(ICallbackClientAdaptor *adaptorReplacement = NULL)
			:	CRingSessionManagerWebClientItf(adaptorReplacement)
		{}

		/// Connect the interface client to the callback server at the specified address and port
		virtual void connectItf(NLNET::CInetAddress address)
		{
			NLNET::TCallbackItem *arrayPtr;
			uint32 arraySize;

			static bool callbackAdded = false;
			if (!callbackAdded)
			{

				// add callback array of the base interface class
				CRingSessionManagerWebClientItf::getCallbakArray(arrayPtr, arraySize);
				_CallbackClient->addCallbackArray(arrayPtr, arraySize);
				callbackAdded = true;
				// add callback array of this interface

				getCallbakArray(arrayPtr, arraySize);
				_CallbackClient->addCallbackArray(arrayPtr, arraySize);
			}

			_CallbackClient->setDisconnectionCallback (_cbDisconnection, this);

			_CallbackClient->connect(address);
		}

		/** Must be called evenly, update the network subclass to receive message
		 *	and dispatch invokation returns.
		 */
		virtual void update()
		{
			H_AUTO(CSessionBrowserServerWeb_update);

			try
			{
				_CallbackClient->update();
			}
			catch (...)
			{
				nlwarning("CSessionBrowserServerWeb : Exception launch in callback client update");
			}
		}

		// The client send it's cookie information to
		// authenticate himself.
		// The cookie value is checked against the value stored.
		// in the database.
		// Furthermore, the server will check
		// evenly the database to see if the user is still
		// online and still have the same cookie

		void authenticate(uint32 userId, const NLNET::CLoginCookie &cookie)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::authenticate called");
#endif
			NLNET::CMessage message("AUTH");
			nlWrite(message, serial, userId);
			nlWrite(message, serial, const_cast < NLNET::CLoginCookie& > (cookie));

			_CallbackClient->send(message);
		}
		// Ask for the list of session that are available
		// for the requesting character.

		void getSessionList(uint32 charId)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::getSessionList called");
#endif
			NLNET::CMessage message("GSL");
			nlWrite(message, serial, charId);

			_CallbackClient->send(message);
		}
		// Ask for the list of player characters that are available
		// for the requesting session.

		void getCharList(uint32 charId, TSessionId sessionId)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::getCharList called");
#endif
			NLNET::CMessage message("GCL");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);

			_CallbackClient->send(message);
		}
		// Invite a player in a session given it's name
		// This method make a certain number of asumption :
		// The sessionId is deducted from the current session id of
		// the requester character.
		// The invited char id is deducted from the name by using.
		// the full name rules for shard resolution.
		// Return invoke_result with the following error codes :
		//   0   : no error
		//   100 : unknown onwer char
		//   101 : player already invited
		//   102 : no current session
		//   103 : internal error
		//   104 : invited char not found
		//   plus all the error code from inviteCharacter in the ring session manager interface

		void inviteCharacterByName(uint32 charId, std::string invitedCharName)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::inviteCharacterByName called");
#endif
			NLNET::CMessage message("ICBN");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, invitedCharName);

			_CallbackClient->send(message);
		}
		// Ask for character existing rating for the current session scenario
		// return playerRatings.

		void getMyRatings(uint32 charId, uint32 sessionId)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::getMyRatings called");
#endif
			NLNET::CMessage message("GMSR");
			nlWrite(message, serial, charId);
			nlWrite(message, serial, sessionId);

			_CallbackClient->send(message);
		}
		// Ask for average scores of a session
		// return scessionAverageScores.

		void getSessionAverageScores(uint32 sessionId)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::getSessionAverageScores called");
#endif
			NLNET::CMessage message("GSAS");
			nlWrite(message, serial, sessionId);

			_CallbackClient->send(message);
		}
		// Ask for average scores of a scenario
		// return scenarioAverageScores.

		void getScenarioAverageScores(const std::string &md5)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::getScenarioAverageScores called");
#endif
			NLNET::CMessage message("GSCAS");
			nlWrite(message, serial, const_cast < std::string& > (md5));

			_CallbackClient->send(message);
		}
		// Ask for the author rating, the AM rating and the Masterless rating
		// for the requesting character.

		void getRingRatings(uint32 charId)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::getRingRatings called");
#endif
			NLNET::CMessage message("GRR");
			nlWrite(message, serial, charId);

			_CallbackClient->send(message);
		}
		// Ask for ring points of the character

		void getRingPoints(uint32 charId)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::getRingPoints called");
#endif
			NLNET::CMessage message("GRP");
			nlWrite(message, serial, charId);

			_CallbackClient->send(message);
		}
		// simulate the forward of a message (to dss)

		void forwardToDss(uint32 charId, const NLNET::CMessage &msg)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::forwardToDss called");
#endif
			NLNET::CMessage message("DSS_FW");
			nlWrite(message, serial, charId);
			nlWrite(message, serialMessage, const_cast < NLNET::CMessage& > (msg));

			_CallbackClient->send(message);
		}

		static void cb_sessionList (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_sessionList received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebClientItf *callback = (CSessionBrowserServerWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			std::vector < TSessionDesc >	sessions;
			nlRead(message, serial, charId);
			nlRead(message, serialCont, sessions);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_sessionList : calling on_sessionList");
#endif

			callback->on_sessionList(from, charId, sessions);
		}

		static void cb_charList (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_charList received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebClientItf *callback = (CSessionBrowserServerWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			TSessionId	sessionId;
			std::vector < TCharDesc >	characters;
			nlRead(message, serial, charId);
			nlRead(message, serial, sessionId);
			nlRead(message, serialCont, characters);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_charList : calling on_charList");
#endif

			callback->on_charList(from, charId, sessionId, characters);
		}

		static void cb_playerRatings (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_playerRatings received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebClientItf *callback = (CSessionBrowserServerWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			bool	scenarioRated;
			uint32	rateFun;
			uint32	rateDifficulty;
			uint32	rateAccessibility;
			uint32	rateOriginality;
			uint32	rateDirection;
			nlRead(message, serial, charId);
			nlRead(message, serial, scenarioRated);
			nlRead(message, serial, rateFun);
			nlRead(message, serial, rateDifficulty);
			nlRead(message, serial, rateAccessibility);
			nlRead(message, serial, rateOriginality);
			nlRead(message, serial, rateDirection);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_playerRatings : calling on_playerRatings");
#endif

			callback->on_playerRatings(from, charId, scenarioRated, rateFun, rateDifficulty, rateAccessibility, rateOriginality, rateDirection);
		}

		static void cb_sessionAverageScores (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_sessionAverageScores received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebClientItf *callback = (CSessionBrowserServerWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			bool	scenarioRated;
			uint32	rateFun;
			uint32	rateDifficulty;
			uint32	rateAccessibility;
			uint32	rateOriginality;
			uint32	rateDirection;
			uint32	rrpTotal;
			nlRead(message, serial, scenarioRated);
			nlRead(message, serial, rateFun);
			nlRead(message, serial, rateDifficulty);
			nlRead(message, serial, rateAccessibility);
			nlRead(message, serial, rateOriginality);
			nlRead(message, serial, rateDirection);
			nlRead(message, serial, rrpTotal);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_sessionAverageScores : calling on_sessionAverageScores");
#endif

			callback->on_sessionAverageScores(from, scenarioRated, rateFun, rateDifficulty, rateAccessibility, rateOriginality, rateDirection, rrpTotal);
		}

		static void cb_scenarioAverageScores (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_scenarioAverageScores received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebClientItf *callback = (CSessionBrowserServerWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			bool	scenarioRated;
			uint32	rateFun;
			uint32	rateDifficulty;
			uint32	rateAccessibility;
			uint32	rateOriginality;
			uint32	rateDirection;
			uint32	rrpTotal;
			nlRead(message, serial, scenarioRated);
			nlRead(message, serial, rateFun);
			nlRead(message, serial, rateDifficulty);
			nlRead(message, serial, rateAccessibility);
			nlRead(message, serial, rateOriginality);
			nlRead(message, serial, rateDirection);
			nlRead(message, serial, rrpTotal);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_scenarioAverageScores : calling on_scenarioAverageScores");
#endif

			callback->on_scenarioAverageScores(from, scenarioRated, rateFun, rateDifficulty, rateAccessibility, rateOriginality, rateDirection, rrpTotal);
		}

		static void cb_ringRatings (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_ringRatings received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebClientItf *callback = (CSessionBrowserServerWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			uint32	authorRating;
			uint32	AMRating;
			uint32	masterlessRating;
			nlRead(message, serial, charId);
			nlRead(message, serial, authorRating);
			nlRead(message, serial, AMRating);
			nlRead(message, serial, masterlessRating);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_ringRatings : calling on_ringRatings");
#endif

			callback->on_ringRatings(from, charId, authorRating, AMRating, masterlessRating);
		}

		static void cb_ringPoints (NLNET::CMessage &message, NLNET::TSockId from, NLNET::CCallbackNetBase &netbase)
		{
#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_ringPoints received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast< ICallbackClientAdaptor *>(netbase.getUserData());

			CSessionBrowserServerWebClientItf *callback = (CSessionBrowserServerWebClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
			uint32	charId;
			std::string	ringPoints;
			std::string	maxRingPoints;
			nlRead(message, serial, charId);
			nlRead(message, serial, ringPoints);
			nlRead(message, serial, maxRingPoints);


#ifdef NL_DEBUG
			nldebug("CSessionBrowserServerWebClient::cb_ringPoints : calling on_ringPoints");
#endif

			callback->on_ringPoints(from, charId, ringPoints, maxRingPoints);
		}


		/// Disconnection callback : the connection to the server is lost
		virtual void on_CSessionBrowserServerWebClient_Disconnection(NLNET::TSockId from) =0;


		// Return the list of available session
		virtual void on_sessionList(NLNET::TSockId from, uint32 charId, const std::vector < TSessionDesc > &sessions) =0;

		// Return the list of player characters in the session
		virtual void on_charList(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::vector < TCharDesc > &characters) =0;

		// Return current player rating of the current session scenario
		virtual void on_playerRatings(NLNET::TSockId from, uint32 charId, bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection) =0;

		// Return average scores of a session
		virtual void on_sessionAverageScores(NLNET::TSockId from, bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection, uint32 rrpTotal) =0;

		// Return average scores of a scenario
		virtual void on_scenarioAverageScores(NLNET::TSockId from, bool scenarioRated, uint32 rateFun, uint32 rateDifficulty, uint32 rateAccessibility, uint32 rateOriginality, uint32 rateDirection, uint32 rrpTotal) =0;

		// Return the author rating, the AM rating and the Masterless rating
		virtual void on_ringRatings(NLNET::TSockId from, uint32 charId, uint32 authorRating, uint32 AMRating, uint32 masterlessRating) =0;

		// Return the ring points of the character
		virtual void on_ringPoints(NLNET::TSockId from, uint32 charId, const std::string &ringPoints, const std::string &maxRingPoints) =0;

	};

}

#endif
