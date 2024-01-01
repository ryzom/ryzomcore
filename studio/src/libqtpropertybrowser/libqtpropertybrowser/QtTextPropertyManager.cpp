#include <libqtpropertybrowser/QtTextPropertyManager.h>

#if defined(Q_CC_MSVC)
#    pragma warning(disable: 4786) /* MS VS 6: truncating debug info after 255 characters */
#endif

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

QString QtTextPropertyManager::valueText(const QtProperty *property) const
{
	QString text = QtStringPropertyManager::valueText(property);
	for (int i = 0; i < text.size(); i++)
	{
		if (text.at(i) == '\n')
		{
			QStringRef ret(&text, 0, i);
			return ret.toString() + " ...";
		}
	}
	return text;
}

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif
