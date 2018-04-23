#pragma once

#include "MapLayer.h"

class MapLayerManager : public QWidget
{
	Q_OBJECT
public:
	MapLayerManager();
	MapLayerManager(int max);
	~MapLayerManager();

	int maxSize;
	
	MapLayer *currentLayer;
	MapLayer *previousLayer;
	QStandardItemModel *layerModel;
	QStandardItem *rootNode;
	QHash<QString, MapLayer*> allLayers;
	QList<QString> allLayersName;

	//QList<MapLayer *> allLayer;
	//QList<QString> allLayerName;
	bool addLayer(MapLayer *lyr);
	bool removeLayer(QString lyr);
	void removeAllLayers();
	bool setCurLayer(QString curLyr);
	MapLayer *getCurLayer();
	bool updateLayerModel();

signals:
	void layerChanged();
};