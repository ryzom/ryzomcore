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

#ifndef VALUE_FROM_EMITTER_DIALOG_H
#define VALUE_FROM_EMITTER_DIALOG_H

#include <nel/misc/types_nl.h>

// Qt includes
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QWidget>

// NeL include
#include "nel/3d/ps_attrib_maker.h"

// Project includes
#include "ps_wrapper.h"
#include "attrib_widget.h"

namespace NL3D
{
template <typename T> class CPSAttribMakerMemory ;
}

namespace NLQT
{

class CValueFromEmitterDialog : public QDialog
{
	Q_OBJECT

public:
	CValueFromEmitterDialog(QWidget *widget, QWidget *parent = 0);
	~CValueFromEmitterDialog();

	virtual void init() = 0;

protected:

	QGridLayout *_gridLayout;
	QWidget *_widget;
};

/** construct a dialog that allow to edit a scheme used for initial attribute generation in a particle
  */
template <class T> class CValueFromEmitterDialogT : public CValueFromEmitterDialog
{
public:
	CValueFromEmitterDialogT(NL3D::CPSAttribMakerMemory<T> *editedScheme, CAttribWidgetT<T> *srcDlg, QWidget *parent = 0)
		: CValueFromEmitterDialog(srcDlg, parent), _AttrbDlg(srcDlg)
	{
		nlassert(srcDlg);
		_SchemeWrapper.S = editedScheme ;
	}
	// inherited from CValueFromEmitterDialog
	void init()
	{
		_AttrbDlg->setEnabledConstantValue(false) ;
		_AttrbDlg->setWrapper(&_DummyWrapper) ;
		_AttrbDlg->setSchemeWrapper(&_SchemeWrapper) ;
		_AttrbDlg->init();
	}

	~CValueFromEmitterDialogT()
	{
		delete _AttrbDlg ;
	}

protected:

	/// the dialog that allow us to edit the scheme
	CAttribWidgetT<T> *_AttrbDlg ;

	/// a wrapper to edit the scheme (which himself owns a scheme !!)
	struct CSchemeWrapper : public IPSSchemeWrapper<T>
	{
		NL3D::CPSAttribMakerMemory<T> *S ;
		virtual NL3D::CPSAttribMaker<T> *getScheme(void) const
		{
			return S->getScheme() ;
		}
		virtual void setScheme(NL3D::CPSAttribMaker<T> *s)
		{
			S->setScheme(s) ;
		} ;
	} _SchemeWrapper ;

	/// a dummy wrapper for constant value. This shouldn't be called , however
	struct CDummyWrapper : public IPSWrapper<T>
	{
		T get(void) const
		{
			nlassert(false) ;
			return T() ;
		}
		void set(const T &)
		{
			nlassert(false) ;
		}
	} _DummyWrapper ;
} ;

} /* namespace NLQT */

#endif // VALUE_FROM_EMITTER_DIALOG_H
