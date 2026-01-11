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



#ifndef CL_CANDIDATE_LIST_H
#define CL_CANDIDATE_LIST_H


/////////////
// Include //
/////////////
// misc
#include "nel/misc/types_nl.h"
// client
#include "multi_list.h"


/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2001
 */
class CCandidateList : public CMultiList
{
public:

	/// Constructors
	CCandidateList(uint id);
	CCandidateList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, const CPen &pen);
	CCandidateList(uint id, float x, float y, float x_pixel, float y_pixel, float w, float h, float w_pixel, float h_pixel, uint32 fontSize, CRGBA color, bool shadow);

	///destructor
	~CCandidateList();


	/**
	 * set the function called on a left-click event
	 * \param uint16 the new function number
	 */
	void setLeftClickFunction( uint16 num) { _LeftClickFunction = num; }

	/**
	 * set the function called on a right-click event
	 * \param uint16 the new function number
	 */
	void setRightClickFunction( uint16 num) { _RightClickFunction = num; }


	/**
	 * set the coor of the box which apperas when a candidate is selected
	 * \param CRGBA &color of the box
	 */
	void setSelectedColor( const CRGBA &color) { _SelectedColor = color; }

	/// Display the control.
	virtual void display();


	/// Manage the left click of the mouse for the list
	virtual void click(float x, float y, bool &taken);

	/// Manage the right click of the mouse for the list
	virtual void clickRight(float x, float y, bool &taken);

	/**
	 * get the name of the selected candidate
	 * \return ucstring* the selected candidate name (or NULL if no selection)
	 */
	const ucstring *getSelectedName() const { return _SelectedCandidate; }



private:
	/**
	* search into _NamesYPos list for the specified y coordinate
	* \param the searched y coordinate
	* \param ucstring*& the variable that will receive the pointer to the name of the candidate (or NULL if not found)
	* \return bool true if a candidate was found, false otherwise
	*/
	bool searchCandidateAtPos(float y, ucstring *&name) const;

//attributes
private:
	/**
	* list Y coordinates of displayed candidate name, used when a user left click on the control to determine if he clicked on a candidate
	* and which one
	* NB : the second type (ucstring*) is a pointer to the candidate name in _ItemsList
	*/
	typedef std::list< std::pair< std::pair<float,float>, const ucstring*> >  TPairPFloatStr;
	TPairPFloatStr		_NamesYPos;

	/// current mouse position
//	mutable float		_MouseX;
//	mutable float		_MouseY;

	/// currently selected candidate (NULL if no candidate is selected)
	ucstring*			_SelectedCandidate;

	/// number of the function to run when the user left click on a candidate name
	uint				_LeftClickFunction;

	/// number of the function to run when user right click on a candidate name
	uint				_RightClickFunction;

	/// the color of the box which appears behind the selected name
	NLMISC::CRGBA		_SelectedColor;
};


#endif // CL_CANDIDATE_LIST_H

/* End of candidate_list.h */
