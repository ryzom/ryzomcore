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



#include "stdpch.h"

#include "interface_config.h"
#include "interface_manager.h"
#include "nel/gui/group_container.h"
#include "nel/gui/ctrl_scroll.h"

using namespace NLMISC;
using namespace std;


// ***************************************************************************
/*
	Version 2:
		- save of interface element specific infos
	Version 1:
		- save of the top window between modes.
	Version 0:
		- first version (well version not added but see loadconfig() hack).
*/
#define	INTERFACE_CONFIG_STREAM_VERSION	2


// ***************************************************************************
// SCont Data from container to be saved
// ***************************************************************************

// ***************************************************************************
void CInterfaceConfig::SCont::serial(NLMISC::IStream &f)
{
	// version 10 : added minW & maxW
	// version 9 : Backuped position & touchFlag
	// version 8 : ContainerMode
	// version 7 : RolloverAlphaContainer and RolloverAlphaContent separated
	// version 6 : added 'pop_max_h' value
	// version 5 : added 'active_savable' flag
	// version 4 : added 'movable' flag
	// version 3 : added 'locked' flag
	// version 2 : added 'useGlobalAlpha' flag
	// version 1 : added alpha's & popup coords & size
	// version 0 : base version
	sint ver = f.serialVersion(10);
	if (ver >= 10)
	{
		f.serial(MinW);
		f.serial(MaxW);
	}
	if (ver >= 8)
	{
		if (f.isReading())
		{
			f.serial(Id);
			Id = "ui:interface:"+Id;
		}
		else
		{
			std::string shortId;
			std::string startString;
			if (Id.size() >= 13)
			{
				startString = Id.substr(0, 13);
			}
			if (startString == "ui:interface:")
			{
				shortId = Id.substr(13,Id.size());
			}
			else
			{
				shortId = Id;
			}
			f.serial(shortId);
		}
		f.serial(ContainerMode);
		if (ContainerMode == 0)
		{
			f.serial(Popuped);
			f.serial(Opened);
			f.serial(X);
			f.serial(Y);
			f.serial(W);
			f.serial(H);
			f.serial(Active);
			f.serial(ScrollPos);
			f.serial(BgAlpha);
			f.serial(ContentAlpha);
			f.serial(RolloverAlphaContent);
			f.serial(PopupX);
			f.serial(PopupY);
			f.serial(PopupW);
			f.serial(PopupH);
			f.serial(UseGlobalAlpha);
			f.serial(Locked);
			f.serial(Movable);
			f.serial(ActiveSavable);
			f.serial(PopupMaxH);
			f.serial(RolloverAlphaContainer);
			if (ver >= 9)
			{
				f.serial(BackupedPositionValid);
				if (BackupedPositionValid)
				{
					f.serial(BackupX);
					f.serial(BackupY);
				}
				f.serial(TouchFlag);
			}
		}
		else if (ContainerMode == 1)
		{
			f.serial(Opened);
			f.serial(Active);
			f.serial(ActiveSavable);
		}
	}
	else
	{
		ContainerMode = 0;
		f.serial(Id);
		f.serial(Popuped);
		f.serial(Opened);
		f.serial(X);
		f.serial(Y);
		f.serial(W);
		f.serial(H);
		f.serial(Active);
		f.serial(ScrollPos);
		if (ver >= 1)
		{
			f.serial(BgAlpha);
			f.serial(ContentAlpha);
			f.serial(RolloverAlphaContent);
			f.serial(PopupX);
			f.serial(PopupY);
			f.serial(PopupW);
			f.serial(PopupH);
		}
		else
		{
			BgAlpha = 255;
			ContentAlpha = 255;
			RolloverAlphaContent = RolloverAlphaContainer = 255;
			PopupX = -1;
			PopupY = -1;
			PopupW = -1;
			PopupH = -1;
		}
		if (ver >= 2)
			f.serial(UseGlobalAlpha);
		else
			UseGlobalAlpha = true;

		if (ver >= 3)
			f.serial(Locked);
		else
			Locked = false;

		if (ver >= 4)
			f.serial(Movable);
		else
			Movable = true;

		if (ver >= 5)
			f.serial(ActiveSavable);
		else
			ActiveSavable = true;

		if (ver >= 6)
			f.serial(PopupMaxH);
		else
			PopupMaxH = 512;

		if (ver >= 7)
			f.serial(RolloverAlphaContainer);
		else
			RolloverAlphaContainer = RolloverAlphaContent;
	}
}



