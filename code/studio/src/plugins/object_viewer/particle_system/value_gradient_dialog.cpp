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

// Projects include
#include "stdpch.h"
#include "particle_texture_widget.h"
#include "value_gradient_dialog.h"

// Qt include
#include <QtGui/QMessageBox>

// NeL includes
#include <nel/3d/texture_grouped.h>
#include <nel/3d/texture_file.h>
#include <nel/misc/path.h>

namespace NLQT
{

CGradientDialog::CGradientDialog(CWorkspaceNode *ownerNode,
								 IValueGradientClient *clientInterface,
								 bool destroyClientInterface,
								 bool canTuneNbStages,
								 uint minSize,
								 QWidget *parent)
	: QDialog(parent),
	  m_minSize(minSize),
	  m_canTuneNbStages(canTuneNbStages),
	  m_clientInterface(clientInterface),
	  m_destroyClientInterface(destroyClientInterface),
	  m_node(ownerNode)
{
	nlassert(m_clientInterface);

	m_ui.setupUi(this);
	setWindowTitle(m_clientInterface->getTitleDialog());
	QWidget *widget = m_clientInterface->createDialog(this);
	m_ui.gridLayout->addWidget(widget, 4, 0, 1, 2);

	if (m_canTuneNbStages)
	{
		m_ui.nbStepWidget->setRange(1, 255);
		m_ui.nbStepWidget->enableLowerBound(0, true);
		m_ui.nbStepWidget->setValue(m_clientInterface->getNbSteps());
		connect(m_ui.nbStepWidget, SIGNAL(valueChanged(uint32)), this, SLOT(setNbSteps(uint32)));
	}
	else
	{
		m_ui.nbStepWidget->hide();
		m_ui.label->hide();
	}

	connect(m_ui.addButton, SIGNAL(clicked()), this, SLOT(addValue()));
	connect(m_ui.insButton, SIGNAL(clicked()), this, SLOT(insertValue()));
	connect(m_ui.delButton, SIGNAL(clicked()), this, SLOT(removeValue()));
	connect(m_ui.upButton, SIGNAL(clicked()), this, SLOT(valueUp()));
	connect(m_ui.downButton, SIGNAL(clicked()), this, SLOT(valueDown()));
	connect(m_ui.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(changeCurrentRow(int)));
	connect(clientInterface, SIGNAL(itemChanged()), this, SLOT(updateItem()));

	m_size = m_clientInterface->getSchemeSize();
	for (uint k = 0; k < m_size; ++k)
	{
		QListWidgetItem *item = new QListWidgetItem();
		m_clientInterface->displayValue(k, item);
		m_ui.listWidget->addItem(item);
	}
	m_ui.delButton->setEnabled(m_size > m_minSize ? true : false);
	m_ui.listWidget->setCurrentRow(0);
}

CGradientDialog::~CGradientDialog()
{
	if (m_destroyClientInterface) delete m_clientInterface;
}

void CGradientDialog::addValue()
{
	nlassert(m_clientInterface);
	if (!m_clientInterface->modifyGradient(IValueGradientClient::Add, 0)) return;
	++m_size;
	QListWidgetItem *item = new QListWidgetItem();
	m_clientInterface->displayValue(m_size - 1, item);
	m_ui.listWidget->addItem(item);

	m_ui.delButton->setEnabled(true);

	m_ui.listWidget->setCurrentRow(m_size - 1);
}

void CGradientDialog::insertValue()
{
	nlassert(m_clientInterface);
	int oldIndex = m_ui.listWidget->currentRow();
	if (!m_clientInterface->modifyGradient(IValueGradientClient::Insert, oldIndex)) return;
	++m_size;
	QListWidgetItem *item = new QListWidgetItem();
	m_clientInterface->displayValue(m_size - 1, item);
	m_ui.listWidget->insertItem(oldIndex, item);
	m_ui.listWidget->setCurrentRow(oldIndex);
	m_ui.delButton->setEnabled(true);
}

void CGradientDialog::removeValue()
{
	nlassert(m_clientInterface);
	int oldIndex = m_ui.listWidget->currentRow();
	if (oldIndex == -1)
		return;

	if (uint(oldIndex) == 0)
		m_ui.listWidget->setCurrentRow(oldIndex + 1);
	else
		m_ui.listWidget->setCurrentRow(oldIndex - 1);

	if (!m_clientInterface->modifyGradient(IValueGradientClient::Delete, oldIndex))
	{
		m_ui.listWidget->setCurrentRow(oldIndex);
		return;
	}

	--m_size;
	if (m_size <= m_minSize)
		m_ui.delButton->setEnabled(false);

	QListWidgetItem *removeItem = m_ui.listWidget->takeItem(oldIndex);
	if (!removeItem)
		delete removeItem;
}

void CGradientDialog::valueDown()
{
	nlassert(m_clientInterface);
	int currentRow = m_ui.listWidget->currentRow();
	if (!((currentRow == m_ui.listWidget->count()-1) || (currentRow == -1)))
	{
		if (!m_clientInterface->modifyGradient(IValueGradientClient::Down, currentRow)) return;
		QListWidgetItem *item = m_ui.listWidget->takeItem(currentRow);
		m_ui.listWidget->insertItem(++currentRow, item);
		m_ui.listWidget->setCurrentRow(currentRow);
	}
	m_ui.listWidget->setCurrentRow(currentRow);
	--currentRow;
	QListWidgetItem *item = m_ui.listWidget->item(currentRow);
	m_clientInterface->displayValue(currentRow, item);
}

void CGradientDialog::valueUp()
{
	nlassert(m_clientInterface);
	int currentRow = m_ui.listWidget->currentRow();
	if (!((currentRow == 0) || (currentRow == -1)))
	{
		if (!m_clientInterface->modifyGradient(IValueGradientClient::Up, currentRow)) return;
		QListWidgetItem *item = m_ui.listWidget->takeItem(currentRow);
		m_ui.listWidget->insertItem(--currentRow, item);
		m_ui.listWidget->setCurrentRow(currentRow);
	}
	m_ui.listWidget->setCurrentRow(currentRow);
}

void CGradientDialog::changeCurrentRow(int currentRow)
{
	m_clientInterface->setCurrentIndex(currentRow);
}

void CGradientDialog::updateItem()
{
	m_clientInterface->displayValue(m_ui.listWidget->currentRow(), m_ui.listWidget->currentItem());
}

void CGradientDialog::setNbSteps(uint32 nbSteps)
{
	m_clientInterface->setNbSteps(nbSteps);
}

QWidget *CTextureGradientInterface::createDialog(QWidget *parent)
{
	m_editWidget = new CParticleTextureWidget();

	m_textureWrapper.P = m_tp;
	m_textureWrapper.Index = 0;
	m_textureWrapper.OwnerNode = m_node;
	m_editWidget->setWrapper(&m_textureWrapper);
	connect(m_editWidget, SIGNAL(textureChanged(QString)), this, SIGNAL(itemChanged()));
	return m_editWidget;
}

bool CTextureGradientInterface::modifyGradient(TAction action, uint index)
{
	nlassert(m_tp);
	nlassert(m_tp->getTextureGroup());

	NLMISC::CSmartPtr<NL3D::ITexture> tex = m_tp->getTextureGroup()->getTexture(index);
	std::string texName = (static_cast<NL3D::CTextureFile *>(tex.getPtr()))->getFileName().c_str();
	if (texName.empty())
		return false;

	std::vector< NLMISC::CSmartPtr<NL3D::ITexture> > textureList;
	textureList.resize(m_tp->getTextureGroup()->getNbTextures());
	m_tp->getTextureGroup()->getTextures(&textureList[0]);

	switch(action)
	{
	case IValueGradientClient::Add:
	{
		// we duplicate the last texture, so that they have the same size
		NLMISC::CSmartPtr<NL3D::ITexture> lastTex = textureList[textureList.size() - 1];
		textureList.push_back(lastTex);
	}
	break;
	case IValueGradientClient::Insert:
	{
		// we duplicate the current texture, so that they have the same size
		NLMISC::CSmartPtr<NL3D::ITexture> tex = textureList[index];
		textureList.insert(textureList.begin() + index, tex);
	}
	break;
	case IValueGradientClient::Delete:
		textureList.erase(textureList.begin() + index);
		break;
	case IValueGradientClient::Up:
		return false;
	case IValueGradientClient::Down:
		return false;
	}

	m_tp->getTextureGroup()->setTextures(&textureList[0], (uint)textureList.size());
	return true;
}

void CTextureGradientInterface::displayValue(uint index, QListWidgetItem *item)
{
	QPixmap pixmap;
	NLMISC::CSmartPtr<NL3D::ITexture> tex = m_tp->getTextureGroup()->getTexture(index);

	if (dynamic_cast<NL3D::CTextureFile *>(tex.getPtr()))
	{
		std::string texName = (static_cast<NL3D::CTextureFile *>(tex.getPtr()))->getFileName().c_str();
		if (!texName.empty())
		{
			pixmap.load(NLMISC::CPath::lookup(texName, false).c_str());
			item->setText(texName.c_str());
		}
		else
			item->setText("Dummy texture");
	}
	item->setIcon(QIcon(pixmap));
}

void CTextureGradientInterface::setCurrentIndex(uint index)
{
	m_textureWrapper.Index = index;
	m_editWidget->updateUi();
}

uint32 CTextureGradientInterface::getSchemeSize(void) const
{
	nlassert(m_tp->getTextureGroup());
	return m_tp->getTextureGroup()->getNbTextures();
}
uint32 CTextureGradientInterface::getNbSteps(void) const
{
	return 1;
}
void CTextureGradientInterface::setNbSteps(uint32 value)
{
	// this should never be called, as we don't allow nbsteps to be called
	nlassert(false);
}

NL3D::ITexture *CTextureGradientInterface::CTextureWrapper::get(void)
{
	nlassert(P);
	nlassert(P->getTextureGroup());
	return P->getTextureGroup()->getTexture(Index);
}

void CTextureGradientInterface::CTextureWrapper::set(NL3D::ITexture *t)
{
	nlassert(P);
	nlassert(P->getTextureGroup());

	// if a texture is added, it must have the same size than other textures
	if (P->getTextureGroup()->getNbTextures() > 1)
	{
		NLMISC::CSmartPtr<NL3D::ITexture> tex = P->getTextureGroup()->getTexture(0);
		tex->generate();
		t->generate();

		if (t->getWidth() != tex->getWidth() || t->getHeight() != tex->getHeight())
		{
			QMessageBox::critical(0, QString("Texture error"), QString("All textures must have the same size !"),  QMessageBox::Ok);
			return;
		}

		if (t->PixelFormat != tex->PixelFormat)
		{
			QMessageBox::critical(0, QString("Texture error"), QString("All textures must have the same pixel format !"),  QMessageBox::Ok);
			return;
		}
	}

	std::vector< NLMISC::CSmartPtr<NL3D::ITexture> > textureList;
	textureList.resize(P->getTextureGroup()->getNbTextures());
	P->getTextureGroup()->getTextures(&textureList[0]);

	textureList[Index] = t;

	P->getTextureGroup()->setTextures(&textureList[0], (uint)textureList.size());
}

} /* namespace NLQT */