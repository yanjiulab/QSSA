#pragma once

// Qt Header
#include <QtWidgets>

// OpenCV Headers
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

// GDAL Headers
#include <gdal_priv.h>
#include <cpl_conv.h>
#include <gdal.h>
#include <gdal_utils.h>
#include <ogr_spatialref.h>

// C++ Standard Libraries
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

// User Headers
#include "MapLayer.h"

using namespace std;


class Submerge : public QWidget
{
	Q_OBJECT
public:
	Submerge();
	~Submerge();

	// define files
	MapLayer *m_landsat = nullptr;
	MapLayer *m_dem = nullptr;

	// define the corner points
	cv::Point2d landsat_tl;
	cv::Point2d landsat_tr;
	cv::Point2d landsat_bl;
	cv::Point2d landsat_br;

	cv::Point2d dem_bl;
	cv::Point2d dem_tr;

	// define methods used
	enum MatchMethod
	{
		BASE_GEOGCS = 0,
		BASE_PROJCS = 1,
		BASE_SIFT = 2,
		BASE_SURF = 3
	}m_matchMethod;
	enum SubmergeMethod
	{
		PASSIVE_SUBMERGING = 0,
		ACTIVE_SUBMERGING = 1
	}m_submergeMethod;

	// range of the heat map colors
	std::vector<std::pair<cv::Vec3b, double> > color_range;

	// List of all function prototypes
	cv::Point2d lerp(const cv::Point2d&, const cv::Point2d&, const double&);
	
	cv::Vec3b Submerge::lerp(
		cv::Vec3b const& minColor, 
		cv::Vec3b const& maxColor,
		double const& t);
	cv::Vec3b get_dem_color(const double&);

	//cv::Point2d pixel2utm(const int&, const int&, const cv::Size&);
	//void utm2world();

	cv::Point2d world2dem(const cv::Point2d&, const cv::Size&);

	cv::Point2d pixel2world(const int&, const int&, const cv::Size&);

	void add_color(cv::Vec3b& pix, const uchar& b, const uchar& g, const uchar& r);

	int run(const char *image_name, const char *dem_name);

	bool run();
	bool runWithGeog();

signals:
	void submergeProgress(int line);
	void submergeFinish();
};





