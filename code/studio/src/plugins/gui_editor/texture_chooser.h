#ifndef TEXTURE_CHOOSER_H
#define TEXTURE_CHOOSER_H

#include "ui_texture_chooser.h"

class TextureChooser : public QDialog, public Ui::TextureChooser
{
	Q_OBJECT

public:
	TextureChooser( QDialog *parent = NULL );
	~TextureChooser();

	void load();
	QString getSelection(){ return selection; }

public Q_SLOTS:
	void accept();
	void reject();

private Q_SLOTS:
	void onCurrentRowChanged( int row );

private:
	void setupConnections();

	QString selection;
};

#endif

