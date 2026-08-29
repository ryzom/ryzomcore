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

#ifndef FORM_DIALOG_H
#define FORM_DIALOG_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

class GeorgesEditorDoc;
class CGeorgesEditDocSub;

/**
 * Right panel widget for editing a Form node, ported from CFormDialog.
 * Dynamically creates widgets based on the DFN structure.
 */
class FormDialog : public QWidget
{
	Q_OBJECT

public:
	FormDialog(QWidget *parent = nullptr);
	~FormDialog();

	void setDocument(GeorgesEditorDoc *doc);
	void updateFromDocument(CGeorgesEditDocSub *sub = nullptr);

private slots:
	void onFieldChanged();

private:
	void setupUi();
	void clearFormWidgets();
	void buildFormWidgets(CGeorgesEditDocSub *sub);

	GeorgesEditorDoc *_doc;
	QScrollArea *_scrollArea;
	QWidget *_formContainer;
	QVBoxLayout *_formLayout;
};

#endif // FORM_DIALOG_H
