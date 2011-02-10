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

#ifndef VALUE_GRADIENT_DIALOG_H
#define VALUE_GRADIENT_DIALOG_H

#include <nel/misc/types_nl.h>

// Qt includes
#include <QtGui/QIcon>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QPainter>

// NeL includes
#include <nel/3d/ps_attrib_maker_template.h>

// Project includes
#include "basic_edit_widget.h"
#include "color_edit_widget.h"
#include "edit_range_widget.h"
#include "ps_wrapper.h"
#include "particle_node.h"

namespace NL3D
{
class CPSTexturedParticle;
}

namespace NLQT
{

class  CParticleTextureWidget;

class IValueGradientClient: public QObject
{
	Q_OBJECT
public:
	IValueGradientClient(QObject *parent = 0): QObject(parent) {}
	virtual ~IValueGradientClient() {}

	virtual QWidget *createDialog(QWidget *parent) = 0;

	/// Return the title of dialog in client
	virtual QString getTitleDialog() const = 0;

	/// This enumerate the action that we can apply on a gradient
	enum TAction { Add, Insert, Delete, Up, Down };

	/// A function that can display a value in a gradient, with the given offset
	virtual void displayValue(uint index, QListWidgetItem *item) = 0;

	virtual void setCurrentIndex(uint index) = 0;

	/// A function that can add, remove, or insert a new element in the gradient
	virtual bool modifyGradient(TAction, uint index) = 0;

	/// Return the number of values in a scheme
	virtual uint32 getSchemeSize(void) const = 0;

	/// Get the number of interpolation step
	virtual uint32 getNbSteps(void) const = 0;

	/// Set the number of interpolation steps
	virtual void setNbSteps(uint32 value) = 0;

Q_SIGNALS:
	void itemChanged();
};

class CGradientDialog: public  QDialog
{
	Q_OBJECT

public:
	CGradientDialog(CWorkspaceNode *ownerNode,
					IValueGradientClient *clientInterface,
					bool destroyClientInterface,
					bool canTuneNbStages = true,
					uint minSize = 2,
					QWidget *parent = 0);
	~CGradientDialog();

private Q_SLOTS:
	void addValue();
	void insertValue();
	void removeValue();
	void valueDown();
	void valueUp();
	void changeCurrentRow(int currentRow);
	void updateItem();

protected:

	// the minimum number of element in the gradient
	uint _MinSize;

	// false to disable the dialog that control the number of stages between each value
	bool _CanTuneNbStages;

	IValueGradientClient *_ClientInterface;

	bool _DestroyClientInterface;

	// the current size of the gradient
	uint _Size;

	CWorkspaceNode *_Node;

	// a wrapper to tune the number of step
	struct CNbStepWrapper :public IPSWrapperUInt
	{
		// the interface that was passed to the dialog this struct is part of
		IValueGradientClient *I;
		uint32 get(void) const
		{
			return I->getNbSteps();
		}
		void set(const uint32 &nbSteps)
		{
			I->setNbSteps(nbSteps);
		}

	} _NbStepWrapper;

	QGridLayout *_gridLayout;
	QListWidget *_listWidget;
	QHBoxLayout *_horizontalLayout;
	QPushButton *_addPushButton;
	QPushButton *_removePushButton;
	QPushButton *_upPushButton;
	QPushButton *_downPushButton;
	QSpacerItem *_horizontalSpacer;
	QLabel *_label;
	NLQT::CEditRangeUIntWidget *_nbStepWidget;
	QSpacerItem *_verticalSpacer;
	QWidget *editWidget;
}; /* class CGradientDialog */

/**
@class CValueGradientClientT
@brief This template generate an interface that is used with the gradient edition dialog
This the type to be edited (color, floet, etc..)
 */
template <typename T>
class CValueGradientClientT : public IValueGradientClient, public IPSWrapper<T>
{
public:
	CValueGradientClientT(QObject *parent = 0): IValueGradientClient(parent) {}
	virtual ~CValueGradientClientT() {}

	/// the gradient being edited, must be filled by the instancier
	NL3D::CPSValueGradientFunc<T> *Scheme;

	/// the gradient dialog, must be filled by the instancier
	T DefaultValue;

	/// inherited from IPSWrapper
	virtual T get(void) const
	{
		return Scheme->getValue(_CurrentEditedIndex);
	}
	virtual void set(const T &v)
	{
		T *tab = new T[Scheme->getNumValues()];
		Scheme->getValues(tab);
		tab[_CurrentEditedIndex] = v;
		Scheme->setValues(tab, Scheme->getNumValues(), Scheme->getNumStages());
		delete[] tab;
	}

	virtual QWidget *createDialog(QWidget *parent)
	{
		return newDialog(this, parent);
	}

	virtual QString getTitleDialog() const = 0;

	/// create a new dialog with given id and wrapper
	virtual  QWidget *newDialog(IPSWrapper<T> *wrapper, QWidget *parent) = 0;

	virtual void setCurrentIndex(uint index) = 0;

	virtual void displayValue(uint index, QListWidgetItem *item) = 0;

