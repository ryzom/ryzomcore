/*

Copyright (C) 2015  by authors
Author: Jan Boon <jan.boon@kaetemi.be>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

// Source: https://github.com/kaetemi/errorlist

#include "error_list.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTextEdit>
#include <QSplitter>
#include <QDateTime>

namespace NLQT {

#define DATAROLE_TYPE (Qt::UserRole + 0)
#define DATAROLE_TIMESTAMP (Qt::UserRole + 1)
#define DATAROLE_GROUP (Qt::UserRole + 2)
#define DATAROLE_MESSAGE (Qt::UserRole + 3)
#define DATAROLE_LINE (Qt::UserRole + 4)
#define DATAROLE_USERDATA (Qt::UserRole + 5)
#define DATAROLE_COLLAPSED (Qt::UserRole + 6)

class CErrorListItem : public QListWidgetItem
{
public:
	CErrorListItem(const QIcon &icon, const QString &message) : QListWidgetItem(icon, message)
	{

	}

	virtual bool operator<(const QListWidgetItem &other) const Q_DECL_OVERRIDE
	{
		time_t this_timestamp = data(DATAROLE_TIMESTAMP).toInt();
		time_t other_timestamp = other.data(DATAROLE_TIMESTAMP).toInt();
		return this_timestamp < other_timestamp;
	}
};

const char *iconNames[] = {
	":/icons/cross-circle.png",
	":/icons/exclamation.png",
	":/icons/information-white.png"
};

const char *filterText[] = {
	"Errors",
	"Warnings",
	"Messages"
};

CErrorList::CErrorList(QWidget *parent) : QWidget(parent), m_Collapse(true), m_CollapseBtn(NULL), m_Message(NULL)
{
	m_Filter[0] = true;
	m_Filter[1] = true;
	m_Filter[2] = true;
	m_FilterBtn[0] = NULL;
	m_FilterBtn[1] = NULL;
	m_FilterBtn[2] = NULL;
	m_FilterCounts[0] = 0;
	m_FilterCounts[1] = 0;
	m_FilterCounts[2] = 0;

	QVBoxLayout *layout = new QVBoxLayout(this);

	QHBoxLayout *buttons = new QHBoxLayout(this);

	QPushButton *clearBtn = new QPushButton(this);
	clearBtn->setText(tr("Clear"));
	connect(clearBtn, SIGNAL(clicked()), this, SLOT(clear()));
	buttons->addWidget(clearBtn);

	QPushButton *collapseBtn = new QPushButton(this);
	m_CollapseBtn = collapseBtn;
	collapseBtn->setText(tr("Collapse"));
	collapseBtn->setCheckable(true);
	collapseBtn->setChecked(m_Collapse);
	connect(collapseBtn, SIGNAL(toggled(bool)), this, SLOT(collapse(bool)));
	buttons->addWidget(collapseBtn);

	buttons->addStretch();

	QPushButton *errorBtn = new QPushButton(this);
	m_FilterBtn[0] = errorBtn;
	errorBtn->setIcon(QIcon(iconNames[0]));
	updateFilterCount(0);
	errorBtn->setCheckable(true);
	errorBtn->setChecked(m_Filter[0]);
	connect(errorBtn, SIGNAL(toggled(bool)), this, SLOT(filterError(bool)));
	buttons->addWidget(errorBtn);

	QPushButton *warningBtn = new QPushButton(this);
	m_FilterBtn[1] = warningBtn;
	warningBtn->setIcon(QIcon(iconNames[1]));
	updateFilterCount(1);
	warningBtn->setCheckable(true);
	warningBtn->setChecked(m_Filter[1]);
	connect(warningBtn, SIGNAL(toggled(bool)), this, SLOT(filterWarning(bool)));
	buttons->addWidget(warningBtn);

	QPushButton *messageBtn = new QPushButton(this);
	m_FilterBtn[2] = messageBtn;
	messageBtn->setIcon(QIcon(iconNames[2]));
	updateFilterCount(2);
	messageBtn->setCheckable(true);
	messageBtn->setChecked(m_Filter[2]);
	connect(messageBtn, SIGNAL(toggled(bool)), this, SLOT(filterMessage(bool)));
	buttons->addWidget(messageBtn);

	layout->addLayout(buttons);

	QSplitter *splitter = new QSplitter(Qt::Vertical, this);

	m_List = new QListWidget(this);
	m_List->setTextElideMode(Qt::ElideRight);
	m_List->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	// m_List->setSortingEnabled(true);
	connect(m_List, &QListWidget::itemClicked, this, &CErrorList::listItemClicked);
	connect(m_List, &QListWidget::itemDoubleClicked, this, &CErrorList::listItemDoubleClicked);
	splitter->addWidget(m_List);

	QTextEdit *messageText = new QTextEdit(this);
	m_Message = messageText;
	messageText->setReadOnly(true);
	messageText->setVisible(false);
	messageText->setAcceptRichText(true);
	splitter->addWidget(messageText);

	layout->addWidget(splitter);

	setLayout(layout);
}

CErrorList::~CErrorList()
{

}

void CErrorList::clear()
{
	m_CurrentMessage = NULL;
	m_Message->setVisible(false);
	m_CollapseItems.clear();
	m_List->clear();
	m_FilterCounts[0] = 0;
	m_FilterCounts[1] = 0;
	m_FilterCounts[2] = 0;
	updateFilterCount(0);
	updateFilterCount(1);
	updateFilterCount(2);
}

void CErrorList::clear(const QString &group)
{
	for (int i = 0; i < m_List->count(); ++i)
	{
		QListWidgetItem *item = m_List->item(i);
		if (item->data(DATAROLE_GROUP).toString() == group)
		{
			if (m_CurrentMessage == item)
			{
				m_CurrentMessage = NULL;
				m_Message->setVisible(false);
			}
			if (m_Collapse)
			{
				QString cs = getCollapseString(item);
				std::map<QString, QListWidgetItem *>::iterator it
					= m_CollapseItems.find(cs);
				if (it != m_CollapseItems.end())
					m_CollapseItems.erase(it);
			}
			--m_FilterCounts[(int)item->data(DATAROLE_TYPE).toInt()];
			delete item;
			--i;
		}
	}
	updateFilterCount(0);
	updateFilterCount(1);
	updateFilterCount(2);
}

void CErrorList::collapse(bool c)
{
	if (c != m_Collapse)
	{
		m_Collapse = c;
		m_CollapseBtn->setChecked(c);

		if (c)
		{
			for (int i = 0; i < m_List->count(); ++i)
			{
				QListWidgetItem *item = m_List->item(i);
				QString cs = getCollapseString(item);
				QListWidgetItem *ref = m_CollapseItems[cs];

				if (ref != NULL)
				{
					ref->setData(DATAROLE_COLLAPSED, ref->data(DATAROLE_COLLAPSED).toInt() + 1);
					updateCollapseText(ref);
					item->setData(DATAROLE_COLLAPSED, 0);
					item->setHidden(true);
					if (m_CurrentMessage == item)
					{
						m_CurrentMessage = NULL;
						m_Message->setHidden(true);
					}
				}
				else
				{
					m_CollapseItems[cs] = item;
				}
			}
		}
		else
		{
			m_CollapseItems.clear();
			for (int i = 0; i < m_List->count(); ++i)
			{
				int cc = m_List->item(i)->data(DATAROLE_COLLAPSED).toInt();
				if (cc > 1)
				{
					m_List->item(i)->setText(m_List->item(i)->data(DATAROLE_LINE).toString());
				}
				if (cc == 0)
				{
					bool hide = !m_Filter[m_List->item(i)->data(DATAROLE_TYPE).toInt()];
					m_List->item(i)->setHidden(hide);
				}
				if (cc != 1)
				{
					m_List->item(i)->setData(DATAROLE_COLLAPSED, 1);
				}
			}
		}
	}
}

void CErrorList::filter(ErrorType type, bool f)
{
	if (f != m_Filter[(int)type])
	{
		m_Filter[(int)type] = f;
		m_FilterBtn[(int)type]->setChecked(f);

		for (int i = 0; i < m_List->count(); ++i)
		{
			if (m_List->item(i)->data(DATAROLE_TYPE).toInt() == (int)type)
			{
				bool hide =
					m_List->item(i)->data(DATAROLE_COLLAPSED).toInt() == 0
					|| !f;
				m_List->item(i)->setHidden(hide);
				if (hide && m_CurrentMessage == m_List->item(i))
				{
					m_CurrentMessage = NULL;
					m_Message->setHidden(true);
				}
			}
		}
	}
}

void CErrorList::updateFilterCount(int filter)
{
	m_FilterBtn[filter]->setText(QString(" ") + QString::number(m_FilterCounts[filter]) + " " + filterText[filter]);
}

void CErrorList::filterError(bool f)
{
	filter(Error, f);
}

void CErrorList::filterWarning(bool f)
{
	filter(Warning, f);
}

void CErrorList::filterMessage(bool f)
{
	filter(Message, f);
}

void CErrorList::markClear(const QString &group)
{
	for (int i = 0; i < m_List->count(); ++i)
	{
		QListWidgetItem *item = m_List->item(i);
		if (item->data(DATAROLE_GROUP).toString() == group)
		{
			QString cs = getCollapseString(item);
			m_MarkedClear.insert(cs);
		}
	}
}

void CErrorList::clearMarked()
{
	for (int i = 0; i < m_List->count(); ++i)
	{
		QListWidgetItem *item = m_List->item(i);
		QString cs = getCollapseString(item);
		if (m_MarkedClear.find(cs) != m_MarkedClear.end())
		{
			if (m_CurrentMessage == item)
			{
				m_CurrentMessage = NULL;
				m_Message->setVisible(false);
			}
			if (m_Collapse)
			{
				QString cs = getCollapseString(item);
				std::map<QString, QListWidgetItem *>::iterator it
					= m_CollapseItems.find(cs);
				if (it != m_CollapseItems.end())
					m_CollapseItems.erase(it);
			}
			--m_FilterCounts[(int)item->data(DATAROLE_TYPE).toInt()];
			delete item;
			--i;
		}
	}
}

void CErrorList::listItemClicked(QListWidgetItem *item)
{
	if (item != m_CurrentMessage)
	{
		if (item->isSelected())
		{
			QString text = item->data(DATAROLE_MESSAGE).toString();
			m_Message->setHtml(text);
			m_Message->setVisible(true);
		}
		else
		{
			m_CurrentMessage = NULL;
			m_Message->setVisible(false);
		}
	}
}

void CErrorList::listItemDoubleClicked(QListWidgetItem *item)
{
	emit request(item->data(DATAROLE_GROUP).toString(), item->data(DATAROLE_USERDATA).toMap());
}

QString CErrorList::getCollapseString(QListWidgetItem *item)
{
	return item->data(DATAROLE_GROUP).toString() + " / " + item->data(DATAROLE_LINE).toString();
}

void CErrorList::updateCollapseText(QListWidgetItem *item)
{
	item->setText(QString("(" + QString::number(item->data(DATAROLE_COLLAPSED).toInt()) + "x) " + item->data(DATAROLE_LINE).toString()));// .replace('\n', ' ').replace('\r', ' ')));
}

void CErrorList::add(ErrorType type, const QString &group, time_t timestamp, const QString &message, const QMap<QString, QVariant> &userData)
{
	QTextDocument doc;
	doc.setHtml(message);
	QString line = doc.toPlainText().replace('\n', ' ').replace('\r', ' ');

	CErrorListItem *item = new CErrorListItem(QIcon(iconNames[(int)type]), line);
	item->setData(DATAROLE_TYPE, (int)type);
	item->setData(DATAROLE_TIMESTAMP, timestamp);
	item->setData(DATAROLE_GROUP, group);
	item->setData(DATAROLE_MESSAGE, message);
	item->setData(DATAROLE_LINE, line);
	item->setData(DATAROLE_USERDATA, userData);

	QString cs = getCollapseString(item);
	if (m_MarkedClear.find(cs) != m_MarkedClear.end())
	{
		m_MarkedClear.erase(cs);
		return;
	}

	bool hide = m_Collapse;
	if (hide)
	{
		QListWidgetItem *ref = m_CollapseItems[cs];
		hide = ref != NULL;
		if (hide)
		{
			ref->setData(DATAROLE_COLLAPSED, ref->data(DATAROLE_COLLAPSED).toInt() + 1);
			updateCollapseText(ref);
			item->setData(DATAROLE_COLLAPSED, 0);
		}
		else
		{
			m_CollapseItems[cs] = item;
		}
	}
	if (!hide)
	{
		item->setData(DATAROLE_COLLAPSED, 1);
		hide = !m_Filter[(int)type];
	}

	m_List->addItem(item);
	item->setHidden(hide);
	++m_FilterCounts[(int)type];
	updateFilterCount((int)type);
}

void CErrorList::update(const QString &group, const QString &message)
{
	// NOTE: This does not play well with collapse, so you should only have one message present in the category
	// Automatically adds a message if no message exists in the category
	for (int i = 0; i < m_List->count(); ++i)
	{
		QListWidgetItem *item = m_List->item(i);
		if (item->data(DATAROLE_GROUP).toString() == group)
		{
			QTextDocument doc;
			doc.setHtml(message);
			QString line = doc.toPlainText().replace('\n', ' ').replace('\r', ' ');
			item->setData(DATAROLE_MESSAGE, message);
			item->setData(DATAROLE_LINE, line);
			item->setText(line);
			return;
		}
	}
	add(Message, group, message);
}

void CErrorList::add(ErrorType type, const QString &group, time_t timestamp, const QString &message)
{
	QMap<QString, QVariant> nullMap;
	add(type, group, timestamp, message, nullMap);
}

void CErrorList::add(ErrorType type, const QString &group, const QString &message, const QMap<QString, QVariant> &userData)
{
	add(type, group, QDateTime::currentDateTime().toTime_t(), message, userData);
}

void CErrorList::add(ErrorType type, const QString &group, const QString &message)
{
	QMap<QString, QVariant> nullMap;
	add(type, group, message, nullMap);
}

}

/* end of file */