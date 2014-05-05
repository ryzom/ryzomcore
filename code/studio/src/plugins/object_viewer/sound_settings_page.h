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


#ifndef SOUND_SETTINGS_PAGE_H
#define SOUND_SETTINGS_PAGE_H

// Project includes
#include "../core/ioptions_page.h"
#include "ui_sound_settings_page.h"

// Qt includes
#include <QtCore/QObject>

class QWidget;

namespace NLQT
{
/**
@class SoundSettingsPage
*/
class SoundSettingsPage : public Core::IOptionsPage
{
	Q_OBJECT
public:
	SoundSettingsPage(QObject *parent = 0);
	virtual ~SoundSettingsPage() {}

	virtual QString id() const;
	virtual QString trName() const;
	virtual QString category() const;
	virtual QString trCategory() const;
	QIcon categoryIcon() const;
	virtual QWidget *createPage(QWidget *parent);

	virtual void apply();
	virtual void finish();

private Q_SLOTS:
	void setSheetPath();
	void setSamplePath();

private:
	QWidget *m_page;
	Ui::SoundSettingsPage m_ui;
};

} // namespace NLQT

#endif // SOUND_SETTINGS_H
