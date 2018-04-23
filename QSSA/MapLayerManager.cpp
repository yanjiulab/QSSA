#include "MapLayerManager.h"

MapLayerManager::MapLayerManager()
{
	maxSize = 3;
	currentLayer = NULL;
	previousLayer = NULL;
	layerModel = new QStandardItemModel;
	layerModel->setHorizontalHeaderLabels(QStringList() << QStringLiteral("opened"));
	rootNode = layerModel->invisibleRootItem();
}

MapLayerManager::MapLayerManager(int max)
{
	maxSize = max;
	currentLayer = NULL;
	currentLayer = NULL;
}

MapLayerManager::~MapLayerManager()
{
	allLayers.clear();
	allLayersName.clear();
}

bool MapLayerManager::addLayer(MapLayer * lyr)
{
	// test if layer exits
	QHash<QString, MapLayer *>::const_iterator i = allLayers.constBegin();
	while (i != allLayers.constEnd())
	{
		if (i.key() == lyr->m_filename) {
			return false;
		}
		++i;
	}

	// add layer
	allLayers.insert(lyr->m_filename, lyr);
	allLayersName.append(lyr->m_filename);

	// set current layer
	if (currentLayer != NULL) {
		previousLayer = currentLayer;
		currentLayer = lyr;
	}
	else {
		currentLayer = lyr;
	}

	return true;
}

bool MapLayerManager::removeLayer(QString lyr)
{	
	//relese layer pointer
	delete allLayers.value(lyr);
	allLayers[lyr] = NULL;

	// remove layer
	if (allLayers.remove(lyr)) {
		currentLayer = previousLayer;
		return true;
	}
	return false;
}

void MapLayerManager::removeAllLayers()
{
	allLayers.clear();
	currentLayer = NULL;
	previousLayer = NULL;
}

bool MapLayerManager::setCurLayer(QString curLyr)
{
	QHash<QString, MapLayer *>::const_iterator i = allLayers.constBegin();
	while (i != allLayers.constEnd()) 
	{
		if (i.value() == allLayers.value(curLyr)) {
			currentLayer = i.value();
			return true;
		}
		++i;
	}
	return false;
}

MapLayer * MapLayerManager::getCurLayer()
{
	return currentLayer;
}

bool MapLayerManager::updateLayerModel()
{
	{
		// clear all model
		//rootNode->removeRows(0, allLayers.size());

		//// add layer model based in allLayers
		//QHash<QString, MapLayer *>::const_iterator i = allLayers.constBegin();
		//while (i != allLayers.constEnd())
		//{
		//	QStandardItem *layerNameItem = new QStandardItem(i.key());
		//	rootNode->appendRow(layerNameItem);

		//	for (int j = 1; j <= i.value()->m_channels; ++j)
		//	{
		//		QStandardItem *layerBandItem = new QStandardItem(tr("Band %1").arg(j));
		//		layerNameItem->appendRow(layerBandItem);
		//	}

		//	++i;
		//}
	}

	QStandardItem *layerNameItem = new QStandardItem(currentLayer->m_filename);
	rootNode->appendRow(layerNameItem);

	for (int j = 1; j <= currentLayer->m_channels; ++j)
	{
		QStandardItem *layerBandItem = new QStandardItem(tr("band %1").arg(j));
		layerNameItem->appendRow(layerBandItem);
	}
	return true;
}
