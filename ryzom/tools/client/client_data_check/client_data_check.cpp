// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// client_data_check.cpp : Defines the class behaviors for the application.
//

#include "nel/misc/time_nl.h"
#include "nel/misc/ucstring.h"
#include "nel/misc/algo.h"
#include "data_scan.h"
#include "nel/misc/debug.h"
#include "nel/misc/big_file.h"
#include "nel/misc/file.h"
#include "3d/shape.h"
#include "3d/mesh_mrm.h"
#include "3d/mesh_mrm_skinned.h"
#include "3d/register_3d.h"

#include "stdafx.h"
#include "client_data_check.h"
#include "client_data_checkDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma optimize("",off)

/////////////////////////////////////////////////////////////////////////////
// CClient_data_checkApp

BEGIN_MESSAGE_MAP(CClient_data_checkApp, CWinApp)
	//{{AFX_MSG_MAP(CClient_data_checkApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClient_data_checkApp construction

CClient_data_checkApp::CClient_data_checkApp()
{
	_CheckDlg= NULL;
	_StateProcess= Init;
	_BnpIndex= 0;
	_FileIndex= 0;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CClient_data_checkApp object

CClient_data_checkApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CClient_data_checkApp initialization

BOOL CClient_data_checkApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// init path
	NLMISC::CPath::addSearchPath("data");
	NLMISC::CPath::addSearchPath("unpack");
	NL3D::registerSerial3d();

	// init debug
	NLMISC::createDebug();
	NLMISC::CFileDisplayer	*DataCheckLogDisplayer= new NLMISC::CFileDisplayer("data_check.log", true, "DATA_CHECK.LOG");
	NLMISC::DebugLog->addDisplayer (DataCheckLogDisplayer);
	NLMISC::InfoLog->addDisplayer (DataCheckLogDisplayer);
	NLMISC::WarningLog->addDisplayer (DataCheckLogDisplayer);
	NLMISC::ErrorLog->addDisplayer (DataCheckLogDisplayer);
	NLMISC::AssertLog->addDisplayer (DataCheckLogDisplayer);
	

	// init dlg
	_CheckDlg= new CClient_data_checkDlg(this);
	_CheckDlg->Create(CClient_data_checkDlg::IDD);
	m_pMainWnd = _CheckDlg;
	
	/*int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}*/

	return TRUE;
}

// ***************************************************************************
void CClient_data_checkApp::setLog()
{
	if(_CheckDlg)
	{
		_CheckDlg->ListBox.ResetContent();
		std::string	tam= _LastDataScanLog.toString();
		std::vector<std::string>	listStr;
		NLMISC::splitString(tam, "\n", listStr);
		for(uint i=0;i<listStr.size();i++)
			_CheckDlg->ListBox.AddString(listStr[i].c_str());
		_CheckDlg->UpdateData(FALSE);
	}
}

// ***************************************************************************
void CClient_data_checkApp::addLog(const std::string &logAdd)
{
	_LastDataScanLog+= std::string("\n") + logAdd;
	setLog();
}

// ***************************************************************************
void CClient_data_checkApp::setStatus(const std::string &status)
{
	if(_CheckDlg)
	{
		_CheckDlg->TextStatus= status.c_str();
		_CheckDlg->UpdateData(FALSE);
	}
}


// ***************************************************************************
void CClient_data_checkApp::checkMeshMrm(const std::string &fileName, NL3D::CMeshMRM *meshMrm)
{
	uint	numLod= meshMrm->getNbLod();
	uint	numVerts= meshMrm->getMeshGeom().getVertexBuffer().getNumVertices();
	if(numLod==0)
	{
		addLog(std::string("Warning: The File: ") + fileName + "has 0 Lods");
	}
	else
	{
		const NL3D::CMeshMRMGeom& meshGeom= meshMrm->getMeshGeom();
		// Scan all Geomoprhs
		for(uint i=0;i<numLod;i++)
		{
			const std::vector<NL3D::CMRMWedgeGeom>	&geoms= meshGeom.getGeomorphs(i);
			for(uint j=0;j<geoms.size();j++)
			{
				if(geoms[j].Start>=numVerts || geoms[j].End>=numVerts)
				{
					addLog(NLMISC::toString("ERROR: The Mrm File: %s has a bad Geomorph. Lod: %d. Vert: %d. Start/End: %d/%d. Max:%d",
						fileName.c_str(), i, j, geoms[j].Start, geoms[j].End, numVerts));
				}
			}
		}
	}
}

