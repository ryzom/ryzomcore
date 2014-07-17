#ifndef UXT_EDITOR_H
#define UXT_EDITOR_H

#include "translation_manager_editor.h"

namespace TranslationManager
{

class UXTEditorPvt;

class UXTEditor : public CEditor
{
	Q_OBJECT
public:
	UXTEditor( QMdiArea *parent = NULL );
	~UXTEditor();
	
	void open( QString filename );
	void save();
	void saveAs( QString filename );
	void activateWindow();

protected:
	void closeEvent( QCloseEvent *e );

private:
	UXTEditorPvt *d_ptr;
};

}

#endif

