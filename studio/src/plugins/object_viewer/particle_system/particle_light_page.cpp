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
#include "particle_light_page.h"

// Qt includes

// NeL includes

// Project includes
#include "modules.h"

namespace NLQT
{

CLightPage::CLightPage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_ui.lightColorWidget->setWrapper(&_ColorWrapper);
	_ui.lightColorWidget->setSchemeWrapper(&_ColorWrapper);
	_ui.lightColorWidget->init();

	_ui.attenStartWidget->setRange(0.01f, 5.f);
	_ui.attenStartWidget->setWrapper(&_AttenStartWrapper);
	_ui.attenStartWidget->setSchemeWrapper(&_AttenStartWrapper);
	_ui.attenStartWidget->init();

	_ui.attenEndWidget->setRange(0.01f, 5.f);
	_ui.attenEndWidget->setWrapper(&_AttenEndWrapper);
	_ui.attenEndWidget->setSchemeWrapper(&_AttenEndWrapper);
	_ui.attenEndWidget->init();
}

CLightPage::~CLightPage()
{
}

void CLightPage::setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocatedBindable *locatedBindable)
{
	_Node = ownerNode;
	_Light = static_cast<NL3D::CPSLight *>(locatedBindable);

	_ColorWrapper.L = _Light;
	_ui.lightColorWidget->setWorkspaceNode(_Node);
	_ui.lightColorWidget->updateUi();

	_AttenStartWrapper.L = _Light;
	_ui.attenStartWidget->setWorkspaceNode(_Node);
	_ui.attenStartWidget->updateUi();

	_AttenEndWrapper.L = _Light;
	_ui.attenEndWidget->setWorkspaceNode(_Node);
	_ui.attenEndWidget->updateUi();
}

} /* namespace NLQT */