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



#ifndef CL_AUTOMATON_LIST_SHEET_H
#define CL_AUTOMATON_LIST_SHEET_H

/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"

#include "entity_sheet.h"
#include "animation_set_list_sheet.h"

#include "game_share/mode_and_behaviour.h"

/////////////
// CLASSES //
/////////////

/**
 * Automaton state sheet used by automaton sheet
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2003
 */
class CAutomatonStateSheet
{
public:

	class CNextStateInf
	{
	public:
		/// next state to switch to
		TAnimStateKey NextStateKey;

		/// true if the animation is breakable(don't have to wait for the end of anim to switch)
		bool Breakable;

		/// Constructor.
		CNextStateInf()
		{
			NextStateKey = CAnimationStateSheet::UnknownState;
			Breakable = true;
		}

		/// Serialize a CNextStateInf.
		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serialEnum (NextStateKey);
			f.serial(Breakable);
		}
	};

public:
	/// id of anims that can be used for this state
	TAnimStateKey MoveState;
	/// next state to switch to
	TAnimStateKey NextState;

	// can the mesh move
	bool Move;
	// 'true' if rotation in the state should be consider.
	bool Rotation;
	// 'true' if the animation is an attack (melee attack).
	bool Attack;
	// Slide
	bool Slide;
	// If 'true', adjust the orientation while playing the current state
	bool AdjustOri;
	// Play the animation in a constant time if there is a move after.
	float MaxAnimDuration;

	//
	uint32			MaxLoop;				// Breakable on max loop (the animation cannot be played more than ... 0 means no limits).

	// Move
	bool			BreakableOnMove;		// 'true' if 1 of the Forward/backward/Left/right is field.
	double			BreakableOnMoveDist;
	TAnimStateKey	OnMoveForward;
	TAnimStateKey	OnMoveBackward;
	TAnimStateKey	OnMoveLeft;
	TAnimStateKey	OnMoveRight;
	// Rotation
	bool			BreakableOnRotation;
	TAnimStateKey	OnLeftRotation;			// Need a left and right because of about face animations.
	TAnimStateKey	OnRightRotation;		// Need a left and right because of about face animations.
	// Big Bend
	bool			BrkOnBigBend;			// 'true' when the angle between the current direction and the next one is too big.
	TAnimStateKey	OnBigBendLeft;
	TAnimStateKey	OnBigBendRight;
	// Speed
	CNextStateInf	OnMinSpeed;
	CNextStateInf	OnMaxSpeed;
	// Bad Heading (heading too far from the front).
	bool			BreakableOnBadHeading;
	TAnimStateKey	OnBadHeadingForward;
	TAnimStateKey	OnBadHeadingBackward;
	TAnimStateKey	OnBadHeadingLeft;
	TAnimStateKey	OnBadHeadingRight;
	double			BadHeadingMin;			// If the current state is < at value -> bad heading
	double			BadHeadingMax;			// If the current state is > at value -> bad heading


	std::vector<bool>			ModeConnection;
	std::vector<TAnimStateKey>	ModeTransition;

	/// The automaton to use At the end of the animation.
	MBEHAV::EMode	NextMode;


	/// ...
	double			DirFactor;

	float			RotFactor;

	/// Go to this state if there is an attack.
	TAnimStateKey	OnAtk;

	bool			BreakableOnImpact;
	// 'true' if the animation need to stop once arrived at destination.
	bool			BrkAtDest;
	//
	float			XFactor;
	float			YFactor;
	float			ZFactor;

	NLMISC::CMatrix	Matrix;

	// -----------------------------------------------------------------------

	/// Constructor
	CAutomatonStateSheet();

	/// Build the sheet given a base string
	void build(const NLGEORGES::UFormElm &item, const std::string &baseKeyStr);

	/// Serialize a CMovingAutomatonState.
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	/** Return if there is a connection with the mode in parameter from the current automaton and filled the transition to use.
	 * \param mode : is there a connection for this mode.
	 * \param transition : will be filled with the result if there is a connection.
	 * \return bool : If 'true' there is a connection and 'transition' is filled, else 'transition' left untouched.
	 */
	bool getModeConnection(MBEHAV::EMode mode, TAnimStateKey &transition) const;

	const NLMISC::CMatrix & getMatrix() const { return Matrix; }
};

/**
 * Automaton sheet to be used in the EAM
 * No need to herit from entity sheet because read by CAutomatonListSheet
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2003
 */
class CAutomatonSheet
{
public:
	typedef std::map<uint32, CAutomatonStateSheet> TAutomatonStateSheets;
	/// States of the automaton
	TAutomatonStateSheets _States;

public:
	/// Build the sheet
	void build(const NLGEORGES::UFormElm &item);

	/// Serialize sheet into binary data file.
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	/**
	 * Return a pointer on the information about the state corresponding to the key.
	 * \param key : state key.
	 * \return CAutomatonStateSheet * : pointer on the state or 0.
	 */
	const CAutomatonStateSheet *state(const TAnimStateKey &key) const;

protected:

	static std::vector<std::string> _MotionStates;
	static std::vector<std::string> _GenericStates;
	static std::vector<std::string> _ModeStates;
	static std::vector<std::string> _AtkStates;
	static std::vector<std::string> _SpellStates;
	static std::vector<std::string> _OtherStates;

protected:

	void buildStateSet(const NLGEORGES::UFormElm &item, const std::vector<std::string> &states, const std::string &setName, double moveDist);

};

/**
 * List of automaton sheet to be used in the EAM
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2003
 */
class CAutomatonListSheet : public CEntitySheet
{
public:

	std::map<std::string, CAutomatonSheet*> Automatons;

public:
	/// Constructor
	CAutomatonListSheet();
	virtual ~CAutomatonListSheet();

	/// Build the sheet from an external script.
	virtual void build(const NLGEORGES::UFormElm &item);

	/// Serialize sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	/// Tool to read a dfn composed with an array of string
	static void readDfnStringArray(std::vector<std::string> &states, const std::string &dfnName);

};


#endif // CL_AUTOMATON_LIST_SHEET_H

/* End of automaton_sheet.h */