// ***************************************************************************
void CInterfaceConfig::SCont::setFrom (CGroupContainer *pGC)
{
	ContainerMode = 0;
	Id = pGC->getId();

	MinW = pGC->getMinW();
	MaxW = pGC->getMaxW();

	// Check container mode
	if ((pGC->getLayerSetup()>0) && (!pGC->isPopable()))
	{
		// The container is contained by another one and cannot be transformed in root container
		ContainerMode = 1;
		Opened = pGC->isOpen();
		ActiveSavable = pGC->isActiveSavable();
		Active = pGC->getActive();
	}
	else
	{
		// Other type of container save all
		Popuped = pGC->isPopuped();
		Opened = pGC->isOpen();

		ActiveSavable = pGC->isActiveSavable();
		Active = pGC->getActive();

		X = pGC->getX();
		Y = pGC->getY();
		W = pGC->getW(false);
		H = pGC->getH(false);
		//
		BgAlpha = pGC->getContainerAlpha();
		ContentAlpha = pGC->getContentAlpha();
		RolloverAlphaContent = pGC->getRolloverAlphaContent();
		RolloverAlphaContainer = pGC->getRolloverAlphaContainer();
		UseGlobalAlpha = pGC->isUsingGlobalAlpha();
		//
		if (Popuped)
		{
			PopupX = pGC->getX();
			PopupY = pGC->getY();
			PopupW = pGC->getW();
			PopupH = pGC->getH();
		}
		else
		{
			PopupX = pGC->getPopupX();
			PopupY = pGC->getPopupY();
			PopupW = pGC->getPopupW();
			PopupH = pGC->getPopupH();
		}
		PopupMaxH = pGC->getPopupMaxH();
		//
		Locked = pGC->isLocked();
		Movable = pGC->isMovable();

		ScrollPos = 0;
		CCtrlScroll *pSB = dynamic_cast<CCtrlScroll*>(pGC->getCtrl("sb"));
		if (pSB != NULL) ScrollPos = pSB->getTrackPos();
	}
	if (pGC->isPositionBackuped())
	{
		BackupedPositionValid = true;
		BackupX = pGC->getBackupX();
		BackupY = pGC->getBackupY();
	}
	else
	{
		BackupedPositionValid = false;
	}
	TouchFlag = pGC->getTouchFlag(false);
}