	/// a function that can add, remove, or insert a new element in the gradient
	virtual bool modifyGradient(TAction action, uint index)
	{

		T *tab = new T[Scheme->getNumValues() + 1]; // +1 is for the add / insert case
		Scheme->getValues(tab);

		switch(action)
		{
		case IValueGradientClient::Add:
			tab[Scheme->getNumValues()] = DefaultValue;
			Scheme->setValues(tab, Scheme->getNumValues() + 1, Scheme->getNumStages());
			break;
		case IValueGradientClient::Insert:
			::memmove(tab + (index + 1), tab + index, sizeof(T) * (Scheme->getNumValues() - index));
			tab[index] = DefaultValue;
			Scheme->setValues(tab, Scheme->getNumValues() + 1, Scheme->getNumStages());
			break;
		case IValueGradientClient::Delete:
			::memmove(tab + index, tab + index + 1, sizeof(T) * (Scheme->getNumValues() - index - 1));
			Scheme->setValues(tab, Scheme->getNumValues() - 1, Scheme->getNumStages());
			break;
		case IValueGradientClient::Up:
			nlassert(index > 0);
			std::swap(tab[index], tab[index - 1]);
			Scheme->setValues(tab, Scheme->getNumValues(), Scheme->getNumStages());
			break;
		case IValueGradientClient::Down:
			nlassert(index <  Scheme->getNumValues() - 1);
			std::swap(tab[index], tab[index + 1]);
			Scheme->setValues(tab, Scheme->getNumValues(), Scheme->getNumStages());
			break;
		}

		delete[] tab;
		return true;
	}
	virtual uint32 getSchemeSize(void) const
	{
		return Scheme->getNumValues();
	}

	/// Get the number of interpolation step
	uint32 getNbSteps(void) const
	{
		return Scheme->getNumStages();
	}

	/// Set the number of interpolation steps
	void setNbSteps(uint32 value)
	{
		Scheme->setNumStages(value);
	}

protected:
	// index of the value OF the current dialog that exist
	uint32 _CurrentEditedIndex;
}; /* class CValueGradientClientT */

/// FLOAT GRADIENT EDITION INTERFACE
class CFloatGradientWrapper : public CValueGradientClientT<float>
{
	Q_OBJECT
public:
	CFloatGradientWrapper(QObject *parent = 0): CValueGradientClientT<float>(parent) {}
	~CFloatGradientWrapper() {}

	virtual QWidget *newDialog(IPSWrapperFloat *wrapper, QWidget *parent)
	{
		editWidget = new CEditRangeFloatWidget(parent);
		editWidget->setRange(MinRange, MaxRange);
		editWidget->setWrapper(wrapper);
		connect(editWidget, SIGNAL(valueChanged(float)), this, SIGNAL(itemChanged()));
		return editWidget;
	}

	virtual QString getTitleDialog() const
	{
		return tr("Float values gradient dialog");
	}

	virtual void setCurrentIndex(uint index)
	{
		_CurrentEditedIndex = index;
		editWidget->updateUi();
	}

	virtual void displayValue(uint index, QListWidgetItem *item)
	{
		item->setText(QString("%1").arg(Scheme->getValue(index),0,'f',2));
	}

	CEditRangeFloatWidget *editWidget;
	float MinRange, MaxRange;
}; /* CFloatGradientWrapper */

/// UINT GRADIENT EDITION INTERFACE
class CUIntGradientWrapper : public CValueGradientClientT<uint32>
{
	Q_OBJECT
public:
	CUIntGradientWrapper(QObject *parent = 0): CValueGradientClientT<uint32>(parent) {}
	~CUIntGradientWrapper() {}

	virtual QWidget *newDialog(IPSWrapperUInt *wrapper, QWidget *parent)
	{
		editWidget = new CEditRangeUIntWidget(parent);
		editWidget->setRange(MinRange, MaxRange);
		editWidget->setWrapper(wrapper);
		connect(editWidget, SIGNAL(valueChanged(uint32)), this, SIGNAL(itemChanged()));
		return editWidget;
	}

	virtual QString getTitleDialog() const
	{
		return tr("UInt values gradient dialog");
	}

	virtual void setCurrentIndex(uint index)
	{
		_CurrentEditedIndex = index;
		editWidget->updateUi();
	}

	virtual void displayValue(uint index, QListWidgetItem *item)
	{
		item->setText(QString("%1").arg(Scheme->getValue(index)));
	}

	CEditRangeUIntWidget *editWidget;
	uint32 MinRange, MaxRange;
}; /* CUIntGradientWrapper */

/// INT GRADIENT EDITION INTERFACE
class CIntGradientWrapper : public CValueGradientClientT<sint32>
{
	Q_OBJECT
public:
	CIntGradientWrapper(QObject *parent = 0): CValueGradientClientT<sint32>(parent) {}
	~CIntGradientWrapper() {}

	virtual QWidget *newDialog(IPSWrapper<sint32> *wrapper, QWidget *parent)
	{
		editWidget = new CEditRangeIntWidget(parent);
		editWidget->setRange(MinRange, MaxRange);
		editWidget->setWrapper(wrapper);
		connect(editWidget, SIGNAL(valueChanged(sint32)), this, SIGNAL(itemChanged()));
		return editWidget;
	}

