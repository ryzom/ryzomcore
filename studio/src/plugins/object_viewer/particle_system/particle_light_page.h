/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef PARTICLE_LIGHT_PAGE_H
#define PARTICLE_LIGHT_PAGE_H

#include <nel/misc/types_nl.h>
#include "ui_particle_light_form.h"

// STL includes

// NeL includes
#include <nel/misc/rgba.h>
#include "nel/3d/ps_light.h"

// Project includes
#include "ps_wrapper.h"

namespace NLQT
{

/**
@class CLightPage
@brief Page for QStackWidget, to edit lights in a particle system
*/
class CLightPage: public QWidget
{
	Q_OBJECT

public:
	CLightPage(QWidget *parent = 0);
	~CLightPage();

	/// Set the light to edit.
	void setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocatedBindable *locatedBindable);

private Q_SLOTS:
private:

	/// wrapper to set the color of light
	struct CColorWrapper : public IPSWrapperRGBA, IPSSchemeWrapperRGBA
	{
		NL3D::CPSLight *L;
		NLMISC::CRGBA get(void) const
		{
			return L->getColor();
		}
		void set(const NLMISC::CRGBA &v)
		{
			L->setColor(v);
		}
		scheme_type *getScheme(void) const
		{
			return L->getColorScheme();
		}
		void setScheme(scheme_type *s)
		{
			L->setColorScheme(s);
		}
	} _ColorWrapper;

	/// wrapper to set start atten radius
	struct CAttenStartWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSLight *L;
		float get(void) const
		{
			return L->getAttenStart();
		}
		void set(const float &v)
		{
			L->setAttenStart(v);
		}
		scheme_type *getScheme(void) const
		{
			return L->getAttenStartScheme();
		}
		void setScheme(scheme_type *s)
		{
			L->setAttenStartScheme(s);
		}
	} _AttenStartWrapper;

	/// wrapper to set end atten radius
	struct CAttenEndWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSLight *L;
		float get(void) const
		{
			return L->getAttenEnd();
		}
		void set(const float &v)
		{
			L->setAttenEnd(v);
		}
		scheme_type *getScheme(void) const
		{
			return L->getAttenEndScheme();
		}
		void setScheme(scheme_type *s)
		{
			L->setAttenEndScheme(s);
		}
	} _AttenEndWrapper;

	NL3D::CPSLight *_Light;

	CWorkspaceNode *_Node;

	Ui::CLightPage _ui;
}; /* class CLightPage */

} /* namespace NLQT */

#endif // PARTICLE_LIGHT_PAGE_H
