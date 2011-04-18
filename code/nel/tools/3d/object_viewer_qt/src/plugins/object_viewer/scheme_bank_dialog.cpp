// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

// Project includes
#include "stdpch.h"
#include "scheme_bank_dialog.h"
#include "modules.h"

namespace NLQT
{

CSchemeBankDialog::CSchemeBankDialog(CAttribWidget *attribWidget, QWidget *parent)
	: QDialog(parent)
{
	_ui.setupUi(this);
	_attribWidget = attribWidget;
}

CSchemeBankDialog::~CSchemeBankDialog()
{
}

void CSchemeBankDialog::createScheme()
{
}

void CSchemeBankDialog::setCurrentScheme()
{
		//SchemeManager.insertScheme(cn.getName(), getCurrentSchemePtr()->clone());
}

void CSchemeBankDialog::removeScheme()
{
}

void CSchemeBankDialog::saveBank()
{
}

void CSchemeBankDialog::loadBank()
{
}

void CSchemeBankDialog::buildList()
{
}

} /* namespace NLQT */