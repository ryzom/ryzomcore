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

#include "stdpch.h"
#include "particle_texture_widget.h"

// Qt include
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

// NeL includes
#include "nel/3d/texture_file.h"
#include "nel/3d/texture_bump.h"
#include "nel/3d/ps_particle_basic.h"
#include "nel/misc/path.h"

namespace NLQT
{

CParticleTextureWidget::CParticleTextureWidget(QWidget *parent)
	: QWidget(parent), _Wrapper(NULL)
{
	_ui.setupUi(this);

	_ui.imageLabel->setScaledContents(true);
	_ui.removePushButton->setVisible(false);

	connect(_ui.chooseTexPushButton, SIGNAL(clicked()), this, SLOT(chooseTexture()));
	connect(_ui.removePushButton, SIGNAL(clicked()), this, SLOT(removeTexture()));
}

CParticleTextureWidget::~CParticleTextureWidget()
{
}

void CParticleTextureWidget::updateUi()
{
	nlassert(_Wrapper);

	_Texture = _Wrapper->get();

	updateTexture();
}

void CParticleTextureWidget::chooseTexture()
{
	std::string texName;
	/// get the name of the previously set texture if there is one
	if (dynamic_cast<NL3D::CTextureFile *>(_Wrapper->get()))
	{
		if (!texName.empty())
			texName = NLMISC::CPath::lookup((static_cast<NL3D::CTextureFile *>(_Wrapper->get()))->getFileName());
	}

	QString fileName = QFileDialog::getOpenFileName(this,
					   tr("Open texture file"), texName.c_str(),
					   tr("Image file (*.tga *.png)"));

	setCursor(Qt::WaitCursor);
	if (!fileName.isEmpty())
	{
		// Add search path for the texture
		NLMISC::CPath::addSearchPath(NLMISC::CFile::getPath(fileName.toUtf8().constData()));
		try
		{
			texName = NLMISC::CFile::getFilename(fileName.toUtf8().constData());
			NL3D::CTextureFile *tf = new NL3D::CTextureFile(texName);
			_Wrapper->setAndUpdateModifiedFlag(tf);
			_Texture = tf;
			Q_EMIT textureChanged(QString(texName.c_str()));
			updateTexture();
		}
		catch (NLMISC::Exception &e)
		{
			QMessageBox::critical(this, tr("Texture loading error"), e.what(),  QMessageBox::Ok);
		}

	}
	setCursor(Qt::ArrowCursor);
}

void CParticleTextureWidget::removeTexture()
{
	if (_Texture)
	{
		_Texture->release();
		_Texture = NULL;
	}
	_Wrapper->setAndUpdateModifiedFlag(NULL);
	Q_EMIT textureChanged("");
	updateTexture();
}

void CParticleTextureWidget::updateTexture()
{
	_ui.nameLabel->setText(tr("Name:"));
	_ui.sizeLabel->setText(tr("Size:"));
	_ui.depthLabel->setText(tr("Depth:"));
	_ui.imageLabel->setText("");
	_ui.imageLabel->setPixmap(QPixmap());
	if (!_Texture)
		return;
	if (dynamic_cast<NL3D::CTextureFile *>(_Wrapper->get()))
	{
		std::string texName = (static_cast<NL3D::CTextureFile *>(_Wrapper->get()))->getFileName().c_str();
		_ui.nameLabel->setText(tr("Name: %1").arg(texName.c_str()));
		if (!NLMISC::CFile::getFilename(texName).empty())
		{
			std::string path;
			try
			{
				path = NLMISC::CPath::lookup(texName);
			}
			catch (NLMISC::Exception &e)
			{
				_ui.imageLabel->setText(e.what());
				return;
			}
			QPixmap pixmap(path.c_str());
			_ui.sizeLabel->setText(tr("Size: %1x%2").arg(pixmap.width()).arg(pixmap.height()));
			_ui.depthLabel->setText(tr("Depth: %1").arg(pixmap.depth()));
			_ui.imageLabel->setPixmap(pixmap);
		}
	}
}

} /* namespace NLQT */