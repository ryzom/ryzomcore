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

#ifndef BIN_OP_DIALOG_H
#define BIN_OP_DIALOG_H


#include <nel/misc/types_nl.h>

// Qt includes
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

// NeL includes
#include "nel/3d/ps_attrib_maker_bin_op.h"

// Project includes
#include "ps_wrapper.h"
#include "attrib_widget.h"

namespace NLQT
{

class CBinOpDialog : public  QDialog
{
	Q_OBJECT

public:
	CBinOpDialog(QWidget *widget1, QWidget *widget2, QWidget *parent = 0);
	virtual ~CBinOpDialog();

	virtual void init() = 0;

	/// called when a new operator has been selected
	virtual void newOp(uint32 op) = 0 ;

private Q_SLOTS:
	void setNewOp(int index);

protected:

	QGridLayout *_gridLayout;
	QWidget *_widget1;
	QComboBox *_comboBox;
	QSpacerItem *_horizontalSpacer;
	QWidget *_widget2;
};


/**
@class CBinOpDialogT
@brief Construct a dialog that allow to edit a binary operator that produce argument of a particle system
*/
template <class T> class CBinOpDialogT : public CBinOpDialog
{
public:
	/// ctruct the given dialog from the given scheme that owns memory
	CBinOpDialogT(NL3D::CPSAttribMakerBinOp<T> *editedScheme, CAttribWidgetT<T> **attrbDlg, QWidget *parent = 0)
		: CBinOpDialog(attrbDlg[0], attrbDlg[1], parent), _EditedScheme(editedScheme)
	{
		for (uint k = 0 ; k < 2 ; ++k)
		{
			nlassert(attrbDlg);
			_AttrbDlg[k] = attrbDlg[k];
			_SchemeWrapper[k].S = _EditedScheme ;
			_SchemeWrapper[k].Index =  k ;
		}
	}
	void init()
	{
		uint k ;
		for (k = 0 ; k < 2 ; ++k)
		{
			_AttrbDlg[k]->setEnabledConstantValue(false) ;
			_AttrbDlg[k]->setWrapper(&_DummyWrapper) ;
			_AttrbDlg[k]->setSchemeWrapper(&_SchemeWrapper[k]) ;
			_AttrbDlg[k]->init();
		}

		static const char *const operators[] =
		{
			QT_TR_NOOP("Select Arg1"),
			QT_TR_NOOP("Select Arg2"),
			QT_TR_NOOP("Modulate"),
			QT_TR_NOOP("Add"),
			QT_TR_NOOP("Subtract"),
			0
		};
		_comboBox->blockSignals(true);
		for (k = 0 ; k < (uint) NL3D::CPSBinOp::last ; ++k)
		{
			if (_EditedScheme->supportOp( (NL3D::CPSBinOp::BinOp) k))
			{
				_comboBox->insertItem(_comboBox->count(), operators[k]);

				if ((uint) _EditedScheme->getOp() == k)
					_comboBox->setCurrentIndex(k);
			}
		}
		_comboBox->blockSignals(false);
	}

	~CBinOpDialogT()
	{
		for (uint k = 0 ; k < 2 ; ++k)
		{
			delete _AttrbDlg[k] ;
		}
	}

protected:

	NL3D::CPSAttribMakerBinOp<T> *_EditedScheme ;

	/// the dialogs that allow us to edit the schemes
	CAttribWidgetT<T> *_AttrbDlg[2] ;

	/// a wrapper to edit the scheme (which himself owns a scheme !!)
	struct CSchemeWrapper : public IPSSchemeWrapper<T>
	{
		NL3D::CPSAttribMakerBinOp<T> *S ;
		uint Index ;
		virtual NL3D::CPSAttribMaker<T> *getScheme(void) const
		{
			return S->getArg(Index) ;
		}
		virtual void setScheme(NL3D::CPSAttribMaker<T> *s)
		{
			S->setArg(Index, s) ;
		} ;
	} _SchemeWrapper[2] ;

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


	void newOp(uint32 op)
	{
		nlassert(_EditedScheme) ;
		if (_EditedScheme->getOp() != (NL3D::CPSBinOp::BinOp) op)
			_EditedScheme->setOp((NL3D::CPSBinOp::BinOp) op);
	}

} ;

} /* namespace NLQT */

#endif // BIN_OP_DIALOG_H
