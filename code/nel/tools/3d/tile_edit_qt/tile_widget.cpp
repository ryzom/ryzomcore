#include <QtGui/QtGui>
#include "tile_widget.h"

 CTile_Widget::CTile_Widget(QWidget *parent)
     : QWidget(parent)

 {
	 ui.setupUi(this);
 }


void CTile_Widget::initWidget(const QPixmap& pixmap, const int pixmapSide, const QString& label)
{
	int nbPixel = pixmapSide;
	this->resize( nbPixel + (PIXMAP_MARGIN * 2), nbPixel + (PIXMAP_MARGIN * 2) + ui.tileLabel->height());

	ui.tilePixmapLabel->resize(pixmapSide, pixmapSide);
	ui.tilePixmapLabel->move(PIXMAP_MARGIN, PIXMAP_MARGIN);
	ui.tilePixmapLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	
	ui.tileLabel->setText(label);
	ui.tileLabel->setToolTip(label);
	ui.tileLabel->resize(nbPixel, ui.tileLabel->height());
	ui.tileLabel->move(PIXMAP_MARGIN, nbPixel + PIXMAP_MARGIN * 2);
	ui.tileLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	if(!pixmap.isNull())
	{
		ui.tilePixmapLabel->setPixmap(pixmap);
		ui.tilePixmapLabel->setFrameShape(QFrame::NoFrame);
	}
}
