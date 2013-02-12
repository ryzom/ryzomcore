/*
Georges Editor Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "formdelegate.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/georges/u_type.h>
#include <nel/georges/u_form_elm.h>

// Qt includes
#include <QSpinBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QColorDialog>
#include <QComboBox>
#include <QApplication>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
// Project includes
#include "georgesform_model.h"
#include "georgesform_proxy_model.h"
#include "formitem.h"

namespace NLQT 
{

	FormDelegate::FormDelegate(QObject *parent)
		: QStyledItemDelegate(parent)
	{
	}

	QWidget *FormDelegate::createEditor(QWidget *parent,
		const QStyleOptionViewItem & option ,
		const QModelIndex &index) const
	{
		const CGeorgesFormProxyModel * mp = dynamic_cast<const CGeorgesFormProxyModel *>(index.model());
		const CGeorgesFormModel * m = dynamic_cast<const CGeorgesFormModel *>(mp->sourceModel());
		CFormItem *item = static_cast<CFormItem*>(mp->mapToSource(index).internalPointer());
		QString value = item->data(1).toString();

		if (value.isEmpty() || !mp || !m)
			return 0;

		CFormItem* curItem = m->getItem(mp->mapToSource(index));
		NLGEORGES::UFormElm *curElm = curItem->getFormElm();
		if (!curElm) {
			// TODO: create new Element
			return 0;
		}
		const NLGEORGES::UType *type = curElm->getType();
		if(type) 
		{
			int numDefinitions = type->getNumDefinition();

			if (numDefinitions) 
			{
				std::string l, v;
				QString label,value;

				QComboBox *editor = new QComboBox(parent);
				for (int i = 0; i < numDefinitions; i++) 
				{
					type->getDefinition(i,l,v);
					label = l.c_str();
					value = v.c_str();
					editor->addItem(label);
				}
				return editor;
			}
			else 
			{
				switch (type->getType()) 
				{
				case NLGEORGES::UType::UnsignedInt:
				case NLGEORGES::UType::SignedInt:
					{
						QSpinBox *editor = new QSpinBox(parent);

						//QString min = QString(type->getMin().c_str());
						//QString max = QString(type->getMax().c_str());
						//QString inc = QString(type->getIncrement().c_str());
						//nldebug(QString("min %1 max %2 inc %3").arg(min).arg(max).arg(inc).toUtf8().constData());

						// TODO: use saved min/max values
						editor->setMinimum(-99999);
						editor->setMaximum(99999);
						editor->setSingleStep(1);
						return editor;
					}
				case NLGEORGES::UType::Double:
					{
						QDoubleSpinBox *editor = new QDoubleSpinBox(parent);

						//QString min = QString(type->getMin().c_str());
						//QString max = QString(type->getMax().c_str());
						//QString inc = QString(type->getIncrement().c_str());
						//nldebug(QString("min %1 max %2 inc %3").arg(min).arg(max).arg(inc).toStdString().c_str());

						// TODO: use saved min/max values
						editor->setMinimum(-99999);
						editor->setMaximum(99999);
						editor->setSingleStep(0.1);
						editor->setDecimals(1);
						return editor;
					}
				case NLGEORGES::UType::Color:
					{
						return new QColorDialog();
					}
				default: // UType::String
					{
						QLineEdit *editor = new QLineEdit(parent);
						return editor;
					}
				}
			}
		}
		return 0;
	}

	void FormDelegate::setEditorData(QWidget *editor,
		const QModelIndex &index) const
	{
		const CGeorgesFormProxyModel * mp = dynamic_cast<const CGeorgesFormProxyModel *>(index.model());
		const CGeorgesFormModel * m = dynamic_cast<const CGeorgesFormModel *>(mp->sourceModel());

		const NLGEORGES::UType *type = m->getItem(mp->mapToSource(index))->getFormElm()->getType();
		int numDefinitions = type->getNumDefinition();
		QString value = index.model()->data(index, Qt::DisplayRole).toString();

		if (numDefinitions) 
		{
			QComboBox *cb = static_cast<QComboBox*>(editor);
			cb->setCurrentIndex(cb->findText(value));
			//cb->setIconSize()
		}
		else 
		{
			switch (type->getType()) 
			{
			case NLGEORGES::UType::UnsignedInt:
			case NLGEORGES::UType::SignedInt:
				{
					QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
					spinBox->setValue((int)value.toDouble());
					break;
				}
			case NLGEORGES::UType::Double:
				{
					QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
					spinBox->setValue(value.toDouble());
					break;
				}
			case NLGEORGES::UType::Color:
				{
					break;
				}
			default:
				{
					QLineEdit *textEdit = static_cast<QLineEdit*>(editor);
					textEdit->setText(value);
					break;
				}
			}
		}
	}

	void FormDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
		const QModelIndex &index) const
	{
		const CGeorgesFormProxyModel * mp = dynamic_cast<const CGeorgesFormProxyModel *>(index.model());
		const CGeorgesFormModel * m = dynamic_cast<const CGeorgesFormModel *>(mp->sourceModel());

		const NLGEORGES::UType *type = m->getItem(mp->mapToSource(index))->getFormElm()->getType();
		int numDefinitions = type->getNumDefinition();

		if (numDefinitions) 
		{
			QComboBox *comboBox = static_cast<QComboBox*>(editor);
			QString value = comboBox->currentText();
			QString oldValue = index.model()->data(index, Qt::DisplayRole).toString();
			if (value == oldValue) 
			{
				// nothing's changed
			}
			else 
			{
				nldebug(QString("setModelData from %1 to %2")
					.arg(oldValue).arg(value).toUtf8().constData());
				model->setData(index, value, Qt::EditRole);
			}
		}
		else 
		{
			switch (type->getType()) 
			{
			case NLGEORGES::UType::UnsignedInt:
			case NLGEORGES::UType::SignedInt:
				{
					QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
					int value = spinBox->value();
					QString oldValue = index.model()->data(index, Qt::DisplayRole).toString();
					if (QString("%1").arg(value) == oldValue) 
					{
						// nothing's changed
					}
					else 
					{
						nldebug(QString("setModelData from %1 to %2")
							.arg(oldValue).arg(value).toUtf8().constData());
						model->setData(index, value, Qt::EditRole);
					}
					break;
				}
			case NLGEORGES::UType::Double:
				{
					QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
					double value = spinBox->value();
					QString oldValue = index.model()->data(index, Qt::DisplayRole).toString();
					if (QString("%1").arg(value) == oldValue) 
					{
						// nothing's changed
					}
					else 
					{
						nldebug(QString("setModelData from %1 to %2")
							.arg(oldValue).arg(value).toUtf8().constData());
						model->setData(index, value, Qt::EditRole);
					}
					break;
				}
			case NLGEORGES::UType::Color:
				{
					break; // TODO
				}
			default: // UType::String
				{
					QLineEdit *textEdit = static_cast<QLineEdit*>(editor);
					QString value = textEdit->text();
					QString oldValue = index.model()->data(index, Qt::DisplayRole).toString();
					if (value == oldValue) 
					{
						// nothing's changed
					}
					else 
					{
						nldebug(QString("setModelData from %1 to %2")
							.arg(oldValue).arg(value).toUtf8().constData());
						model->setData(index, value, Qt::EditRole);
					}
					break;
				}
			}
		}
	}

	void FormDelegate::updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QRect r = option.rect;
		editor->setGeometry(r);
		//option.decorationAlignment = QStyleOptionViewItem::Right;
	}

	//void FormDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	//{
	//	QStyleOptionViewItemV4 optionV4 = option;
	//	optionV4.decorationPosition = QStyleOptionViewItem::Right;
	//	//optionV4.decorationSize = QSize(32,32);
	//	initStyleOption(&optionV4, index);

	//	QStyledItemDelegate::paint(painter,optionV4,index);

	//	//QStyle *style = optionV4.widget? optionV4.widget->style() : QApplication::style();

	//	//QTextDocument doc;
	//	//doc.setHtml(optionV4.text);

	//	///// Painting item without text
	//	//optionV4.text = QString();
	//	//style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);

	//	//QAbstractTextDocumentLayout::PaintContext ctx;

	//	//// Highlighting text if item is selected
	//	//if (optionV4.state & QStyle::State_Selected)
	//	//    ctx.palette.setColor(QPalette::Text, optionV4.palette.color(QPalette::Active, QPalette::HighlightedText));

	//	//QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4);
	//	//painter->save();
	//	//painter->translate(textRect.topLeft());
	//	//painter->setClipRect(textRect.translated(-textRect.topLeft()));
	//	//doc.documentLayout()->draw(painter, ctx);
	//	//painter->restore();
	//}

} /* namespace NLQT */
