// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef VALUE_GRADIENT_DIALOG_H
#define VALUE_GRADIENT_DIALOG_H

// Project includes
#include "ui_value_gradient_form.h"
#include "basic_edit_widget.h"
#include "color_edit_widget.h"
#include "edit_range_widget.h"
#include "ps_wrapper.h"
#include "particle_node.h"

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
	void setNbSteps(uint32 nbSteps);

protected:

	// the minimum number of element in the gradient
	uint m_minSize;

	// false to disable the dialog that control the number of stages between each value
	bool m_canTuneNbStages;

	IValueGradientClient *m_clientInterface;

	bool m_destroyClientInterface;

	// the current size of the gradient
	uint m_size;

	CWorkspaceNode *m_node;
	Ui::CGradientDialog m_ui;
}; /* class CGradientDialog */

/**
@class CValueGradientClientT
@brief This template generate an interface that is used with the gradient edition dialog
This the type to be edited (color, float, etc..)
 */
template <typename T>
class CValueGradientClientT : public IValueGradientClient, public IPSWrapper<T>
{
public:
	CValueGradientClientT(QObject *parent = 0): IValueGradientClient(parent) {}
	virtual ~CValueGradientClientT() {}

	/// the gradient being edited, must be filled by the instancier
	NL3D::CPSValueGradientFunc<T> *m_scheme;

	/// the gradient dialog, must be filled by the instancier
	T m_defaultValue;

	/// inherited from IPSWrapper
	virtual T get(void) const
	{
		return m_scheme->getValue(m_currentEditedIndex);
	}
	virtual void set(const T &v)
	{
		T *tab = new T[m_scheme->getNumValues()];
		m_scheme->getValues(tab);
		tab[m_currentEditedIndex] = v;
		m_scheme->setValues(tab, m_scheme->getNumValues(), m_scheme->getNumStages());
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

		T *tab = new T[m_scheme->getNumValues() + 1]; // +1 is for the add / insert case
		m_scheme->getValues(tab);

		switch(action)
		{
		case IValueGradientClient::Add:
			tab[m_scheme->getNumValues()] = m_defaultValue;
			m_scheme->setValues(tab, m_scheme->getNumValues() + 1, m_scheme->getNumStages());
			break;
		case IValueGradientClient::Insert:
			::memmove(tab + (index + 1), tab + index, sizeof(T) * (m_scheme->getNumValues() - index));
			tab[index] = m_defaultValue;
			m_scheme->setValues(tab, m_scheme->getNumValues() + 1, m_scheme->getNumStages());
			break;
		case IValueGradientClient::Delete:
			::memmove(tab + index, tab + index + 1, sizeof(T) * (m_scheme->getNumValues() - index - 1));
			m_scheme->setValues(tab, m_scheme->getNumValues() - 1, m_scheme->getNumStages());
			break;
		case IValueGradientClient::Up:
			nlassert(index > 0);
			std::swap(tab[index], tab[index - 1]);
			m_scheme->setValues(tab, m_scheme->getNumValues(), m_scheme->getNumStages());
			break;
		case IValueGradientClient::Down:
			nlassert(index <  m_scheme->getNumValues() - 1);
			std::swap(tab[index], tab[index + 1]);
			m_scheme->setValues(tab, m_scheme->getNumValues(), m_scheme->getNumStages());
			break;
		}

		delete[] tab;
		return true;
	}
	virtual uint32 getSchemeSize(void) const
	{
		return m_scheme->getNumValues();
	}

	/// Get the number of interpolation step
	uint32 getNbSteps(void) const
	{
		return m_scheme->getNumStages();
	}

