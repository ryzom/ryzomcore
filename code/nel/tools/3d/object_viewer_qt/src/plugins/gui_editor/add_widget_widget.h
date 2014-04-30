#ifndef ADD_WIDGET_WIDGET_H
#define ADD_WIDGET_WIDGET_H

#include "ui_add_widget_widget.h"

namespace GUIEditor
{
	class CWidgetInfoTree;

	class AddWidgetWidget : public QWidget, public Ui::AddWidgetWidget
	{
		Q_OBJECT
	public:
		AddWidgetWidget( QWidget *parent = NULL );
		~AddWidgetWidget();

		void setCurrentGroup( const QString &g );
		void setupWidgetInfo( const CWidgetInfoTree *tree );

	private:
		void setupConnections();

	private Q_SLOTS:
		void onAddClicked();

	Q_SIGNALS:
		void adding( const QString &parentGroup, const QString &widgetType, const QString &name );
	};

}

#endif