// ***************************************************************************
void CClient_data_checkApp::checkMeshMrmSkinned(const std::string &fileName, NL3D::CMeshMRMSkinned	*meshMrm)
{
	uint	numLod= meshMrm->getNbLod();
	NL3D::CVertexBuffer	vb;
	meshMrm->getMeshGeom().getVertexBuffer(vb);
	uint	numVerts= vb.getNumVertices();
	if(numLod==0)
	{
		addLog(std::string("Warning: The File: ") + fileName + "has 0 Lods");
	}
	else
	{
		const NL3D::CMeshMRMSkinnedGeom& meshGeom= meshMrm->getMeshGeom();
		// Scan all Geomoprhs
		for(uint i=0;i<numLod;i++)
		{
			const std::vector<NL3D::CMRMWedgeGeom>	&geoms= meshGeom.getGeomorphs(i);
			for(uint j=0;j<geoms.size();j++)
			{
				if(geoms[j].Start>=numVerts || geoms[j].End>=numVerts)
				{
					addLog(NLMISC::toString("ERROR: The MrmSkinned File: %s has a bad Geomorph. Lod: %d. Vert: %d. Start/End: %d/%d. Max:%d",
						fileName.c_str(), i, j, geoms[j].Start, geoms[j].End, numVerts));
				}
			}
		}
	}
}

// ***************************************************************************
bool CClient_data_checkApp::processBnpMRM()
{
	// init bnp list?
	if(_BnpList.empty())
	{
		NLMISC::CPath::getPathContent("./data/", false, false, true, _BnpList);
		if(_BnpList.empty())
		{
			std::string	tam= "Don't find any BNP files for MRM Check";
			_LastDataScanLog+= tam;
			nlinfo(tam.c_str());
		}
		else
		{
			// add path into BNP
			NLMISC::CPath::addSearchPath("data", true, false);
		}
		_BnpIndex= 0;
		_FileIndex= 0;
	}
	
	// if some BNP to look through
	if(_BnpIndex<_BnpList.size())
	{
		std::string	bnpName= NLMISC::CFile::getFilename(_BnpList[_BnpIndex]);

		// get the list of file in this bnp
		if(_CurrentBnpFileList.empty())
		{
			// List all files From the bnp
			if(NLMISC::CFile::getExtension(bnpName)=="bnp" || NLMISC::CFile::getExtension(bnpName)==".bnp")
				NLMISC::CBigFile::getInstance().list(bnpName, _CurrentBnpFileList);
			_FileIndex= 0;
		}

		// if some file to look throuh
		if(_FileIndex<_CurrentBnpFileList.size())
		{
			std::string	fileName= NLMISC::CFile::getFilename(_CurrentBnpFileList[_FileIndex]);
			std::string	status= NLMISC::toString("Checking File: BNP(%d/%d) - File(%d/%d)", 
				_BnpIndex, _BnpList.size(), _FileIndex, _CurrentBnpFileList.size());
			if(NLMISC::CFile::getExtension(fileName)=="shape" || NLMISC::CFile::getExtension(fileName)==".shape")
			{
				// log
				setStatus(status + "\nChecking Mesh File: " + fileName);

				// try to load as mrm
				std::string	pathName= NLMISC::CPath::lookup(fileName);
				NLMISC::CIFile		file;
				if(!pathName.empty() && file.open(pathName))
				{
					NL3D::CShapeStream	ss;
					ss.serial(file);
					NL3D::CMeshMRM			*meshMrm= dynamic_cast<NL3D::CMeshMRM*>(ss.getShapePointer());
					NL3D::CMeshMRMSkinned	*meshMrmSkinned= dynamic_cast<NL3D::CMeshMRMSkinned*>(ss.getShapePointer());
					if(meshMrm)
						checkMeshMrm(fileName, meshMrm);
					if(meshMrmSkinned)
						checkMeshMrmSkinned(fileName, meshMrmSkinned);
				}
			}
			else
			{
				// log
				setStatus(status);
			}
			// next file in this bnp
			_FileIndex++;
		}
		else
		{
			// else next one
			_BnpIndex++;
			_CurrentBnpFileList.clear();
		}
		

		return false;
	}
	// else ended
	else
		return true;
}