	/// Set the number of interpolation steps
	void setNbSteps(uint32 value)
	{
		m_scheme->setNumStages(value);
	}

protected:
	// index of the value OF the current dialog that exist
	uint32 m_currentEditedIndex;
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
		m_editWidget = new CEditRangeFloatWidget(parent);
		m_editWidget->setRange(m_minRange, m_maxRange);
		m_editWidget->setWrapper(wrapper);
		connect(m_editWidget ,SIGNAL(valueChanged(float)), this, SIGNAL(itemChanged()));
		return m_editWidget;
	}

	virtual QString getTitleDialog() const
	{
		return tr("Float values gradient dialog");
	}

	virtual void setCurrentIndex(uint index)
	{
		m_currentEditedIndex = index;
		m_editWidget->updateUi();
	}

	virtual void displayValue(uint index, QListWidgetItem *item)
	{
		item->setText(QString("%1").arg(m_scheme->getValue(index), 0, 'f', 2));
	}

	CEditRangeFloatWidget *m_editWidget;
	float m_minRange, m_maxRange;
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
		m_editWidget = new CEditRangeUIntWidget(parent);
		m_editWidget->setRange(m_minRange, m_maxRange);
		m_editWidget->setWrapper(wrapper);
		connect(m_editWidget, SIGNAL(valueChanged(uint32)), this, SIGNAL(itemChanged()));
		return m_editWidget;
	}

	virtual QString getTitleDialog() const
	{
		return tr("UInt values gradient dialog");
	}

	virtual void setCurrentIndex(uint index)
	{
		m_currentEditedIndex = index;
		m_editWidget->updateUi();
	}

	virtual void displayValue(uint index, QListWidgetItem *item)
	{
		item->setText(QString("%1").arg(m_scheme->getValue(index)));
	}

	CEditRangeUIntWidget *m_editWidget;
	uint32 m_minRange, m_maxRange;
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
		m_editWidget = new CEditRangeIntWidget(parent);
		m_editWidget->setRange(m_minRange, m_maxRange);
		m_editWidget->setWrapper(wrapper);
		connect(m_editWidget, SIGNAL(valueChanged(sint32)), this, SIGNAL(itemChanged()));
		return m_editWidget;
	}

	virtual QString getTitleDialog() const
	{
		return tr("Int values gradient dialog");
	}

	virtual void setCurrentIndex(uint index)
	{
		m_currentEditedIndex = index;
		m_editWidget->updateUi();
	}

	virtual void displayValue(uint index, QListWidgetItem *item)
	{
		item->setText(QString("%1").arg(m_scheme->getValue(index)));
	}

	CEditRangeIntWidget *m_editWidget;
	sint32 m_minRange, m_maxRange;
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
		m_editWidget = new CColorEditWidget(parent);
		m_editWidget->setWrapper(wrapper);
		connect(m_editWidget, SIGNAL(colorChanged(NLMISC::CRGBA)), this, SIGNAL(itemChanged()));
		return m_editWidget;
	}

	virtual QString getTitleDialog() const
	{
		return tr("Color gradient dialog");
	}

	virtual void setCurrentIndex(uint index)
	{
		m_currentEditedIndex = index;
		m_editWidget->updateUi();
	}

	virtual void displayValue(uint index, QListWidgetItem *item)
	{
		NLMISC::CRGBA color = m_scheme->getValue(index);
		item->setText(QString("RGBA(%1,%2,%3,%4)").arg(color.R).arg(color.G).arg(color.B).arg(color.A));
		QPixmap pixmap(QSize(16, 16));
		QPainter painter(&pixmap);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setBrush(QBrush(QColor(color.R, color.G, color.B)));
		painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
		painter.drawRect(0, 0, pixmap.width(), pixmap.height());
		item->setIcon(QIcon(pixmap));
	}

	CColorEditWidget *m_editWidget;
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
		m_editWidget = new CBasicEditWidget(parent);
		m_editWidget->setWrapper(wrapper);
		return m_editWidget;
	}
	virtual QString getTitleDialog() const
	{
		return tr("Plane basis gradient dialog");
	}
	virtual void setCurrentIndex(uint index)
	{
		m_currentEditedIndex = index;
		m_editWidget->updateUi();
	}

	virtual void displayValue(uint index, QListWidgetItem *item)
	{
		item->setText(QString("Plane %1").arg(index));
	}

	CBasicEditWidget *m_editWidget;
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

	CTextureGradientInterface(NL3D::CPSTexturedParticle *tp, CWorkspaceNode *ownerNode): m_node(ownerNode), m_tp(tp) {}

	~CTextureGradientInterface() {}

	CWorkspaceNode *m_node;
	NL3D::CPSTexturedParticle *m_tp;

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
	} m_textureWrapper;

	CParticleTextureWidget *m_editWidget;
};

} /* namespace NLQT */

#endif // VALUE_GRADIENT_DIALOG_H
