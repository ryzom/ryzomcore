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

#ifndef NL_LOAD_FORM_H
#define NL_LOAD_FORM_H

#include "nel/misc/types_nl.h"

#include <map>
#include <string>
#include <vector>

#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/algo.h"

#include "u_form_loader.h"
#include "u_form.h"

/** This function is used to load values from georges sheet in a quick way.
 * The first time it loads the sheet and parse it with the readGeorges function
 * provided by the user to read the value he wants. It'll generate a packed file
 * that contains this values (using serialCont). The next launch, the function will
 * only load the packed file and if some sheet have changed, it'll automatically regenerate
 * the packed file.
 *
 * To use the loadForm(), you first have to create a class that will contains values for one sheet.
 * This class must also implements 2 functions (readGeorges() and serial()) and 1 static function (getVersion())
 *
 * Extension file name for the packedFilename must be ".packed_sheets"
 *
 * Classical use (copy/paste this in your code):

	// For each sheet in the packed sheet, an instance of this class
	// is created and stored into an stl container.
	// This class must be default and copy constructable.
	class CContainerEntry
	{
	public:
		CContainerEntry () : WalkSpeed(1.3f), RunSpeed(6.0f) {}

		float WalkSpeed, RunSpeed;

		// load the values using the george sheet
		void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
		{
			// the form was found so read the true values from George
			form->getRootNode ().getValueByName (WalkSpeed, "Basics.MovementSpeeds.WalkSpeed");
			form->getRootNode ().getValueByName (RunSpeed, "Basics.MovementSpeeds.RunSpeed");
		}

		// load/save the values using the serial system
		void serial (NLMISC::IStream &s)
		{
			s.serial (WalkSpeed, RunSpeed);
		}

		// Event to implement any action when the sheet no longer exist.
		// This method is called when a sheet have been read from the packed sheet
		// and the associated sheet file no more exist in the directories.
		void removed()
		{
			// any action that is needed if the sheet no more exist.
		}

		// return the version of this class, increments this value when the content of this class changed
		static uint getVersion () { return 1; }
	};

	// this structure is filled by the loadForm() function and will contain all you need
	std::map<NLMISC::CSheetId,CContainerEntry> Container;

	void init ()
	{
		// load the values using the george sheet or packed file and fill the container
		loadForm(".creature", "test.packed_sheets", Container);
	}

 * Now you can access the Container (using the CSheedId) to know the WalkSpeed and RunSpeed of all creatures.
 *
 */


/// Dictionnary entry for dependency information.
/*
struct TLoadFormDicoEntry
{
	std::string		Filename;
	uint32			ModificationDate;

	void serial(NLMISC::IStream &s)
	{
		s.serial(Filename);
		s.serial(ModificationDate);
	}
};
*/
const uint32		PACKED_SHEET_HEADER = NELID("PKSH");
const uint32		PACKED_SHEET_VERSION = 5;
// This Version may be used if you want to use the serialVersion() system in loadForm()
const uint32		PACKED_SHEET_VERSION_COMPATIBLE = 0;

// ***************************************************************************
/** This function is used to load values from georges sheet in a quick way.
 * \param sheetFilter a vector of string to filter the sheet in the case you need more than one filter
 * \param packedFilename the name of the file that this function will generate (extension must be "packed_sheets")
 * \param container the map that will be filled by this function
 */
