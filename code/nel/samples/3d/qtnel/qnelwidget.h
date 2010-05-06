#ifndef NL_QTNEL_H
#define NL_QTNEL_H

#include <QGLWidget>
#include <QX11Info>

#include <nel/3d/scene.h>
#include <nel/3d/font_manager.h>
#include <nel/3d/computed_string.h>
#include <nel/3d/text_context.h>
#include <nel/3d/driver_user.h>

/** A QWidget specialised for embedding an Nel window. */
class QNelWidget : public QGLWidget
{
	Q_OBJECT

///--QWidget Methods------------------------------------------------------------
public:

	QNelWidget(QWidget *parent = 0) : QGLWidget( parent ),_SceneRoot(NULL)
	{
		// Initialize NeL
		init();
	}

	// Do nothing.
	QNelWidget(const QNelWidget &copy) : QGLWidget(copy.parentWidget()) { }

	virtual ~QNelWidget ()
	{
		// Destroy NeL.
		// ...

		destroy();
	}

	virtual void swapBuffers();
protected:
	virtual void initializeGL();
	virtual void resizeGL( int, int );
	virtual void paintGL();


	/// Initialize NeL
	void init();

	NL3D::CScene *_SceneRoot;
	NL3D::CFontManager fontManager;
	NL3D::CTextContext tc;
};

#endif // NL_QTNEL_H
