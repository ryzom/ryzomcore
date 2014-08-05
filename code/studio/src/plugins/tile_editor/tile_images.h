#ifndef TILE_IMAGES_H
#define TILE_IMAGES_H

#include <QString>

struct TileImages
{
	QString diffuse;
	QString additive;
	QString alpha;

	void clear()
	{
		diffuse.clear();
		additive.clear();
		alpha.clear();
	}
};



#endif

