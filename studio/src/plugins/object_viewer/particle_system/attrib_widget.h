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

#ifndef ATTRIB_WIDGET_H
#define ATTRIB_WIDGET_H

#include <nel/misc/types_nl.h>
#include "ui_attrib_form.h"

// Qt includes
#include <QtGui/QDialog>

// STL includes

// NeL includes
#include <nel/misc/config_file.h>
#include <nel/misc/rgba.h>
#include <nel/3d/ps_plane_basis.h>
#include <nel/3d/ps_attrib_maker.h>

// Project includes
#include "ps_wrapper.h"
#include "particle_node.h"

namespace NLQT
{
/**
@class CAttribWidget
@brief Base attrib maker edition dialog.
*/
class CAttribWidget: public  QGroupBox
{
	Q_OBJECT

public:
	CAttribWidget(QWidget *parent = 0);
	~CAttribWidget();

	///  @param enableConstantValue - when false, only a scheme is available
	void setEnabledConstantValue(bool enableConstantValue = true);

	/// Force to update dialog content
	void updateUi();

	/// Connects all the slots with signals
	void init();

	/// Sets the pointer CWorkspaceNode* in the wrappers.
	virtual void setWorkspaceNode(CWorkspaceNode *node) = 0;

	/// Private usage (not private because accessed by a static function) : return the nbCycles parameter of the scheme (e.g the input multiplier).
	virtual float getSchemeNbCycles(void) const = 0;

	/// Private usage (not private because accessed by a static function) : set the nbCycles parameter of the scheme (e.g the input multiplier)
	virtual void setSchemeNbCycles(float nbCycles) = 0;

	/// Enable the srcInput
	void enableSrcInput(bool enable = true)
	{
		_SrcInputEnabled = enable;
	}

	bool isSrcInputEnabled() const
	{
		return _SrcInputEnabled;
	}

	/// Disable the possibility to choose a scheme that has memory. (for example, a scheme for lifetime of a located has no sense
	/// because located have already some memory to store it)
	void enableMemoryScheme(bool enabled = true);

	/// Tells wether memory schemes are enables
	/// @see enableMemoryScheme()
	bool isMemorySchemeEnabled() const
	{
		return !_DisableMemoryScheme;
	}

	/// Enable Nb Cycle tuning
	void	enableNbCycles(bool enabled)
	{
		_NbCycleEnabled = enabled;
	}
	bool    isNbCycleEnabled() const
	{
		return _NbCycleEnabled;
	}

private Q_SLOTS:
	virtual void clickedEdit();
	virtual void setClamp(bool state);
	virtual void changeCurrentScheme(int index);
	virtual void setCurrentSrc(int index);
	virtual void setUserIndex();
	virtual void changeUseScheme(int index);
	virtual void openSchemeBankDialog();

protected:

	/// change the dialog for constant values
	virtual void cstValueUpdate() = 0;

	/// enable / disable the 'edit input' button, when input can be edited
	void inputValueUpdate(void);

	/// toggle back from scheme to cst value
	virtual void resetCstValue(void) = 0;

	/// change the dialog for scheme usage
	void schemeValueUpdate();

	/// return true if a scheme is used
	virtual bool useScheme(void) const = 0;

	/// edit the current scheme. And return a window on it
	virtual QDialog *editScheme(void) = 0;

	/// set the current scheme
	virtual void setCurrentScheme(uint index) = 0;

	/// set the current scheme ptr
	virtual void setCurrentSchemePtr(NL3D::CPSAttribMakerBase *) = 0;

	/// get the current scheme, -1 if the scheme is unknow (created outside the editor ?)
	virtual sint getCurrentScheme(void) const = 0;

	/// get a pointer on the current scheme base class
	virtual NL3D::CPSAttribMakerBase *getCurrentSchemePtr(void) const = 0;

	/// tells wether the scheme supports custom input
	virtual bool hasSchemeCustomInput(void) const = 0;

	/// retrieve the scheme input id
	virtual NL3D::CPSInputType getSchemeInput(void) const = 0;

	/// set the scheme input id
	virtual void setSchemeInput(const NL3D::CPSInputType &input) = 0;

