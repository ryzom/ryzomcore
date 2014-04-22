// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef PROPERTY_EDITOR_WIDGET_H
#define PROPERTY_EDITOR_WIDGET_H

// Project includes
#include "ui_property_editor_widget.h"
#include "primitives_model.h"
#include "primitive_item.h"


// 3rdparty
#include "qtvariantproperty.h"
#include "qtpropertymanager.h"
#include "qteditorfactory.h"

// NeL includes

// Qt includes

namespace WorldEditor
{
/**
@class PropertyEditorWidget
@brief PropertyEditorWidget
@details
*/
class PropertyEditorWidget: public QWidget
{
	Q_OBJECT

public:
	explicit PropertyEditorWidget(QWidget *parent = 0);
	~PropertyEditorWidget();

public Q_SLOTS:
	void clearProperties();

	/// Update of selections
	void updateSelection(Node *node);

	void propertyChanged(QtProperty *property);
	void resetProperty(QtProperty *property);

private:
	QtProperty *addBoolProperty(const NLLIGO::IProperty *property,
								const NLLIGO::CPrimitiveClass::CParameter &parameter,
								const NLLIGO::IPrimitive *primitive);
	QtProperty *addConstStringProperty(const NLLIGO::IProperty *property,
									   const NLLIGO::CPrimitiveClass::CParameter &parameter,
									   const NLLIGO::IPrimitive *primitive);
	QtProperty *addStringProperty(const NLLIGO::IProperty *property,
								  const NLLIGO::CPrimitiveClass::CParameter &parameter,
								  const NLLIGO::IPrimitive *primitive);
	QtProperty *addStringArrayProperty(const NLLIGO::IProperty *property,
									   const NLLIGO::CPrimitiveClass::CParameter &parameter,
									   const NLLIGO::IPrimitive *primitive);
	QtProperty *addConstStringArrayProperty(const NLLIGO::IProperty *property,
											const NLLIGO::CPrimitiveClass::CParameter &parameter,
											const NLLIGO::IPrimitive *primitive);

	QStringList getComboValues(const NLLIGO::CPrimitiveClass::CParameter &parameter);

	void blockSignalsOfProperties(bool block);

	QtBoolPropertyManager *m_boolManager;
	QtStringPropertyManager *m_stringManager;
	QtEnumPropertyManager *m_enumManager;
	QtGroupPropertyManager *m_groupManager;
	QtTextPropertyManager *m_stringArrayManager;

	Ui::PropertyEditorWidget m_ui;
}; /* PropertyEditorWidget */

} /* namespace WorldEditor */

#endif // PROPERTY_EDITOR_WIDGET_H