template <class T>
void loadForm (const std::vector<std::string> &sheetFilters, const std::string &packedFilename, std::map<NLMISC::CSheetId, T> &container, bool updatePackedSheet=true, bool errorIfPackedSheetNotGood=true)
{
	std::vector<std::string>						dictionnary;
	std::map<std::string, uint>						dictionnaryIndex;
	std::map<NLMISC::CSheetId, std::vector<uint32> >	dependencies;
	std::vector<uint32>								dependencyDates;

	// check the extension (i know that file like "foo.packed_sheetsbar" will be accepted but this check is enough...)
	nlassert (packedFilename.find (".packed_sheets") != std::string::npos);

	std::string packedFilenamePath = NLMISC::CPath::lookup(NLMISC::CFile::getFilename(packedFilename), false, false);
	if (packedFilenamePath.empty())
	{
		packedFilenamePath = packedFilename;
	}

	// make sure the CSheetId singleton has been properly initialised
	NLMISC::CSheetId::init(updatePackedSheet);

	// load the packed sheet if exists
	try
	{
		NLMISC::CIFile ifile;
		ifile.setCacheFileOnOpen(true);
		if (!ifile.open (packedFilenamePath))
		{
			throw	NLMISC::Exception("can't open PackedSheet %s", packedFilenamePath.c_str());
		}
		// an exception will be launch if the file is not the good version or if the file is not found

		//nlinfo ("loadForm(): Loading packed file '%s'", packedFilename.c_str());

		// read the header
		ifile.serialCheck(PACKED_SHEET_HEADER);
		ifile.serialCheck(PACKED_SHEET_VERSION);
		ifile.serialVersion(PACKED_SHEET_VERSION_COMPATIBLE);

		// Read depend block size
		uint32	dependBlockSize;
		ifile.serial(dependBlockSize);

		// Read the dependencies only if update packed sheet
		if(updatePackedSheet)
		{
			// read the dictionnary
			{
				ifile.serialCont(dictionnary);
			}
			// read the dependency data
			{
				uint32 depSize;
				ifile.serial(depSize);
				for (uint i=0; i<depSize; ++i)
				{
					NLMISC::CSheetId sheetId;

					// Avoid copy, use []
					ifile.serial(sheetId);
					ifile.serialCont(dependencies[sheetId]);
				}
			}
		}
		// else dummy read one big block => no heavy reallocation / free
		else if(dependBlockSize>0)
		{
			std::vector<uint8>	bigBlock;
			bigBlock.resize(dependBlockSize);
			ifile.serialBuffer(&bigBlock[0], dependBlockSize);
		}

		// read the packed sheet data
		uint32	nbEntries;
		uint32	ver;
		ifile.serial (nbEntries);
		ifile.serial (ver);
		if(ver != T::getVersion ())
			throw NLMISC::Exception("The packed sheet version in stream is different of the code");
		ifile.serialCont (container);
		ifile.close ();
	}
	catch (const NLMISC::Exception &e)
	{
		// clear the container because it can contains partially loaded sheet so we must clean it before continue
		container.clear ();
		if (!updatePackedSheet)
		{
			if (errorIfPackedSheetNotGood)
				nlerror ("loadForm(): Exception during reading the packed file and can't reconstruct them (%s)", e.what());
			else
				nlinfo ("loadForm(): Exception during reading the packed file and can't reconstruct them (%s)", e.what());

			return;
		}
		else
		{
			nlinfo ("loadForm(): Exception during reading the packed file, I'll reconstruct it (%s)", e.what());
		}
	}

	// if we don't want to update packed sheet, we have nothing more to do
	if (!updatePackedSheet)
	{
		//nlinfo ("Don't update the packed sheet with real sheet");
		return;
	}

	// retreive the date of all dependency file
	{
		for (uint i=0; i<dictionnary.size(); ++i)
		{
			std::string p = NLMISC::CPath::lookup (dictionnary[i], false, false);
			if (!p.empty())
			{
				uint32 d = NLMISC::CFile::getFileModificationDate(p);
				dependencyDates.push_back(d);
			}
			else
			{
				// file not found !
				// write a future date to invalidate any file dependent on it
				nldebug("Can't find dependent file %s !", dictionnary[i].c_str());
				dependencyDates.push_back(0xffffffff);
			}
		}
	}

	// build a vector of the sheetFilters sheet ids (ie: "item")
	std::vector<NLMISC::CSheetId> sheetIds;
	std::vector<std::string> filenames;
	for (uint i = 0; i < sheetFilters.size(); i++)
		NLMISC::CSheetId::buildIdVector(sheetIds, filenames, sheetFilters[i]);

	// if there's no file, nothing to do
	if (sheetIds.empty())
		return;

	// set up the current sheet in container to remove sheet that are in the container and not in the directory anymore
	std::map<NLMISC::CSheetId, bool> sheetToRemove;
	for (typename std::map<NLMISC::CSheetId, T>::iterator it = container.begin(); it != container.end(); it++)
	{
		sheetToRemove.insert (std::make_pair((*it).first, true));
	}

	// check if we need to create a new .pitems or just read it
	uint32 packedFiledate = NLMISC::CFile::getFileModificationDate(packedFilenamePath);

	bool containerChanged = false;

	NLGEORGES::UFormLoader *formLoader = NULL;

	std::vector<uint> NeededToRecompute;

	for (uint k = 0; k < filenames.size(); k++)
	{
		std::string p = NLMISC::CPath::lookup (filenames[k], false, false);
		if (p.empty()) continue;
		uint32 d = NLMISC::CFile::getFileModificationDate(p);

		// no need to remove this sheet
		sheetToRemove[sheetIds[k]] = false;

		if( d > packedFiledate || container.find (sheetIds[k]) == container.end())
		{
			NeededToRecompute.push_back(k);
		}
		else
		{
			// check the date of each parent
			nlassert(dependencies.find(sheetIds[k]) != dependencies.end());
			std::vector<uint32> &depends = dependencies[sheetIds[k]];

			for (uint i=0; i<depends.size(); ++i)
			{
				if (dependencyDates[depends[i]] > packedFiledate)
				{
					nldebug("Dependency on %s for %s not up to date !",
						dictionnary[depends[i]].c_str(), sheetIds[k].toString().c_str());
					NeededToRecompute.push_back(k);
					break;
				}
			}
		}
	}

	nlinfo ("%d sheets checked, %d need to be recomputed", filenames.size(), NeededToRecompute.size());

	NLMISC::TTime last = NLMISC::CTime::getLocalTime ();
	NLMISC::TTime start = NLMISC::CTime::getLocalTime ();

	NLMISC::CSmartPtr<NLGEORGES::UForm> form;
	std::vector<NLMISC::CSmartPtr<NLGEORGES::UForm> >	cacheFormList;

	for (uint j = 0; j < NeededToRecompute.size(); j++)
	{
		if(NLMISC::CTime::getLocalTime () > last + 5000)
		{
			last = NLMISC::CTime::getLocalTime ();
			if(j>0)
				nlinfo ("%.0f%% completed (%d/%d), %d seconds remaining", (float)j*100.0/NeededToRecompute.size(),j,NeededToRecompute.size(), (NeededToRecompute.size()-j)*(last-start)/j/1000);
		}

		// create the georges loader if necessary
		if (formLoader == NULL)
		{
			NLMISC::WarningLog->addNegativeFilter("CFormLoader: Can't open the form file");
			formLoader = NLGEORGES::UFormLoader::createLoader ();
		}

		//	cache used to retain information (to optimize time).
		if (form)
			cacheFormList.push_back	(form);

		// Load the form with given sheet id
		form = formLoader->loadForm (sheetIds[NeededToRecompute[j]].toString().c_str ());
		if (form)
		{
			// build the dependency data
			{
				std::vector<uint32>		depends;
				std::set<std::string>	dependFiles;
				form->getDependencies (dependFiles);
				nlassert(dependFiles.find(sheetIds[NeededToRecompute[j]].toString()) != dependFiles.end());
				// remove the sheet itself from the container
				dependFiles.erase(sheetIds[NeededToRecompute[j]].toString());

				std::set<std::string>::iterator first(dependFiles.begin()), last(dependFiles.end());
				for (; first != last; ++first)
				{
					const	std::string filename = NLMISC::CFile::getFilename(*first);
					std::map<std::string,uint>::iterator	findDicIt=dictionnaryIndex.find(filename);

					if	(findDicIt!=dictionnaryIndex.end())
					{
						depends.push_back(findDicIt->second);
						continue;
					}

					std::string p = NLMISC::CPath::lookup (*first, false, false);
					if	(!p.empty())
					{
						uint dicIndex;
						// add a new dictionnary entry
						dicIndex = (uint)dictionnary.size();
						dictionnaryIndex.insert(std::make_pair(filename, (uint)dictionnary.size()));
						dictionnary.push_back(filename);

						// add the dependecy index
						depends.push_back(dicIndex);
					}
				}
				// store the dependency list with the sheet ID
				dependencies[sheetIds[NeededToRecompute[j]]] = depends;
			}

			// add the new creature, it could be already loaded by the packed sheets but will be overwritten with the new one
			typedef typename std::map<NLMISC::CSheetId, T>::iterator TType1;
            typedef typename std::pair<TType1, bool> TType2;
			TType2 res = container.insert(std::make_pair(sheetIds[NeededToRecompute[j]],T()));

			(*res.first).second.readGeorges (form, sheetIds[NeededToRecompute[j]]);
			containerChanged = true;
		}
	}

	if(NeededToRecompute.size() > 0)
		nlinfo ("%d seconds to recompute %d sheets", (uint32)(NLMISC::CTime::getLocalTime()-start)/1000, NeededToRecompute.size());

	// free the georges loader if necessary
	if (formLoader != NULL)
	{
		NLGEORGES::UFormLoader::releaseLoader (formLoader);
		NLMISC::WarningLog->removeFilter ("CFormLoader: Can't open the form file");
	}

	// we have now to remove sheets that are in the container and not exist anymore in the sheet directories
	for (std::map<NLMISC::CSheetId, bool>::iterator it2 = sheetToRemove.begin(); it2 != sheetToRemove.end(); it2++)
	{
		if((*it2).second)
		{
			nlinfo ("the sheet '%s' is not in the directory, remove it from container", (*it2).first.toString().c_str());
			container.find((*it2).first)->second.removed();
			container.erase((*it2).first);
			containerChanged = true;
			dependencies.erase((*it2).first);
		}
	}

	// now, save the new container in the packedfile
	try
	{
		if(containerChanged)
		{
			NLMISC::COFile ofile;
			ofile.open(packedFilenamePath);

			// write the header.
			ofile.serialCheck(PACKED_SHEET_HEADER);
			ofile.serialCheck(PACKED_SHEET_VERSION);
			ofile.serialVersion(PACKED_SHEET_VERSION_COMPATIBLE);

			// Write a dummy block size for now
			sint32	posBlockSize= ofile.getPos();
			uint32	dependBlockSize= 0;
			ofile.serial(dependBlockSize);

			// write the dictionnary
			ofile.serialCont(dictionnary);

			// write the dependencies data
			uint32 depSize = (uint32)dependencies.size();
			ofile.serial(depSize);
			std::map<NLMISC::CSheetId, std::vector<uint32> >::iterator first(dependencies.begin()), last(dependencies.end());
			for (; first != last; ++first)
			{
				NLMISC::CSheetId si = first->first;
				ofile.serial(si);
				ofile.serialCont(first->second);
			}

			// Then get the dictionary + dependencies size, and write it back to posBlockSize
			sint32	endBlockSize= ofile.getPos();
			dependBlockSize= (endBlockSize - posBlockSize) - 4;
			ofile.seek(posBlockSize, NLMISC::IStream::begin);
			ofile.serial(dependBlockSize);
			ofile.seek(endBlockSize, NLMISC::IStream::begin);

			// write the sheet data
			uint32 nbEntries = (uint32)sheetIds.size();
			uint32 ver = T::getVersion ();
			ofile.serial (nbEntries);
			ofile.serial (ver);
			ofile.serialCont(container);
			ofile.close ();
		}
	}
	catch (const NLMISC::Exception &e)
	{
		nlinfo ("loadForm(): Exception during saving the packed file, it will be recreated next launch (%s)", e.what());
	}

	// housekeeping
	sheetIds.clear ();
	filenames.clear ();
}

