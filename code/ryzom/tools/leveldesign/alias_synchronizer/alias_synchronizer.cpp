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


#include "nel/misc/types_nl.h"
#include "nel/misc/config_file.h"
#include "nel/misc/path.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive_utils.h"
#include "nel/misc/algo.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;

CLigoConfig	LigoConfig;

vector<string>	Filters;


// always true predicate
struct TAllPrimitivePredicate : public std::unary_function<IPrimitive*, bool>
{
	bool operator () (IPrimitive *prim)
	{

		return true;
	}

};

bool isFiltered(const std::string &path)
{
	for (uint i=0; i<Filters.size(); ++i)
	{
		if (path.find(Filters[i]) != string::npos)
			return true;
	}

	return false;
}

void syncFile(const string &srcPath, const string &dstPath)
{
	if (isFiltered(srcPath))
		return;

	nlinfo("Synchronizing file '%s'", CFile::getFilename(srcPath).c_str());

	bool modified = false;

	CPrimitives srcDoc;
	CPrimitives dstDoc;

	// load the src primitive
	CPrimitiveContext::instance().CurrentPrimitive = &srcDoc;
	loadXmlPrimitiveFile(srcDoc, srcPath, LigoConfig);

	// load the dst primitive
	CPrimitiveContext::instance().CurrentPrimitive = &dstDoc;
	loadXmlPrimitiveFile(dstDoc, dstPath, LigoConfig);
	CPrimitiveContext::instance().CurrentPrimitive = NULL;

	// retrieve ALL the alias node in the src doc
	TPrimitiveClassPredicate pred("alias");
	TPrimitiveSet	primAlias;
	CPrimitiveSet<TPrimitiveClassPredicate>	setBuilder;

	setBuilder.buildSet(srcDoc.RootNode, pred, primAlias);
	
	// look at each matched node
	for (uint i=0; i<primAlias.size(); ++i)
	{
		CPrimAlias *srcPa = dynamic_cast<CPrimAlias*>(primAlias[i]);
		if (srcPa == NULL)
			continue;

		// build a ascii path to the prim
		string primPath = buildPrimPath(srcPa);

		// look in the dst doc for a node with this path
		TPrimitiveSet	dstAlias;
		selectPrimByPath(dstDoc.RootNode, primPath, dstAlias);

		if (dstAlias.size() == 1)
		{
			// yeah ! we found a match
			CPrimAlias *pa = dynamic_cast<CPrimAlias*>(dstAlias.front());
			if (pa != NULL && pa->getAlias() != srcPa->getAlias())
			{
//				nldebug("%s : forcing alias %u", primPath.c_str(), srcPa->getAlias());
				dstDoc.forceAlias(pa, srcPa->getAlias());
				modified = true;
			}
		}
		else if (dstAlias.size() == 0)
		{
			// try to go up one level and add a prim alias node
			while (!primPath.empty() && primPath[primPath.size()-1] != '.')
				primPath.resize(primPath.size()-1);

			if (primPath.empty())
				continue;

			// remove the '.'
			primPath.resize(primPath.size()-1);
			if (primPath.empty())
				continue;

			// ok, retry the primitive select now
			selectPrimByPath(dstDoc.RootNode, primPath, dstAlias);

			if (dstAlias.size() == 1)
			{
				IPrimitive *parent = dstAlias.front();

				CPrimAlias *pa = static_cast<CPrimAlias*> (CClassRegistry::create ("CPrimAlias"));
				CPropertyString *pname = new CPropertyString ("alias");
				pa->addPropertyByName("name", pname);
				CPropertyString *pclass = new CPropertyString ("alias");
				pa->addPropertyByName("class", pclass);

//				nldebug("%s : creating new CPrimAlias for %u", primPath.c_str(), srcPa->getAlias());
				// insert the prim alias at first pos
				CPrimitiveContext::instance().CurrentPrimitive = &dstDoc;
				parent->insertChild(pa, 0);
				dstDoc.forceAlias(pa, srcPa->getAlias());

				modified = true;
			}
			else
			{
				// multiple node match !
				// try to affect the alias if src an dst have the same number of match
				TPrimitiveSet srcAlias;
				selectPrimByPath(srcDoc.RootNode, primPath, srcAlias);

				if (dstAlias.size() == srcAlias.size())
				{
					// ok, we can match
					for (uint i=0; i<srcAlias.size(); ++i)
					{
						IPrimitive *parent = dstAlias[i];
						TPrimitiveClassPredicate	pred("alias");
						CPrimAlias *srcPa = static_cast<CPrimAlias*>(getPrimitiveChild(srcAlias[i], pred));

						CPrimAlias *pa = static_cast<CPrimAlias*> (CClassRegistry::create ("CPrimAlias"));
						CPropertyString *pname = new CPropertyString ("alias");
						pa->addPropertyByName("name", pname);
						CPropertyString *pclass = new CPropertyString ("alias");
						pa->addPropertyByName("class", pclass);

//						nldebug("%s : creating new CPrimAlias for %u", primPath.c_str(), srcPa->getAlias());
						// insert the prim alias at first pos
						CPrimitiveContext::instance().CurrentPrimitive = &dstDoc;
						parent->insertChild(pa, 0);
						dstDoc.forceAlias(pa, srcPa->getAlias());

						modified = true;

					}
				}
				else if (!dstAlias.empty())
				{
					nlwarning("In '%s', the path '%s' match to a different multiple node set, no alias restored !", 
						CFile::getFilename(srcPath).c_str(),
						primPath.c_str());
				}

			}
		}
		else
		{
			// multiple dst node mapping
			// try to affect the alias if src an dst have the same number of match
			TPrimitiveSet srcAlias;
			selectPrimByPath(srcDoc.RootNode, primPath, srcAlias);

			if (dstAlias.size() == srcAlias.size())
			{
				// ok, we can match
				for (uint i=0; i<srcAlias.size(); ++i)
				{
					CPrimAlias *pa = dynamic_cast<CPrimAlias*>(dstAlias[i]);
					CPrimAlias *srcPa = dynamic_cast<CPrimAlias*>(srcAlias[i]);
					if (pa != NULL && pa->getAlias() != srcPa->getAlias())
					{
//						nldebug("%s : forcing alias %u", primPath.c_str(), srcPa->getAlias());
						dstDoc.forceAlias(pa, srcPa->getAlias());
						modified = true;
					}

				}
			}
			else if (!dstAlias.empty())
			{
				nlwarning("In '%s', the path '%s' match to a different multiple node set, no alias restored !",
						CFile::getFilename(srcPath).c_str(),
						primPath.c_str());

			}

		}
	}

	// second loop : for each dst node, look if it need a missing CPrimAlias node and generated it if needed
	{
		TAllPrimitivePredicate pred;
		CPrimitiveEnumerator<TAllPrimitivePredicate> pe(dstDoc.RootNode, pred);
		IPrimitive *prim;

		while ((prim = pe.getNextMatch()) != NULL)
		{
			const CPrimitiveClass *pc = LigoConfig.getPrimitiveClass(*prim);
			if (pc)
			{
				// check all the static childrens
				for (uint i=0; i<pc->StaticChildren.size(); ++i)
				{
					const CPrimitiveClass::CChild &cc = pc->StaticChildren[i];

					if (cc.ClassName == "alias" && cc.Name == "alias")
					{
						bool found = false;
						// ok, this node need an alias, check if it is present
						for (uint i=0; i<prim->getNumChildren(); ++i)
						{
							IPrimitive *child;
							if (prim->getChild(child, i))
							{
								string className;
								if (child->getPropertyByName("class", className) && className == "alias")
								{
									found = true;
									break;
								}
							}
						}

						if (!found)
						{
							// ok, we need to add the alias class
							CPrimAlias *pa = static_cast<CPrimAlias*> (CClassRegistry::create ("CPrimAlias"));
							CPropertyString *pname = new CPropertyString ("alias");
							pa->addPropertyByName("name", pname);
							CPropertyString *pclass = new CPropertyString ("alias");
							pa->addPropertyByName("class", pclass);

	//						nldebug("%s : creating new CPrimAlias for %u", primPath.c_str(), srcPa->getAlias());
							// insert the prim alias at first pos
							CPrimitiveContext::instance().CurrentPrimitive = &dstDoc;
							prim->insertChild(pa, 0);

							modified = true;
						}
					}
				}
			}
		}
	}

	if (modified)
	{
		saveXmlPrimitiveFile(dstDoc, dstPath);
	}
}


