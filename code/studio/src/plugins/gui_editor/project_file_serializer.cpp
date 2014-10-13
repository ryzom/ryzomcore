// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#include "project_file_serializer.h"


namespace GUIEditor
{
	bool CProjectFileSerializer::serialize( const SProjectFiles &project )
	{
		if( fileName.empty() )
			return false;

		if( project.version >= SProjectFiles::MAX_PROJECTFILE_VERSION )
			return false;

		out.open( fileName.c_str() );
		if( !out.is_open() )
			return false;

		out << "<project>" << std::endl;

		if( !serializeHeader( project ) )
		{
			out.close();
			return false;
		}

		if( !serializeGUIFiles( project ) )
		{
			out.close();
			return false;
		}

		if( !serializeMapFiles( project ) )
		{
			out.close();
			return false;
		}

		out << "</project>" << std::endl;

		out.close();

		return true;
	}

	bool CProjectFileSerializer::serializeHeader( const SProjectFiles &project )
	{
		if( !out.good() )
			return false;

		out << '\t' << "<header>" << std::endl;
		out << "\t\t" << "<name>" << project.projectName << "</name>" << std::endl;
		out << "\t\t" << "<version>" << project.version << "</version>" << std::endl;
		out << "\t\t" << "<mastergroup>" << project.masterGroup << "</mastergroup>" << std::endl;
		out << "\t\t" << "<activegroup>" << project.activeGroup << "</activegroup>" << std::endl;
		out << '\t' << "</header>" << std::endl;

		if( !out.good() )
			return false;

		return true;
	}

	bool CProjectFileSerializer::serializeGUIFiles( const SProjectFiles &project )
	{
		if( !out.good() )
			return false;

		out << '\t' << "<guifiles>" << std::endl;

		std::vector< std::string >::const_iterator itr;
		for( itr = project.guiFiles.begin(); itr != project.guiFiles.end(); ++itr )
		{
			out << "\t\t" << "<file>" << *itr << "</file>" << std::endl; 
		}

		out << '\t' << "</guifiles>" << std::endl;

		if( !out.good() )
			return false;

		return true;
	}

	bool CProjectFileSerializer::serializeMapFiles( const SProjectFiles &project )
	{
		if( !out.good() )
			return false;

		out << '\t' << "<mapfiles>" << std::endl;

		std::vector< std::string >::const_iterator itr;
		for( itr = project.mapFiles.begin(); itr != project.mapFiles.end(); ++itr )
		{
			out << "\t\t" << "<file>" << *itr << "</file>" << std::endl; 
		}

		out << '\t' << "</mapfiles>" << std::endl;

		if( !out.good() )
			return false;

		return true;
	}
}