// ***************************************************************************
/** This function is used to load values from georges sheet in a quick way.
 * \param sheetFilter a string to filter the sheet (ie: ".item")
 * \param packedFilename the name of the file that this function will generate (extension must be "packed_sheets")
 * \param container the map that will be filled by this function
 */
template <class T>
void loadForm (const std::string &sheetFilter, const std::string &packedFilename, std::map<NLMISC::CSheetId, T> &container, bool updatePackedSheet=true, bool errorIfPackedSheetNotGood=true)
{
	std::vector<std::string> vs;
	vs.push_back(sheetFilter);
	loadForm(vs, packedFilename, container, updatePackedSheet, errorIfPackedSheetNotGood);
}


// ***************************************************************************
// variant with smart pointers, maintain with function above
template <class T>
void loadForm2(const std::vector<std::string> &sheetFilters, const std::string &packedFilename, std::map<NLMISC::CSheetId, NLMISC::CSmartPtr<T> > &container, bool updatePackedSheet=true, bool errorIfPackedSheetNotGood=true)
{
	std::vector<std::string>						dictionnary;
	std::map<std::string, uint>						dictionnaryIndex;
	std::map<NLMISC::CSheetId, std::vector<uint32> >	dependencies;
	std::vector<uint32>								dependencyDates;

	// check the extension (i know that file like "foo.packed_sheetsbar" will be accepted but this check is enough...)
	nlassert (packedFilename.find (".packed_sheets") != std::string::npos);

	std::string packedFilenamePath = NLMISC::CPath::lookup(NLMISC::CFile::getFilename(packedFilename), false, false);
	if (packedFilenamePath.empty())
	{
		packedFilenamePath = packedFilename;
	}

	// make sure the CSheetId singleton has been properly initialised
	NLMISC::CSheetId::init(updatePackedSheet);

	// load the packed sheet if exists
	try
	{
		NLMISC::CIFile ifile;
		ifile.setCacheFileOnOpen(true);
		if (!ifile.open (packedFilenamePath))
		{
			throw	NLMISC::Exception("can't open PackedSheet %s", packedFilenamePath.c_str());
		}
		// an exception will be launch if the file is not the good version or if the file is not found

		nlinfo ("loadForm(): Loading packed file '%s'", packedFilename.c_str());

		// read the header
		ifile.serialCheck(PACKED_SHEET_HEADER);
		ifile.serialCheck(PACKED_SHEET_VERSION);
		sint	loadFormVersion= ifile.serialVersion(PACKED_SHEET_VERSION_COMPATIBLE);

		// Read depend block size
		uint32	dependBlockSize;
		ifile.serial(dependBlockSize);

		// Read the dependencies only if update packed sheet
		if(updatePackedSheet)
		{
			// read the dictionnary
			{
				ifile.serialCont(dictionnary);
			}
			// read the dependency data
			{
				uint32 depSize;
				ifile.serial(depSize);
				for (uint i=0; i<depSize; ++i)
				{
					NLMISC::CSheetId sheetId;

					// Avoid copy, use []
					ifile.serial(sheetId);
					ifile.serialCont(dependencies[sheetId]);
				}
			}
		}
		// else dummy read one big block => no heavy reallocation / free
		else if(dependBlockSize>0)
		{
			std::vector<uint8>	bigBlock;
			bigBlock.resize(dependBlockSize);
			ifile.serialBuffer(&bigBlock[0], dependBlockSize);
		}

		// read the packed sheet data
		uint32	nbEntries;
		uint32	ver;
		ifile.serial (nbEntries);
		ifile.serial (ver);
		if(ver != T::getVersion ())
			throw NLMISC::Exception("The packed sheet version in stream is different of the code");
		ifile.serialPtrCont (container);
		ifile.close ();
	}
	catch (const NLMISC::Exception &e)
	{
		// clear the container because it can contains partially loaded sheet so we must clean it before continue
		container.clear ();
		if (!updatePackedSheet)
		{
			if (errorIfPackedSheetNotGood)
				nlerror ("loadForm(): Exception during reading the packed file and can't reconstruct them (%s)", e.what());
			else
				nlinfo ("loadForm(): Exception during reading the packed file and can't reconstruct them (%s)", e.what());

			return;
		}
		else
		{
			nlinfo ("loadForm(): Exception during reading the packed file, I'll reconstruct it (%s)", e.what());
		}
	}

	// if we don't want to update packed sheet, we have nothing more to do
	if (!updatePackedSheet)
	{
		nlinfo ("Don't update the packed sheet with real sheet");
		return;
	}

	// retreive the date of all dependency file
	{
		for (uint i=0; i<dictionnary.size(); ++i)
		{
			std::string p = NLMISC::CPath::lookup (dictionnary[i], false, false);
			if (!p.empty())
			{
				uint32 d = NLMISC::CFile::getFileModificationDate(p);
				dependencyDates.push_back(d);
			}
			else
			{
				// file not found !
				// write a future date to invalidate any file dependent on it
				nldebug("Can't find dependent file %s !", dictionnary[i].c_str());
				dependencyDates.push_back(0xffffffff);
			}
		}
	}

	// build a vector of the sheetFilters sheet ids (ie: "item")
	std::vector<NLMISC::CSheetId> sheetIds;
	std::vector<std::string> filenames;
	for (uint i = 0; i < sheetFilters.size(); i++)
		NLMISC::CSheetId::buildIdVector(sheetIds, filenames, sheetFilters[i]);

	// if there's no file, nothing to do
	if (sheetIds.empty())
		return;

	// set up the current sheet in container to remove sheets that are in the container and not in the directory anymore
	std::map<NLMISC::CSheetId, bool> sheetToRemove;
	for (typename std::map<NLMISC::CSheetId, NLMISC::CSmartPtr<T> >::iterator it = container.begin(); it != container.end(); it++)
	{
		sheetToRemove.insert (std::make_pair((*it).first, true));
	}

	// check if we need to create a new .pitems or just read it
	uint32 packedFiledate = NLMISC::CFile::getFileModificationDate(packedFilenamePath);

	bool containerChanged = false;

	NLGEORGES::UFormLoader *formLoader = NULL;

	std::vector<uint> NeededToRecompute;

	for (uint k = 0; k < filenames.size(); k++)
	{
		std::string p = NLMISC::CPath::lookup (filenames[k], false, false);
		if (p.empty()) continue;
		uint32 d = NLMISC::CFile::getFileModificationDate(p);

		// no need to remove this sheet
		sheetToRemove[sheetIds[k]] = false;

		if( d > packedFiledate || container.find (sheetIds[k]) == container.end())
		{
			NeededToRecompute.push_back(k);
		}
		else
		{
			// check the date of each parent
			nlassert(dependencies.find(sheetIds[k]) != dependencies.end());
			std::vector<uint32> &depends = dependencies[sheetIds[k]];

			for (uint i=0; i<depends.size(); ++i)
			{
				if (dependencyDates[depends[i]] > packedFiledate)
				{
					nldebug("Dependency on %s for %s not up to date !",
						dictionnary[depends[i]].c_str(), sheetIds[k].toString().c_str());
					NeededToRecompute.push_back(k);
					break;
				}
			}
		}
	}

	nlinfo ("%d sheets checked, %d need to be recomputed", filenames.size(), NeededToRecompute.size());

	NLMISC::TTime last = NLMISC::CTime::getLocalTime ();
	NLMISC::TTime start = NLMISC::CTime::getLocalTime ();

	NLMISC::CSmartPtr<NLGEORGES::UForm> form;
	std::vector<NLMISC::CSmartPtr<NLGEORGES::UForm> >	cacheFormList;

	for (uint j = 0; j < NeededToRecompute.size(); j++)
	{
		if(NLMISC::CTime::getLocalTime () > last + 5000)
		{
			last = NLMISC::CTime::getLocalTime ();
			if(j>0)
				nlinfo ("%.0f%% completed (%d/%d), %d seconds remaining", (float)j*100.0/NeededToRecompute.size(),j,NeededToRecompute.size(), (NeededToRecompute.size()-j)*(last-start)/j/1000);
		}

		// create the georges loader if necessary
		if (formLoader == NULL)
		{
			NLMISC::WarningLog->addNegativeFilter("CFormLoader: Can't open the form file");
			formLoader = NLGEORGES::UFormLoader::createLoader ();
		}

		//	cache used to retain information (to optimize time).
		if (form)
			cacheFormList.push_back	(form);

		// Load the form with given sheet id
		form = formLoader->loadForm (sheetIds[NeededToRecompute[j]].toString().c_str ());
		if (form)
		{
			// build the dependency data
			{
				std::vector<uint32>		depends;
				std::set<std::string>	dependFiles;
				form->getDependencies (dependFiles);
				nlassert(dependFiles.find(sheetIds[NeededToRecompute[j]].toString()) != dependFiles.end());
				// remove the sheet itself from the container
				dependFiles.erase(sheetIds[NeededToRecompute[j]].toString());

				std::set<std::string>::iterator first(dependFiles.begin()), last(dependFiles.end());
				for (; first != last; ++first)
				{
					const	std::string filename = NLMISC::CFile::getFilename(*first);
					std::map<std::string,uint>::iterator	findDicIt=dictionnaryIndex.find(filename);

					if	(findDicIt!=dictionnaryIndex.end())
					{
						depends.push_back(findDicIt->second);
						continue;
					}

					std::string p = NLMISC::CPath::lookup (*first, false, false);
					if	(!p.empty())
					{
						uint dicIndex;
						// add a new dictionnary entry
						dicIndex = (uint)dictionnary.size();
						dictionnaryIndex.insert(std::make_pair(filename, (NLMISC::TSStringId)dictionnary.size()));
						dictionnary.push_back(filename);

						// add the dependency index
						depends.push_back(dicIndex);
					}
				}
				// store the dependency list with the sheet ID
				dependencies[sheetIds[NeededToRecompute[j]]] = depends;
			}

			// add the new creature, it could be already loaded by the packed sheets but will be overwritten with the new one
			typedef typename std::map<NLMISC::CSheetId, NLMISC::CSmartPtr<T> >::iterator TType1;
            typedef typename std::pair<TType1, bool> TType2;
			TType2 res = container.insert(std::make_pair(sheetIds[NeededToRecompute[j]], NLMISC::CSmartPtr<T>(new T())));

			(*res.first).second->readGeorges (form, sheetIds[NeededToRecompute[j]]);
			containerChanged = true;
		}
	}

	if(NeededToRecompute.size() > 0)
		nlinfo ("%d seconds to recompute %d sheets", (uint32)(NLMISC::CTime::getLocalTime()-start)/1000, NeededToRecompute.size());

	// free the georges loader if necessary
	if (formLoader != NULL)
	{
		NLGEORGES::UFormLoader::releaseLoader (formLoader);
		NLMISC::WarningLog->removeFilter ("CFormLoader: Can't open the form file");
	}

	// we have now to remove sheets that are in the container and not exist anymore in the sheet directories
	for (std::map<NLMISC::CSheetId, bool>::iterator it2 = sheetToRemove.begin(); it2 != sheetToRemove.end(); it2++)
	{
		if((*it2).second)
		{
			nlinfo ("the sheet '%s' is not in the directory, remove it from container", (*it2).first.toString().c_str());
			container.find((*it2).first)->second->removed();
			container.erase((*it2).first);
			containerChanged = true;
			dependencies.erase((*it2).first);
		}
	}

	// now, save the new container in the packedfile
	try
	{
		if(containerChanged)
		{
			NLMISC::COFile ofile;
			ofile.open(packedFilenamePath);

			// write the header.
			ofile.serialCheck(PACKED_SHEET_HEADER);
			ofile.serialCheck(PACKED_SHEET_VERSION);
			ofile.serialVersion(PACKED_SHEET_VERSION_COMPATIBLE);

			// Write a dummy block size for now
			sint32	posBlockSize= ofile.getPos();
			uint32	dependBlockSize= 0;
			ofile.serial(dependBlockSize);

			// write the dictionnary
			ofile.serialCont(dictionnary);

			// write the dependencies data
			uint32 depSize = (uint32)dependencies.size();
			ofile.serial(depSize);
			std::map<NLMISC::CSheetId, std::vector<uint32> >::iterator first(dependencies.begin()), last(dependencies.end());
			for (; first != last; ++first)
			{
				NLMISC::CSheetId si = first->first;
				ofile.serial(si);
				ofile.serialCont(first->second);
			}

			// Then get the dicionary + dependencies size, and write it back to posBlockSize
			sint32	endBlockSize= ofile.getPos();
			dependBlockSize= (endBlockSize - posBlockSize) - 4;
			ofile.seek(posBlockSize, NLMISC::IStream::begin);
			ofile.serial(dependBlockSize);
			ofile.seek(endBlockSize, NLMISC::IStream::begin);

			// write the sheet data
			uint32 nbEntries = (uint32)sheetIds.size();
			uint32 ver = T::getVersion ();
			ofile.serial (nbEntries);
			ofile.serial (ver);
			ofile.serialPtrCont(container);
			ofile.close ();
		}
	}
	catch (const NLMISC::Exception &e)
	{
		nlinfo ("loadForm(): Exception during saving the packed file, it will be recreated next launch (%s)", e.what());
	}

	// housekeeping
	sheetIds.clear ();
	filenames.clear ();
}

