#ifndef NL_NEL_LAUNCHER_DLG_H
#define NL_NEL_LAUNCHER_DLG_H

#include <nel/misc/config_file.h>

#include "ui_nel_launcher_dlg.h"

#include "connection.h"

extern NLMISC::CConfigFile ConfigFile;

class CNelLauncherDlg : public QDialog, private Ui_NelLauncherDlg
{
	Q_OBJECT

public:
	CNelLauncherDlg(QWidget *parent = 0);
	virtual ~CNelLauncherDlg();

	static NLMISC::CConfigFile configFile;
	NLMISC::CFileDisplayer *fileDisplayer;

	bool displayerAdded;
	bool verboseLog;
public slots:
	void clickedSignUp();
	void clickedLogin();
	void clickedConnect();
	void doubleClickedShard(int row, int column);

protected:
	uint32 selectedShardId;
	std::string username;
	std::string password;
	CNelLauncherConnection m_Connection;
};

#endif // NL_NEL_LAUNCHER_DLG_H
