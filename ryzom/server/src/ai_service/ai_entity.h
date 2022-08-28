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

#ifndef RYAI_ENTITY_H
#define RYAI_ENTITY_H

#include <string>
#include <vector>

//--------------------------------------------------------------------------
// The CAIEntiy class
//--------------------------------------------------------------------------
class CAIEntity;
class CAIInstance;

extern void removeFromWatch(CAIEntity* entity);

class CAIEntity
{
public:
	/// @name Destructor
	//@{
	virtual ~CAIEntity();
	//@}
	
	/// @name Virtual interface
	//@{
	virtual CAIInstance* getAIInstance() const = 0;
	// display verbose status information regarding the entity 
//	virtual void display(CStringWriter& stringWriter) const;
//	virtual std::string getIndexString() const = 0;
	virtual std::string	getOneLineInfoString() const;
	virtual std::vector<std::string> getMultiLineInfoString() const;
	//@}
	
public:
	/// @name Debug info display methods
	//@{
	// public interface for the 'buildDebugString' virtual
	// method set - it is safe, returning cleanly if called on a NULL pointer
//	std::string debugString(uint idx = 0) const;
	//@}
};

inline
CAIEntity::~CAIEntity()
{
	removeFromWatch(this);
}

inline
std::string	CAIEntity::getOneLineInfoString() const
{
	return std::string("<no debug info available for this entity>");
}

inline
std::vector<std::string> CAIEntity::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	MULTI_LINE_FORMATER::pushTitle(container, "CAIEntity");
	MULTI_LINE_FORMATER::pushEntry(container, "<default multi line info>");
	MULTI_LINE_FORMATER::pushEntry(container, getOneLineInfoString());
	MULTI_LINE_FORMATER::pushFooter(container);
	return container;
}

#endif
