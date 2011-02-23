#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QFileDialog>

namespace Ui
{
class Dialog;
}

class Dialog : public QDialog
{
	Q_OBJECT

public:
	explicit Dialog(QWidget *parent = 0);
	~Dialog();
public Q_SLOTS:
	void reloadTable();
	void getDir();

private:
	QString dir;
	Ui::Dialog *ui;
};

#endif // DIALOG_H
