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

#ifndef VALUE_BLENDER_DIALOG_H
#define VALUE_BLENDER_DIALOG_H

#include <nel/misc/types_nl.h>

// Qt includes
#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

// STL includes

// NeL includes
#include <nel/3d/ps_float.h>
#include <nel/3d/ps_int.h>
#include <nel/3d/ps_color.h>

#include "nel/misc/rgba.h"
#include "nel/3d/ps_attrib_maker.h"

// Project includes
#include "ps_wrapper.h"
#include "edit_range_widget.h"
#include "color_edit_widget.h"

namespace NLQT
{

struct IValueBlenderDialogClient
{
	/// Create a dialog to edit a single value.
	///  @param index - must be 0 or 1, it says which value is being edited
	virtual QWidget *createDialog(uint index, CWorkspaceNode *ownerNode, QWidget *parent = 0) = 0;

	virtual ~IValueBlenderDialogClient() {}
};

class CValueBlenderDialog : public QDialog
{
	Q_OBJECT

public:
	/// Create the dialog.
	/// @param createInterface - interface that allows to create a dialog to edit one of the 2 values used for the blend.
	/// @param destroyInterface - true if this object must take care to call 'delete' on the 'createInterface' pointer
	CValueBlenderDialog(IValueBlenderDialogClient *createInterface,
						CWorkspaceNode *ownerNode,
						bool destroyInterface,
						QWidget *parent = 0);

	~CValueBlenderDialog() ;

// Implementation
protected:
	IValueBlenderDialogClient *_CreateInterface ;

	CWorkspaceNode *_Node;

	bool _DestroyInterface;

	QGridLayout *_gridLayout;

	QLabel *_startLabel;

	QLabel *_endLabel;

	// the 2 dialog used to choose the blending value
	QWidget *_startWidget, *_endWidget;
};

/// GENERAL INTERFACE FOR BLENDER EDITION

///  T is the type to be edited (color, float, etc..), even if it is unused
template <typename T>
class CValueBlenderDialogClientT : public IValueBlenderDialogClient
{
public:
	virtual ~CValueBlenderDialogClientT() {}

	// the scheme being used. Must be set by the user
	NL3D::CPSValueBlendFuncBase<T> *SchemeFunc;

protected:
	virtual QWidget *createDialog(uint index, CWorkspaceNode *ownerNode, QWidget *parent = 0)
	{
		_ValueInfos[index].ValueIndex = index;
		_ValueInfos[index].SchemeFunc = SchemeFunc;
		_ValueInfos[index].OwnerNode = ownerNode;
		return newDialog(&_ValueInfos[index], parent);
	}


	// construct a dialog
	virtual QWidget *newDialog(IPSWrapper<T> *wrapper, QWidget *parent) = 0;

	// inherited from IPSWrapper<T>
	struct COneValueInfo : public IPSWrapper<T>
	{
		// value 0 or 1 being edited
		uint ValueIndex;
		// the scheme being edited
		NL3D::CPSValueBlendFuncBase<T> *SchemeFunc;

		virtual T get(void) const
		{
			T t1, t2;
			SchemeFunc->getValues(t1, t2);
			return ValueIndex == 0 ? t1 : t2;
		}
		virtual void set(const T &value)
		{
			T t1, t2;
			SchemeFunc->getValues(t1, t2);
			if (ValueIndex == 0 ) t1 = value;
			else t2 = value;
			SchemeFunc->setValues(t1, t2);
		}
	};

	COneValueInfo _ValueInfos[2];
};

/// FLOAT BLENDER EDITION INTERFACE
class CFloatBlenderDialogClient : public CValueBlenderDialogClientT<float>
{
public:
	~CFloatBlenderDialogClient() {}

	QWidget *newDialog(IPSWrapper<float> *wrapper, QWidget *parent)
	{
		CEditRangeFloatWidget *erf = new CEditRangeFloatWidget(parent);
		erf->setRange(MinRange, MaxRange);
		erf->setWrapper(wrapper);
		erf->updateUi();
		return erf;
	}
	float MinRange, MaxRange;
};

/// UINT BLENDER EDITION INTERFACE
class CUIntBlenderDialogClient : public CValueBlenderDialogClientT<uint32>
{
public:
	~CUIntBlenderDialogClient() {}

	QWidget *newDialog(IPSWrapper<uint32> *wrapper, QWidget *parent)
	{
		CEditRangeUIntWidget *erf = new CEditRangeUIntWidget(parent);
		erf->setRange(MinRange, MaxRange);
		erf->setWrapper(wrapper);
		erf->updateUi();
		return erf;
	}
	uint32 MinRange, MaxRange;
};

/// INT BLENDER EDITION INTERFACE
class CIntBlenderDialogClient : public CValueBlenderDialogClientT<sint32>
{
public:
	~CIntBlenderDialogClient() {}

	QWidget *newDialog(IPSWrapper<sint32> *wrapper, QWidget *parent)
	{
		CEditRangeIntWidget *erf = new CEditRangeIntWidget(parent);
		erf->setRange(MinRange, MaxRange);
		erf->setWrapper(wrapper);
		erf->updateUi();
		return erf;
	}
	sint32 MinRange, MaxRange;
};

/// RGBA BLENDER EDITION INTERFACE
class CRGBABlenderDialogClient : public CValueBlenderDialogClientT<NLMISC::CRGBA>
{
public:
	~CRGBABlenderDialogClient() {}

	QWidget *newDialog(IPSWrapper<NLMISC::CRGBA> *wrapper, QWidget *parent)
	{
		CColorEditWidget *ce = new CColorEditWidget(parent);
		ce->setWrapper(wrapper);
		ce->updateUi();
		return ce;
	}
};

} /* namespace NLQT */

#endif // VALUE_BLENDER_DIALOG_H
