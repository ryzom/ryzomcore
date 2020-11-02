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

#include "stdligo.h"
#include "nel/ligo/primitive_configuration.h"
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive.h"
#include "nel/misc/i_xml.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;

extern bool ReadColor (CRGBA &color, xmlNodePtr node);

// ***************************************************************************

bool CPrimitiveConfigurations::read (xmlNodePtr configurationNode, const char *filename, const char *name, NLLIGO::CLigoConfig &config)
{
	// The name
	Name = name;

	// Read the color
	ReadColor (Color, configurationNode);

	// Get the first matching pair
	MatchPairs.reserve (CIXml::countChildren (configurationNode, "MATCH_GROUP"));
	xmlNodePtr matchGroups = CIXml::getFirstChildNode (configurationNode, "MATCH_GROUP");
	if (matchGroups)
	{
		do
		{
			// Add a pair
			MatchPairs.push_back(CMatchGroup());
			CMatchGroup &matchGroup = MatchPairs.back();

			// Get the first matching pair
			matchGroup.Pairs.reserve (CIXml::countChildren (matchGroups, "MATCH"));
			xmlNodePtr match = CIXml::getFirstChildNode (matchGroups, "MATCH");
			if (match)
			{
				do
				{
					// Add the match
					matchGroup.Pairs.resize (matchGroup.Pairs.size()+1);
					std::pair<std::string, std::string> &pair = matchGroup.Pairs.back();

					// Get the match name
					std::string name;
					if (config.getPropertyString (name, filename, match, "NAME"))
					{
						pair.first = name;
					}
					else
					{
						config.syntaxError (filename, match, "Missing match name in configuration (%s)", name.c_str());
						return false;
					}

					// Get the match value
					if (config.getPropertyString (name, filename, match, "VALUE"))
					{
						pair.second = name;
					}

					match = CIXml::getNextChildNode (match, "MATCH");
				}
				while (match);
			}

			matchGroups = CIXml::getNextChildNode (matchGroups, "MATCH_GROUP");
		}
		while (matchGroups);
	}
	return true;
}

// ***************************************************************************

bool	CPrimitiveConfigurations::belong (const IPrimitive &primitive) const
{
	// For each match group
	uint group;
	const uint numGroup = (uint)MatchPairs.size();
	for (group=0; group<numGroup; group++)
	{
		const CMatchGroup &matchGroup = MatchPairs[group];

		// For each rules
		uint rules;
		const uint numRules = (uint)matchGroup.Pairs.size();
		for (rules=0; rules<numRules; rules++)
		{
			const std::pair<std::string, std::string> &pairs = matchGroup.Pairs[rules];
			string key = toLowerAscii(pairs.second);

			// Get the property
			string value;
			if (primitive.getPropertyByName (pairs.first.c_str(), value))
			{
				if (toLowerAscii(value) == key)
					continue;
			}

			// Get the property
			const std::vector<string> *array = NULL;
			if (primitive.getPropertyByName (pairs.first.c_str(), array) && array)
			{
				uint i;
				for (i=0; i<array->size(); i++)
				{
					if (toLowerAscii((*array)[i]) == key)
						break;
				}
				if (i!=array->size())
					continue;
			}

			// Don't match
			break;
		}

		// Match ?
		if (rules == numRules)
			return true;
	}
	return false;
}

// ***************************************************************************

