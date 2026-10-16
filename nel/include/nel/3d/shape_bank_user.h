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

#ifndef NL_SHAPE_BANK_USER_H
#define NL_SHAPE_BANK_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_shape_bank.h"
#include "nel/3d/shape_bank.h"


namespace NL3D
{

class CDriverUser;
/**
 * Implementation of the user interface managing shape bank
 *
 * \author Matthieu Besson
 * \author Nevrax France
 * \date 2001
 */
class CShapeBankUser : public UShapeBank
{
public:
	virtual ~CShapeBankUser() NL_OVERRIDE
	{
	}

	virtual void addShapeCache(const std::string &shapeCacheName) NL_OVERRIDE;
	virtual void removeShapeCache(const std::string &shapeCacheName) NL_OVERRIDE;
	virtual void reset() NL_OVERRIDE;
	virtual void setShapeCacheSize(const std::string &shapeCacheName, sint32 maxSize) NL_OVERRIDE;
	virtual void linkShapeToShapeCache(const std::string &shapeName, const std::string &shapeCacheName) NL_OVERRIDE;
	virtual void	preLoadShapesFromDirectory(const std::string &shapeCacheName,
		const std::string &path, const std::string &wildCard, bool recurs, NLMISC::IProgressCallback *progress = NULL, bool flushTextures = false) NL_OVERRIDE;
	virtual void	preLoadShapesFromBNP(const std::string &shapeCacheName,
		const std::string &bnpName, const std::string &wildCard, NLMISC::IProgressCallback *progress = NULL, bool flushTextures = false) NL_OVERRIDE;
	virtual UShape	getShape(const std::string &shapeName) NL_OVERRIDE;
	virtual void	buildSystemGeometryForshape(const std::string &shapeName) NL_OVERRIDE;

	// The real shape bank
	CShapeBank _ShapeBank;
	CDriverUser *_DriverUser;
};


} // NL3D


#endif // NL_SHAPE_BANK_USER_H

/* End of shape_bank_user.h */
