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

#ifndef PS_MOVER_PAGE_H
#define PS_MOVER_PAGE_H

#include <nel/misc/types_nl.h>
#include "ui_ps_mover_form.h"

// STL includes

// NeL includes
#include <nel/misc/config_file.h>
#include <nel/misc/rgba.h>
#include <nel/3d/ps_edit.h>
#include <nel/3d/particle_system.h>

// Project includes
#include "ps_wrapper.h"

namespace NLQT
{

/**
@class CLocatedBindableItem
@brief Contain pointer to CPSLocatedBindable.
*/
class CLocatedBindableItem: public QListWidgetItem
{
public:
	CLocatedBindableItem ( const QString &text, QListWidget *parent = 0, int type = UserType ):
		QListWidgetItem(text, parent, type), _lb(NULL) {}

	void setUserData(NL3D::CPSLocatedBindable *loc)
	{
		_lb = loc;
	}
	NL3D::CPSLocatedBindable *getUserData() const
	{
		return _lb;
	}

private:

	NL3D::CPSLocatedBindable *_lb;
}; /* class CTargetItem */

/**
@class CPSMoverPage
@brief Page for QStackWidget, to edit instance in a particle system.
*/
class CPSMoverPage: public QWidget
{
	Q_OBJECT

public:
	CPSMoverPage(QWidget *parent = 0);
	~CPSMoverPage();

	/// Set the instance to edit.
	void setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocated *located, uint32 editedLocatedIndex);

	/// position has to be updated (for mouse edition)
	void updatePosition(void) ;

	/// get the current moving interface, or NULL, if the selected object has no IPSMover interface
	NL3D::IPSMover *getMoverInterface(void);

	/// get the located being edited
	NL3D::CPSLocated *getLocated(void)
	{
		return _EditedLocated ;
	}
	const NL3D::CPSLocated *getLocated(void) const
	{
		return _EditedLocated;
	}

	/// get the index of the current edited item
	uint32 getLocatedIndex(void) const
	{
		return _EditedLocatedIndex;
	}

	/// ghet the current located bindable being edited, or null
	NL3D::CPSLocatedBindable *getLocatedBindable(void);

private Q_SLOTS:
	void setXPosition(double value);
	void setYPosition(double value);
	void setZPosition(double value);
	void changeSubComponent();

	void setDir(const NLMISC::CVector &value);

private:

	/// wrappers to scale objects
	struct CUniformScaleWrapper : public IPSWrapperFloat
	{
		uint32 Index ;
		NL3D::IPSMover *M ;
		float get(void) const
		{
			return M->getScale(Index).x ;
		}
		void set(const float &v)
		{
			M->setScale(Index, v) ;
		}
	} _UniformScaleWrapper ;

	/// wrapper to scale the X coordinate
	struct CXScaleWrapper : public IPSWrapperFloat
	{
		uint32 Index ;
		NL3D::IPSMover *M ;
		float get(void) const
		{
			return M->getScale(Index).x ;
		}
		void set(const float &s)
		{
			NLMISC::CVector v = M->getScale(Index) ;
			M->setScale(Index, NLMISC::CVector(s, v.y, v.z)) ;
		}
	} _XScaleWrapper ;

	/// wrapper to scale the Y coordinate
	struct CYScaleWrapper : public IPSWrapperFloat
	{
		uint32 Index ;
		NL3D::IPSMover *M ;
		float get(void) const
		{
			return M->getScale(Index).y ;
		}
		void set(const float &s)
		{
			NLMISC::CVector v = M->getScale(Index) ;
			M->setScale(Index, NLMISC::CVector(v.x, s, v.z) ) ;
		}
	} _YScaleWrapper ;

	/// wrapper to scale the Z coordinate
	struct CZScaleWrapper : public IPSWrapperFloat
	{
		uint32 Index ;
		NL3D::IPSMover *M ;
		float get(void) const
		{
			return M->getScale(Index).z ;
		}
		void set(const float &s)
		{
			NLMISC::CVector v = M->getScale(Index) ;
			M->setScale(Index, NLMISC::CVector(v.x, v.y, s) ) ;
		}
	} _ZScaleWrapper ;

	void hideAdditionalWidget();

	void updateModifiedFlag()
	{
		if (_Node) _Node->setModified(true);
	}

	/// update the mouse listener position when the user entered a value with the keyboard
	void updateListener(void) ;

	CWorkspaceNode *_Node;

	NL3D::CPSLocated *_EditedLocated ;

	uint32 _EditedLocatedIndex ;

	Ui::CPSMoverPage _ui;

}; /* class CPSMoverPage */

} /* namespace NLQT */


#endif // PS_MOVER_PAGE_H
