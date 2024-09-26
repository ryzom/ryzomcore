

#include "completer_line_edit.h"

#include <QKeyEvent>
#include <QtGui/QListView>
#include <QtGui/QStringListModel>
#include <QDebug>
#include <QApplication>
#include <QScrollBar>

CompleteLineEdit::CompleteLineEdit(QWidget *parent, QStringList words)
    : QLineEdit(parent), _words(words) 
{
    listView = new QListView(this);
    model = new QStringListModel(this);
	listView->setModel(model);
    listView->setWindowFlags(Qt::ToolTip);
	listView->setUniformItemSizes(true);

    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(setCompleter(const QString &)));
    connect(listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(completeText(const QModelIndex &)));
}

void CompleteLineEdit::setStringList(QStringList list) 
{
	_words = list;
}

//void CompleteLineEdit::focusOutEvent(QFocusEvent *e) 
//{
//	listView->hide();
//}

void CompleteLineEdit::keyPressEvent(QKeyEvent *e) 
{
    if (!listView->isHidden()) 
	{
        int key = e->key();
        int count = listView->model()->rowCount();
        QModelIndex currentIndex = listView->currentIndex();

        if (Qt::Key_Down == key) 
		{
            int row = currentIndex.row() + 1;
            if (row >= count) 
			{
                row = 0;
            }

            QModelIndex index = listView->model()->index(row, 0);
            listView->setCurrentIndex(index);
        } 
		else if (Qt::Key_Up == key) 
		{
            int row = currentIndex.row() - 1;
            if (row < 0) 
			{
                row = count - 1;
            }

            QModelIndex index = listView->model()->index(row, 0);
            listView->setCurrentIndex(index);
        } 
		else if (Qt::Key_Escape == key) 
		{
            listView->hide();
        } 
		else if (Qt::Key_Enter == key || Qt::Key_Return == key) 
		{
            if (currentIndex.isValid()) 
			{
                QString text = listView->currentIndex().data().toString();
                setText(text);
            }

            listView->hide();
        } 
		else 
		{
            //listView->hide();
            QLineEdit::keyPressEvent(e);
        }
    } 
	else 
	{
        QLineEdit::keyPressEvent(e);
    }
}

void CompleteLineEdit::setCompleter(const QString &text) 
{
    if (text.isEmpty()) 
	{
        listView->hide();
        return;
    }

    if ((text.length() > 1) && (!listView->isHidden())) 
	{
        //return;
    }

    QStringList sl;
    Q_FOREACH(QString word, _words) 
	{
        if (word.contains(text)) 
		{
            sl << word;
        }
    }

	if (sl.isEmpty()) 
	{
		if (_words.isEmpty()) 
		{
			setText(tr("No files found"));
			setEnabled(false);
			return;
		}
		else 
		{
			model->setStringList(_words);
		}
	}
	else
	{
		model->setStringList(sl);
	}

    // Position the text edit
    listView->setMinimumWidth(width());
    listView->setMaximumWidth(width());
	//listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QPoint p(0, height());
    int x = mapToGlobal(p).x();
    int y = mapToGlobal(p).y() + 1;

	listView->move(x, y);
	if(!listView->isVisible())
		listView->show();
}

void CompleteLineEdit::completeText(const QModelIndex &index) 
{
    QString text = index.data().toString();
    setText(text);
    listView->hide();
}