// ***************************************************************************
// variant with smart pointers, maintain with function above
template <class T>
void loadForm2(const std::string &sheetFilter, const std::string &packedFilename, std::map<NLMISC::CSheetId, T> &container, bool updatePackedSheet=true, bool errorIfPackedSheetNotGood=true)
{
	std::vector<std::string> vs;
	vs.push_back(sheetFilter);
	loadForm2(vs, packedFilename, container, updatePackedSheet, errorIfPackedSheetNotGood);
}



// ***************************************************************************
/** This function is used to load values from georges sheet in a quick way.
 * \param sheetFilter a vector of string to filter the sheet in the case you need more than one filter
 * \param packedFilename the name of the file that this function will generate (extension must be "packed_sheets")
 * \param container the map that will be filled by this function
 */
template <class T>
void loadForm (const std::vector<std::string> &sheetFilters, const std::string &packedFilename, std::map<std::string, T> &container, bool updatePackedSheet=true, bool errorIfPackedSheetNotGood=true)
{
	std::vector<std::string>						dictionnary;
	std::map<std::string, uint>						dictionnaryIndex;
	std::map<std::string, std::vector<uint32> >		dependencies;
	std::vector<uint32>								dependencyDates;

	// check the extension (i know that file like "foo.packed_sheetsbar" will be accepted but this check is enough...)
	nlassert (packedFilename.find (".packed_sheets") != std::string::npos);

	std::string packedFilenamePath = NLMISC::CPath::lookup(packedFilename, false, false);
	if (packedFilenamePath.empty())
	{
		packedFilenamePath = packedFilename;
	}

	// make sure the CSheetId singleton has been properly initialised
//	NLMISC::CSheetId::init(updatePackedSheet);

	// load the packed sheet if exists
	try
	{
		NLMISC::CIFile ifile;
		ifile.setCacheFileOnOpen(true);
		ifile.open (packedFilenamePath);
		// an exception will be launch if the file is not the good version or if the file is not found

		nlinfo ("loadForm(): Loading packed file '%s'", packedFilename.c_str());

		// read the header
		ifile.serialCheck(PACKED_SHEET_HEADER);
		ifile.serialCheck(PACKED_SHEET_VERSION);
		ifile.serialVersion(PACKED_SHEET_VERSION_COMPATIBLE);

		// Read depend block size
		uint32	dependBlockSize;
		ifile.serial(dependBlockSize);

		// Read the dependencies only if update packed sheet
		if(updatePackedSheet)
		{
			// read the dictionnary
			{
				ifile.serialCont(dictionnary);
			}
			// read the dependency data
			{
				uint32 depSize;
				ifile.serial(depSize);
				for (uint i=0; i<depSize; ++i)
				{
					std::string sheetName;

					// Avoid copy, use []
					ifile.serial(sheetName);
					ifile.serialCont(dependencies[sheetName]);
				}
			}
		}
		// else dummy read one big block => no heavy reallocation / free
		else if(dependBlockSize>0)
		{
			std::vector<uint8>	bigBlock;
			bigBlock.resize(dependBlockSize);
			ifile.serialBuffer(&bigBlock[0], dependBlockSize);
		}

		// read the packed sheet data
		uint32	nbEntries;
		uint32	ver;
		ifile.serial (nbEntries);
		ifile.serial (ver);
		if(ver != T::getVersion ())
			throw NLMISC::Exception("The packed sheet version in stream is different of the code");
		ifile.serialCont (container);
		ifile.close ();
	}
	catch (const NLMISC::Exception &e)
	{
		// clear the container because it can contains partially loaded sheet so we must clean it before continue
		container.clear ();
		if (!updatePackedSheet)
		{
			if (errorIfPackedSheetNotGood)
				nlerror ("loadForm(): Exception during reading the packed file and can't reconstruct them (%s)", e.what());
			else
				nlinfo ("loadForm(): Exception during reading the packed file and can't reconstruct them (%s)", e.what());

			return;
		}
		else
		{
			nlinfo ("loadForm(): Exception during reading the packed file, I'll reconstruct it (%s)", e.what());
		}
	}

	// if we don't want to update packed sheet, we have nothing more to do
	if (!updatePackedSheet)
	{
		nlinfo ("Don't update the packed sheet with real sheet");
		return;
	}

	// retreive the date of all dependency file
	{
		for (uint i=0; i<dictionnary.size(); ++i)
		{
			std::string p = NLMISC::CPath::lookup (dictionnary[i], false, false);
			if (!p.empty())
			{
				uint32 d = NLMISC::CFile::getFileModificationDate(p);
				dependencyDates.push_back(d);
			}
			else
			{
				// file not found !
				// write a future date to invalidate any file dependent on it
				nldebug("Can't find dependent file %s !", dictionnary[i].c_str());
				dependencyDates.push_back(0xffffffff);
			}
		}
	}

	// build a vector of the sheetFilters sheet ids (ie: "item")
	std::vector<std::string> sheetNames;
	{
		std::vector<std::string>::const_iterator first(sheetFilters.begin()), last(sheetFilters.end());
		for (; first != last; ++first)
			NLMISC::CPath::getFileList(*first, sheetNames);

	}

	// if there's no file, nothing to do
	if (sheetNames.empty())
		return;

	// set up the current sheet in container to remove sheet that are in the container and not in the directory anymore
	std::map<std::string, bool> sheetToRemove;
	{
		typename std::map<std::string, T>::iterator first(container.begin()), last(container.end());
		for(; first != last; ++first)
			sheetToRemove.insert (make_pair(first->first, true));
	}

	// check if we need to create a new .pitems or just read it
	uint32 packedFiledate = NLMISC::CFile::getFileModificationDate(packedFilenamePath);

	bool containerChanged = false;

	NLGEORGES::UFormLoader *formLoader = NULL;

	std::vector<uint> NeededToRecompute;

	for (uint k = 0; k < sheetNames.size(); k++)
	{
		std::string p = NLMISC::CPath::lookup (sheetNames[k], false, false);
		if (p.empty())
		{
			continue;
		}
		uint32 d = NLMISC::CFile::getFileModificationDate(p);

		// no need to remove this sheet
		sheetToRemove[sheetNames[k]] = false;

		if( d > packedFiledate || container.find (sheetNames[k]) == container.end())
		{
			NeededToRecompute.push_back(k);
		}
		else
		{
			// check the date of each parent
			nlassert(dependencies.find(sheetNames[k]) != dependencies.end());
			std::vector<uint32> &depends = dependencies[sheetNames[k]];

			for (uint i=0; i<depends.size(); ++i)
			{
				if (dependencyDates[depends[i]] > packedFiledate)
				{
					nldebug("Dependency on %s for %s not up to date !",
						dictionnary[depends[i]].c_str(), sheetNames[k].c_str());
					NeededToRecompute.push_back(k);
					break;
				}
			}
		}
	}

	nlinfo ("%d sheets checked, %d need to be recomputed", sheetNames.size(), NeededToRecompute.size());

	NLMISC::TTime lastTime = NLMISC::CTime::getLocalTime ();
	NLMISC::TTime start = NLMISC::CTime::getLocalTime ();

	NLMISC::CSmartPtr<NLGEORGES::UForm> form;

	for (uint j = 0; j < NeededToRecompute.size(); j++)
	{
		if(NLMISC::CTime::getLocalTime () > lastTime + 5000)
		{
			lastTime = NLMISC::CTime::getLocalTime ();
			if(j>0)
				nlinfo ("%.0f%% completed (%d/%d), %d seconds remaining", (float)j*100.0/NeededToRecompute.size(),j,NeededToRecompute.size(), (NeededToRecompute.size()-j)*(lastTime-start)/j/1000);
		}

		// create the georges loader if necessary
		if (formLoader == NULL)
		{
			NLMISC::WarningLog->addNegativeFilter("CFormLoader: Can't open the form file");
			formLoader = NLGEORGES::UFormLoader::createLoader ();
		}

		// Load the form with given sheet id
		form = formLoader->loadForm (sheetNames[NeededToRecompute[j]].c_str ());
		if (form)
		{
			// build the dependency data
			{
				std::vector<uint32>		depends;
				std::set<std::string>	dependFiles;
				form->getDependencies (dependFiles);
				nlassert(dependFiles.find(sheetNames[NeededToRecompute[j]]) != dependFiles.end());
				// remove the sheet itself from the container
				dependFiles.erase(sheetNames[NeededToRecompute[j]]);

				std::set<std::string>::iterator first(dependFiles.begin()), last(dependFiles.end());
				for (; first != last; ++first)
				{
					std::string p = NLMISC::CPath::lookup (*first, false, false);
					if (!p.empty())
					{
//						uint32 date = NLMISC::CFile::getFileModificationDate(p);

						uint dicIndex;
						std::string filename = NLMISC::CFile::getFilename(p);

						if (dictionnaryIndex.find(filename) == dictionnaryIndex.end())
						{
							// add a new dictionnary entry
							dicIndex = (uint)dictionnary.size();
							dictionnaryIndex.insert(std::make_pair(filename, (uint)dictionnary.size()));
							dictionnary.push_back(filename);
						}
						else
						{
							dicIndex = dictionnaryIndex.find(filename)->second;
						}

						// add the dependecy index
						depends.push_back(dicIndex);
					}
				}
				// store the dependency list with the sheet ID
				dependencies[sheetNames[NeededToRecompute[j]]] = depends;
			}

			// add the new creature, it could be already loaded by the packed sheets but will be overwritten with the new one
			typedef typename std::map<std::string, T>::iterator TType1;
            typedef typename std::pair<TType1, bool> TType2;
			TType2 res = container.insert(std::make_pair(sheetNames[NeededToRecompute[j]],T()));

			(*res.first).second.readGeorges (form, sheetNames[NeededToRecompute[j]]);
			containerChanged = true;
		}
	}

	nlinfo ("%d seconds to recompute %d sheets", (uint32)(NLMISC::CTime::getLocalTime()-start)/1000, NeededToRecompute.size());

	// free the georges loader if necessary
	if (formLoader != NULL)
	{
		NLGEORGES::UFormLoader::releaseLoader (formLoader);
		NLMISC::WarningLog->removeFilter ("CFormLoader: Can't open the form file");
	}

	// we have now to remove sheet that are in the container and not exist anymore in the sheet directories
	for (std::map<std::string, bool>::iterator it2 = sheetToRemove.begin(); it2 != sheetToRemove.end(); it2++)
	{
		if(it2->second)
		{
			// inform the contained object that it is no more needed.
			container.find(it2->first)->second.removed();
			container.erase(it2->first);
			containerChanged = true;
			dependencies.erase((*it2).first);
		}
	}

	// now, save the new container in the packedfile
	try
	{
		if(containerChanged)
		{
			NLMISC::COFile ofile;
			ofile.open(packedFilenamePath);

			// write the header.
			ofile.serialCheck(PACKED_SHEET_HEADER);
			ofile.serialCheck(PACKED_SHEET_VERSION);
			ofile.serialVersion(PACKED_SHEET_VERSION_COMPATIBLE);

			// Write a dummy block size for now
			sint32	posBlockSize= ofile.getPos();
			uint32	dependBlockSize= 0;
			ofile.serial(dependBlockSize);

			// write the dictionnary
			ofile.serialCont(dictionnary);

			// write the dependencies data
			uint32 depSize = (uint32)dependencies.size();
			ofile.serial(depSize);
			std::map<std::string, std::vector<uint32> >::iterator first(dependencies.begin()), last(dependencies.end());
			for (; first != last; ++first)
			{
				std::string  sheetName = first->first;
				ofile.serial(sheetName);
				ofile.serialCont(first->second);
			}

			// Then get the dicionary + dependencies size, and write it back to posBlockSize
			sint32	endBlockSize= ofile.getPos();
			dependBlockSize= (endBlockSize - posBlockSize) - 4;
			ofile.seek(posBlockSize, NLMISC::IStream::begin);
			ofile.serial(dependBlockSize);
			ofile.seek(endBlockSize, NLMISC::IStream::begin);

			// write the sheet data
			uint32 nbEntries = (uint32)sheetNames.size();
			uint32 ver = T::getVersion ();
			ofile.serial (nbEntries);
			ofile.serial (ver);
			ofile.serialCont(container);
			ofile.close ();
		}
	}
	catch (const NLMISC::Exception &e)
	{
		nlinfo ("loadForm(): Exception during saving the packed file, it will be recreated next launch (%s)", e.what());
	}

	// housekeeping
	sheetNames.clear ();
}

