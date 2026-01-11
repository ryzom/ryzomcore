#ifndef WORDS_DICDLG_H
#define WORDS_DICDLG_H

#include <QMainWindow>
#include "ui_words_dic_Qt.h"

class QCheckBox;
class QPushButton;
class QLineEdit;
class QLabel;
class QListWidget;

class CWords_dicDlg : public QWidget
{
	Q_OBJECT

public:
	CWords_dicDlg(QWidget *parent = 0);

private slots:
	void on_lookUpEdit_textChanged();
	void on_findButton_clicked();
	void on_clearButton_clicked();
	void on_showAllCheckBox_stateChanged();
	void on_fileListButton_clicked();
	void on_resultsListWidget_itemSelectionChanged();
	void on_lookUpEdit_returnPressed();

private:

	void initDialog();
	void clear();
	void getFileList();
	void lookUp( const std::string& inputStr, const bool showAll );
	void copyIntoClipboard( const std::string& selectedStr );

	Ui::Form ui;
};


#endif