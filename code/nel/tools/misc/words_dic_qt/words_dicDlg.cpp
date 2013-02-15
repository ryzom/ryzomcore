#include <QtGui>
#include <QString>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QtAlgorithms>
#include <nel/misc/words_dictionary.h>
#include "words_dicDlg.h"

using namespace std;
using namespace NLMISC;

CWordsDictionary Dico;



 bool caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
	 return QString::compare(s1, s2, Qt::CaseInsensitive) < 0;
}


 CWords_dicDlg::CWords_dicDlg(QWidget *parent)
     : QWidget(parent)

 {
	 ui.setupUi(this);
     initDialog();
 }



void CWords_dicDlg::initDialog()
{
	QPixmap pixmap(300, 60);
    pixmap.fill(Qt::gray);

	QPixmap brushPixmap = QPixmap(":/nel.png").scaled(QSize(85,60), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QBrush brush(brushPixmap);
	QPainter painter(&pixmap);
    painter.fillRect(0, 0, brushPixmap.width(), brushPixmap.height(), brush);

	QSplashScreen *splash = new QSplashScreen(pixmap);
	splash->show();

	// Loading some items
	splash->showMessage("Please wait while loading dictionary...       ", Qt::AlignVCenter | Qt::AlignRight);

	if(!Dico.init())
	{
		QMessageBox::information( this, "Error Loading Dic", "Can't init dictionary, see reason in log.log" );
	}

	splash->finish(this);

	ui.lookUpEdit->setFocus();
	ui.statusLabel->setText( "Tip: ^ and $ can be used to represent the start and the end of string" );

}

void CWords_dicDlg::lookUp( const std::string& inputStr, const bool showAll )
{
	//Clearing
	clear();
	ui.statusLabel->setText( "searching ..." );

	// Look up
	CVectorSString resultVec;
	Dico.lookup( CSString(inputStr), resultVec );

	// Display results
	
	if ( resultVec.empty() )
	{
		//QStringList list;
		//list.append("<no result>");

		//ui_resultsListWidget->addItems(QStringList(list));
		ui.statusLabel->setText( "no result found." );
	}
	else
	{
		bool lvlRemoved = false;
		QStringList list;
		for ( CVectorSString::const_iterator ivs=resultVec.begin(); ivs!=resultVec.end(); ++ivs )
		{
			const CSString& res = (*ivs);
			if ( showAll || (res.find( "lvl" ) == (unsigned)std::string::npos) )
			{
				list.append(res.c_str());
			}
			else
				lvlRemoved = true;
		}

		qSort(list.begin(), list.end(), caseInsensitiveLessThan);
		//list.sort();
		ui.resultsListWidget->addItems(list);

		QString s = QString("%1 results found for \"%2\".%3").arg(list.size()).arg(QString(inputStr.c_str())).arg(lvlRemoved?" Results containing \"lvl\" not shown":"");
		ui.statusLabel->setText( s );
	}
}


void CWords_dicDlg::getFileList()
{
	clear();

	const vector<string>& fileList = Dico.getFileList();
	QStringList list;
	for ( vector<string>::const_iterator ifl=fileList.begin(); ifl!=fileList.end(); ++ifl )
	{
		list.append( (*ifl).c_str() );
	}

	ui.resultsListWidget->addItems(list);

	QString s = QString("%1 file(s) found.").arg(list.size());
	ui.statusLabel->setText( s );
}

void CWords_dicDlg::copyIntoClipboard(const std::string &selectedStr)
{
	CSString key = Dico.getWordsKey(  CSString(selectedStr) );

	QClipboard *cb = QApplication::clipboard();

    // Copy text into the clipboard
    cb->setText( key.c_str(), QClipboard::Clipboard );
    
	// Copy text from the clipboard (paste)
    QString text = cb->text(QClipboard::Clipboard);
    if ( !text.isNull() )
	{
		QString s = QString("\"%1\" copied into the clipboard").arg(key.c_str());
		ui.statusLabel->setText( s );
	}
	else
	{
		ui.statusLabel->setText( "Cannot access the clipboard"  );
	}
}

void CWords_dicDlg::clear()
{
	ui.resultsListWidget->clear();
}	


void CWords_dicDlg::on_findButton_clicked()
{
	QString inputStr = ui.lookUpEdit->text();
	bool showAll = ui.showAllCheckBox->isChecked();
	lookUp( inputStr.toUtf8().constData(), showAll );
}

void CWords_dicDlg::on_lookUpEdit_textChanged()
{
	// Ignore if input string too short
	if ( ui.lookUpEdit->text().length() < 3 )
		return;

	on_findButton_clicked();
}

void CWords_dicDlg::on_lookUpEdit_returnPressed()
{
	on_findButton_clicked();
}


void CWords_dicDlg::on_clearButton_clicked()
{
	ui.lookUpEdit->clear();
	on_findButton_clicked();
}


void CWords_dicDlg::on_showAllCheckBox_stateChanged()
{
	on_findButton_clicked();
}


void CWords_dicDlg::on_fileListButton_clicked()
{
	getFileList();
}


void CWords_dicDlg::on_resultsListWidget_itemSelectionChanged()
{
	QListWidgetItem *i = ui.resultsListWidget->currentItem();
	copyIntoClipboard( i->text().toUtf8().constData() );
}