struct TSetFileName
{
	void operator () (std::string &str)
	{
		str = CFile::getFilename(str);
	}
};

struct TSetLastFolder
{
	void operator () (std::string &str)
	{
		vector<string> folders;
		str = CPath::standardizePath(str, false);
#ifdef NL_OS_WINDOWS 
		explode(str, std::string("/"), folders, true);
#else // NL_OS_WINDOW
		NLMISC::splitString(str, "/", folders); 
#endif // NL_OS_WINDOWS 
		if (!folders.empty())
			str = folders.back();
		else
			str = "";
	}
};

void syncFolder(const string &srcPath, const string &dstPath)
{
	string tmp(srcPath);
	TSetLastFolder slf;
	slf.operator()(tmp);
	nlinfo("Synchronizing folder '%s'",	tmp.c_str());

	// check all the file in the current folder
	vector<string> srcFiles;
	CPath::getPathContent(srcPath, false, false, true, srcFiles);
	for_each(srcFiles.begin(), srcFiles.end(), TSetFileName());
	sort(srcFiles.begin(), srcFiles.end());

	vector<string> dstFiles;
	CPath::getPathContent(dstPath, false, false, true, dstFiles);
	for_each(dstFiles.begin(), dstFiles.end(), TSetFileName());
	sort(dstFiles.begin(), dstFiles.end());

	uint si=0; uint di=0;
	for (; si < srcFiles.size() && di < dstFiles.size(); ++si, ++di)
	{
		if (srcFiles[si] < dstFiles[di])
			--di;
		else if  (srcFiles[si] > dstFiles[di])
			--si;
		else if (srcFiles[si].find(".primitive") != string::npos)
		{
			// two file identical, synch the alias inside
			syncFile(srcPath+"/"+srcFiles[si], dstPath+"/"+dstFiles[di]);
		}
	}

	// check all the folders in the current folder
	vector<string> srcFolders;
	CPath::getPathContent(srcPath, false, true, false, srcFolders);
	for_each(srcFolders.begin(), srcFolders.end(), TSetLastFolder());
	sort(srcFolders.begin(), srcFolders.end());

	vector<string> dstFolders;
	CPath::getPathContent(dstPath, false, true, false, dstFolders);
	for_each(dstFolders.begin(), dstFolders.end(), TSetLastFolder());
	sort(dstFolders.begin(), dstFolders.end());

	si=0; di=0;
	for (; si < srcFolders.size() && di < dstFolders.size(); ++si, ++di)
	{
		if (srcFolders[si] < dstFolders[di])
			--di;
		else if  (srcFolders[si] > dstFolders[di])
			--si;
		else
		{
			// two folder identical, synch the files inside
			syncFolder(srcPath+"/"+srcFolders[si], dstPath+"/"+dstFolders[di]);
		}
	}
}


int main()
{
	new NLMISC::CApplicationContext;

	CConfigFile cf;
	
	// register ligo primitives
	Register();

	cf.load("alias_synchronizer.cfg");

	CConfigFile::CVar &paths = cf.getVar("Paths");
	CConfigFile::CVar &srcPath = cf.getVar("SrcPath");
	CConfigFile::CVar &dstPath = cf.getVar("DstPath");
	CConfigFile::CVar &filters = cf.getVar("Filters");

	// store the filters
	for (uint i=0; i<filters.size(); ++i)
	{
		Filters.push_back(filters.asString(i));
	}

	// add the search paths
	for (uint i=0; i<paths.size(); ++i)
	{
		CPath::addSearchPath(paths.asString(i), true, false);
	}

	// init ligo
	LigoConfig.readPrimitiveClass("world_editor_classes.xml", false);
	CPrimitiveContext::instance().CurrentLigoConfig = &LigoConfig;

	nlinfo("Synchronizing folder '%s' to '%s'",
		srcPath.asString().c_str(),
		dstPath.asString().c_str());

	// do a recursive scan of the src and dst folder
	syncFolder(srcPath.asString(), dstPath.asString());

	return 0;
}