	/// tells wether the scheme input value is clamped or not
	virtual bool isSchemeClamped(void) const = 0;

	/// clamp / unclamp the scheme
	virtual void clampScheme(bool clamped = true) = 0;

	/// return true if clamping is supported
	virtual bool isClampingSupported(void) const = 0;

	/// bool : true is src input are allowed
	bool _SrcInputEnabled;

	/// true if constant values are allowed
	bool _EnableConstantValue;

	/// this is equal to true when memory schemes are not permitted
	bool _DisableMemoryScheme;

	/// true to enable 'nb cycles' control
	bool _NbCycleEnabled;

	/// wrapper to tune the number of cycles
	struct CNbCyclesWrapper : public IPSWrapperFloat
	{
		CAttribWidget *widget;
		float get(void) const
		{
			return widget->getSchemeNbCycles();
		}
		void set(const float &v)
		{
			widget->setSchemeNbCycles(v);
		}
	} _NbCyclesWrapper;

	CWorkspaceNode *_Node;

	QDialog *_SchemeWidget;

	Ui::CAttribWidget _ui;
	friend class CSchemeBankDialog;
}; /* class CAttribWidget */

/**
@class CAttribWidgetT
@brief A template class that helps to specialize the attrib maker edition dialog with various types.
*/
template <typename T> class CAttribWidgetT : public CAttribWidget
{
public:
	CAttribWidgetT(QWidget *parent = 0): CAttribWidget(parent),
		_Wrapper(NULL),
		_SchemeWrapper(NULL)
	{
	}
	virtual void setWrapper(IPSWrapper<T> *wrapper) = 0;
	void setSchemeWrapper(IPSSchemeWrapper<T> *schemeWrapper)
	{
		nlassert(schemeWrapper);
		_SchemeWrapper = schemeWrapper;
	}

	// Inherited from CAttribWidget
	virtual QDialog *editScheme(void) = 0;
	virtual void setCurrentScheme(uint index) = 0;
	virtual sint getCurrentScheme(void) const  = 0;

	virtual void resetCstValue(void)
	{
		_Wrapper->setAndUpdateModifiedFlag(_Wrapper->get()); // reuse current color
	}

	virtual bool hasSchemeCustomInput(void) const
	{
		return _SchemeWrapper->getScheme()->hasCustomInput();
	}
	virtual NL3D::CPSInputType getSchemeInput(void) const
	{
		return  _SchemeWrapper->getScheme()->getInput();
	}
	virtual void setSchemeInput(const NL3D::CPSInputType &input)
	{
		_SchemeWrapper->getScheme()->setInput(input);
	}

	virtual void setWorkspaceNode(CWorkspaceNode *node)
	{
		_Node = node;
		if (_Wrapper != NULL) _Wrapper->OwnerNode = _Node;
		if (_SchemeWrapper != NULL) _SchemeWrapper->OwnerNode = _Node;
	};

	virtual float getSchemeNbCycles(void) const
	{
		return _SchemeWrapper->getScheme()->getNbCycles();
	}
	virtual void setSchemeNbCycles(float nbCycles)
	{
		_SchemeWrapper->getScheme()->setNbCycles(nbCycles);
	}

	virtual bool isSchemeClamped(void) const
	{
		return _SchemeWrapper->getScheme()->getClamping();
	}
	virtual void clampScheme(bool clamped = true)
	{
		_SchemeWrapper->getScheme()->setClamping(clamped);
	}
	virtual bool isClampingSupported(void) const
	{
		return _SchemeWrapper->getScheme()->isClampingSupported();
	};
	virtual NL3D::CPSAttribMakerBase *getCurrentSchemePtr(void) const
	{
		return _SchemeWrapper->getScheme();
	}
	virtual void setCurrentSchemePtr(NL3D::CPSAttribMakerBase *s)
	{
		_SchemeWrapper->setSchemeAndUpdateModifiedFlag(NLMISC::safe_cast<NL3D::CPSAttribMaker<T> *>(s));
	}
	virtual void cstValueUpdate() = 0;

protected:
	virtual bool useScheme(void) const
	{
		nlassert(_SchemeWrapper);
		return(_SchemeWrapper->getScheme() != NULL);
	}
public:
	/// Wrapper to set/get a constant float
	IPSWrapper<T> *_Wrapper;
	/// Wrapper to set/get a scheme
	IPSSchemeWrapper<T> *_SchemeWrapper;
};

/**
@class CAttribFloatWidget
@brief An attribute editor specialized for float values
*/
class CAttribFloatWidget: public CAttribWidgetT<float>
{
	Q_OBJECT

public:
	CAttribFloatWidget(QWidget *parent = 0);
	~CAttribFloatWidget();

