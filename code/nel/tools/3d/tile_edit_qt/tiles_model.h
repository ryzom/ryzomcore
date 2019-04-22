#ifndef TILESMODEL_H
#define TILESMODEL_H

#include <QtCore/QtCore>
#include <QtGui/QtGui>

class TileModel
{

public:
	TileModel();
	TileModel(int pixmapSide, QString tileLabel, int index);
	TileModel(const QPixmap &pixmap, QString tileLabel, int index);

	int getPixmapSide() const {	return pixmapSide;	};
	QPixmap getPixmap() const {	return pixmap;	};
	QString getTileLabel() const {	return tileLabel;	};
	int getIndex() const {	return index;	};

private:
	int pixmapSide;
	QPixmap pixmap;
	QString tileLabel;
	int index;
};



class tiles_model : public QAbstractListModel
{
    Q_OBJECT

public:
    tiles_model(QObject *parent = 0);

	Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool removeRows(int row, int count, const QModelIndex &parent);
    int rowCount(const QModelIndex &parent) const;
	
	void sort ( int column = 0, Qt::SortOrder order = Qt::AscendingOrder );

    void addTile(const TileModel &tile);

	void removeAllTiles();

private:
	QList<TileModel> tiles;
};

#endif
