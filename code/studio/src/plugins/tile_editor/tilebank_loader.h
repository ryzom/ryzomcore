#ifndef TILEBANK_LOADER_H
#define TILEBANK_LOADER_H

#include "land.h"

class TileModel;
class TileBankLoaderPvt;

class TileBankLoader
{
public:
	TileBankLoader();
	~TileBankLoader();

	bool load( const char *filename, TileModel *model, QList< Land > &lands );

private:
	TileBankLoaderPvt *p;
};


#endif

