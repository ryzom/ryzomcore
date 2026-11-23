#include "nel_launcher_dlg.h"
#include "connection.h"
#include <cerrno>

#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

#ifndef NL_OS_WINDOWS
#define _chdir chdir
#define _execvp execvp
#else // NL_OS_WINDOWS
#include <direct.h>
#endif // NL_OS_WINDOWS

NLMISC::CConfigFile ConfigFile;

// the page the Sign Up button opens, empty when the config file has none
static std::string signUpUrl()
{
	return ConfigFile.exists("SignUpURL") ? ConfigFile.getVar("SignUpURL").asString() : std::string();
}

CNelLauncherDlg::CNelLauncherDlg(QWidget *parent)
{
	displayerAdded = false;
	verboseLog = false;

	// Set up the Qt UI.
	setupUi(this);

	// Connect singals/slots.
	connect( pbLogin, SIGNAL( clicked() ), this, SLOT( clickedLogin() ) );
	connect( tblShardList, SIGNAL( cellDoubleClicked(int,int) ), this, SLOT( doubleClickedShard(int,int) ) );
	connect( pbConnect, SIGNAL( clicked() ), this, SLOT( clickedConnect() ) );
	connect( pbSignup, SIGNAL( clicked() ), this, SLOT( clickedSignUp() ) );

	// Set up the table.
	QStringList tableLabels;
	tableLabels << "Nb Players" << "Version" << "Status" << "Shard Name";
	tblShardList->setColumnCount(4);
	tblShardList->setHorizontalHeaderLabels(tableLabels);

	// Set up the NeL stuff.
	fileDisplayer = new NLMISC::CFileDisplayer("nel_launcher.log", true);
        NLMISC::createDebug();
        NLMISC::DebugLog->addDisplayer(fileDisplayer);
        NLMISC::InfoLog->addDisplayer(fileDisplayer);
        NLMISC::WarningLog->addDisplayer(fileDisplayer);
        NLMISC::ErrorLog->addDisplayer(fileDisplayer);
        NLMISC::AssertLog->addDisplayer(fileDisplayer);
        displayerAdded = true;

        nlinfo("Loading config file");

        ConfigFile.load("nel_launcher.cfg");

	if(ConfigFile.exists("VerboseLog"))
		verboseLog =  ConfigFile.getVar("VerboseLog").asBool();

	if(verboseLog) nlinfo("Using verbose log mode");

	// without a page to open, the Sign Up button stays disabled
	pbSignup->setEnabled(!signUpUrl().empty());
}

CNelLauncherDlg::~CNelLauncherDlg()
{
	if(displayerAdded)
	{
		NLMISC::createDebug();
		NLMISC::DebugLog->removeDisplayer(fileDisplayer);
		NLMISC::InfoLog->removeDisplayer(fileDisplayer);
		NLMISC::WarningLog->removeDisplayer(fileDisplayer);
		NLMISC::ErrorLog->removeDisplayer(fileDisplayer);
		NLMISC::AssertLog->removeDisplayer(fileDisplayer);
	}
}

void CNelLauncherDlg::clickedSignUp()
{
	std::string url = signUpUrl();
	if(url.empty())
		return;

	// a web page only, never a local file or another program's scheme
	QUrl pageUrl(QString::fromUtf8(url.c_str()));
	if(!pageUrl.isValid() || (pageUrl.scheme().compare("http", Qt::CaseInsensitive) != 0 && pageUrl.scheme().compare("https", Qt::CaseInsensitive) != 0))
	{
		nlwarning("SignUpURL '%s' in nel_launcher.cfg is not an http or https address, nothing is opened", url.c_str());
		QMessageBox::about(this, "Sign Up", QString("SignUpURL in nel_launcher.cfg is not an http or https address, nothing is opened: %1").arg(QString::fromUtf8(url.c_str())));
		return;
	}

	nlinfo("opening the sign up page %s", url.c_str());
	if(!QDesktopServices::openUrl(pageUrl))
	{
		nlwarning("Can't open the sign up page '%s'", url.c_str());
		QMessageBox::about(this, "Sign Up", "Can't open the sign up page " + QString::fromUtf8(url.c_str()));
	}
}

