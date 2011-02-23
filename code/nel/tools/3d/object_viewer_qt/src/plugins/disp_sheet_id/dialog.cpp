#include "dialog.h"
#include "ui_dialog.h"
#include "bin_reader.h"

#include "QMessageBox"
#include "QSettings"

Dialog::Dialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Dialog)
{
	ui->setupUi(this);
	QSettings settings("ovqt_sheet_builder.ini", QSettings::IniFormat);
	dir = settings.value("PathToSheetId", "").toString();
	connect(ui->reloadButton,SIGNAL(clicked()),this,SLOT(reloadTable()));
	connect(ui->browseButton,SIGNAL(clicked()),this,SLOT(getDir()));

}

Dialog::~Dialog()
{
	delete ui;
}
void Dialog::reloadTable()
{
	QFile file(dir + QString("./sheet_id.bin"));
	if(file.exists() == true)
	{
		QSettings settings("ovqt_sheet_builder.ini", QSettings::IniFormat);
		settings.setValue("PathToSheetId", dir);
		BinReader reader(this->dir);
		reader.pushToTable(ui->table);

	}
	else
	{
		QMessageBox errorBox;
		errorBox.setText(QString("File sheet_id.bin doesn't exist in direcotry: \n") + this->dir);
		errorBox.exec();
	}
}
void Dialog::getDir()
{
	QFileDialog fileDialog(this);
	fileDialog.setFileMode(QFileDialog::DirectoryOnly);
	this->dir = fileDialog.getExistingDirectory();
}
