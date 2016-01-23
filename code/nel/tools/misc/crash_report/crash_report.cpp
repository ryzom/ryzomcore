// Nel MMORPG framework - Error Reporter
//
// Copyright (C) 2015 Laszlo Kis-Adam
// Copyright (C) 2010 Ryzom Core <http://ryzomcore.org/>
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


#include "crash_report_widget.h"
#include <QApplication>
#include <QMessageBox>

#include <stack>
#include <vector>
#include <string>

#include "../../3d/shared_widgets/common.h"

class CCmdLineParser
{
public:
	static void parse( int argc, char **argv, std::vector< std::pair< std::string, std::string > > &v )
	{
		std::stack< std::string > stack;
		std::string key;
		std::string value;

		for( int i = argc - 1 ; i >= 0; i-- )
		{
			stack.push( std::string( argv[ i ] ) );
		}

		while( !stack.empty() )
		{
			key = stack.top();
			stack.pop();

			// If not a real parameter ( they start with '-' ), discard.
			if( key[ 0 ] != '-' )
				continue;

			// Remove the '-'
			key = key.substr( 1 );

			// No more parameters
			if( stack.empty() )
			{
				v.push_back( std::make_pair( key, "" ) );
				break;
			}

			 value = stack.top();

			 // If next parameter is a key, process it in the next iteration
			 if( value[ 0 ] == '-' )
			 {
				 v.push_back( std::make_pair( key, "" ) );
				 continue;
			 }
			 // Otherwise store the pair
			 else
			 {
				 v.push_back( std::make_pair( key, value ) );
				 stack.pop();
			 }
		}
	}
};

#ifdef QT_STATICPLUGIN

#include <QtPlugin>

#if defined(Q_OS_WIN32)
	Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#elif defined(Q_OS_MAC)
	Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
#elif defined(Q_OS_UNIX)
	Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif

	Q_IMPORT_PLUGIN(QICOPlugin)

#endif

int main(int argc, char **argv)
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	// Workaround to default -style=gtk+ on recent Cinnamon versions
	char *currentDesktop = getenv("XDG_CURRENT_DESKTOP");
	if (currentDesktop)
	{
		printf("XDG_CURRENT_DESKTOP: %s\n", currentDesktop);
		if (!strcmp(currentDesktop, "X-Cinnamon"))
		{
			setenv("XDG_CURRENT_DESKTOP", "gnome", 1);
		}
	}
#endif

	NLQT::preApplication();
	QApplication app(argc, argv);

	QApplication::setWindowIcon(QIcon(":/icons/nevraxpill.ico"));

	std::vector< std::pair< std::string, std::string > > params;

	CCmdLineParser::parse( argc, argv, params );

	CCrashReportWidget w;
	w.setup(params);
	w.show();

	int ret = app.exec();

	if(ret != EXIT_SUCCESS)
		return ret;
	else
		return w.getReturnValue();
}