	virtual QString getTitleDialog() const
	{
		return tr("Int values gradient dialog");
	}

	virtual void setCurrentIndex(uint index)
	{
		_CurrentEditedIndex = index;
		editWidget->updateUi();
	}

	virtual void displayValue(uint index, QListWidgetItem *item)
	{
		item->setText(QString("%1").arg(Scheme->getValue(index)));
	}

	CEditRangeIntWidget *editWidget;
	sint32 MinRange, MaxRange;
}; /* CIntGradientWrapper */

/// COLOR GRADIENT EDITION INTERFACE
class CColorGradientWrapper : public CValueGradientClientT<NLMISC::CRGBA>
{
	Q_OBJECT
public:
	CColorGradientWrapper(QObject *parent = 0): CValueGradientClientT<NLMISC::CRGBA>(parent) {}
	~CColorGradientWrapper() {}

	virtual QWidget *newDialog(IPSWrapper<NLMISC::CRGBA> *wrapper, QWidget *parent)
	{
		editWidget = new CColorEditWidget(parent);
		editWidget->setWrapper(wrapper);
		connect(editWidget, SIGNAL(colorChanged(NLMISC::CRGBA)), this, SIGNAL(itemChanged()));
		return editWidget;
	}

	virtual QString getTitleDialog() const
	{
		return tr("Color gradient dialog");
	}

	virtual void setCurrentIndex(uint index)
	{
		_CurrentEditedIndex = index;
		editWidget->updateUi();
	}

	virtual void displayValue(uint index, QListWidgetItem *item)
	{
		NLMISC::CRGBA color = Scheme->getValue(index);
		item->setText(QString("RGBA(%1,%2,%3,%4)").arg(color.R).arg(color.G).arg(color.B).arg(color.A));
		QPixmap pixmap(QSize(16, 16));
		QPainter painter(&pixmap);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setBrush(QBrush(QColor(color.R, color.G, color.B)));
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
		painter.drawRect(0, 0, pixmap.width() , pixmap.height());
		item->setIcon(QIcon(pixmap));
	}

	CColorEditWidget *editWidget;
}; /* CColorGradientWrapper  */

/// PLANE BASIS GRADIENT EDITION INTERFACE
class CPlaneBasisGradientWrapper : public CValueGradientClientT<NL3D::CPlaneBasis>
{
	Q_OBJECT
public:
	CPlaneBasisGradientWrapper(QObject *parent = 0): CValueGradientClientT<NL3D::CPlaneBasis>(parent) {}
	~CPlaneBasisGradientWrapper() {}

	virtual QWidget *newDialog(IPSWrapper<NL3D::CPlaneBasis> *wrapper, QWidget *parent)
	{
		editWidget = new CBasicEditWidget(parent);
		editWidget->setWrapper(wrapper);
		return editWidget;
	}
	virtual QString getTitleDialog() const
	{
		return tr("Plane basis gradient dialog");
	}
	virtual void setCurrentIndex(uint index)
	{
		_CurrentEditedIndex = index;
		editWidget->updateUi();
	}

	virtual void displayValue(uint index, QListWidgetItem *item)
	{
		item->setText(QString("Plane %1").arg(index));
	}

	CBasicEditWidget *editWidget;
}; /* class CPlaneBasisGradientWrapper */

/**
@class CTextureGradientInterface
@brief The implementation of this struct tells the gradient dialog bow how to edit a texture list
*/
class CTextureGradientInterface : public IValueGradientClient
{
	Q_OBJECT
public:
	CTextureGradientInterface(QObject *parent = 0): IValueGradientClient(parent) {}

	CTextureGradientInterface(NL3D::CPSTexturedParticle *tp, CWorkspaceNode *ownerNode): Node(ownerNode), TP(tp) {}

	~CTextureGradientInterface() {}

	CWorkspaceNode *Node;
	NL3D::CPSTexturedParticle *TP;

	// all method inherited from IValueGradientClient
	virtual QWidget *createDialog(QWidget *parent);
	virtual QString getTitleDialog() const
	{
		return tr("Texture grouped dialog");
	}
	virtual bool modifyGradient(TAction, uint index);
	virtual void displayValue(uint index, QListWidgetItem *item);
	virtual void setCurrentIndex(uint index);
	virtual uint32 getSchemeSize(void) const;
	virtual uint32 getNbSteps(void) const;
	virtual void setNbSteps(uint32 value);

	/// wrapper for the texture chooser
	/// that allows to choose a texture in the list
	struct CTextureWrapper : public IPSWrapperTexture
	{
		NL3D::CPSTexturedParticle *P;
		// index of the particle in the list
		uint32 Index;
		NL3D::ITexture *get(void);
		void set(NL3D::ITexture *t);
	} _TextureWrapper;

	CParticleTextureWidget *editWidget;
};

} /* namespace NLQT */

#endif // VALUE_GRADIENT_DIALOG_H
