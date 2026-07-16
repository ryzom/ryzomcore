/* 
	backup_service_interface test

	project: RYZOM / TEST
*/

#include "nel/misc/variable.h"
#include "game_share/backup_service_interface.h"
#include "game_share/singleton_registry.h"

using namespace std;
using namespace NLMISC;

class CBackupFileReceiveCallbackTest: public IBackupFileReceiveCallback
{
public:
	virtual void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		std::string s;
		dataStream.serial(s);
		nlinfo("Received file: %s (timestamp:%d, size:%d)  containing string: %s",fileDescription.FileName.c_str(),fileDescription.FileTimeStamp,fileDescription.FileSize,s.c_str());
	}

	~CBackupFileReceiveCallbackTest()
	{
		nlinfo("~CBackupFileReceiveCallbackTest()");
	}
};

class CBackupFileListReceiveCallbackTest: public IBackupFileListReceiveCallback
{
public:
	virtual void callback(const CFileDescriptionContainer& fileList)
	{
		nlinfo("Received file list");
		fileList.display(NLMISC::InfoLog);
	}

	~CBackupFileListReceiveCallbackTest()
	{
		nlinfo("~CBackupFileListReceiveCallbackTest()");
	}
};

extern NLMISC::CVariable<bool> UseBS;

class CBackupServiceInterfaceTest: public IServiceSingleton
{
public:
	void init()
	{

		// *** TODO !!!! ***
		// need to ensure that BS interface internals are OK - the queues should be empty here...
	}
};

static CBackupServiceInterfaceTest Test;

NLMISC_COMMAND(backupTestRequestFile,"","")
{
	bool oldUseBS= UseBS;

	// test CBackupServiceInterface::requestFile()
	{
		UseBS= true;
		nlinfo("----- request file using BS -----");
		Bsi.requestFile("backup_service_interface_test0.bin",new CBackupFileReceiveCallbackTest);

		UseBS= false;
		nlinfo("----- request file no BS -----");
		Bsi.requestFile("backup_service_interface_test1.bin",new CBackupFileReceiveCallbackTest);
	}

	nlinfo("----- END OF REQUEST FILE TEST -----");
	UseBS= oldUseBS;
	return true;
}

NLMISC_COMMAND(backupTestRequestFileList,"","")
{
	bool oldUseBS= UseBS;

	// test CBackupServiceInterface::requestFileList()
	{
		UseBS= true;
		nlinfo("----- request file list using BS => * -----");
		Bsi.requestFileList("data_shard", new CBackupFileListReceiveCallbackTest);
		nlinfo("----- request file list using BS => egs* -----");
		Bsi.requestFileList("data_shard", "egs*", new CBackupFileListReceiveCallbackTest,true);

		std::vector<std::string> vect;
		vect.push_back("ais*");
		vect.push_back("*.rbank");
		nlinfo("----- request file list using BS => ais*,*.rbank -----");
		Bsi.requestFileList("data_shard", vect, new CBackupFileListReceiveCallbackTest,true);

		UseBS= false;
		nlinfo("----- request file list no BS -----");
		Bsi.requestFileList("data_shard", new CBackupFileListReceiveCallbackTest);
	}

	nlinfo("----- END OF REQUEST FILE LIST TEST -----");
	UseBS= oldUseBS;
	return true;
}

NLMISC_COMMAND(backupTestSendFile,"","")
{
	bool oldUseBS= UseBS;

	// test CBackupServiceInterface::sendFile()
	{
		std::string s ="Hello World";

		UseBS= true;
		nlinfo("----- save file using BS => backup_service_interface_test0.bin -----");
		CBackupMsgSaveFile saveFileMsg("backup_service_interface_test0.bin", CBackupMsgSaveFile::SaveFile, Bsi );
		saveFileMsg.DataMsg.serial(s);
		saveFileMsg.DataMsg.serial(s);
		Bsi.sendFile(saveFileMsg);

		UseBS= false;
		nlinfo("----- save file no BS => backup_service_interface_test1.bin -----");
		CBackupMsgSaveFile saveFileMsg2("backup_service_interface_test1.bin", CBackupMsgSaveFile::SaveFile, Bsi );
		saveFileMsg2.DataMsg.serial(s);
		saveFileMsg2.DataMsg.serial(s);
		Bsi.sendFile(saveFileMsg2);
	}

	nlinfo("----- END OF SEND FILE TEST -----");
	UseBS= oldUseBS;
	return true;
}

NLMISC_COMMAND(backupTestOutputStream,"","")
{
	bool oldUseBS= UseBS;

	// test CBackupOutputStream - with BS
	{
		UseBS=true;
		CBackupOutputStream outputStream("backup_service_interface_test2.bin","remote_","local_");
		NLMISC::IStream& stream= outputStream.getStream();
		std::string s="CBackupOutputStream test - with BS";
		stream.serial(s);
	}

	// test CBackupOutputStream	- without BS
	{
		UseBS=false;
		CBackupOutputStream outputStream("backup_service_interface_test3.bin","remote_","local_");
		NLMISC::IStream& stream= outputStream.getStream();
		std::string s="CBackupOutputStream test- without BS";
		stream.serial(s);
	}

	nlinfo("----- END OF OUTPUT STREAM TEST -----");
	UseBS= oldUseBS;
	return true;
}













CBackupServiceInterface	BSInterface;

void	cbBackupAppend(CBackupMsgAppendCallback& append)
{
	uint	i;
	for (i=0; i<append.Appends.size(); ++i)
		nlinfo("Appended '%s' to file '%s'", append.Appends[i].c_str(), append.FileName.c_str());
}

NLMISC_COMMAND(listenTo, "test bs listen to", "filename")
{
	if (args.size() < 1)
		return false;
	
	BSInterface.listenTo(args[0], cbBackupAppend, false);

	return true;
}

NLMISC_COMMAND(endListenTo, "test bs end listen to", "filename")
{
	if (args.size() < 1)
		return false;
	
	BSInterface.endListenTo(args[0]);

	return true;
}

NLMISC_COMMAND(bsAppend, "test bs append", "filename value")
{
	if (args.size() < 2)
		return false;
	
	BSInterface.append(args[0], args[1]);

	return true;
}


