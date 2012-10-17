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



#ifndef RY_PRIMITIVES_PARSER_H
#define RY_PRIMITIVES_PARSER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/ligo_config.h"

/**
 * Singleton used to do all the generic parsing job on primitive file
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */
class CPrimitivesParser : public NLMISC::CSingleton<CPrimitivesParser>
{
public:

	struct TLoadedPrimitive
	{
		NLLIGO::CPrimitives		Primitive;
		std::string				FileName;
	};

	typedef std::list< TLoadedPrimitive >	TPrimitivesList;

	///\return the primitives
	inline const TPrimitivesList & getPrimitives(){ return _Primitives; }
	///\return the primitives file names
//	inline const std::vector< std::string > & getPrimitiveFiles(){ return _PrimitiveFiles; }

	void init();

	static bool getAlias(const NLLIGO::IPrimitive *prim, uint32 &alias);

	static std::string aliasToString(uint32 alias);
	static uint32 aliasFromString(const std::string& alias);

	static uint32 aliasGetStaticPart(uint32 alias);

	static uint32 aliasGetDynamicPart(uint32 alias);

private:
	/// Constructor
//PrimitivesParser();

	/**
	 * load a primitive files in the specified CPrimitives class
	 * \return true if the parsing was correctly made
	 */
	bool loadPrimitive(const char* fileName,NLLIGO::CPrimitives & primitives);

	/// parsed primitives
	TPrimitivesList				_Primitives;
	/// Parsed primitive file names
//	std::vector<std::string>	_PrimitiveFiles;

	static bool					_LigoInit;
	/// Ligo config
	static NLLIGO::CLigoConfig	_LigoConfig;
};


#endif // RY_PRIMITIVES_PARSER_H

/* End of primitives_parser.h */
