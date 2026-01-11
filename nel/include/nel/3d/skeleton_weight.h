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

#ifndef NL_SKELETON_WEIGHT_H
#define NL_SKELETON_WEIGHT_H

#include "nel/misc/types_nl.h"
#include <vector>

namespace NLMISC
{
class IStream;
struct EStream;
}

namespace NL3D {

/**
  * A skeleton template is a set of couple value_name / float.
  *
  * The value name is used to find a channel in the CChannelMixer.
  * The float is a weight used on a slot of the channel mixer by
  * the channel.
  *
  * \author Cyril 'Hulud' Corvazier
  * \author Nevrax France
  * \date 2001
  */
class CSkeletonWeight
{
public:
	/// An element of the template. A pair of node name / node weight.
	class CNode
	{
	public:
		/// Name of the node
		std::string Name;

		/// Weight of the node
		float Weight;

		/// Serial the node
		void serial (NLMISC::IStream& f);
	};

	/// A vector of template elements.
	typedef std::vector<CNode> TNodeArray;

	/// Get number of node in this template
	uint getNumNode () const;

	/// Get a node name
	const std::string& getNodeName (uint node) const;

	/// Get a node weight
	float getNodeWeight (uint node) const;

	/// Build the template
	void build (const TNodeArray& array);

	/// Serial the template
	void serial (NLMISC::IStream& f);

private:
	TNodeArray		_Elements;
};


} // NL3D


#endif // NL_SKELETON_WEIGHT_H

/* End of skeleton_weight.h */
