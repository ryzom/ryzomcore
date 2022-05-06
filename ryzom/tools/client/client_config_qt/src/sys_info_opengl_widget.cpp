// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "stdpch.h"
#include "sys_info_opengl_widget.h"

#include "system.h"

CSysInfoOpenGLWidget::CSysInfoOpenGLWidget( QWidget *parent ) :
	QWidget( parent )
{
	setupUi( this );
	vendorLabel->setText(QString::fromUtf8(CSystem::GetInstance().openglInfo.vendor.c_str()));
	rendererLabel->setText(QString::fromUtf8( CSystem::GetInstance().openglInfo.renderer.c_str()));
	versionLabel->setText(QString::fromUtf8( CSystem::GetInstance().openglInfo.driverVersion.c_str()));
	extensionsBox->setPlainText(QString::fromUtf8( CSystem::GetInstance().openglInfo.extensions.c_str()));
}

CSysInfoOpenGLWidget::~CSysInfoOpenGLWidget()
{
}
