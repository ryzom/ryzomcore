// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/vector.h"
#include "nel/misc/algo.h"


using namespace std;
using namespace NLMISC;


#define myinfo NLMISC::createDebug (), NLMISC::InfoLog->setPosition( __LINE__, __FILE__ ), NLMISC::InfoLog->displayRawNL


// ***************************************************************************
void	filterRyzomBug(const char *dirSrc, const char *dirDst, uint patchVersionWanted, const string specialFilter)
{
	if(!CFile::isDirectory(dirDst))
	{
		if(!CFile::createDirectory(dirDst))
		{
			myinfo("%s is not a directory and cannot create it", dirDst);
			return;
		}
	}

	vector<string>	fileList;
	CPath::getPathContent(dirSrc, false, false, true, fileList, NULL, true);

	for(uint i=0;i<fileList.size();i++)
	{
		const string &fileFullPath= fileList[i];
		CIFile	f;
		if(f.open(fileFullPath, true))
		{
			// Parse all "UserId: ", this get the number of crash in this file
			const	string	userIdTok= "UserId: ";
			const	string	patchVersionTok= "PatchVersion: ";
			uint	numUserId= 0;
			uint	numPatchVersion= 0;
			bool	precUserId= false;
			bool	ok= true;
			bool	filterFound= false;			
			while(!f.eof())
			{
				char	tmp[1000];
				f.getline(tmp, 1000);
				string	str= tmp;
				if(str.compare(0, userIdTok.size(), userIdTok)==0)
				{
					numUserId++;
					if(precUserId)
					{
						nlwarning("Don't find a PatchVersion for all UserId in %s", fileFullPath.c_str());
						ok= false; 
						break;
					}
					else
						precUserId= true;
				}
				if(str.compare(0, patchVersionTok.size(), patchVersionTok)==0)
				{
					numPatchVersion++;
					if(!precUserId)
					{
						nlwarning("Don't find a PatchVersion for all UserId in %s", fileFullPath.c_str());
						ok= false; 
						break;
					}
					else
						precUserId= false;

					// parse the version number
					sint	version;
					sscanf(tmp, "PatchVersion: %d", &version);
					if(version!=(sint)patchVersionWanted)
					{
						nlwarning("The Log %s contains a PatchVersion different: %d", fileFullPath.c_str(), version);
						ok= false;
						break;
					}
				}
				if(!specialFilter.empty() && str.compare(0, specialFilter.size(), specialFilter)==0)
					filterFound= true;
			}
			f.close();
			if(ok && numUserId!=numPatchVersion)
			{
				nlwarning("Don't find a PatchVersion for all UserId in %s", fileFullPath.c_str());
				ok= false;
			}
			// if specialFitler defined, but not found in the file, abort
			if(ok && !specialFilter.empty() && !filterFound)
				ok =false;

			// if ok, copy the file and the associated .dmp dir
			if(ok)
			{
				//myinfo("Copy  %s", fileFullPath.c_str());

				// get the log size
				uint	size= CFile::getFileSize(fileFullPath);

				// copy the log
				string	fileNoDir= CFile::getFilename(fileFullPath);
				string	fileNoExt= CFile::getFilenameWithoutExtension(fileFullPath);
				string	dirDest= dirDst;
				dirDest+= "/" + toString("%05d_", size/1024) + fileNoExt;
				string	dmpDirSrc= string(dirSrc) + "/" + fileNoExt;

				CFile::createDirectory(dirDest);
				
				// copy near the dmp
				CFile::copyFile(dirDest + "/" + fileNoDir, fileFullPath);


				// copy all the .dmp in a new dir
				static vector<string>	dmpList;
				dmpList.clear();
				CPath::getPathContent(dmpDirSrc, false, false, true, dmpList, NULL);
				for(uint j=0;j<dmpList.size();j++)
				{
					string	dmpNoDir= CFile::getFilename(dmpList[j]);
					CFile::copyFile(dirDest+ "/" + dmpNoDir, dmpList[j]);
				}
			}
		}
		else
			nlwarning("cannot open %s", fileFullPath.c_str());
	}
}


// ***************************************************************************
struct CStatVal
{
	uint	Val;

	CStatVal()
	{
		Val = 0;
	}
};

typedef	map<sint, CStatVal>			TStatMap;
typedef	map<string, CStatVal>		TStatStrMap;
typedef	map<string, TStatStrMap>	TStatStrStrMap;

class CCrashCont
{
public:
	string	Name;
	sint	X0,Y0,X1,Y1;
	uint	NumCrash;
	CCrashCont(const std::string &name, sint x0, sint y0, sint x1, sint y1)
	{
		Name= name;
		X0= min(x0,x1);
		Y0= min(y0,y1);
		X1= max(x0,x1);
		Y1= max(y0,y1);
		NumCrash= 0;
	}

