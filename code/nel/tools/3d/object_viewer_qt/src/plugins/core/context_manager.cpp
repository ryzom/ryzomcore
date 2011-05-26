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

// Project includes
#include "context_manager.h"
#include "icontext.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QTabWidget>

namespace Core
{

struct ContextManagerPrivate
{
	explicit ContextManagerPrivate(QTabWidget *tabWidget);
	QTabWidget *m_tabWidget;
	QVector<IContext *> m_contexts;
	int m_oldCurrent;
};

ContextManagerPrivate::ContextManagerPrivate(QTabWidget *tabWidget)
	: m_tabWidget(tabWidget),
	  m_oldCurrent(-1)
{
}

ContextManager::ContextManager(QTabWidget *tabWidget)
	: d(new ContextManagerPrivate(tabWidget))
{
}

ContextManager::~ContextManager()
{
	delete d;
}

Core::IContext* ContextManager::currentContext() const
{
	int currentIndex = d->m_tabWidget->currentIndex();
	if (currentIndex < 0)
		return 0;
	return d->m_contexts.at(currentIndex);
}

Core::IContext* ContextManager::context(const QString &id) const
{
	const int index = indexOf(id);
	if (index >= 0)
		return d->m_contexts.at(index);
	return 0;
}

void ContextManager::activateContext(const QString &id)
{
	const int index = indexOf(id);
	if (index >= 0)
		d->m_tabWidget->setCurrentIndex(index);
}

void ContextManager::addContextObject(IContext *context)
{
}

void ContextManager::removeContextObject(IContext *context)
{
}

void ContextManager::currentTabChanged(int index)
{
}

int ContextManager::indexOf(const QString &id) const
{
	for (int i = 0; i < d->m_contexts.count(); ++i)
	{
		if (d->m_contexts.at(i)->id() == id)
			return i;
	}
	nlwarning(QString("Warning, no such context: %1").arg(id).toStdString().c_str());
	return -1;
}

} /* namespace Core */