#ifndef ACTION_LIST_H
#define ACTION_LIST_H


#include "ui_action_list.h"
#include <QString>


class ActionList : public QDialog, public Ui::ActionListDialog
{
	Q_OBJECT

public:
	ActionList( QDialog *parent = NULL );
	~ActionList();
	void load();

	QString getSelectedAction(){ return selectedAction; }


public Q_SLOTS:
	void accept();
	void reject();

private:
	QString selectedAction;
};

#endif