	bool	testPos(const CVector &pos)
	{
		if(pos.x>=X0 && pos.x<=X1 && pos.y>=Y0 && pos.y<=Y1)
		{
			NumCrash++;
			return true;
		}
		return false;
	}
};

void	statRyzomBug(const char *dirSrc)
{
	vector<string>	fileList;
	CPath::getPathContent(dirSrc, false, false, true, fileList, NULL, true);

	// delete the log.log
	CFile::deleteFile(getLogDirectory() + "log.log");

	TStatStrMap				senderMap;
	TStatMap				shardMap;
	TStatMap				timeInGameMap;
	TStatMap				patchVersionMap;
	TStatMap				info3dMap;
	std::vector<CVector>	userPosArray;
	TStatStrStrMap			senderToCrashFileMap;
	TStatStrStrMap			crashFileToSenderMap;
	uint					totalCrash= 0;
	uint					totalCrashDuplicate= 0;
	

	// **** parse all files
	for(uint i=0;i<fileList.size();i++)
	{
		const string &fileFullPath= fileList[i];
		string	fileNoDir= CFile::getFilename(fileFullPath);
		// skip not .log files
		if(!testWildCard(fileFullPath, "*.log"))
			continue;
		// parse
		CIFile	f;
		if(f.open(fileFullPath, true))
		{
			const	string	senderIdTok= "Sender: ";
			const	string	shardIdTok= "ShardId: ";
			const	string	userPosTok= "UserPosition: ";
			const	string	timeInGameIdTok= "Time in game: ";
			const	string	patchVersionIdTok= "PatchVersion: ";
			const	string	localTimeIdTok= "LocalTime: ";
			const	string	nel3dIdTok= "NeL3D: ";
			const	string	card3dIdTok= "3DCard: ";
			
			string	precSenderId;
			string	precSenderId2;
			sint	precShardId= -1;
			sint	precTimeInGame= -1;	// 0 means "never in game", 1 means < 10 min, 2 means more
			sint	precNel3DMode= -1;	// 0 OpenGL, 1 D3D, 2 ????
			sint	precCard3D= -1;		// 0 NVidia, 1 ATI, 2 ????
			sint	precPatchVersion= -1;
			sint64	precLocalTime= -1;	// local time in second
			sint64	precLocalTime2= -1;	// local time in second
			CVector	precUserPos;
			while(!f.eof())
			{
				char	tmp[1000];
				f.getline(tmp, 1000);
				string	str= tmp;
				if(str.compare(0, senderIdTok.size(), senderIdTok)==0)
				{
					precSenderId2= precSenderId;
					precSenderId= str.c_str()+senderIdTok.size();
				}
				else if(str.compare(0, shardIdTok.size(), shardIdTok)==0)
				{
					precShardId= atoi(str.c_str()+shardIdTok.size());
				}
				else if(str.compare(0, userPosTok.size(), userPosTok)==0)
				{
					string	posStr= str.substr(userPosTok.size());
					sscanf(posStr.c_str(), "%f%f%f", &precUserPos.x, &precUserPos.y, &precUserPos.z);
				}
				else if(str.compare(0, timeInGameIdTok.size(), timeInGameIdTok)==0)
				{
					string timeStr= str.substr(timeInGameIdTok.size(), string::npos);
					if(timeStr=="0h 0min 0sec" || timeStr.size()<12)
						precTimeInGame= 0;
					else
					{
						if(timeStr[1]=='h' && timeStr[4]=='m' && timeStr[3]<='5')
							precTimeInGame= 1;
						else
							precTimeInGame= 2;
					}
				}
				else if(str.compare(0, patchVersionIdTok.size(), patchVersionIdTok)==0)
				{
					precPatchVersion= atoi(str.c_str()+patchVersionIdTok.size());
				}
				else if(str.compare(0, localTimeIdTok.size(), localTimeIdTok)==0)
				{
					precLocalTime2= precLocalTime;
					// 2004/09/17 04:21:16
					string timeStr= str.substr(localTimeIdTok.size(), string::npos);
					if(timeStr.size()<19)
						precLocalTime= 0;
					else
					{
						sint64	year= atoi(timeStr.substr(0,4).c_str());
						sint64	month= atoi(timeStr.substr(5,2).c_str());
						sint64	day= atoi(timeStr.substr(8,2).c_str());
						sint64	hour= atoi(timeStr.substr(11,2).c_str());
						sint64	minute= atoi(timeStr.substr(14,2).c_str());
						sint64	sec= atoi(timeStr.substr(17,2).c_str());
						year= max(year, (sint64)2004);
						year-=2004;
						precLocalTime= ((year*366+month)*12)+day;
						precLocalTime= (((precLocalTime*24)+hour)*60+minute)*60+sec;
					}
				}
				else if(str.compare(0, nel3dIdTok.size(), nel3dIdTok)==0)
				{
					string	tmp= toLower(str);
					if(tmp.find("opengl")!=string::npos)
						precNel3DMode= 0;
					else if(tmp.find("direct3d")!=string::npos)
						precNel3DMode= 1;
					else
						precNel3DMode= 2;
				}
				else if(str.compare(0, card3dIdTok.size(), card3dIdTok)==0)
				{
					string	tmp= str;
					if(tmp.find("NVIDIA")!=string::npos)
						precCard3D= 0;
					else if(tmp.find("RADEON")!=string::npos)
						precCard3D= 1;
					else
						precCard3D= 2;

					// END a block, add info in map (only if not repetition)
					sint64	absTime= precLocalTime-precLocalTime2;
					if(absTime<0)	absTime= -absTime;
					if( precSenderId!=precSenderId2 ||
						absTime>sint64(60) )
					{
						senderMap[precSenderId].Val++;
						shardMap[precShardId].Val++;
						timeInGameMap[precTimeInGame].Val++;
						patchVersionMap[precPatchVersion].Val++;
						if(precNel3DMode!=0 && precNel3DMode!=1)
							precNel3DMode= 2;
						if(precCard3D!=0 && precCard3D!=1)
							precCard3D= 2;
						info3dMap[precNel3DMode*256+precCard3D].Val++;
						userPosArray.push_back(precUserPos);
						crashFileToSenderMap[fileNoDir][precSenderId].Val++;
						senderToCrashFileMap[precSenderId][fileNoDir].Val++;
						totalCrash++;
					}
					totalCrashDuplicate++;
				}
			}
		}
	}

	// **** display Stats
	// general stats
	myinfo("**** Total: %d Crashs (%d with duplicates)", totalCrash, totalCrashDuplicate);
	myinfo("NB: 'duplicates' means: crashs that are removed because suppose the player click 'ignore' (same sender/same Localtime, within about 1 min)");
	
	// senderId
	TStatMap::iterator	it;
	TStatStrMap::iterator	itStr;
	myinfo("");
	myinfo("**** Stat Per Sender:");
	multimap<uint, string>	resortSender;
	for(itStr=senderMap.begin();itStr!=senderMap.end();itStr++)
	{
		resortSender.insert(make_pair(itStr->second.Val, itStr->first));
	}
	multimap<uint, string>::iterator it2;
	for(it2=resortSender.begin();it2!=resortSender.end();it2++)
	{
		myinfo("**** %d Crashs for UserId %s", it2->first, it2->second.c_str());
	}
	// shardId
	myinfo("");
	myinfo("**** Stat Per ShardId:");
	for(it=shardMap.begin();it!=shardMap.end();it++)
	{
		myinfo("**** %d Crashs for ShardId %d", it->second.Val, it->first);
	}
	// timeInGame
	myinfo("");
	myinfo("**** Stat Per TimeInGame:");
	for(it=timeInGameMap.begin();it!=timeInGameMap.end();it++)
	{
		myinfo("**** %d Crashs for TimeInGame %s", it->second.Val, it->first==0?"0h 0min 0sec":
				(it->first==1?"<=5 min":
				 it->first==2?"> 5 min":"??? Bad parse ???"));
	}
	// infoPatch
	myinfo("");
	myinfo("**** Stat Per PatchVersion:");
	for(it=patchVersionMap.begin();it!=patchVersionMap.end();it++)
	{
		myinfo("**** %d Crashs for PatchVersion %d", it->second.Val, it->first);
	}
	// info3d
	myinfo("");
	myinfo("**** Stat Per 3d Mode:");
	for(it=info3dMap.begin();it!=info3dMap.end();it++)
	{
		uint	card3d= it->first&255;
		uint	mode3d= it->first>>8;
		
		myinfo("**** %d Crashs for %s / Card %s", it->second.Val, 
				mode3d==0?"OpenGL":(mode3d==1?"Direct3D":"??? No Driver ???"),
				card3d==0?"NVIDIA":(card3d==1?"RADEON":"Misc"));
	}
	// crash by continent
	{
		// init cont info
		CCrashCont	crashCont[]=
		{
			// New First, because bbox may be included in Main bbox
			CCrashCont("Matis Newb", 0,-5000,2800,-8500),
			CCrashCont("Zorai Newb", 6500,-4000,9300,-6000),
			CCrashCont("Trykr Newb", 20000,-32000,24000,-36000),
			CCrashCont("Fyros Newb", 20500,-24500,24000,-27500),
			CCrashCont("Matis Main", 0,0,6500,-8500),
			CCrashCont("Zorai Main", 6500,0,13000,-6000),
			CCrashCont("Trykr Main ", 13000,-29000,20000,-36000),
			CCrashCont("Fyros Main ", 15000,-23000,20500,-27500)
		};
		uint	numCont= sizeof(crashCont)/sizeof(crashCont[0]);
		uint	numNotFound= 0;
		// count stats
		uint	i;
		for(i=0;i<userPosArray.size();i++)
		{
			bool	ok= false;
			for(uint j=0;j<numCont;j++)
			{
				if(crashCont[j].testPos(userPosArray[i]))
				{
					ok= true;
					break;
				}
			}
			if(!ok)
				numNotFound++;
		}
		myinfo("");
		myinfo("**** Stat Per continent:");
		// display stats
		for(i=0;i<numCont;i++)
		{
			myinfo("   %s: %d", crashCont[i].Name.c_str(), crashCont[i].NumCrash);
		}
		myinfo("   NotFound: %d", numNotFound);
	}


	// **** display detailed Stats
	myinfo("");
	myinfo("");
	myinfo("**************************");
	myinfo("**************************");
	myinfo("********* DETAIL *********");
	myinfo("**************************");
	myinfo("**************************");
	myinfo("");
	
	// Stats per User
	myinfo("");
	myinfo("**** Detailed Crashs per user:");
	for(it2=resortSender.begin();it2!=resortSender.end();it2++)
	{
		string		userId= it2->second;
		TStatStrMap	&crashFileMap= senderToCrashFileMap[userId];
		if(crashFileMap.empty())
		{
			myinfo("    Error parsing Crashs for UserId %s (?????)", userId.c_str());
		}
		else
		{
			myinfo("    %d Crashs for %s:", it2->first, userId.c_str());
			for(TStatStrMap::iterator it=crashFileMap.begin();it!=crashFileMap.end();it++)
			{
				myinfo("        %d in %s", it->second.Val, it->first.c_str());
			}
		}
	}
	
	// Stats per Crash File
	myinfo("");
	myinfo("**** Detailed Crashs per crash Log:");
	multimap<uint, string>	resortCrashLog;
	for(TStatStrStrMap::iterator	itStrStr=crashFileToSenderMap.begin();itStrStr!=crashFileToSenderMap.end();itStrStr++)
	{
		// count total crash instance
		uint	numCrash= 0;
		TStatStrMap	&userIdMap= itStrStr->second;
		for(TStatStrMap::iterator it=userIdMap.begin();it!=userIdMap.end();it++)
			numCrash+= it->second.Val;
		// insert for resort by this number
		resortCrashLog.insert(make_pair(numCrash, itStrStr->first));
	}
	for(it2=resortCrashLog.begin();it2!=resortCrashLog.end();it2++)
	{
		string		crashLog= it2->second;
		TStatStrMap	&userIdMap= crashFileToSenderMap[crashLog];
		if(userIdMap.empty())
		{
			myinfo("    Error parsing Crashs for CrashFile %s (?????)", crashLog.c_str());
		}
		else
		{
			myinfo("    %d Crashs in  %s:", it2->first, crashLog.c_str());
			for(TStatStrMap::iterator it=userIdMap.begin();it!=userIdMap.end();it++)
			{
				myinfo("        %d for %s", it->second.Val, it->first.c_str());
			}
		}
	}
	
	// RAW userPos
	myinfo("");
	myinfo("**** RAW Crashs Pos (copy in excel, and use insert/chart/(X/Y)Scatter):");
	for(uint i=0;i<userPosArray.size();i++)
	{
		myinfo("%.2f\t%.2f\t%.2f", userPosArray[i].x, userPosArray[i].y, userPosArray[i].z);
	}
}

// ***************************************************************************
int	main(int argc, char *argv[])
{
	bool	ok= false;
	bool	statMode= false;
	bool	filterMode= false;
	if(argc == 3 && argv[2]==string("-s"))
		ok=	true,statMode= true;
	if(argc >= 5 && argv[2]==string("-p"))
		ok=	true,filterMode= true;
	
	if(!ok)
	{
		myinfo("Usage1 (stats):\n\t%s src_dir -s\n", CFile::getFilename(argv[0]).c_str());
		myinfo("Usage2 (patch filter):\n\t%s src_dir -p patch_version dst_dir [specialFilter]\n", CFile::getFilename(argv[0]).c_str());
	}
	else
	{
		if(!CFile::isDirectory(argv[1]))
		{
			myinfo("%s is not a directory", argv[1]);
			return 1;
		}
		
		if(filterMode)
		{
			string	specialFilter;
			if(argc>=6)
				specialFilter= argv[5];
			filterRyzomBug(argv[1], argv[4], atoi(argv[3]), specialFilter);
		}
		else if(statMode)
		{
			statRyzomBug(argv[1]);
		}
	}

	return 0;
}

