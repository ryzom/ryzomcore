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
#include "navigation_tools.h"
#include "primitives_model.h"
#include "primitive_item.h"
#include "world_editor_misc.h"

// NeL includes
#include <nel/ligo/primitive.h>
#include <nel/ligo/ligo_config.h>

// Qt includes
#include <QtCore/QRegularExpression>
#include <QtGui/QClipboard>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>

namespace WorldEditor
{
namespace Navigation
{

QPointF worldToScene(const QPointF &world)
{
	const qreal cellSize = Utils::ligoConfig()->CellSize;
	return QPointF(world.x(), cellSize - world.y());
}

QPointF sceneToWorld(const QPointF &scene)
{
	const qreal cellSize = Utils::ligoConfig()->CellSize;
	return QPointF(scene.x(), cellSize - scene.y());
}

bool parseWorldPosition(const QString &text, QPointF &world)
{
	// Numbers with an optional sign and decimals, separated by anything else.
	static const QRegularExpression number("[-+]?\\d+(?:[.,]\\d+)?");

	// A decimal comma only counts when it cannot be the separator: "20123,5 -24010,2"
	// has a space between the values, "20123,-24010" does not.
	QString normalised = text.trimmed();
	if (normalised.contains(' ') || normalised.contains(';') || normalised.contains('/') ||
		normalised.contains('\t'))
		normalised.replace(',', '.');

	QList<double> values;
	QRegularExpressionMatchIterator it = number.globalMatch(normalised);
	while (it.hasNext())
		values << it.next().captured(0).replace(',', '.').toDouble();

	if (values.size() != 2)
		return false;
	world = QPointF(values[0], values[1]);
	return true;
}

QString formatWorldPosition(const QPointF &world)
{
	return QString("%1 %2").arg(world.x(), 0, 'f', 2).arg(world.y(), 0, 'f', 2);
}

} // namespace Navigation

// ---------------------------------------------------------------------------

namespace
{
QDoubleSpinBox *coordinateBox(double value, QWidget *parent)
{
	QDoubleSpinBox *box = new QDoubleSpinBox(parent);
	box->setRange(-100000.0, 100000.0);
	box->setDecimals(2);
	box->setSingleStep(1.0);
	box->setValue(value);
	box->setMinimumWidth(120);
	return box;
}
}

PositionDialog::PositionDialog(Mode mode, const QPointF &world, const QString &subject, QWidget *parent)
	: QDialog(parent),
	  m_zoom(0)
{
	setWindowTitle(mode == GotoMode ? tr("Go to Position") : tr("Position"));

	QVBoxLayout *layout = new QVBoxLayout(this);
	if (!subject.isEmpty())
	{
		QLabel *label = new QLabel(subject, this);
		label->setWordWrap(true);
		layout->addWidget(label);
	}

	QFormLayout *form = new QFormLayout();
	m_x = coordinateBox(world.x(), this);
	m_y = coordinateBox(world.y(), this);
	form->addRow(tr("X:"), m_x);
	form->addRow(tr("Y:"), m_y);

	m_paste = new QLineEdit(this);
	m_paste->setPlaceholderText(tr("or paste \"x y\" here"));
	connect(m_paste, SIGNAL(textChanged(QString)), this, SLOT(splitPastedText(QString)));
	form->addRow(tr("Paste:"), m_paste);
	layout->addLayout(form);

	if (mode == GotoMode)
	{
		m_zoom = new QCheckBox(tr("Zoom in on the position"), this);
		m_zoom->setChecked(true);
		layout->addWidget(m_zoom);
	}
	else
	{
		QLabel *note = new QLabel(tr("Changing the values moves the primitive - a group with "
									 "everything in it - so its centre lands there. Ctrl+Z undoes it."), this);
		note->setWordWrap(true);
		layout->addWidget(note);
	}

	QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	buttons->button(QDialogButtonBox::Ok)->setText(mode == GotoMode ? tr("Go") : tr("Move"));
	QPushButton *copy = buttons->addButton(tr("Copy"), QDialogButtonBox::ActionRole);
	copy->setToolTip(tr("Copy the position as \"x y\""));
	connect(copy, SIGNAL(clicked()), this, SLOT(copyPosition()));
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
	layout->addWidget(buttons);

	m_x->setFocus();
	m_x->selectAll();
}

QPointF PositionDialog::position() const
{
	return QPointF(m_x->value(), m_y->value());
}

bool PositionDialog::zoomIn() const
{
	return (m_zoom != 0) && m_zoom->isChecked();
}

void PositionDialog::copyPosition()
{
	QApplication::clipboard()->setText(Navigation::formatWorldPosition(position()));
}

void PositionDialog::splitPastedText(const QString &text)
{
	QPointF world;
	if (Navigation::parseWorldPosition(text, world))
	{
		m_x->setValue(world.x());
		m_y->setValue(world.y());
	}
}

// ---------------------------------------------------------------------------

namespace
{
enum ResultColumn
{
	NameColumn = 0,
	ClassColumn,
	MatchColumn,
	FileColumn
};
const int INDEX_ROLE = Qt::UserRole + 1;
}

FindPrimitiveDialog::FindPrimitiveDialog(PrimitivesTreeModel *model, QWidget *parent)
	: QDialog(parent),
	  m_model(model)
{
	setWindowTitle(tr("Find Primitives"));
	resize(640, 420);

	QVBoxLayout *layout = new QVBoxLayout(this);

	QHBoxLayout *row = new QHBoxLayout();
	m_text = new QLineEdit(this);
	m_text->setPlaceholderText(tr("Text to find - name, class or any property value"));
	m_text->setClearButtonEnabled(true);
	row->addWidget(m_text, 3);

	// Empty means every property. Editable: any property name works.
	m_property = new QComboBox(this);
	m_property->setEditable(true);
	m_property->addItems(QStringList() << QString() << "name" << "class" << "alias"
						 << "sheet" << "sheet_client" << "ai_type" << "ai_profile_params");
	m_property->lineEdit()->setPlaceholderText(tr("in any property"));
	m_property->setToolTip(tr("Look only in this property. Empty: name, class and all properties."));
	row->addWidget(m_property, 1);

	QPushButton *find = new QPushButton(tr("Find"), this);
	find->setDefault(true);
	row->addWidget(find);
	layout->addLayout(row);

	m_results = new QTreeWidget(this);
	m_results->setHeaderLabels(QStringList() << tr("Name") << tr("Class") << tr("Match") << tr("File"));
	m_results->setRootIsDecorated(false);
	m_results->setUniformRowHeights(true);
	m_results->setSortingEnabled(true);
	m_results->header()->setSectionResizeMode(QHeaderView::Interactive);
	layout->addWidget(m_results, 1);

	m_summary = new QLabel(tr("Double click a hit to select it and jump to it on the map."), this);
	layout->addWidget(m_summary);

	connect(find, SIGNAL(clicked()), this, SLOT(search()));
	connect(m_text, SIGNAL(returnPressed()), this, SLOT(search()));
	connect(m_results, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(activateItem(QTreeWidgetItem *)));
}

void FindPrimitiveDialog::activate()
{
	show();
	raise();
	activateWindow();
	m_text->setFocus();
	m_text->selectAll();
}

void FindPrimitiveDialog::search()
{
	m_results->clear();
	const QString text = m_text->text().trimmed();
	if (text.isEmpty())
		return;

	m_results->setSortingEnabled(false);
	int hits = 0;
	searchBelow(QModelIndex(), m_property->currentText().trimmed(), text, hits);
	m_results->setSortingEnabled(true);
	for (int i = 0; i < m_results->columnCount(); ++i)
		m_results->resizeColumnToContents(i);

	m_summary->setText(hits == 0 ? tr("Nothing found.")
					   : tr("%n hit(s). Double click one to select it and jump to it on the map.", "", hits));
}

void FindPrimitiveDialog::searchBelow(const QModelIndex &parent, const QString &property,
									  const QString &text, int &hits)
{
	const int count = m_model->rowCount(parent);
	for (int row = 0; row < count; ++row)
	{
		const QModelIndex index = m_model->index(row, 0, parent);
		Node *node = static_cast<Node *>(index.internalPointer());
		if ((node != 0) && ((node->type() == Node::PrimitiveNodeType) ||
							(node->type() == Node::RootPrimitiveNodeType)))
		{
			const NLLIGO::IPrimitive *primitive = static_cast<PrimitiveNode *>(node)->primitive();
			std::string name, className;
			primitive->getPropertyByName("name", name);
			primitive->getPropertyByName("class", className);

			// First property that matches, reported as "property: value".
			QString match;
			for (uint i = 0; match.isEmpty() && (i < primitive->getNumProperty()); ++i)
			{
				std::string propertyName;
				const NLLIGO::IProperty *prop = 0;
				if (!primitive->getProperty(i, propertyName, prop) || (prop == 0))
					continue;
				const QString qName = QString::fromUtf8(propertyName.c_str());
				if (!property.isEmpty() && (qName.compare(property, Qt::CaseInsensitive) != 0))
					continue;

				if (const NLLIGO::CPropertyString *value = dynamic_cast<const NLLIGO::CPropertyString *>(prop))
				{
					const QString s = QString::fromUtf8(value->String.c_str());
					if (s.contains(text, Qt::CaseInsensitive))
						match = qName + ": " + s;
				}
				else if (const NLLIGO::CPropertyStringArray *values = dynamic_cast<const NLLIGO::CPropertyStringArray *>(prop))
				{
					for (size_t k = 0; k < values->StringArray.size(); ++k)
					{
						const QString s = QString::fromUtf8(values->StringArray[k].c_str());
						if (s.contains(text, Qt::CaseInsensitive))
						{
							match = qName + ": " + s.simplified().left(120);
							break;
						}
					}
				}
			}

			if (!match.isEmpty())
			{
				QString file;
				if (RootPrimitiveNode *root = static_cast<PrimitiveNode *>(node)->rootPrimitiveNode())
					file = root->data(Qt::DisplayRole).toString();

				QTreeWidgetItem *item = new QTreeWidgetItem(m_results);
				item->setText(NameColumn, QString::fromUtf8(name.c_str()));
				item->setIcon(NameColumn, node->data(Qt::DecorationRole).value<QIcon>());
				item->setText(ClassColumn, QString::fromUtf8(className.c_str()));
				item->setText(MatchColumn, match);
				item->setText(FileColumn, file);
				item->setData(NameColumn, INDEX_ROLE, QVariant::fromValue(QPersistentModelIndex(index)));
				++hits;
			}
		}
		searchBelow(index, property, text, hits);
	}
}

void FindPrimitiveDialog::activateItem(QTreeWidgetItem *item)
{
	const QPersistentModelIndex index = item->data(NameColumn, INDEX_ROLE).value<QPersistentModelIndex>();
	if (index.isValid())
		Q_EMIT primitiveActivated(index);
	else
		m_summary->setText(tr("That primitive is gone - search again."));
}

} /* namespace WorldEditor */
