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
#include "main_window.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QTabWidget>
#include <QtGui/QGridLayout>

namespace Core
{

struct ContextManagerPrivate
{
	explicit ContextManagerPrivate(Core::MainWindow *mainWindow, QTabWidget *tabWidget);
	Core::MainWindow *m_mainWindow;
	QTabWidget *m_tabWidget;
	QVector<IContext *> m_contexts;
	int m_oldCurrent;
};

ContextManagerPrivate::ContextManagerPrivate(Core::MainWindow *mainWindow, QTabWidget *tabWidget)
	: m_mainWindow(mainWindow),
	  m_tabWidget(tabWidget),
	  m_oldCurrent(-1)
{
}

ContextManager::ContextManager(Core::MainWindow *mainWindow, QTabWidget *tabWidget)
	: d(new ContextManagerPrivate(mainWindow, tabWidget))
{
	QObject::connect(d->m_mainWindow->pluginManager(), SIGNAL(objectAdded(QObject *)),
					 this, SLOT(objectAdded(QObject *)));
	QObject::connect(d->m_mainWindow->pluginManager(), SIGNAL(aboutToRemoveObject(QObject *)),
					 this, SLOT(aboutToRemoveObject(QObject *)));

	QObject::connect(d->m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));
}

ContextManager::~ContextManager()
{
	delete d;
}

Core::IContext *ContextManager::currentContext() const
{
	int currentIndex = d->m_tabWidget->currentIndex();
	if (currentIndex < 0)
		return 0;
	return d->m_contexts.at(currentIndex);
}

Core::IContext *ContextManager::context(const QString &id) const
{
	const int index = indexOf(id);
	if (index >= 0)
		return d->m_contexts.at(index);
	return 0;
}

void ContextManager::registerUndoStack(QUndoStack *stack)
{
	nlassert(stack);
	d->m_mainWindow->undoGroup()->addStack(stack);
}

void ContextManager::unregisterUndoStack(QUndoStack *stack)
{
	nlassert(stack);
	d->m_mainWindow->undoGroup()->removeStack(stack);
}

void ContextManager::activateContext(const QString &id)
{
	const int index = indexOf(id);
	if (index >= 0)
		d->m_tabWidget->setCurrentIndex(index);
}

void ContextManager::updateCurrentContext()
{
	d->m_mainWindow->updateContext(currentContext());
}

void ContextManager::objectAdded(QObject *obj)
{
	IContext *context = qobject_cast<IContext *>(obj);
	if (context)
		addContextObject(context);
}

void ContextManager::aboutToRemoveObject(QObject *obj)
{
	IContext *context = qobject_cast<IContext *>(obj);
	if (context)
		removeContextObject(context);
}

void ContextManager::addContextObject(IContext *context)
{
	d->m_contexts.push_back(context);
	d->m_mainWindow->addContextObject(context);

	QWidget *tabWidget = new QWidget(d->m_tabWidget);
	d->m_tabWidget->addTab(tabWidget, context->icon(), context->trName());
	QGridLayout *gridLayout = new QGridLayout(tabWidget);
	gridLayout->setObjectName(QString::fromUtf8("gridLayout_") + context->id());
	gridLayout->setContentsMargins(0, 0, 0, 0);
	gridLayout->addWidget(context->widget(), 0, 0, 1, 1);
}

void ContextManager::removeContextObject(IContext *context)
{
	d->m_mainWindow->removeContextObject(context);

	const int index = indexOf(context->id());
	QWidget *widget = d->m_tabWidget->widget(index);
	d->m_contexts.remove(index);
	d->m_tabWidget->removeTab(index);
	delete widget;
}

void ContextManager::currentTabChanged(int index)
{
	if (index >= 0)
	{
		IContext *context = d->m_contexts.at(index);
		IContext *oldContext = 0;
		if (d->m_oldCurrent >= 0)
			oldContext = d->m_contexts.at(d->m_oldCurrent);
		d->m_oldCurrent = index;
		Q_EMIT currentContextChanged(context, oldContext);
	}
}

int ContextManager::indexOf(const QString &id) const
{
	for (int i = 0; i < d->m_contexts.count(); ++i)
	{
		if (d->m_contexts.at(i)->id() == id)
			return i;
	}
	nlwarning(QString("Warning, no such context: %1").arg(id).toUtf8().constData());
	return -1;
}

} /* namespace Core */