void CNelLauncherDlg::clickedLogin()
{
	std::string username = leUsername->text().toStdString();
	std::string password = lePassword->text().toStdString();

	nlinfo("received login attempt for %s with %s", username.c_str(), password.c_str());

	// Set Registry Key or home settings.
	// TODO

	// Disable buttons while logging in.
	pbLogin->setEnabled(false);
	pbSignup->setEnabled(false);

	// Check the login and password.
	//string res = checkLogin(l, p, ConfigFile.getVar("Application").asString(0));
	std::string res = m_Connection.checkLogin(username, password, ConfigFile.getVar("Application").asString(0));
	if(res.empty()) // successful login
	{
		TShardList shards = m_Connection.getShards();
		for(uint idx=0 ; idx<shards.size() ; idx++)
		{
			nlinfo("row count %d" , tblShardList->rowCount());
			uint row = idx;
			tblShardList->insertRow(row);

			std::string strNbPlr = NLMISC::toString(shards[idx].NbPlayers).c_str();
			QTableWidgetItem *nbPlayers = new QTableWidgetItem(QString(strNbPlr.c_str()));
			nbPlayers->setFlags(Qt::ItemIsEnabled);
			tblShardList->setItem(row,0,nbPlayers);

			QTableWidgetItem *version = new QTableWidgetItem(QString(shards[idx].Version.c_str()));
			version->setFlags(Qt::ItemIsEnabled);
			tblShardList->setItem(row,1,version);

			std::string strStatus = shards[idx].Online?"Online":"Offline";
			QTableWidgetItem *status = new QTableWidgetItem(QString(strStatus.c_str()));
			status->setFlags(Qt::ItemIsEnabled);
			tblShardList->setItem(row,2,status);

			QTableWidgetItem *shardName = new QTableWidgetItem(QString(shards[idx].Name.c_str()));
			shardName->setFlags(Qt::ItemIsEnabled);
			tblShardList->setItem(row,3,shardName);
			nlinfo("inserting row %d into table widget. %d" , row, tblShardList->rowCount());
		}
		pbConnect->setEnabled(true);
	}
	else
	{
		QMessageBox::about(this, "Failed Login", "Failed to log in to Login Service: " + QString(res.c_str()));
		// Enable buttons if logging in fails.
		pbLogin->setEnabled(true);
		pbSignup->setEnabled(!signUpUrl().empty());
		pbConnect->setEnabled(false);
	}
}

void CNelLauncherDlg::doubleClickedShard(int row, int column)
{
	nlinfo("a shard was double clicked");

	// Execute the clickedConnect slot. It has all this logic already.
	clickedConnect();
}

void CNelLauncherDlg::clickedConnect()
{
	pbConnect->setEnabled(false);
	TShardList shards = m_Connection.getShards();
	nlinfo("a shard was double clicked. row selected: %d", tblShardList->currentRow());
	if(tblShardList->currentRow() < 0 || tblShardList->currentRow() >= (int)shards.size())
	{
		QMessageBox::about(this, "Connect to Shard", "Please, select a shard and then press Connect button.");
		pbConnect->setEnabled(true);
		return;
	}

	pbConnect->setEnabled(false);

	CShard shard = shards[tblShardList->currentRow()];

	if(!shard.Online)
	{
		QMessageBox::about(this, "Connect to Shard", "You can't connect to an offline shard (error code 15)");
		pbConnect->setEnabled(true);
		return;
	}

	// TODO implement the patching stuff.
	//if(!shard.Version.empty() && shard.Version != getVersion())

	std::string cookie, addr;
	std::string res = m_Connection.selectShard(shard.ShardId, cookie, addr);

	if(res.empty())
	{
		nlinfo("successfully connected to shard, launch client.");
		std::string rapp = ConfigFile.getVar("Application").asString(1);
		std::string dir = ConfigFile.getVar("Application").asString(2);

		std::vector<std::string> vargs;
		//const char *args[50];
		vargs.push_back(rapp);
		vargs.push_back(cookie);
		vargs.push_back(addr);

		// Create the ArgV from a vector.
		uint nArgs = vargs.size();
		char **buf = new char*[nArgs + 1];
		for(uint i=0; i<nArgs; ++i)
		{
			buf[i] = new char[vargs[i].size() + 1];
			strcpy(buf[i], vargs[i].c_str());
			//strcat(buf[i], '\0');
		}
		buf[nArgs]=nullptr;

		if(!dir.empty())
			_chdir(dir.c_str());

		// _execvp only returns when it could not start the game
		_execvp(rapp.c_str(), buf);
		int execError = errno;

		for(uint i=0; i<nArgs; ++i)
			delete [] buf[i];
		delete [] buf;

		nlwarning("Can't execute the game '%s' in '%s': %s", rapp.c_str(), dir.c_str(), strerror(execError));
		QMessageBox::about(this, "Launch Client", QString("Can't execute the game '%1' (error code 17): %2").arg(QString::fromUtf8(rapp.c_str()), QString::fromLocal8Bit(strerror(execError))));
		pbConnect->setEnabled(true);
	}
	else
	{
		QMessageBox::about(this, "Connect to Shard", res.c_str());
		pbConnect->setEnabled(true);
	}
}