// ***************************************************************************
/** This function is used to load values from georges sheet in a quick way.
 * \param sheetFilter a string to filter the sheet (ie: ".item")
 * \param packedFilename the name of the file that this function will generate (extension must be "packed_sheets")
 * \param container the map that will be filled by this function
 */
template <class T>
void loadForm (const std::string &sheetFilter, const std::string &packedFilename, std::map<std::string, T> &container, bool updatePackedSheet=true, bool errorIfPackedSheetNotGood=true)
{
	std::vector<std::string> vs;
	vs.push_back(sheetFilter);
	loadForm(vs, packedFilename, container, updatePackedSheet, errorIfPackedSheetNotGood);
}

// ***************************************************************************
template <class T>
void loadFormNoPackedSheet (const std::string &sheetFilter, std::map<NLMISC::CSheetId, T> &container, const std::string &wildcardFilter)
{
	std::vector<std::string> vs;
	vs.push_back(sheetFilter);
	loadFormNoPackedSheet(vs, container, wildcardFilter);
}

// ***************************************************************************
// variant with smart pointers, maintain with function above
template <class T>
void loadFormNoPackedSheet2 (const std::string &sheetFilter, std::map<NLMISC::CSheetId, T> &container, const std::string &wildcardFilter)
{
	std::vector<std::string> vs;
	vs.push_back(sheetFilter);
	loadFormNoPackedSheet2(vs, container, wildcardFilter);
}

