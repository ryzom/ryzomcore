#ifndef WIDGET_ADD_WATCHER
#define WIDGET_ADD_WATCHER

#include <string>

namespace NLGUI
{
	class IWidgetAdditionWatcher
	{
	public:
		virtual void widgetAdded( const std::string &name ) = 0;
	};
}

#endif

