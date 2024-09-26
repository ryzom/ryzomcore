// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_VEGETABLE_EDIT_TOOLS_H
#define NL_VEGETABLE_EDIT_TOOLS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// range_selector.h : header file
//


#include "nel/misc/types_nl.h"
#include "editable_range.h"
#include "vegetable_refresh.h"


// ***************************************************************************
// Defaults Sliders ranges.
#define	NL_VEGETABLE_EDIT_DEFAULT_MAX_DENSITY	50.f

// General Frequence
#define	NL_VEGETABLE_FREQ_RANGE_MIN	0.0001f
#define	NL_VEGETABLE_FREQ_RANGE_MAX	1.f
#define	NL_VEGETABLE_FREQ_DEFAULT	0.1f
// Density
#define	NL_VEGETABLE_DENSITY_ABS_RANGE_MIN	-10.f
#define	NL_VEGETABLE_DENSITY_ABS_RANGE_MAX	10.f
#define	NL_VEGETABLE_DENSITY_RAND_RANGE_MIN	0.f
#define	NL_VEGETABLE_DENSITY_RAND_RANGE_MAX	10.f
#define	NL_VEGETABLE_DENSITY_ABS_DEFAULT	0.f
#define	NL_VEGETABLE_DENSITY_RAND_DEFAULT	0.25f
// BendPhase
#define	NL_VEGETABLE_BENDPHASE_RANGE_MIN	0.f
#define	NL_VEGETABLE_BENDPHASE_RANGE_MAX	2.f
#define	NL_VEGETABLE_BENDPHASE_ABS_DEFAULT		0.f
#define	NL_VEGETABLE_BENDPHASE_RAND_DEFAULT		2.f
// BendFactor
#define	NL_VEGETABLE_BENDFACTOR_RANGE_MIN	0.f
#define	NL_VEGETABLE_BENDFACTOR_RANGE_MAX	1.f
#define	NL_VEGETABLE_BENDFACTOR_ABS_DEFAULT		0.5f
#define	NL_VEGETABLE_BENDFACTOR_RAND_DEFAULT	0.5f
// ColorNoise
#define	NL_VEGETABLE_COLOR_RANGE_MIN		-1.f
#define	NL_VEGETABLE_COLOR_RANGE_MAX		3.f
#define	NL_VEGETABLE_COLOR_ABS_DEFAULT		-1.f
#define	NL_VEGETABLE_COLOR_RAND_DEFAULT		3.f
// Scale									
#define	NL_VEGETABLE_SCALE_RANGE_MIN		0.f
#define	NL_VEGETABLE_SCALE_RANGE_MAX		1.f
#define	NL_VEGETABLE_SCALE_ABS_DEFAULT		0.5f
#define	NL_VEGETABLE_SCALE_RAND_DEFAULT		0.5f
// Rotate									
#define	NL_VEGETABLE_ROTATE_RANGE_MIN		-90.f
#define	NL_VEGETABLE_ROTATE_RANGE_MAX		90.f
#define	NL_VEGETABLE_ROTATEX_ABS_DEFAULT	-20.f
#define	NL_VEGETABLE_ROTATEX_RAND_DEFAULT	40.f
#define	NL_VEGETABLE_ROTATEY_ABS_DEFAULT	0.f
#define	NL_VEGETABLE_ROTATEY_RAND_DEFAULT	0.f
#define	NL_VEGETABLE_ROTATEZ_ABS_DEFAULT	0.f
#define	NL_VEGETABLE_ROTATEZ_RAND_DEFAULT	3000.f
#define	NL_VEGETABLE_ROTATEZ_FREQ_DEFAULT	10.f
// BendFreq									
#define	NL_VEGETABLE_BENDFREQ_RANGE_MIN		0.f
#define	NL_VEGETABLE_BENDFREQ_RANGE_MAX		4.f



// ***************************************************************************
/**
 * An edition of a float with a pointer to him
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CDirectEditableRangeFloat : public CEditableRangeFloat
{
public:
	// Approximate Height of this control
	enum	{ControlHeight= 34};

public:

	// ctor.
	CDirectEditableRangeFloat(const std::string &id, float defaultMin, float defaultMax, const std::string &title);

	// Init the dialog, and the static text.
	virtual void init(uint32 x, uint32 y, CWnd *pParent);

	// Set our pointer to the float, and update view.
	void		setFloat(float *value, IVegetableRefresh *vegetRefresh);

	// if false, nothgin will be written to the Float.
	void		enableWrite(bool enb);

// *****************
private:

	// Our easy wrapper.
	class CDirectFloatWrapper : public IPSWrapperFloat
	{
	public:
		CDirectFloatWrapper()
		{
			Value= NULL;
			WriteEnabled= true;
		}

		float				*Value ;
		IVegetableRefresh	*VegetableRefresh;
		bool	WriteEnabled;
		float get(void) const;
		void set(const float &f);
	};

	CDirectFloatWrapper		_Wrapper;


	// Our Caption
	std::string				_Title;
	CStatic					_StaticText;

};


#endif // NL_VEGETABLE_EDIT_TOOLS_H

/* End of vegetable_edit_tools.h */
