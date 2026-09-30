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
#include "script_editor.h"

// Qt includes
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QToolButton>
#include <QVBoxLayout>

namespace WorldEditor
{

ScriptEditDialog::ScriptEditDialog(const QString &title, QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle(title);

	m_edit = new QPlainTextEdit(this);
	m_edit->setLineWrapMode(QPlainTextEdit::NoWrap);
	m_edit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	m_edit->setTabChangesFocus(false);

	QDialogButtonBox *buttons =
			new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(m_edit);
	layout->addWidget(buttons);

	resize(760, 560);
}

void ScriptEditDialog::setText(const QString &text)
{
	m_edit->setPlainText(text);
}

QString ScriptEditDialog::text() const
{
	return m_edit->toPlainText();
}

ScriptEditor::ScriptEditor(QWidget *parent)
	: QWidget(parent)
{
	m_lineEdit = new QLineEdit(this);

	// The row shows the first line only. Editing the block in here would be worse than
	// useless - a stray Return would silently cut the script in half - so the summary is
	// read only and all editing happens in the dialog.
	m_lineEdit->setReadOnly(true);

	m_button = new QToolButton(this);
	m_button->setText(QLatin1String("..."));
	m_button->setToolTip(tr("Edit this block in a window"));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->addWidget(m_lineEdit);
	layout->addWidget(m_button);

	setFocusProxy(m_lineEdit);
	connect(m_button, SIGNAL(clicked()), this, SLOT(openDialog()));
}

void ScriptEditor::setPropertyName(const QString &name)
{
	m_propertyName = name;
}

void ScriptEditor::setValue(const QString &value)
{
	m_value = value;

	// Same summary the property browser paints when no editor is open, so opening a row
	// does not make its text jump.
	const int lineBreak = value.indexOf(QLatin1Char('\n'));
	m_lineEdit->setText(lineBreak < 0 ? value
									  : value.left(lineBreak) + QLatin1String(" ..."));
	m_lineEdit->setToolTip(value);
}

void ScriptEditor::openDialog()
{
	ScriptEditDialog dialog(m_propertyName.isEmpty() ? tr("Edit") : m_propertyName, this);
	dialog.setText(m_value);

	if (dialog.exec() != QDialog::Accepted)
		return;

	const QString value = dialog.text();
	if (value == m_value)
		return;

	setValue(value);
	Q_EMIT valueChanged(value);
}

ScriptEditorFactory::ScriptEditorFactory(QObject *parent)
	: QtAbstractEditorFactory<QtTextPropertyManager>(parent)
{
}

void ScriptEditorFactory::connectPropertyManager(QtTextPropertyManager *manager)
{
	connect(manager, SIGNAL(valueChanged(QtProperty *, const QString &)),
			this, SLOT(onManagerValueChanged(QtProperty *, const QString &)));
}

void ScriptEditorFactory::disconnectPropertyManager(QtTextPropertyManager *manager)
{
	disconnect(manager, SIGNAL(valueChanged(QtProperty *, const QString &)),
			   this, SLOT(onManagerValueChanged(QtProperty *, const QString &)));
}

QWidget *ScriptEditorFactory::createEditor(QtTextPropertyManager *manager,
										   QtProperty *property, QWidget *parent)
{
	ScriptEditor *editor = new ScriptEditor(parent);
	editor->setPropertyName(property->propertyName());
	editor->setValue(manager->value(property));

	m_createdEditors[property].append(editor);
	m_editorToProperty.insert(editor, property);

	connect(editor, SIGNAL(valueChanged(const QString &)),
			this, SLOT(onEditorValueChanged(const QString &)));
	connect(editor, SIGNAL(destroyed(QObject *)), this, SLOT(onEditorDestroyed(QObject *)));

	return editor;
}

void ScriptEditorFactory::onManagerValueChanged(QtProperty *property, const QString &value)
{
	Q_FOREACH (ScriptEditor *editor, m_createdEditors.value(property))
	{
		editor->blockSignals(true);
		editor->setValue(value);
		editor->blockSignals(false);
	}
}

void ScriptEditorFactory::onEditorValueChanged(const QString &value)
{
	ScriptEditor *editor = qobject_cast<ScriptEditor *>(sender());
	if (editor == 0)
		return;

	QtProperty *property = m_editorToProperty.value(editor, 0);
	if (property == 0)
		return;

	QtTextPropertyManager *manager = propertyManager(property);
	if (manager != 0)
		manager->setValue(property, value);
}

void ScriptEditorFactory::onEditorDestroyed(QObject *object)
{
	QMap<ScriptEditor *, QtProperty *>::iterator ite = m_editorToProperty.begin();
	while (ite != m_editorToProperty.end())
	{
		if (ite.key() == object)
		{
			QtProperty *property = ite.value();
			m_createdEditors[property].removeAll(ite.key());
			if (m_createdEditors.value(property).isEmpty())
				m_createdEditors.remove(property);

			m_editorToProperty.erase(ite);
			return;
		}
		++ite;
	}
}

} /* namespace WorldEditor */