// ***************************************************************************
BOOL CClient_data_checkApp::OnIdle(LONG lCount) 
{
	// Init 
	if(_StateProcess==Init)
	{
		CPatchManager::getInstance()->init();
		CPatchManager::getInstance()->startScanDataThread();
		_StateProcess=CheckSum;
	}

	// CheckSum pass?
	if(_StateProcess==CheckSum || _StateProcess==CancelCheckSum)
	{
		CPatchManager	*pPM= CPatchManager::getInstance();

		// update messages
		if(_StateProcess==CheckSum)
		{
			// get state
			ucstring				state;
			std::vector<ucstring>	stateLog;
			if(pPM->getThreadState(state, stateLog))
			{
				setStatus(state.toString());
			}

			// get Log
			ucstring	dsLog;
			if(pPM->getDataScanLog(dsLog))
			{
				_LastDataScanLog= dsLog;
				setLog();
			}
		}

		// if ended
		bool	res;
		if(pPM->isScanDataThreadEnded(res))
		{
			// the log may have changed
			ucstring	dsLog;
			if(pPM->getDataScanLog(dsLog))
			{
				_LastDataScanLog= dsLog;
				setLog();
			}
			
			// Next
			_StateProcess= MRMCheck;
		}
	}

	// MRM Pass?
	if(_StateProcess==MRMCheck )
	{
		if(processBnpMRM())
			_StateProcess= NearEnd;
	}

	// Quit
	if(_StateProcess==NearEnd )
	{
		// Report result
		if(_LastDataScanLog.empty())
		{
			setStatus("Data Scan reported no corrupted Files.\nPress 'Close'");
			nlinfo("Data Scan reported no corrupted Files.");
		}
		else
		{
			setStatus("Data Scan reported some corrupted Files.\nPlease send us the file 'data_check.log'.\nPress 'Close'");
			nlinfo("");
			nlinfo("");
			nlinfo("*******************************************");
			nlinfo("*******************************************");
			nlinfo("Data Scan reported some corrupted Files.");
			nlinfo("*******************************************");
			nlinfo("*******************************************");
			nlinfo("");
			nlinfo("");
			
			// Log
			std::string	tam= _LastDataScanLog.toString();
			std::vector<std::string>	listStr;
			NLMISC::splitString(tam, "\n", listStr);
			for(uint i=0;i<listStr.size();i++)
				nlinfo("%s", listStr[i].c_str());
		}
		
		// Change Text
		if(_CheckDlg)
		{
			_CheckDlg->CancelButton.SetWindowText("Close");
		}

		// End
		_StateProcess= End;
	}

	return TRUE;
	//return CWinApp::OnIdle(lCount);
}

// ***************************************************************************
void	CClient_data_checkApp::cancel() 
{
	if(_StateProcess==CheckSum)
	{
		_StateProcess= CancelCheckSum;
		CPatchManager::getInstance()->askForStopScanDataThread();

		if(_CheckDlg)
		{
			_CheckDlg->TextStatus= "Canceling Data Scan. Please wait...";
			_CheckDlg->UpdateData(FALSE);
		}
	}

	if(_StateProcess==MRMCheck)
	{
		_StateProcess= NearEnd;
	}
		
	if(_StateProcess==End)
	{
		_CheckDlg->DestroyWindow();
		delete	_CheckDlg;
		_CheckDlg= NULL;

		// quit
		exit(0);
	}
}

