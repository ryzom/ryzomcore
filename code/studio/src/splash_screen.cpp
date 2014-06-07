// Ryzom Core - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#include "splash_screen.h"
#include <QPainter>
#include <QStyleOptionProgressBarV2>
#include <QCoreApplication>
#include <QPixmap>

SplashScreen::SplashScreen() :
QSplashScreen()
{
	progress = 0;
	textX = 5;
	textY = 20;
	pbLeft = 0;
	pbTop = 0;
	pbWidth = 100;
	pbHeight = 20;
}

SplashScreen::~SplashScreen()
{
}

void SplashScreen::setPixmap( const QPixmap &pixmap )
{
	QSplashScreen::setPixmap( pixmap );

	if( this->pixmap().width() > 0 )
		pbWidth = this->pixmap().width();
	
	if( this->pixmap().height() > 0 )
		pbTop = this->pixmap().height();

	textY = pbTop - pbHeight / 2;

	resize( pbWidth, pbTop + pbHeight );
}

void SplashScreen::setText( const QString &text )
{
	this->text = text;
	repaint();
	QCoreApplication::instance()->processEvents();
}

void SplashScreen::clearText()
{
	setText( "" );
}

void SplashScreen::setProgress( int percent )
{
	progress = percent;
	repaint();
	QCoreApplication::instance()->processEvents();
}

void SplashScreen::drawContents( QPainter *painter )
{
	QSplashScreen::drawContents( painter );

	if( progressBarEnabled )
	{
		QStyleOptionProgressBarV2 pbStyle;
		pbStyle.initFrom( this );
		pbStyle.state = QStyle::State_Enabled;
		pbStyle.textVisible = false;
		pbStyle.minimum = 0;
		pbStyle.maximum = 100;
		pbStyle.progress = progress;
		pbStyle.invertedAppearance = false;
		pbStyle.rect = QRect( 0, pbTop, pbWidth, pbHeight );
		
		style()->drawControl( QStyle::CE_ProgressBar, &pbStyle, painter, this );
	}

	if( !text.isEmpty() )
	{
		QPen oldPen = painter->pen();
		QPen pen;
		pen.setColor( Qt::white );
		painter->setPen( pen );
		painter->drawText( textX, textY, text );
		painter->setPen( oldPen );
	}
}


