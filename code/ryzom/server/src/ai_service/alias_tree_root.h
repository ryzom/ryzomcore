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

#ifndef ALIAS_TREE_ROOT_H
#define ALIAS_TREE_ROOT_H

#include "nel/misc/string_mapper.h"
#include "child_container.h"
#include <string>

class CAliasTreeRoot
{
	typedef std::vector<NLMISC::TStringId> TStringIdList;
	TStringIdList _RelatedFiles;
	
public:
	class CMarkTagForDelete
	{
	public:
		CMarkTagForDelete(NLMISC::TStringId const& fileId);
		virtual ~CMarkTagForDelete() { }
		void operator()(CAliasTreeRoot* const treeRoot) const;
	private:
		NLMISC::TStringId const& _fileId;
	};
	
	template <class T>
	class CDeleteTagged
	{
	public:
		CDeleteTagged(CAliasCont<T>& container)
		: _Container(container)
		{
		}
		virtual ~CDeleteTagged()
		{
		}
		void operator()(T* const treeRoot) const
		{
			if (!treeRoot)
				return;
			
			if (!treeRoot->isRegisteredByFiles())
				this->_Container.removeChildByIndex(treeRoot->getChildIndex());
		}
	private:
		CCont<T>& _Container;
	};
	
public:
	CAliasTreeRoot(std::string const& filename);
	CAliasTreeRoot(NLMISC::TStringId filename);
	
	void registerForFile(NLMISC::TStringId const& filename);
	void registerForFile(std::string const& filename);
	bool isRegisteredForFile(NLMISC::TStringId const& filename) const;
	void unRegisterForFile(NLMISC::TStringId const& filename);
	
	bool isRegisteredByFiles() const;
};

inline
CAliasTreeRoot::CAliasTreeRoot(std::string const& filename)
{
	NLMISC::TStringId const stringId = NLMISC::CStringMapper::map(filename);
	if (isRegisteredForFile(stringId))
		return;
	registerForFile(stringId);
}

inline
CAliasTreeRoot::CAliasTreeRoot(NLMISC::TStringId filename)
{
	if (isRegisteredForFile(filename))
		return;
	registerForFile(filename);
}

inline
void CAliasTreeRoot::registerForFile(NLMISC::TStringId const& filename)
{
	_RelatedFiles.push_back(filename);
}

inline
void CAliasTreeRoot::registerForFile(std::string const& filename)
{
	NLMISC::TStringId const stringId = NLMISC::CStringMapper::map(filename);
	registerForFile(stringId);
}

inline
bool CAliasTreeRoot::isRegisteredForFile(NLMISC::TStringId const& filename) const
{
	for (TStringIdList::const_iterator it=_RelatedFiles.begin(), itEnd=_RelatedFiles.end(); it!=itEnd; ++it)
		if (*it==filename)
			return true;
		return false;
}

inline
void CAliasTreeRoot::unRegisterForFile(NLMISC::TStringId const& filename)
{
#if !FINAL_VERSION
	nlassert(isRegisteredForFile(filename));
#endif
	for (TStringIdList::iterator it=_RelatedFiles.begin(), itEnd=_RelatedFiles.end(); it!=itEnd; ++it)
	{
		if (*it!=filename)
			continue;
		
		*it = _RelatedFiles.back();
		_RelatedFiles.pop_back();
		return;
	}
}

inline
bool CAliasTreeRoot::isRegisteredByFiles() const
{
	return _RelatedFiles.size()!=0;
}

inline
CAliasTreeRoot::CMarkTagForDelete::CMarkTagForDelete(NLMISC::TStringId const& fileId)
:_fileId(fileId)
{
}

inline
void CAliasTreeRoot::CMarkTagForDelete::operator()(CAliasTreeRoot* const treeRoot) const
{
	if (!treeRoot)
		return;
	
	if (treeRoot->isRegisteredForFile(_fileId))
		treeRoot->unRegisterForFile(_fileId);
}

#endif
