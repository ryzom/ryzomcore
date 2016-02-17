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

#ifndef ERRORLIST_H
#define ERRORLIST_H

#define ERRORLIST_EXPORT

#include <set>
#include <map>

#include <QWidget>
#include <QString>
#include <QMap>
#include <QVariant>

class QListWidgetItem;
class QListWidget;
class QPushButton;
class QTextEdit;

namespace NLQT {

class ERRORLIST_EXPORT CErrorList : public QWidget
{
	Q_OBJECT

public:
	enum ErrorType
	{
		Error,
		Warning,
		Message
	};

public:
	CErrorList(QWidget *parent);
	virtual ~CErrorList();

	void add(ErrorType type, const QString &group, time_t timestamp, const QString &message, const QMap<QString, QVariant> &userData);
	void add(ErrorType type, const QString &group, time_t timestamp, const QString &message);
	void add(ErrorType type, const QString &group, const QString &message, const QMap<QString, QVariant> &userData);
	void add(ErrorType type, const QString &group, const QString &message);

	void update(const QString &group, const QString &message);

	void markClear(const QString &group);
	void clearMarked();

	void clear(const QString &group);
	void filter(ErrorType type, bool f);

signals:
	void request(const QString &group, const QMap<QString, QVariant> &userData);

public slots:
	void clear();
	void collapse(bool c);
	void filterError(bool f);
	void filterWarning(bool f);
	void filterMessage(bool f);

private slots:
	void listItemClicked(QListWidgetItem *item);
	void listItemDoubleClicked(QListWidgetItem *item);

private:
	void updateFilterCount(int filter);
	static QString getCollapseString(QListWidgetItem *item);
	static void updateCollapseText(QListWidgetItem *item);

private:
	QListWidget *m_List;
	std::map<QString, QListWidgetItem *> m_CollapseItems;
	bool m_Collapse;
	QPushButton *m_CollapseBtn;
	bool m_Filter[3];
	QPushButton *m_FilterBtn[3];
	int m_FilterCounts[3];
	QTextEdit *m_Message;
	QListWidgetItem *m_CurrentMessage;
	std::set<QString> m_MarkedClear;
	
}; /* class CErrorList */

}

#endif /* ERRORLIST_H */

/* end of file */