// ***************************************************************************
/** This function is used to load values from georges sheet in a quick way.
 *	NB: no packedsheet is given for load/write
 * \param sheetFilters a vector of string to filter the sheet (by extension) in the case you need more than one filter
 * \param wildcardFilter an additional by sheet filter (must include the extension)
 * \param container the map that will be filled by this function
 */
template <class T>
void loadFormNoPackedSheet (const std::vector<std::string> &sheetFilters, std::map<NLMISC::CSheetId, T> &container, const std::string &wildcardFilter)
{
	// make sure the CSheetId singleton has been properly initialised
	NLMISC::CSheetId::init(false);

	// build a vector of the sheetFilters sheet ids (ie: "item")
	std::vector<NLMISC::CSheetId> sheetIds;
	std::vector<std::string> filenames;
	for (uint i = 0; i < sheetFilters.size(); i++)
		NLMISC::CSheetId::buildIdVector(sheetIds, filenames, sheetFilters[i]);


	// if there's no file, nothing to do
	if (sheetIds.empty())
		return;


	// compute sheets that needs to be recomputed
	std::vector<uint> NeededToRecompute;
	for (uint k = 0; k < filenames.size(); k++)
	{
		std::string p = NLMISC::CPath::lookup (filenames[k], false, false);
		if (p.empty()) continue;
		// check if wildcardok
		if(!wildcardFilter.empty() && !NLMISC::testWildCard(p,wildcardFilter)) continue;

		NeededToRecompute.push_back(k);
	}
	nlinfo ("%d sheets checked, %d need to be recomputed", filenames.size(), NeededToRecompute.size());


	NLMISC::TTime last = NLMISC::CTime::getLocalTime ();
	NLMISC::TTime start = NLMISC::CTime::getLocalTime ();
	NLGEORGES::UFormLoader *formLoader = NULL;
	NLMISC::CSmartPtr<NLGEORGES::UForm> form;
	std::vector<NLMISC::CSmartPtr<NLGEORGES::UForm> >	cacheFormList;

	// For all sheets need to recompute
	for (uint j = 0; j < NeededToRecompute.size(); j++)
	{
		if(NLMISC::CTime::getLocalTime () > last + 5000)
		{
			last = NLMISC::CTime::getLocalTime ();
			if(j>0)
				nlinfo ("%.0f%% completed (%d/%d), %d seconds remaining", (float)j*100.0/NeededToRecompute.size(),j,NeededToRecompute.size(), (NeededToRecompute.size()-j)*(last-start)/j/1000);
		}

		// create the georges loader if necessary
		if (formLoader == NULL)
		{
			NLMISC::WarningLog->addNegativeFilter("CFormLoader: Can't open the form file");
			formLoader = NLGEORGES::UFormLoader::createLoader ();
		}

		//	cache used to retain information (to optimize time).
		if (form)
			cacheFormList.push_back	(form);

		// Load the form with given sheet id
		form = formLoader->loadForm (sheetIds[NeededToRecompute[j]].toString().c_str ());
		if (form)
		{
			// add the new creature, it could be already loaded by the packed sheets but will be overwritten with the new one
			typedef typename std::map<NLMISC::CSheetId, T>::iterator TType1;
            typedef typename std::pair<TType1, bool> TType2;
			TType2 res = container.insert(std::make_pair(sheetIds[NeededToRecompute[j]],T()));

			(*res.first).second.readGeorges (form, sheetIds[NeededToRecompute[j]]);
		}
	}

	if(NeededToRecompute.size() > 0)
		nlinfo ("%d seconds to recompute %d sheets", (uint32)(NLMISC::CTime::getLocalTime()-start)/1000, NeededToRecompute.size());

	// free the georges loader if necessary
	if (formLoader != NULL)
	{
		NLGEORGES::UFormLoader::releaseLoader (formLoader);
		NLMISC::WarningLog->removeFilter ("CFormLoader: Can't open the form file");
	}

	// housekeeping
	sheetIds.clear ();
	filenames.clear ();
}

