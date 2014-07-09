// Ryzom Core Studio GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef TEXTURE_CHOOSER_H
#define TEXTURE_CHOOSER_H

#include "ui_texture_chooser.h"

namespace NLMISC
{
	class CBitmap;
}

struct TextureChooserPrivate;

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
	void onFileTxtRowChanged( int row );
	void onAtlasTxtRowChanged( int row );

private:
	void setupConnections();
	void setPreviewImage( NLMISC::CBitmap &bm );

	QString selection;

	TextureChooserPrivate *d_ptr;
};

#endif

