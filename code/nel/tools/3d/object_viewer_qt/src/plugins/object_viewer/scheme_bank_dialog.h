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

#ifndef SCHEME_BANK_DIALOG_H
#define SCHEME_BANK_DIALOG_H

#include "ui_scheme_bank_form.h"

// STL includes

// Qt includes

// NeL includes
#include "nel/3d/particle_system.h"

// Project includes
#include "ps_wrapper.h"

namespace NLQT
{
class CAttribWidget;

class CSchemeBankDialog: public QDialog
{
	Q_OBJECT

public:
	CSchemeBankDialog(CAttribWidget *attribWidget, QWidget *parent = 0);
	~CSchemeBankDialog();

private Q_SLOTS:
	void createScheme();
	void setCurrentScheme();
	void removeScheme();
	void saveBank();
	void loadBank();

private:
	void buildList();

	CAttribWidget *_attribWidget;
	Ui::CSchemeBankDialog _ui;
}; /* class CSchemeBankDialog */

} /* namespace NLQT */

#endif // SCHEME_BANK_DIALOG_H
