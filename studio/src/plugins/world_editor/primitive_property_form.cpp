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

// Project includes
#include "primitive_property_form.h"
#include "script_editor.h"
#include "world_editor_misc.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/ligo/ligo_config.h>

// Qt includes
#include <QCheckBox>
#include <QComboBox>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

// STL includes
#include <algorithm>

namespace WorldEditor
{

namespace
{
/// Name of the parameter a widget stands for.
const char *const PARAMETER_NAME = "we_parameter";

/// Whether the parameter is one of the array types, which are stored line by line.
const char *const PARAMETER_IS_ARRAY = "we_parameter_is_array";

/// Fallback height for a text block whose class says nothing, in pixels.
const int DEFAULT_BLOCK_HEIGHT = 100;

/// Beyond this a block would push everything else off the form; it scrolls instead.
const int MAX_BLOCK_HEIGHT = 320;
}

PrimitivePropertyForm::PrimitivePropertyForm(QWidget *parent)
	: QScrollArea(parent),
	  m_building(false),
	  m_primitive(0),
	  m_content(0),
	  m_layout(0)
{
	setWidgetResizable(true);
	setFrameShape(QFrame::NoFrame);
	clear();
}

void PrimitivePropertyForm::clear()
{
	m_primitive = 0;

	m_content = new QWidget(this);
	m_layout = new QVBoxLayout(m_content);
	m_layout->setContentsMargins(6, 6, 6, 6);
	m_layout->setSpacing(2);

	// Replaces and deletes whatever was shown before.
	setWidget(m_content);
}

QString PrimitivePropertyForm::valueOf(const std::string &name) const
{
	if (m_primitive == 0)
		return QString();

	const NLLIGO::IProperty *property = 0;
	if (!m_primitive->getPropertyByName(name.c_str(), property) || (property == 0))
		return QString();

	const NLLIGO::CPropertyString *asString =
			dynamic_cast<const NLLIGO::CPropertyString *>(property);
	if (asString != 0)
		return QString::fromUtf8(asString->String.c_str());

	const NLLIGO::CPropertyStringArray *asArray =
			dynamic_cast<const NLLIGO::CPropertyStringArray *>(property);
	if (asArray != 0)
	{
		QStringList lines;
		for (size_t i = 0; i < asArray->StringArray.size(); ++i)
			lines.append(QString::fromUtf8(asArray->StringArray[i].c_str()));
		return lines.join(QLatin1String("\n"));
	}

	return QString();
}

void PrimitivePropertyForm::store(const QString &name, const QString &value, bool asArray)
{
	if ((m_primitive == 0) || m_building)
		return;

	NLLIGO::IProperty *property = 0;
	if (!m_primitive->getPropertyByName(name.toUtf8().constData(), property) || (property == 0))
		return;

	if (asArray)
	{
		NLLIGO::CPropertyStringArray *target =
				dynamic_cast<NLLIGO::CPropertyStringArray *>(property);
		if (target == 0)
			return;

		target->StringArray.clear();
		const QStringList lines = value.split(QLatin1Char('\n'));
		Q_FOREACH (const QString &line, lines)
			target->StringArray.push_back(line.toUtf8().constData());

		target->Default = false;
		return;
	}

	NLLIGO::CPropertyString *target = dynamic_cast<NLLIGO::CPropertyString *>(property);
	if (target == 0)
		return;

	target->String = value.toUtf8().constData();
	target->Default = false;
}

QStringList PrimitivePropertyForm::comboValues(
		const NLLIGO::CPrimitiveClass::CParameter &parameter) const
{
	// Same two contexts the property browser used: the current one, then the default.
	std::vector<std::string> contexts;
	contexts.push_back("jungle");
	contexts.push_back("default");

	QStringList values;
	for (size_t j = 0; j < contexts.size(); ++j)
	{
		std::map<std::string, NLLIGO::CPrimitiveClass::CParameter::CConstStringValue>::const_iterator ite =
				parameter.ComboValues.find(contexts[j]);
		if (ite == parameter.ComboValues.end())
			continue;

		std::vector<std::string> pathList;
		ite->second.appendFilePath(pathList);

		if (parameter.SortEntries)
			std::sort(pathList.begin(), pathList.end());

		for (size_t i = 0; i < pathList.size(); ++i)
			values.append(QString::fromUtf8(pathList[i].c_str()));
	}

	return values;
}

void PrimitivePropertyForm::addLabel(const NLLIGO::CPrimitiveClass::CParameter &parameter,
									 bool isDefault)
{
	QString text = QString::fromUtf8(parameter.Name.c_str());

	// The original spells this out, and it is worth keeping: it tells you at a glance
	// whether a value was ever set or is just the default coming through.
	if (isDefault)
		text += tr(" (default value)");

	QLabel *label = new QLabel(text, m_content);
	label->setContentsMargins(0, 6, 0, 0);
	m_layout->addWidget(label);
}

void PrimitivePropertyForm::addLineEdit(const NLLIGO::CPrimitiveClass::CParameter &parameter)
{
	QLineEdit *edit = new QLineEdit(m_content);
	edit->setText(valueOf(parameter.Name));
	edit->setReadOnly(parameter.ReadOnly);
	edit->setProperty(PARAMETER_NAME, QString::fromUtf8(parameter.Name.c_str()));
	edit->setProperty(PARAMETER_IS_ARRAY, false);

	connect(edit, SIGNAL(editingFinished()), this, SLOT(onLineEditFinished()));
	m_layout->addWidget(edit);
}

void PrimitivePropertyForm::addTextBlock(const NLLIGO::CPrimitiveClass::CParameter &parameter)
{
	QPlainTextEdit *edit = new QPlainTextEdit(m_content);
	edit->setPlainText(valueOf(parameter.Name));
	edit->setReadOnly(parameter.ReadOnly);
	edit->setLineWrapMode(QPlainTextEdit::NoWrap);
	edit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	edit->setProperty(PARAMETER_NAME, QString::fromUtf8(parameter.Name.c_str()));
	edit->setProperty(PARAMETER_IS_ARRAY, true);

	// The class file gives the height in pixels - that is what WIDGET_HEIGHT is for.
	const int height = (parameter.WidgetHeight > 0)
			? qMin(int(parameter.WidgetHeight), MAX_BLOCK_HEIGHT)
			: DEFAULT_BLOCK_HEIGHT;
	edit->setMinimumHeight(height);
	edit->setMaximumHeight(height);

	connect(edit, SIGNAL(textChanged()), this, SLOT(onTextBlockChanged()));
	m_layout->addWidget(edit);

	// A long script is still easier to work on in a window of its own, so offer the
	// same way out the original does with its "Edit..." button.
	if (!parameter.ReadOnly)
	{
		QPushButton *button = new QPushButton(tr("Edit..."), m_content);
		button->setProperty(PARAMETER_NAME, QString::fromUtf8(parameter.Name.c_str()));
		connect(button, SIGNAL(clicked()), this, SLOT(onEditBlockClicked()));

		QHBoxLayout *row = new QHBoxLayout();
		row->addStretch();
		row->addWidget(button);
		m_layout->addLayout(row);
	}
}

void PrimitivePropertyForm::addCheckBox(const NLLIGO::CPrimitiveClass::CParameter &parameter)
{
	QCheckBox *box = new QCheckBox(QString::fromUtf8(parameter.Name.c_str()), m_content);
	box->setChecked(valueOf(parameter.Name) == QLatin1String("true"));
	box->setEnabled(!parameter.ReadOnly);
	box->setProperty(PARAMETER_NAME, QString::fromUtf8(parameter.Name.c_str()));

	connect(box, SIGNAL(toggled(bool)), this, SLOT(onCheckBoxToggled(bool)));
	m_layout->addWidget(box);
}

void PrimitivePropertyForm::addComboBox(const NLLIGO::CPrimitiveClass::CParameter &parameter)
{
	QComboBox *combo = new QComboBox(m_content);
	combo->setEditable(parameter.Editable);
	combo->addItems(comboValues(parameter));

	const QString value = valueOf(parameter.Name);
	const int index = combo->findText(value);
	if (index >= 0)
	{
		combo->setCurrentIndex(index);
	}
	else if (!value.isEmpty())
	{
		// The stored value is not among the offered ones. Showing the list without it
		// would quietly present a different value as the current one.
		combo->insertItem(0, value);
		combo->setCurrentIndex(0);
	}

	combo->setEnabled(!parameter.ReadOnly);
	combo->setProperty(PARAMETER_NAME, QString::fromUtf8(parameter.Name.c_str()));

	connect(combo, SIGNAL(currentIndexChanged(QString)), this, SLOT(onComboChanged(QString)));
	if (parameter.Editable)
		connect(combo, SIGNAL(editTextChanged(QString)), this, SLOT(onComboChanged(QString)));

	m_layout->addWidget(combo);
}

void PrimitivePropertyForm::buildForFile(Node *node)
{
	QString fileName;
	switch (node->type())
	{
	case Node::WorldEditNodeType:
		fileName = static_cast<WorldEditNode *>(node)->fileName();
		break;
	case Node::LandscapeNodeType:
		fileName = static_cast<LandscapeNode *>(node)->fileName();
		break;
	case Node::RootPrimitiveNodeType:
		fileName = static_cast<RootPrimitiveNode *>(node)->fileName();
		break;
	default:
		return;
	}

	if (fileName.isEmpty())
		return;

	m_layout->addWidget(new QLabel(tr("path"), m_content));

	// A read-only line edit rather than a label: the path is the one thing people want
	// to copy out of here, and a label cannot be selected.
	QLineEdit *edit = new QLineEdit(fileName, m_content);
	edit->setReadOnly(true);
	edit->setCursorPosition(0);
	m_layout->addWidget(edit);
}

void PrimitivePropertyForm::buildForPrimitive(Node *node)
{
	PrimitiveNode *primitiveNode = static_cast<PrimitiveNode *>(node);
	m_primitive = primitiveNode->primitive();

	const NLLIGO::CPrimitiveClass *primitiveClass = primitiveNode->primitiveClass();

	QLabel *title = new QLabel(m_content);
	title->setText(QString("<b>%1</b> (%2)")
				   .arg(node->data(Qt::DisplayRole).toString().toHtmlEscaped())
				   .arg(primitiveClass != 0
						? QString::fromUtf8(primitiveClass->Name.c_str()).toHtmlEscaped()
						: tr("unknown class")));
	title->setTextInteractionFlags(Qt::TextSelectableByMouse);
	m_layout->addWidget(title);

	// The class decides which parameters exist and in which order; "name" goes first.
	std::list<NLLIGO::CPrimitiveClass::CParameter> parameters;
	if (primitiveClass != 0)
	{
		for (uint p = 0; p < primitiveClass->Parameters.size(); ++p)
		{
			const NLLIGO::CPrimitiveClass::CParameter &parameter = primitiveClass->Parameters[p];
			if (!parameter.Visible || (parameter.Name == "class"))
				continue;

			if (parameter.Name == "name")
				parameters.push_front(parameter);
			else
				parameters.push_back(parameter);
		}
	}
	else
	{
		// No class in world_editor_classes.xml: fall back to the raw properties, so the
		// primitive can still be looked at instead of showing an empty form.
		const uint count = m_primitive->getNumProperty();
		for (uint p = 0; p < count; ++p)
		{
			std::string name;
			const NLLIGO::IProperty *property = 0;
			if (!m_primitive->getProperty(p, name, property))
				continue;

			NLLIGO::CPrimitiveClass::CParameter parameter(*property, name.c_str());
			if (parameter.Name == "class")
				continue;

			if (parameter.Name == "name")
				parameters.push_front(parameter);
			else
				parameters.push_back(parameter);
		}
	}

	const bool staticChild = Utils::ligoConfig()->isStaticChild(*m_primitive);

	std::list<NLLIGO::CPrimitiveClass::CParameter>::iterator ite = parameters.begin();
	while (ite != parameters.end())
	{
		NLLIGO::CPrimitiveClass::CParameter parameter = (*ite);
		++ite;

		if (staticChild && (parameter.Name == "name"))
			parameter.ReadOnly = true;

		const NLLIGO::IProperty *property = 0;
		m_primitive->getPropertyByName(parameter.Name.c_str(), property);
		const bool isDefault = (property == 0) || property->Default;

		switch (parameter.Type)
		{
		case NLLIGO::CPrimitiveClass::CParameter::Boolean:
			// The check box carries its own text, so no separate label above it.
			addCheckBox(parameter);
			break;
		case NLLIGO::CPrimitiveClass::CParameter::ConstString:
			addLabel(parameter, isDefault);
			addComboBox(parameter);
			break;
		case NLLIGO::CPrimitiveClass::CParameter::StringArray:
		case NLLIGO::CPrimitiveClass::CParameter::ConstStringArray:
			addLabel(parameter, isDefault);
			addTextBlock(parameter);
			break;
		default:
			addLabel(parameter, isDefault);
			addLineEdit(parameter);
			break;
		}
	}
}

void PrimitivePropertyForm::setNode(Node *node)
{
	clear();

	if (node == 0)
		return;

	m_building = true;

	if (node->type() == Node::PrimitiveNodeType)
		buildForPrimitive(node);
	else
		buildForFile(node);

	m_layout->addStretch();
	m_building = false;
}

void PrimitivePropertyForm::onLineEditFinished()
{
	QLineEdit *edit = qobject_cast<QLineEdit *>(sender());
	if (edit == 0)
		return;

	store(edit->property(PARAMETER_NAME).toString(), edit->text(), false);
}

void PrimitivePropertyForm::onTextBlockChanged()
{
	QPlainTextEdit *edit = qobject_cast<QPlainTextEdit *>(sender());
	if (edit == 0)
		return;

	store(edit->property(PARAMETER_NAME).toString(), edit->toPlainText(), true);
}

void PrimitivePropertyForm::onCheckBoxToggled(bool checked)
{
	QCheckBox *box = qobject_cast<QCheckBox *>(sender());
	if (box == 0)
		return;

	store(box->property(PARAMETER_NAME).toString(),
		  checked ? QLatin1String("true") : QLatin1String("false"), false);
}

void PrimitivePropertyForm::onComboChanged(const QString &value)
{
	QComboBox *combo = qobject_cast<QComboBox *>(sender());
	if (combo == 0)
		return;

	store(combo->property(PARAMETER_NAME).toString(), value, false);
}

void PrimitivePropertyForm::onEditBlockClicked()
{
	QPushButton *button = qobject_cast<QPushButton *>(sender());
	if (button == 0)
		return;

	const QString name = button->property(PARAMETER_NAME).toString();

	// Find the box this button belongs to, so the dialog and the form stay in step.
	QPlainTextEdit *edit = 0;
	Q_FOREACH (QPlainTextEdit *candidate, m_content->findChildren<QPlainTextEdit *>())
	{
		if (candidate->property(PARAMETER_NAME).toString() == name)
		{
			edit = candidate;
			break;
		}
	}

	if (edit == 0)
		return;

	ScriptEditDialog dialog(name, this);
	dialog.setText(edit->toPlainText());
	if (dialog.exec() == QDialog::Accepted)
		edit->setPlainText(dialog.text());
}

} /* namespace WorldEditor */
