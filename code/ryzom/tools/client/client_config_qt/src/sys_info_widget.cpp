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
#include "sys_info_widget.h"

#include "system.h"

CSysInfoWidget::CSysInfoWidget( QWidget *parent ) :
	QWidget( parent )
{
	setupUi( this );

	osLabel->setText(QString::fromUtf8(CSystem::GetInstance().sysInfo.osName.c_str()));
	cpuLabel->setText(QString::fromUtf8(CSystem::GetInstance().sysInfo.cpuName.c_str()));

	ramLabel->setText(QString(tr("%1 MiB").arg(CSystem::GetInstance().sysInfo.totalRAM)));

	gfxcardLabel->setText(QString::fromUtf8(CSystem::GetInstance().sysInfo.videoDevice.c_str()));
	gfxdriverLabel->setText(QString::fromUtf8(CSystem::GetInstance().sysInfo.videoDriverVersion.c_str()));
}

CSysInfoWidget::~CSysInfoWidget()
{
}