	void setRange(float minValue = 0, float maxValue = 10);
	void setWrapper(IPSWrapper<float> *wrapper);

	// inherited from CAttribWidget
	virtual QDialog *editScheme(void);
	virtual void setCurrentScheme(uint index);
	virtual sint getCurrentScheme(void) const;
	virtual void cstValueUpdate();

private:

	float _MinRange, _MaxRange;
}; /* class CAttribFloatWidget */

/**
@class CAttribUIntWidget
@brief An attribute editor specialized for unsigned int values
*/
class CAttribUIntWidget: public CAttribWidgetT<uint32>
{
	Q_OBJECT

public:
	CAttribUIntWidget(QWidget *parent = 0);
	~CAttribUIntWidget();

	void setRange(uint32 minValue = 0, uint32 maxValue = 10);
	void setWrapper(IPSWrapper<uint32> *wrapper);

	// inherited from CAttribWidget
	virtual QDialog *editScheme(void);
	virtual void setCurrentScheme(uint index);
	virtual sint getCurrentScheme(void) const;
	virtual void cstValueUpdate();

private:

	uint32 _MinRange, _MaxRange;
}; /* class CAttribUIntWidget */

/**
@class CAttribIntWidget
@brief An attribute editor specialized for signed int values
*/
class CAttribIntWidget: public CAttribWidgetT<sint32>
{
	Q_OBJECT

public:
	CAttribIntWidget(QWidget *parent = 0);
	~CAttribIntWidget();

	void setRange(sint32 minValue = 0, sint32 maxValue = 10);
	void setWrapper(IPSWrapper<sint32> *wrapper);

	// inherited from CAttribWidget
	virtual QDialog *editScheme(void);
	virtual void setCurrentScheme(uint index);
	virtual sint getCurrentScheme(void) const;
	virtual void cstValueUpdate();

private:

	sint32 _MinRange, _MaxRange;
}; /* class CAttribIntWidget */

/**
@class CAttribRGBAWidget
@brief An attribute editor specialized for RGB values
*/
class CAttribRGBAWidget: public CAttribWidgetT<NLMISC::CRGBA>
{
	Q_OBJECT

public:
	CAttribRGBAWidget(QWidget *parent = 0);
	~CAttribRGBAWidget();

	void setWrapper(IPSWrapper<NLMISC::CRGBA> *wrapper);

	// inherited from CAttribWidget
	virtual QDialog *editScheme(void);
	virtual void setCurrentScheme(uint index);
	virtual sint getCurrentScheme(void) const;
	virtual void cstValueUpdate();

private:

}; /* class CAttribRGBAWidget */

/**
@class CAttribPlaneBasisWidget
@brief An attribute editor specialized for plane basis values
*/
class CAttribPlaneBasisWidget: public CAttribWidgetT<NL3D::CPlaneBasis>
{
	Q_OBJECT

public:
	CAttribPlaneBasisWidget(QWidget *parent = 0);
	~CAttribPlaneBasisWidget();

	void setWrapper(IPSWrapper<NL3D::CPlaneBasis> *wrapper);

	// inherited from CAttribWidget
	virtual QDialog *editScheme(void);
	virtual void setCurrentScheme(uint index);
	virtual sint getCurrentScheme(void) const;
	virtual void cstValueUpdate();

private:

}; /* class CAttribPlaneBasisWidget */

} /* namespace NLQT */

#endif // ATTRIB_WIDGET_H
