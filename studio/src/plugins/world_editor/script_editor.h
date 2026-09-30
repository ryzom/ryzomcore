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

#ifndef SCRIPT_EDITOR_H
#define SCRIPT_EDITOR_H

#define QT_QTPROPERTYBROWSER_IMPORT

// 3rdparty
#include "3rdparty/qtpropertybrowser/qtpropertybrowser.h"
#include "3rdparty/qtpropertybrowser/qtpropertymanager.h"

// Qt includes
#include <QDialog>
#include <QMap>
#include <QString>
#include <QWidget>

class QLineEdit;
class QPlainTextEdit;
class QToolButton;

namespace WorldEditor
{

/**
@class ScriptEditDialog
@brief A plain multi line editor for the script blocks primitives carry.
@details Several primitive parameters hold whole blocks of script - the mission scripts are
	dozens of lines. Squeezing those through the one line editor the property browser uses
	by default makes them practically unreadable and easy to mangle, so they get a window
	with room in it and a fixed width font.
*/
class ScriptEditDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ScriptEditDialog(const QString &title, QWidget *parent = 0);

	void setText(const QString &text);
	QString text() const;

private:
	QPlainTextEdit *m_edit;
};

/**
@class ScriptEditor
@brief The row editor: a read-only summary plus a button that opens the real editor.
*/
class ScriptEditor : public QWidget
{
	Q_OBJECT

public:
	explicit ScriptEditor(QWidget *parent = 0);

	void setPropertyName(const QString &name);

public Q_SLOTS:
	void setValue(const QString &value);

Q_SIGNALS:
	void valueChanged(const QString &value);

private Q_SLOTS:
	void openDialog();

private:
	QLineEdit *m_lineEdit;
	QToolButton *m_button;
	QString m_value;
	QString m_propertyName;
};

/**
@class ScriptEditorFactory
@brief Gives every QtTextPropertyManager property the editor above.
*/
class ScriptEditorFactory : public QtAbstractEditorFactory<QtTextPropertyManager>
{
	Q_OBJECT

public:
	explicit ScriptEditorFactory(QObject *parent = 0);

protected:
	virtual void connectPropertyManager(QtTextPropertyManager *manager);
	virtual void disconnectPropertyManager(QtTextPropertyManager *manager);
	virtual QWidget *createEditor(QtTextPropertyManager *manager, QtProperty *property,
								  QWidget *parent);

private Q_SLOTS:
	void onManagerValueChanged(QtProperty *property, const QString &value);
	void onEditorValueChanged(const QString &value);
	void onEditorDestroyed(QObject *editor);

private:
	QMap<QtProperty *, QList<ScriptEditor *> > m_createdEditors;
	QMap<ScriptEditor *, QtProperty *> m_editorToProperty;
};

} /* namespace WorldEditor */

#endif // SCRIPT_EDITOR_H
