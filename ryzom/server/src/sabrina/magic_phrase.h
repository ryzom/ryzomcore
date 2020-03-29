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



#ifndef RY_MAGIC_PHRASE_H
#define RY_MAGIC_PHRASE_H

#include "nel/misc/types_nl.h"
#include "s_phrase.h"
#include "magic_action.h"
#include "game_share/egs_sheets/egs_static_magic_range.h"

// default range index in the tables
static const sint8 MagicDefaultRangeIndex = 3;

/**
 * This class represents a magic phrase in the Sabrina system. See CSPhrase for methods definitions.
 * A magic phrase contains the global parameters of the phrase (range, cost,...) And the magic actions
 * The real activity of the phrase is managed by these actions
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CMagicPhrase : public CSPhrase
{
public:
	static void defaultCastingTime(float time) {CMagicPhrase::_DefaultCastingTime = time;}
	static float defaultCastingTime() {return CMagicPhrase::_DefaultCastingTime;}
	
public:

	/// ctor
	inline CMagicPhrase() 
		:CSPhrase(),_SabrinaCost(0),_SabrinaCredit(0),_SapCost(0),_HPCost(0),_RangeIndex(0),
		_CastingTime(0),_Targets(1),_Nature(ACTNATURE::UNKNOWN),_Range( 0 ), _BreakResist(0),_ArmorCompensation(0) {}

	/// dtor
	virtual ~CMagicPhrase();

	/// \accessors
	//@{
	inline uint16	getSabrinaCost()							{ return _SabrinaCost;		}
	inline uint		getNbActions()								{ return _Actions.size();	}
	inline sint32	getSapCost()								{ return _SapCost;			}
	inline sint32	getHPCost()									{ return _HPCost;			}
	inline const	std::vector<TDataSetRow> & getTargets()		{ return _Targets;			}
	inline const	TDataSetRow & getActor()					{ return _ActorRowId;		}
	inline sint32	getBreakResist()							{ return _BreakResist;		}
	inline const	std::vector<SKILLS::ESkills>& getSkills()	{ return _Skills;			}
	//@}

	/**
	 * apply a brick parameter to this sentence
	 * \param parem: parameter to take in account in this phrase
	 */
	void applyBrickParam( TBrickParam::IId * param );

	///\name Overriden methods from CSPhrase
	//@{
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks );
	virtual bool evaluate(CEvalReturnInfos *msg = NULL);
	virtual bool validate();
	virtual bool update();
	virtual void execute();
	virtual void apply();
	virtual void stop();
	virtual void setPrimaryTarget( const TDataSetRow &entityRowId ){ _Targets[0] = entityRowId; }
	virtual void addTargetEntity( const TDataSetRow &entityRowId ){ _Targets.push_back(entityRowId);}
	//@}

	/// \name Unused virtual methods from CSPhrase
	//@{
	virtual void setActor( const TDataSetRow & actorRowId ){  }
	virtual void end(){}
	virtual void setPrimaryItem( CGameItemPtr itemPtr ){  }
	virtual void setSecondaryItem( CGameItemPtr itemPtr ){  }
	virtual void addConsumableItem( CGameItemPtr itemPtr ){  }
	//@}

private:
	static float	_DefaultCastingTime;
	
private:

	// active actions triggered by this phrase
	std::vector< IMagicAction* > _Actions;
	/// acting entity
	TDataSetRow					_ActorRowId;
	/// targets
	std::vector<TDataSetRow>	_Targets;
	// nature of the spell
	ACTNATURE::EActionNature	_Nature;
	/// total cost (sabrina system)
	uint16						_SabrinaCost;
	/// total credit (sabrina system)
	uint16						_SabrinaCredit;
	/// sap cost of the attack
	uint16						_SapCost;
	/// hp cost
	uint16						_HPCost;
	/// casting time in ticks
	NLMISC::TGameCycle			_CastingTime;
	/// range index of the spell
	sint8						_RangeIndex;
	/// the skills used in this phrase
	std::vector<SKILLS::ESkills> _Skills;
	/// range in mmmm
	sint32						_Range;
	/// interrupt resistance bonus
	uint16						_BreakResist;
	/// number of skill malus points rhat are compensated
	uint16						_ArmorCompensation;
};


#endif // RY_MAGIC_PHRASE_H

/* End of magic_phrase.h */