// ***************************************************************************
void CInterfaceConfig::SCont::setTo (CGroupContainer *pGC)
{
	if (ContainerMode == 0)
	{
		pGC->setMinW(MinW);
		pGC->setMaxW(MaxW);
		// Normal container
		if ( pGC->isPopable() && Popuped != pGC->isPopuped() )
		{
			if (Popuped)
				pGC->popupCurrentPos();
			else
				pGC->popin();
		}


		pGC->forceRolloverAlpha();
		pGC->setOpen(Opened);

		pGC->setXAndInvalidateCoords(X);
		pGC->setYAndInvalidateCoords(Y);
		// Set the W and H only if resizer is enabled (else always take from scripts)
		if(pGC->getEnabledResizer())
		{
			sint	w= W;
			sint	h= H;
			// use the Popup min and max, it it is not popable.... (yoyo: what a mess....)
			if(!pGC->isPopable())
				clamp(w, pGC->getPopupMinW(), pGC->getPopupMaxW());
			else
				clamp(w, pGC->getMinW(), pGC->getMaxW());
			pGC->setWAndInvalidateCoords(w);
			pGC->setHAndInvalidateCoords(h);
			w= PopupW;
			h= PopupH;
			clamp(w, pGC->getPopupMinW(), pGC->getPopupMaxW());
			clamp(h, pGC->getPopupMinH(), pGC->getPopupMaxH());
			pGC->setPopupW(w);
			pGC->setPopupH(h);
			pGC->setPopupMaxH(PopupMaxH);
		}
		//
		pGC->setPopupX(PopupX);
		pGC->setPopupY(PopupY);

		if (Popuped)
		{
			pGC->setH(pGC->getPopupH());
			pGC->setWAndInvalidateCoords(pGC->getPopupW());
		}

		pGC->setContainerAlpha(BgAlpha);
		pGC->setContentAlpha(ContentAlpha);
		pGC->setRolloverAlphaContent(RolloverAlphaContent);
		pGC->setRolloverAlphaContainer(RolloverAlphaContainer);
		pGC->setUseGlobalAlpha(UseGlobalAlpha);

		if (ActiveSavable)
		{
			pGC->setActive (false);
			pGC->setActive (Active);
		}

		if (pGC->isLockable()) pGC->setLocked(Locked);
		pGC->setMovable(Movable);

		CCtrlScroll *pSB = dynamic_cast<CCtrlScroll*>(pGC->getCtrl("sb"));
		if (pSB != NULL) pSB->setTrackPos(ScrollPos);
		//
		pGC->touch(TouchFlag);
		//
		if (BackupedPositionValid)
		{
			pGC->setBackupPosition(BackupX, BackupY);
		}
		else
		{
			pGC->clearBackup();
		}

	}
	else if (ContainerMode == 1)
	{
		// Container that just need Opened, Active and ActiveSavable state to be retrieved
		pGC->setOpen(Opened);
		if (ActiveSavable)
		{
			pGC->setActive (false);
			pGC->setActive (Active);
		}
	}
}

// ***************************************************************************
// SDBLeaf Data from database to be saved
// ***************************************************************************

// ***************************************************************************
void CInterfaceConfig::SDBLeaf::serial(NLMISC::IStream &f)
{
	// version 1 : added old value ( else some observers are not launched )
	// version 0 : base version
	sint ver = f.serialVersion(1);
	f.serial(Name);
	f.serial(Value);
	if (ver >= 1)
		f.serial(OldValue);
	else
		OldValue = Value;
}

// ***************************************************************************
void CInterfaceConfig::SDBLeaf::setFrom (CCDBNodeLeaf *pNL)
{
	Name = pNL->getFullName();
	Value = pNL->getValue64();
	OldValue = pNL->getOldValue64();
}

// ***************************************************************************
void CInterfaceConfig::SDBLeaf::setTo (CCDBNodeLeaf *pNL)
{
	pNL->setValue64(OldValue);
	pNL->setValue64(Value);
}

