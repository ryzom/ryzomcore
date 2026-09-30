// Ryzom Core Studio - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef PRIMITIVE_PROPERTY_FORM_H
#define PRIMITIVE_PROPERTY_FORM_H

// Project includes
#include "primitive_item.h"

// NeL includes
#include <nel/ligo/primitive.h>
#include <nel/ligo/primitive_class.h>

// Qt includes
#include <QScrollArea>
#include <QString>
#include <QStringList>

class QVBoxLayout;
class QWidget;

namespace WorldEditor
{

/**
@class PrimitivePropertyForm
@brief The properties of one primitive, laid out as a form.
@details Built after the property window of the original editor: the label sits above the
	field and the field spans the full width, rather than everything being squeezed into
	the value column of a two column tree.

	That matters most for the text blocks. Several parameters hold whole scripts, and
	world_editor_classes.xml says how tall their box should be (WIDGET_HEIGHT). Here they
	are shown at that height, in a fixed width font, readable and editable in place -
	through a single line cell they were neither.

	As a side effect every value sits in a real text widget, so selecting and copying it
	works the way it does anywhere else.
*/
class PrimitivePropertyForm : public QScrollArea
{
	Q_OBJECT

public:
	explicit PrimitivePropertyForm(QWidget *parent = 0);

	/// Show this node. A primitive gets its parameters, a land or project file its path.
	void setNode(Node *node);

	/// Forget everything and show nothing.
	void clear();

private Q_SLOTS:
	void onLineEditFinished();
	void onTextBlockChanged();
	void onCheckBoxToggled(bool checked);
	void onComboChanged(const QString &value);
	void onEditBlockClicked();

private:
	void buildForPrimitive(Node *node);
	void buildForFile(Node *node);

	/// Header line above each field, with the "(default value)" note the original shows.
	void addLabel(const NLLIGO::CPrimitiveClass::CParameter &parameter, bool isDefault);

	void addLineEdit(const NLLIGO::CPrimitiveClass::CParameter &parameter);
	void addTextBlock(const NLLIGO::CPrimitiveClass::CParameter &parameter);
	void addCheckBox(const NLLIGO::CPrimitiveClass::CParameter &parameter);
	void addComboBox(const NLLIGO::CPrimitiveClass::CParameter &parameter);

	/// Values offered for a const string parameter.
	QStringList comboValues(const NLLIGO::CPrimitiveClass::CParameter &parameter) const;

	/// Current value of a parameter, lines joined with newlines for the array types.
	QString valueOf(const std::string &name) const;

	/// Write a value back into the primitive.
	void store(const QString &name, const QString &value, bool asArray);

	/// True while the form is being filled, so the widgets' own signals are ignored.
	bool m_building;

	NLLIGO::IPrimitive *m_primitive;
	QWidget *m_content;
	QVBoxLayout *m_layout;
};

} /* namespace WorldEditor */

#endif // PRIMITIVE_PROPERTY_FORM_H
