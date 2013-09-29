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
#include "sys_info_d3d_widget.h"
#include "system.h"

CSysInfoD3DWidget::CSysInfoD3DWidget( QWidget *parent ) :
	QWidget( parent )
{
	setupUi( this );

	descriptionLabel->setText( CSystem::GetInstance().d3dInfo.device.c_str() );
	driverLabel->setText( CSystem::GetInstance().d3dInfo.driver.c_str() );
	versionLabel->setText( CSystem::GetInstance().d3dInfo.driverVersion.c_str() );
}

CSysInfoD3DWidget::~CSysInfoD3DWidget()
{
}

void CSysInfoD3DWidget::changeEvent( QEvent *event )
{
	if( event->type() == QEvent::LanguageChange )
	{
		retranslateUi( this );
	}
	QWidget::changeEvent( event );
}
