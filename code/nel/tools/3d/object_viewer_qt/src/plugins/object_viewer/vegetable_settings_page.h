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


#ifndef VEGETABLE_SETTINGS_PAGE_H
#define VEGETABLE_SETTINGS_PAGE_H

#include <QtCore/QObject>

#include "../core/ioptions_page.h"

#include "ui_vegetable_settings_page.h"

class QWidget;

namespace NLQT
{
/**
@class VegetableSettingsPage
*/
class VegetableSettingsPage : public Core::IOptionsPage
{
	Q_OBJECT
public:
	VegetableSettingsPage(QObject *parent = 0);
	virtual ~VegetableSettingsPage() {}

	virtual QString id() const;
	virtual QString trName() const;
	virtual QString category() const;
	virtual QString trCategory() const;
	QIcon categoryIcon() const;
	virtual QWidget *createPage(QWidget *parent);

	virtual void apply();
	virtual void finish();

private Q_SLOTS:
	void setTileBank();
	void setTileFarBank();
	void setTextureVegetable();
	void setCoarseMeshTexture();
	void addZone();
	void removeZone();

private:
	QWidget *m_page;
	Ui::VegetableSettingsPage m_ui;
};

} // namespace NLQT

#endif // VEGETABLE_SETTINGS_H
