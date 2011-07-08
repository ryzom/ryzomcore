
#include "ftp_selection.h"

#include <QtGui/QMessageBox>
#include <QtNetwork/QFtp>
namespace Plugin
{
    CFtpSelection::CFtpSelection(QWidget *parent): QDialog(parent)
    {
        _ui.setupUi(this);
        connect(_ui.connectButton, SIGNAL(clicked()), this, SLOT(ConnectButtonClicked()));
        connect(_ui.doneButton, SIGNAL(clicked()), this, SLOT(DoneButtonClicked()));
        connect(_ui.cancelButton, SIGNAL(clicked()), this, SLOT(CancelButtonClicked()));
        
       // file list       
        connect(_ui.fileList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),this, SLOT(processItem(QTreeWidgetItem*,int)));
        _ui.fileList->setEnabled(false);
        _ui.fileList->setRootIsDecorated(false);
        _ui.fileList->setHeaderLabels(QStringList() << tr("Name") << tr("Size") << tr("Owner") << tr("Group") << tr("Time"));
        _ui.fileList->header()->setStretchLastSection(false);
    }
    
    void CFtpSelection::ConnectButtonClicked()
    {
        conn = new QFtp(this);
        connect(conn, SIGNAL(commandFinished(int,bool)), this, SLOT(FtpCommandFinished(int,bool)));
        connect(conn, SIGNAL(listInfo(QUrlInfo)), this, SLOT(AddToList(QUrlInfo)));
        
        QUrl url(_ui.url->text());
        if (!url.isValid() || url.scheme().toLower() != QLatin1String("ftp")) {
                conn->connectToHost(_ui.url->text(), 21);
                conn->login();            
        } else {
                conn->connectToHost(url.host(), url.port(21));

                if (!url.userName().isEmpty())
                        conn->login(QUrl::fromPercentEncoding(url.userName().toLatin1()), url.password());
                else
                        conn->login();
                if (!url.path().isEmpty())
                        conn->cd(url.path());
        }
    }
    
    void CFtpSelection::FtpCommandFinished(int, bool error)
    {
                if (conn->currentCommand() == QFtp::ConnectToHost) 
                {
                        if (error) 
                        {
                                QMessageBox::information(this, tr("FTP"),
                                                             tr("Unable to connect to the FTP server "
                                                                "at %1. Please check that the host "
                                                                "name is correct.")
                                                             .arg(_ui.url->text()));
                                return;
                        }

                return;
                }  
                
                if (conn->currentCommand() == QFtp::Login)
                {
                        conn->list();
                }
                
                if (conn->currentCommand() == QFtp::List) 
                {
                        if (isDirectory.isEmpty()) {                               
                                _ui.fileList->addTopLevelItem(new QTreeWidgetItem(QStringList() << tr("<empty>")));
                                _ui.fileList->setEnabled(false);
                        }                    
                }
    }
 
     void CFtpSelection::AddToList(const QUrlInfo &urlInfo)
     {
         QTreeWidgetItem *item = new QTreeWidgetItem;
         item->setText(0, urlInfo.name());
         item->setText(1, QString::number(urlInfo.size()));
         item->setText(2, urlInfo.owner());
         item->setText(3, urlInfo.group());
         item->setText(4, urlInfo.lastModified().toString("MMM dd yyyy"));

         QPixmap pixmap(urlInfo.isDir() ? ":/images/dir.png" : ":/images/file.png");
         item->setIcon(0, pixmap);

         isDirectory[urlInfo.name()] = urlInfo.isDir();
         _ui.fileList->addTopLevelItem(item);
         if (!_ui.fileList->currentItem()) {
             _ui.fileList->setCurrentItem(_ui.fileList->topLevelItem(0));
             _ui.fileList->setEnabled(true);
         }
     }
 
    void CFtpSelection::processItem(QTreeWidgetItem* item, int)
    {
        QString name = item->text(0);
        if (isDirectory.value(name)) 
        {
                _ui.fileList->clear();
                isDirectory.clear();
                currentPath += '/';
                currentPath += name;
                conn->cd(name);
                conn->list();
                //TODO: cursor
                return;
        }         
    }
    
    void CFtpSelection::DoneButtonClicked()
    {
        
    }
    
    void CFtpSelection::CancelButtonClicked()
    {
        
    }
}
