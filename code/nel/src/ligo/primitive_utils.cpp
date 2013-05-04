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
#include "nel/ligo/primitive_utils.h"


namespace NLLIGO
{

std::string buildPrimPath(const IPrimitive *prim)
{
	std::string path;

	while (prim != NULL)
	{
		std::string name;
		prim->getPropertyByName("name", name);
		if (path.empty())
			path = name;
		else
			path = name + "." + path;

		prim = prim->getParent();
	}
	return path;
}

void selectPrimByPath(IPrimitive *rootNode, const std::string &path, TPrimitiveSet &result)
{
	std::vector<std::string>	parts;
	NLMISC::explode(path, std::string("."), parts, false);
//	IPrimitive * tmpChild;

	result.clear();

	if (parts.empty())
		return;

	// handle a special case
	if (parts.size() > 1 && parts[1] == "primitive")
	{
		parts[0] += ".primitive";
		parts.erase(parts.begin()+1);
	}

	TPrimitiveSet	candidats, nextStep;
	candidats.push_back(rootNode);

	// check root validity
	std::string name;
	rootNode->getPropertyByName("name", name);
	if (name != parts.front())
		return;

	for (uint i=1; i<parts.size(); ++i)
	{
		for (uint j=0; j<candidats.size(); ++j)
		{
			for (uint k=0; k<candidats[j]->getNumChildren(); ++k)
			{
				std::string name;
				IPrimitive *child;
				candidats[j]->getChild(child, k);

				child->getPropertyByName("name", name);

				if (name == parts[i])
				{
					nextStep.push_back(child);
				}
			}
		}

		candidats.swap(nextStep);
		nextStep.clear();
	}

	result.swap(candidats);

//	for (uint i=0; i<parts.size(); ++i)
//	{
//		for (uint j=0; j<candidats.size(); ++j)
//		{
//			std::string tmpName;
//			std::vector<std::string> name;
//			candidats[j]->getPropertyByName("name", tmpName);
//			NLMISC::explode(tmpName,".",name);
//
//			bool test=false;
//			for(uint k=0;k<name.size();k++)
//			{
//				if (name.at(k)==parts[i+k])
//					test=true;
//				else
//				{
//					test=false;
//					break;
//				}
//			}
//			if (test)
//			{
//				if (i == parts.size()-1)
//				{
//				}
//				else
//				{
//					for(uint k=0;k<candidats[j]->getNumChildren();k++)
//					{
//						candidats[j]->getChild(tmpChild,k);
//						nextStep.push_back(tmpChild);
//					}
//				}
////				result.clear();
////				result.push_back(candidats[j]);
//				i+=name.size()-1;
//				break;
//			}
//
//		}
//
//		candidats.swap(nextStep);
//		nextStep.clear();
//
//		if (candidats.empty())
//			return;
//	}

	// store the result
//	result.swap(candidats);
	//result.push_back(candidats.at(0)->getParent());
}

} // namespace NLLIGO