// ***************************************************************************
/** Visitor of the ui tree that save the config
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CSaveUIConfigVisitor : public CInterfaceElementVisitor
{
public:
	// build the visitor to fill the given stream
	CSaveUIConfigVisitor(NLMISC::IStream &stream) : Stream(stream) {}
private:
	// The stream where datas are written
	NLMISC::IStream &Stream;
	// From CInterfaceElementVisitor
	void visit(CInterfaceElement *elem)
	{
 		if (!elem) return;
		nlassert(!Stream.isReading());
		if (!elem->wantSerialConfig()) return; // has something to save ?
		// if yes, save the name for further retrieval
		std::string id = elem->getId();
		Stream.serial(id);
		// measure size of object
		// NB : here we write in a separate stream to accomplish this because
		// the object may do some 'serialPtr', this would cause the second serial to have a different size
		// because the object would already have been recorded in the ptr table of the stream
		CMemStream measureStream;
		nlassert(!measureStream.isReading());
		elem->serialConfig(measureStream);
		uint32 chunkSize = measureStream.getPos();
		Stream.serial(chunkSize);
		elem->serialConfig(Stream);
	}
};

// ***************************************************************************
/** Visitor to count the number of element that need config saving
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CCountUIElemWithConfigVisitor : public CInterfaceElementVisitor
{
public:
	CCountUIElemWithConfigVisitor() : Count(0) {}
	uint32 Count;
	// From CInterfaceElementVisitor
	void visit(CInterfaceElement *elem)
	{
		if (elem->wantSerialConfig()) ++ Count;
	}
};

// ***************************************************************************
// visitor to send the 'onLoadConfig' msg
class COnLoadConfigVisitor : public CInterfaceElementVisitor
{
	void visit(CInterfaceElement *elem) { elem->onLoadConfig(); }
};

// ***************************************************************************
CInterfaceConfig::CDesktopImage::CDesktopImage()
{
	Version = INTERFACE_CONFIG_STREAM_VERSION;
}

// ***************************************************************************
void CInterfaceConfig::CDesktopImage::serial(NLMISC::IStream &s)
{
	if (s.isReading()) read(s);
	else write(s);
}

// ***************************************************************************
void CInterfaceConfig::CDesktopImage::read(NLMISC::IStream &f)
{
	nlassert(f.isReading());
	f.serialVersion(Version);
	f.serialCont(GCImages);
	// extra datas go until the end of stream
	sint32 begPos = f.getPos();
	f.seek (0, NLMISC::IStream::end);
	sint32 endPos = f.getPos();
	f.seek (begPos, NLMISC::IStream::begin);
	NLMISC::contReset(ExtraDatas);
	if (ExtraDatas.isReading())
	{
		ExtraDatas.invert();
	}
	sint32 length = endPos - begPos;
	if (length > 0)
	{
		uint8 *pBuffer = new uint8[length];
		f.serialBuffer(pBuffer, length); // read buffer from file
		ExtraDatas.serialBuffer(pBuffer, length); // copy buffer to memstream
		delete [] pBuffer;
	}
}

// ***************************************************************************
void CInterfaceConfig::CDesktopImage::write(NLMISC::IStream &f)
{
	nlassert(!f.isReading());
	// Version is important when the stream will be saved on Disk.
	f.serialVersion(Version);
	f.serialCont(GCImages);
	// serial extra datas
	uint32 length = ExtraDatas.length();
	if (length > 0)
	{
		uint8 *pBuffer = new uint8[length];
		memcpy(pBuffer, ExtraDatas.buffer(), length);
		f.serialBuffer(pBuffer, length);
		delete [] pBuffer;
	}
}

// ***************************************************************************
void CInterfaceConfig::CDesktopImage::fromCurrentDesktop()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	// Count number of container to save
	uint32 nCount = 0, nMasterGroup, i, nCount2;

	const vector<CWidgetManager::SMasterGroup> &rVMG = CWidgetManager::getInstance()->getAllMasterGroup();
	for (nMasterGroup = 0; nMasterGroup < rVMG.size(); nMasterGroup++)
	{
		const CWidgetManager::SMasterGroup &rMG = rVMG[nMasterGroup];
		const vector<CInterfaceGroup*> &rV = rMG.Group->getGroups();
		for (i = 0; i < rV.size(); ++i)
		{
			CGroupContainer		*pGC= dynamic_cast<CGroupContainer*>(rV[i]);
			if ( pGC != NULL && pGC->isSavable() )
				nCount++;
		}
	}

	GCImages.resize(nCount);
	SCont contTmp;
	nCount2 = 0;
	// retrieve all containers
	for (nMasterGroup = 0; nMasterGroup < rVMG.size(); nMasterGroup++)
	{
		const CWidgetManager::SMasterGroup &rMG = rVMG[nMasterGroup];
		const vector<CInterfaceGroup*> &rV = rMG.Group->getGroups();
		for (i = 0; i < rV.size(); ++i)
		{
			CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(rV[i]);
			if ( pGC != NULL && pGC->isSavable() )
			{
				GCImages[nCount2].setFrom(pGC);
				nCount2++;
			}
		}
	}
	nlassert(nCount2 == nCount);
	// set extra data stream version (in memory)
	Version = INTERFACE_CONFIG_STREAM_VERSION;
	// serial extra data in the stream
	NLMISC::CMemStream &f = ExtraDatas;
	if (f.isReading())
	{
		f.invert();
	}
	f.resetPtrTable();
	f.seek(0, NLMISC::IStream::begin);
	// Save the Top Window for this config.
	CInterfaceGroup	*topWindow= CWidgetManager::getInstance()->getTopWindow(CWidgetManager::getInstance()->getLastTopWindowPriority());
	string	topWindowName;
	if (topWindow)
	{
		CGroupContainer *pGC= dynamic_cast<CGroupContainer*>(topWindow);
		if (pGC != NULL && pGC->isSavable())
			topWindowName = pGC->getId();
	}
	f.serial(topWindowName);


	// retrieve number of elements that want their config saved
	CCountUIElemWithConfigVisitor counter;
	pIM->visit(&counter);
	f.serial(counter.Count);
	// Serial specific infos for each widget that reclaims it
	CSaveUIConfigVisitor saver(f);
	pIM->visit(&saver);
}

// ***************************************************************************
void CInterfaceConfig::CDesktopImage::toCurrentDesktop()
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	COnLoadConfigVisitor onLoadVisitor;
	pIM->visit(&onLoadVisitor); // send 'onLoad' msg to every element
//	uint32 nCount = 0;

	for(uint k = 0; k < GCImages.size(); ++k)
	{
		CGroupContainer *pGC = dynamic_cast<CGroupContainer*>(CWidgetManager::getInstance()->getElementFromId(GCImages[k].Id));
		if (pGC != NULL)
			GCImages[k].setTo(pGC);
	}
	// serial extra data from the stream
	NLMISC::CMemStream &f = ExtraDatas;
	if (!f.isReading())
	{
		f.invert();
	}
	f.resetPtrTable();
	f.seek(0, NLMISC::IStream::begin);
	f.seek(0, NLMISC::IStream::end);
	if (f.getPos() == 0) return;
	f.seek(0, NLMISC::IStream::begin);
	// Load TopWindow config
	if(Version>=1)
	{
		string	topWindowName;
		f.serial(topWindowName);
		if(!topWindowName.empty())
		{
			CInterfaceGroup	*window= dynamic_cast<CInterfaceGroup*>(CWidgetManager::getInstance()->getElementFromId(topWindowName));
			if(window && window->getActive())
				CWidgetManager::getInstance()->setTopWindow(window);
		}
	}
	uint32 numElemWithConfig;
	f.serial(numElemWithConfig);
	for(uint k = 0; k < numElemWithConfig; ++k)
	{
		std::string elemID;
		f.serial(elemID);
		uint32 chunkSize = 0;
		f.serial(chunkSize);
		uint startPos = f.getPos();
		CInterfaceManager *im = CInterfaceManager::getInstance();
		CInterfaceElement *elem = CWidgetManager::getInstance()->getElementFromId(elemID);
		if (!elem)
		{
			nlwarning("Element %s not found while loading config, skipping datas", elemID.c_str());
			f.seek(chunkSize, NLMISC::IStream::current);
		}
		else
		{
			try
			{
				elem->serialConfig(f);
			}
			catch (const NLMISC::ENewerStream &)
			{
				nlwarning("Element %s config in stream are too recent to be read by the application,  config ignored", elemID.c_str());
				f.seek(startPos + chunkSize, NLMISC::IStream::begin);
			}
		}
	}
}

// ***************************************************************************
void CInterfaceConfig::CDesktopImage::updateGroupContainerImage(CGroupContainer &gc)
{
	bool updated = false;
	for(uint k = 0; k < GCImages.size(); ++k)
	{
		if (GCImages[k].Id == gc.getId())
		{
			GCImages[k].setFrom(&gc);
			updated = true;
		}
	}
	if (!updated)
	{
		SCont image;
		image.setFrom(&gc);
		GCImages.push_back(image);
	}
}

// predicate to see if a group container image match the given id
class CGroupContainerImageMatch
{
public:
	const std::string &Id;
	CGroupContainerImageMatch(const std::string &id) : Id(id) {}
	bool operator()(const CInterfaceConfig::SCont &image) const
	{
		return image.Id == Id;
	}
};

// ***************************************************************************
void CInterfaceConfig::CDesktopImage::removeGroupContainerImage(const std::string &groupName)
{
	GCImages.erase(std::remove_if(GCImages.begin(), GCImages.end(), CGroupContainerImageMatch(groupName)), GCImages.end());
}



// ***************************************************************************
void CInterfaceConfig::dataBaseToStream (NLMISC::IStream &f)
{
	if (f.isReading())
	{
		nlwarning("stream is not in writing mode");
		return;
	}

	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// Save branch of the database
	SDBLeaf leafTmp;
	CCDBNodeBranch *pDB = NLGUI::CDBManager::getInstance()->getDbBranch ("UI:SAVE");
	if (pDB != NULL)
	{
		// Number of leaf to save
		uint32 nbLeaves = pDB->countLeaves();
		f.serial(nbLeaves);

		for (uint32 i = 0; i < nbLeaves; ++i)
		{
			uint count = i;
			CCDBNodeLeaf *pNL = pDB->findLeafAtCount(count);
			leafTmp.setFrom(pNL);
			f.serial(leafTmp);
		}
	}
}

// ***************************************************************************
void CInterfaceConfig::streamToDataBase (NLMISC::IStream &f, uint32 uiDbSaveVersion)
{
	if (!f.isReading())
	{
		nlwarning("stream is not in reading mode");
		return;
	}

	sint32 begPos = f.getPos();
	f.seek (0, NLMISC::IStream::end);
	sint32 endPos = f.getPos();
	if ((begPos - endPos) == 0) return;
	f.seek (begPos, NLMISC::IStream::begin);

	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	// Load branch of the database
	SDBLeaf leafTmp;
	CCDBNodeBranch *pDB = NLGUI::CDBManager::getInstance()->getDbBranch ("UI:SAVE");
	if (pDB != NULL)
	{
		// Number of leaf to save
		uint32 nbLeaves = 0;
		f.serial(nbLeaves);

		for (uint32 i = 0; i < nbLeaves; ++i)
		{
			f.serial(leafTmp);

			// If there is a define RESET_VER_dbName that exist for this DB, check if version is OK
			bool	wantRead= true;
			// Format dbName for version check
			string	defVerId= "RESET_VER_";
			defVerId+= leafTmp.Name;
			for(uint i=0;i<defVerId.size();i++)
			{
				if(defVerId[i]==':')
					defVerId[i]='_';
			}
			// check if exist
			if( CWidgetManager::getInstance()->getParser()->isDefineExist(defVerId))
			{
				uint32	dbVer;
				fromString(CWidgetManager::getInstance()->getParser()->getDefine(defVerId), dbVer);
				// if the version in the file is older than the version this db want, abort read
				if(uiDbSaveVersion<dbVer)
					wantRead= false;
			}

			// if want read the value from file, read it, else keep the default one
			if(wantRead)
			{
				CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(leafTmp.Name,false);
				if (pNL != NULL)
					leafTmp.setTo(pNL);
			}
		}
	}
}