// ***************************************************************************
// variant with smart pointers, maintain with function above
template <class T>
void loadFormNoPackedSheet2 (const std::vector<std::string> &sheetFilters, std::map<NLMISC::CSheetId, NLMISC::CSmartPtr<T> > &container, const std::string &wildcardFilter)
{
	// make sure the CSheetId singleton has been properly initialised
	NLMISC::CSheetId::init(false);

	// build a vector of the sheetFilters sheet ids (ie: "item")
	std::vector<NLMISC::CSheetId> sheetIds;
	std::vector<std::string> filenames;
	for (uint i = 0; i < sheetFilters.size(); i++)
		NLMISC::CSheetId::buildIdVector(sheetIds, filenames, sheetFilters[i]);


	// if there's no file, nothing to do
	if (sheetIds.empty())
		return;


	// compute sheets that needs to be recomputed
	std::vector<uint> NeededToRecompute;
	for (uint k = 0; k < filenames.size(); k++)
	{
		std::string p = NLMISC::CPath::lookup (filenames[k], false, false);
		if (p.empty()) continue;
		// check if wildcardok
		if(!wildcardFilter.empty() && !NLMISC::testWildCard(p,wildcardFilter)) continue;

		NeededToRecompute.push_back(k);
	}
	nlinfo ("%d sheets checked, %d need to be recomputed", filenames.size(), NeededToRecompute.size());


	NLMISC::TTime last = NLMISC::CTime::getLocalTime ();
	NLMISC::TTime start = NLMISC::CTime::getLocalTime ();
	NLGEORGES::UFormLoader *formLoader = NULL;
	NLMISC::CSmartPtr<NLGEORGES::UForm> form;
	std::vector<NLMISC::CSmartPtr<NLGEORGES::UForm> >	cacheFormList;

	// For all sheets need to recompute
	for (uint j = 0; j < NeededToRecompute.size(); j++)
	{
		if(NLMISC::CTime::getLocalTime () > last + 5000)
		{
			last = NLMISC::CTime::getLocalTime ();
			if(j>0)
				nlinfo ("%.0f%% completed (%d/%d), %d seconds remaining", (float)j*100.0/NeededToRecompute.size(),j,NeededToRecompute.size(), (NeededToRecompute.size()-j)*(last-start)/j/1000);
		}

		// create the georges loader if necessary
		if (formLoader == NULL)
		{
			NLMISC::WarningLog->addNegativeFilter("CFormLoader: Can't open the form file");
			formLoader = NLGEORGES::UFormLoader::createLoader ();
		}

		//	cache used to retain information (to optimize time).
		if (form)
			cacheFormList.push_back	(form);

		// Load the form with given sheet id
		form = formLoader->loadForm (sheetIds[NeededToRecompute[j]].toString().c_str ());
		if (form)
		{
			// add the new creature, it could be already loaded by the packed sheets but will be overwritten with the new one
			typedef typename std::map<NLMISC::CSheetId, NLMISC::CSmartPtr<T> >::iterator TType1;
            typedef typename std::pair<TType1, bool> TType2;
			TType2 res = container.insert(std::make_pair(sheetIds[NeededToRecompute[j]], NLMISC::CSmartPtr<T>(new T())));

			(*res.first).second->readGeorges (form, sheetIds[NeededToRecompute[j]]);
		}
	}

	if(NeededToRecompute.size() > 0)
		nlinfo ("%d seconds to recompute %d sheets", (uint32)(NLMISC::CTime::getLocalTime()-start)/1000, NeededToRecompute.size());

	// free the georges loader if necessary
	if (formLoader != NULL)
	{
		NLGEORGES::UFormLoader::releaseLoader (formLoader);
		NLMISC::WarningLog->removeFilter ("CFormLoader: Can't open the form file");
	}

	// housekeeping
	sheetIds.clear ();
	filenames.clear ();
}

#endif // NL_LOAD_FORM_H

/* End of load_form.h */
