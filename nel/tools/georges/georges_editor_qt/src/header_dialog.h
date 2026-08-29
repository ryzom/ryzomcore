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

#ifndef HEADER_DIALOG_H
#define HEADER_DIALOG_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>

class GeorgesEditorDoc;

/**
 * Right panel widget for editing a Header node, ported from CHeaderDialog.
 */
class HeaderDialog : public QWidget
{
	Q_OBJECT

public:
	HeaderDialog(QWidget *parent = nullptr);
	~HeaderDialog();

	void setDocument(GeorgesEditorDoc *doc);
	void updateFromDocument();

private slots:
	void onIncrementVersion();
	void onStateChanged(int index);
	void onCommentsChanged();

private:
	void setupUi();

	GeorgesEditorDoc *_doc;

	QLabel *_versionLabel;
	QPushButton *_incrementBtn;
	QComboBox *_stateCombo;
	QTextEdit *_commentsEdit;
	QTextEdit *_logEdit;
};

#endif // HEADER_DIALOG_H
