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


#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H


#include <QSplashScreen>

class SplashScreen : public QSplashScreen
{
	Q_OBJECT
public:
	SplashScreen();
	~SplashScreen();

	void setPixmap( const QPixmap &pixmap );

	void setText( const QString &text );
	void clearText();
	void setTextXY( int x, int y ){ textX = x; textY = y; }
	void setProgress( int percent );
	void advanceProgress( int percent );

	void setProgressBarEnabled( bool b ){ progressBarEnabled = b; }
	void setProgressBarRect( int left, int top, int width, int height ){}

protected:
	void drawContents( QPainter *painter );

private:
	int progress;
	int pbLeft;
	int pbTop;
	int pbWidth;
	int pbHeight;
	
	QString text;
	int textX;
	int textY;

	bool progressBarEnabled;
};

#endif

