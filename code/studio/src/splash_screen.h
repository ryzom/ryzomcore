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

