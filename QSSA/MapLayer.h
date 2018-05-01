#pragma once

// Qt Headers
#include <QtWidgets>

// GDAL Headers
#include <gdal_priv.h>
#include <cpl_conv.h>
#include <gdal.h>
#include <gdal_utils.h>

// OpenCV Headers
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h>

// C++ Standard Libraries
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

// using namespace
using namespace std;
using namespace cv;

/**
* Convert GDAL Palette Interpretation to OpenCV Pixel Type
*/
int gdalPaletteInterpretation2OpenCV(GDALPaletteInterp const& paletteInterp,
	GDALDataType const& gdalType);

/**
* Convert a GDAL Raster Type to OpenCV Type
*/
int gdal2opencv(const GDALDataType& gdalType, const int& channels);

class MapLayer : public QWidget
{
	Q_OBJECT
public:
	MapLayer();
	MapLayer(const QString fileName);
	~MapLayer();

	GDALDataset* m_dataset;/// GDAL Dataset			
	GDALDriver* m_driver;/// GDAL Driver
	QString m_filename;/// Filename 	 
	int m_width;
	int m_height;
	int m_channels;

	double m_adfGeoTransform[6];
	QPair<double, double> m_origin;
	QPair<double, double> m_pixelSize;

	bool hasColorTable;/// Check if we are reading from a color table

	QList<GDALRasterBand *> m_bands;/// GDAL Band 
	QList<double> m_min;
	QList<double> m_max;
	QList<QPair<int, int>> m_block;
	QList<GDALDataType> m_gdType;
	QList<GDALColorInterp> m_colorInterp;
	int m_cvType;

	Mat m_image;
	QImage m_imageDraw;

	QStandardItemModel *imgMetaModel;
	QList<QStandardItem *> prepareRow(const QString &first, const QString &second);
	
	void initMatData();
	bool readHeader();
	bool readData();
	void setMetaModel();
	//bool getQImage();
	QImage getQImage();
	QImage cvt16bTo8b(cv::Mat &src);
	

};

