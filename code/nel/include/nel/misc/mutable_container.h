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

#ifndef NL_MUTABLE_CONTAINER_H
#define NL_MUTABLE_CONTAINER_H

namespace NLMISC
{
	/** Container wrapper that allow read/write access to element stored in
	 *	a const container.
	 *	In fact, the template only allow calling begin() and end() over
	 *	a const container.
	 *	This prevent the user to change the structure of the container.
	 *	Usage :
	 *
	 *		class foo
	 *		{
	 *			typedef TMutableContainer<vector<string> > TMyCont;
	 *			TMyCont	_MyCont;
	 *
	 *		public:
	 *			// return the container with mutable item content but const item list
	 *			const TMyCont getContainer() const { return _MyCont; };
	 *		}
	 *
	 */
	template <class BaseContainer>
	struct TMutableContainer : public BaseContainer
	{
		typename BaseContainer::iterator begin() const
		{
			return const_cast<BaseContainer*>(static_cast<const BaseContainer*>(this))->begin();
		}

		typename BaseContainer::iterator end() const
		{
			return const_cast<BaseContainer*>(static_cast<const BaseContainer*>(this))->end();
		}
	};

} // namespace NLMISC

#endif // NL_MUTABLE_CONTAINER_H
