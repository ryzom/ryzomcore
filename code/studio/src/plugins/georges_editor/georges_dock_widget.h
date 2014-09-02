#ifndef GEORGES_DOCK_WIDGET
#define GEORGES_DOCK_WIDGET

#include <QDockWidget>

class QUndoStack;

class GeorgesDockWidget : public QDockWidget
{
public:
	GeorgesDockWidget( QWidget *parent = NULL );
	~GeorgesDockWidget();

	void setUndoStack( QUndoStack *stack ){ m_undoStack = stack; }

	bool isModified() const{ return m_modified; }
	void setModified( bool b ){ m_modified = b; }

	QString fileName() const{ return m_fileName; }

	virtual void write() = 0;

protected:
	QString buildLogMsg( const QString &msg );
	virtual void log( const QString &msg ) = 0;

	QString m_fileName;
	bool m_modified;
	QUndoStack *m_undoStack;
};

#endif
