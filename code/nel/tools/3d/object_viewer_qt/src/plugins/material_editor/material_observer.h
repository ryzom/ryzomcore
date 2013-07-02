// Object Viewer Qt Material Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef MATERIAL_OBSERVER_H
#define MATERIAL_OBSERVER_H

namespace MaterialEditor
{
	/// Observes material changes
	class CMaterialObserver
	{
	public:
		CMaterialObserver(){}
		virtual ~CMaterialObserver(){}

		virtual void onNewMaterial() = 0;
		virtual void onMaterialLoaded() = 0;
		
		virtual void onPassAdded( const char *name ) = 0;
		virtual void onPassRemoved( const char *name ) = 0;
		virtual void onPassMovedUp( const char *name ) = 0;
		virtual void onPassMovedDown( const char *name ) = 0;
		virtual void onPassRenamed( const char *from, const char *to ) = 0;
	};

}


#endif

