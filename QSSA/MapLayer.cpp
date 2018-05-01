#include "MapLayer.h"

/**
* Convert GDAL Palette Interpretation to OpenCV Pixel Type
*/
int  gdalPaletteInterpretation2OpenCV(GDALPaletteInterp const& paletteInterp, GDALDataType const& gdalType) {

	switch (paletteInterp) {

		/// GRAYSCALE
	case GPI_Gray:
		if (gdalType == GDT_Byte) { return CV_8UC1; }
		if (gdalType == GDT_UInt16) { return CV_16UC1; }
		if (gdalType == GDT_Int16) { return CV_16SC1; }
		if (gdalType == GDT_UInt32) { return CV_32SC1; }
		if (gdalType == GDT_Int32) { return CV_32SC1; }
		if (gdalType == GDT_Float32) { return CV_32FC1; }
		if (gdalType == GDT_Float64) { return CV_64FC1; }
		return -1;

		/// RGB
	case GPI_RGB:
		if (gdalType == GDT_Byte) { return CV_8UC3; }
		if (gdalType == GDT_UInt16) { return CV_16UC3; }
		if (gdalType == GDT_Int16) { return CV_16SC3; }
		if (gdalType == GDT_UInt32) { return CV_32SC3; }
		if (gdalType == GDT_Int32) { return CV_32SC3; }
		if (gdalType == GDT_Float32) { return CV_32FC3; }
		if (gdalType == GDT_Float64) { return CV_64FC3; }
		return -1;

		/// otherwise
	default:
		return -1;

	}
}

/**
* Convert gdal type to opencv type
*/
int gdal2opencv(const GDALDataType& gdalType, const int& channels) {

	switch (gdalType) {

		/// UInt8
	case GDT_Byte:
		return CV_8UC(channels);

		/// UInt16
	case GDT_UInt16:
		return CV_16UC(channels);

		/// Int16
	case GDT_Int16:
		return CV_16SC(channels);

		/// UInt32
	case GDT_UInt32:
	case GDT_Int32:
		return CV_32SC(channels);

	case GDT_Float32:
		return CV_32FC(channels);

	case GDT_Float64:
		return CV_64FC(channels);

	default:
		std::cout << "Unknown GDAL Data Type" << std::endl;
		std::cout << "Type: " << GDALGetDataTypeName(gdalType) << std::endl;
		return -1;
	}
}

/**
* Convert data range
*/
double range_cast(const GDALDataType& gdalType, const int& cvDepth, const double& value)
{
	// uint8 -> uint8
	if (gdalType == GDT_Byte && cvDepth == CV_8U) {
		return value;
	}
	// uint8 -> uint16
	if (gdalType == GDT_Byte && (cvDepth == CV_16U || cvDepth == CV_16S)) {
		return (value * 256);
	}

	// uint8 -> uint32
	if (gdalType == GDT_Byte && (cvDepth == CV_32F || cvDepth == CV_32S)) {
		return (value * 16777216);
	}

	// int16 -> uint8
	if ((gdalType == GDT_UInt16 || gdalType == GDT_Int16) && cvDepth == CV_8U) {
		return std::floor(value / 256.0);
	}

	// int16 -> int16
	if ((gdalType == GDT_UInt16 || gdalType == GDT_Int16) &&
		(cvDepth == CV_16U || cvDepth == CV_16S)) {
		return value;
	}

	// float32 -> float32
	// float64 -> float64
	if ((gdalType == GDT_Float32 || gdalType == GDT_Float64) &&
		(cvDepth == CV_32F || cvDepth == CV_64F)) {
		return value;
	}

	std::cout << GDALGetDataTypeName(gdalType) << std::endl;
	std::cout << "warning: unknown range cast requested." << std::endl;
	return (value);
}

/**
* There are some better mpl techniques for doing this.
*/
void write_pixel(const double& pixelValue,
	const GDALDataType& gdalType,
	const int& gdalChannels,
	Mat& image,
	const int& row,
	const int& col,
	const int& channel) 
{

	// convert the pixel
	double newValue = range_cast(gdalType, image.depth(), pixelValue);

	// input: 1 channel, output: 1 channel
	if (gdalChannels == 1 && image.channels() == 1) {
		if (image.depth() == CV_8U) { image.ptr<uchar>(row)[col] = newValue; }
		else if (image.depth() == CV_16U) { image.ptr<unsigned short>(row)[col] = newValue; }
		else if (image.depth() == CV_16S) { image.ptr<short>(row)[col] = newValue; }
		else if (image.depth() == CV_32S) { image.ptr<int>(row)[col] = newValue; }
		else if (image.depth() == CV_32F) { image.ptr<float>(row)[col] = newValue; }
		else if (image.depth() == CV_64F) { image.ptr<double>(row)[col] = newValue; }
		else { throw std::runtime_error("Unknown image depth, gdal: 1, img: 1"); }
	}

	// input: 1 channel, output: 3 channel
	else if (gdalChannels == 1 && image.channels() == 3) {
		if (image.depth() == CV_8U) { image.ptr<Vec3b>(row)[col] = Vec3b(newValue, newValue, newValue); }
		else if (image.depth() == CV_16U) { image.ptr<Vec3s>(row)[col] = Vec3s(newValue, newValue, newValue); }
		else if (image.depth() == CV_16S) { image.ptr<Vec3s>(row)[col] = Vec3s(newValue, newValue, newValue); }
		else if (image.depth() == CV_32S) { image.ptr<Vec3i>(row)[col] = Vec3i(newValue, newValue, newValue); }
		else if (image.depth() == CV_32F) { image.ptr<Vec3f>(row)[col] = Vec3f(newValue, newValue, newValue); }
		else if (image.depth() == CV_64F) { image.ptr<Vec3d>(row)[col] = Vec3d(newValue, newValue, newValue); }
		else { throw std::runtime_error("Unknown image depth, gdal:1, img: 3"); }
	}

	// input: 3 channel, output: 1 channel
	else if (gdalChannels == 3 && image.channels() == 1) {
		if (image.depth() == CV_8U) { image.ptr<uchar>(row)[col] += (newValue / 3.0); }
		else { throw std::runtime_error("Unknown image depth, gdal:3, img: 1"); }
	}

	// input: 4 channel, output: 1 channel
	else if (gdalChannels == 4 && image.channels() == 1) {
		if (image.depth() == CV_8U) { image.ptr<uchar>(row)[col] = newValue; }
		else { throw std::runtime_error("Unknown image depth, gdal: 4, image: 1"); }
	}

	// input: 3 channel, output: 3 channel
	else if (gdalChannels == 3 && image.channels() == 3) {
		if (image.depth() == CV_8U) { (*image.ptr<Vec3b>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_16U) { (*image.ptr<Vec3s>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_16S) { (*image.ptr<Vec3s>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_32S) { (*image.ptr<Vec3i>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_32F) { (*image.ptr<Vec3f>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_64F) { (*image.ptr<Vec3d>(row, col))[channel] = newValue; }
		else { throw std::runtime_error("Unknown image depth, gdal: 3, image: 3"); }
	}

	// input: 4 channel, output: 3 channel
	else if (gdalChannels == 4 && image.channels() == 3) {
		if (channel >= 4) { return; }
		else if (image.depth() == CV_8U && channel < 4) { (*image.ptr<Vec3b>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_16U && channel < 4) { (*image.ptr<Vec3s>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_16S && channel < 4) { (*image.ptr<Vec3s>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_32S && channel < 4) { (*image.ptr<Vec3i>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_32F && channel < 4) { (*image.ptr<Vec3f>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_64F && channel < 4) { (*image.ptr<Vec3d>(row, col))[channel] = newValue; }
		else { throw std::runtime_error("Unknown image depth, gdal: 4, image: 3"); }
	}

	// input: 4 channel, output: 4 channel
	else if (gdalChannels == 4 && image.channels() == 4) {
		if (image.depth() == CV_8U) { (*image.ptr<Vec4b>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_16U) { (*image.ptr<Vec4s>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_16S) { (*image.ptr<Vec4s>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_32S) { (*image.ptr<Vec4i>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_32F) { (*image.ptr<Vec4f>(row, col))[channel] = newValue; }
		else if (image.depth() == CV_64F) { (*image.ptr<Vec4d>(row, col))[channel] = newValue; }
		else { throw std::runtime_error("Unknown image depth, gdal: 4, image: 4"); }
	}

	// input: > 4 channels, output: > 4 channels
	else if (gdalChannels > 4 && image.channels() > 4) {
		if (image.depth() == CV_8U) { image.ptr<uchar>(row, col)[channel] = newValue; }
		else if (image.depth() == CV_16U) { image.ptr<unsigned short>(row, col)[channel] = newValue; }
		else if (image.depth() == CV_16S) { image.ptr<short>(row, col)[channel] = newValue; }
		else if (image.depth() == CV_32S) { image.ptr<int>(row, col)[channel] = newValue; }
		else if (image.depth() == CV_32F) { image.ptr<float>(row, col)[channel] = newValue; }
		else if (image.depth() == CV_64F) { image.ptr<double>(row, col)[channel] = newValue; }
		else { throw std::runtime_error("Unknown image depth, gdal: N, img: N"); }
	}
	// otherwise, throw an error
	else {
		throw std::runtime_error("error: can't convert types.");
	}

}


void write_ctable_pixel(const double& pixelValue,
	const GDALDataType& gdalType,
	GDALColorTable const* gdalColorTable,
	Mat& image,
	const int& y,
	const int& x,
	const int& c)
{
	if (gdalColorTable == NULL) {
		write_pixel(pixelValue, gdalType, 1, image, y, x, c);
	}

	// if we are Grayscale, then do a straight conversion
	if (gdalColorTable->GetPaletteInterpretation() == GPI_Gray) {
		write_pixel(pixelValue, gdalType, 1, image, y, x, c);
	}

	// if we are rgb, then convert here
	else if (gdalColorTable->GetPaletteInterpretation() == GPI_RGB)
	{

		/*short p;
		switch (c)
		{
		case 0:
			p = gdalColorTable->GetColorEntry((int)pixelValue)->c1;
			break;
		case 1:
			p = gdalColorTable->GetColorEntry((int)pixelValue)->c2;
			break;
		case 2:
			p = gdalColorTable->GetColorEntry((int)pixelValue)->c3;
			break;
		case 3:
			p = gdalColorTable->GetColorEntry((int)pixelValue)->c4;
			break;
		default:
			break;
		}
		write_pixel(p, gdalType, 4, image, y, x, c);*/
		// get the pixel
		short r = gdalColorTable->GetColorEntry((int)pixelValue)->c1;
		short g = gdalColorTable->GetColorEntry((int)pixelValue)->c2;
		short b = gdalColorTable->GetColorEntry((int)pixelValue)->c3;
		short a = gdalColorTable->GetColorEntry((int)pixelValue)->c4;


		write_pixel(r, gdalType, 4, image, y, x, 0);
		write_pixel(g, gdalType, 4, image, y, x, 1);
		write_pixel(b, gdalType, 4, image, y, x, 2);
		if (image.channels() > 3) {
			write_pixel(a, gdalType, 4, image, y, x, 1);
		}
	}

	// otherwise, set zeros
	else {
		write_pixel(pixelValue, gdalType, 1, image, y, x, c);
	}

}


MapLayer::MapLayer()
{
}

MapLayer::MapLayer(const QString fileName)
{	
	/// Register the driver
	GDALAllRegister();
	m_driver = NULL;
	m_dataset = NULL;

	imgMetaModel = new QStandardItemModel;
	m_filename = fileName;
}

MapLayer::~MapLayer()
{
	if (m_dataset != NULL)
	{
		GDALClose((GDALDatasetH)m_dataset);
		m_dataset = NULL;
		m_driver = NULL;
	}
}

QList<QStandardItem*> MapLayer::prepareRow(const QString & first, const QString & second)
{
	QList<QStandardItem *> rowItems;
	rowItems << new QStandardItem(first);
	rowItems << new QStandardItem(second);
	return rowItems;
}

void MapLayer::initMatData()
{
	m_image.create(m_height, m_width, m_cvType);
}

bool MapLayer::readHeader()
{
	// load the dataset
	m_dataset = (GDALDataset *)GDALOpen(m_filename.toStdString().c_str(), GA_ReadOnly);

	// if dataset is null, then there was a problem
	if (m_dataset == NULL) { return false; }

	//extract the driver infomation
	m_driver = m_dataset->GetDriver();

	// if the driver failed, then exit
	if (m_driver == NULL) { return false; }

	// get the image dimensions
	m_width = m_dataset->GetRasterXSize();
	m_height = m_dataset->GetRasterYSize();
	m_channels = m_dataset->GetRasterCount();

	// make sure we have at least one band/channel
	if (m_channels <= 0) { return false; }

	// get the image projection reference 

	// get the image origin(upper left coordinate) and pixel size
	if (m_dataset->GetGeoTransform(m_adfGeoTransform) == CE_None)
	{
		m_origin = QPair<double, double>(m_adfGeoTransform[0], m_adfGeoTransform[3]);
		m_pixelSize = QPair<double, double>(m_adfGeoTransform[1], m_adfGeoTransform[5]);
	}

	// check if we have a color palette
	int tempType;
	if (m_dataset->GetRasterBand(1)->GetColorInterpretation() == GCI_PaletteIndex)
	{
		// remember that we have a color palette
		hasColorTable = true;

		// if the color tables does not exist, then we failed
		if (m_dataset->GetRasterBand(1)->GetColorTable() == NULL) { return false; }

		// otherwise, get the pixeltype
		else
		{
			// convert the palette interpretation to opencv type
			tempType = gdalPaletteInterpretation2OpenCV(
				m_dataset->GetRasterBand(1)->GetColorTable()->GetPaletteInterpretation(),
				m_dataset->GetRasterBand(1)->GetRasterDataType());

			if (tempType == -1) {
				return false;
			}
			m_cvType = tempType;
		}

	}
	// otherwise, we have standard channels
	else
	{
		// remember that we don't have a color table
		hasColorTable = false;

		// convert the datatype to opencv
		tempType = gdal2opencv(m_dataset->GetRasterBand(1)->GetRasterDataType(),
			m_dataset->GetRasterCount());
		if (tempType == -1) {
			return false;
		}
		m_cvType = tempType;
	}

	// get bands information of the dataset(block, min, max, type...)
	GDALRasterBand *band;
	QPair<int, int> nBlockSize;
	int bGotMin, bGotMax;
	double adfMinMax[2];
	for (int i = 1; i <= m_dataset->GetRasterCount(); ++i)
	{
		band = m_dataset->GetRasterBand(i);
		band->GetBlockSize(&nBlockSize.first, &nBlockSize.second);
		adfMinMax[0] = band->GetMinimum(&bGotMin);
		adfMinMax[1] = band->GetMaximum(&bGotMax);
		GDALComputeRasterMinMax((GDALRasterBandH)band, TRUE, adfMinMax);

		m_bands << band;
		m_min << adfMinMax[0];
		m_max << adfMinMax[1];
		m_block << nBlockSize;
		m_gdType << band->GetRasterDataType();//GDALGetDataTypeName(m_type);
		m_colorInterp << band->GetColorInterpretation();//GDALGetColorInterpretationName(m_colorInterp);
	}
	return true;
}

void MapLayer::setMetaModel()
{
	// Getting basic dataset information
	QStandardItem *rootNode = imgMetaModel->invisibleRootItem();
	imgMetaModel->setHorizontalHeaderLabels(QStringList() << QStringLiteral("Property") << QStringLiteral("Value"));

	QStandardItem *driverItem = new QStandardItem("Driver");
	QList<QStandardItem *> providerRow = prepareRow("Provider", "GDAL");
	QList<QStandardItem *> descriptionRow = prepareRow("Description", m_driver->GetDescription());
	QList<QStandardItem *> metadataRow = prepareRow("Metadata", m_driver->GetMetadataItem(GDAL_DMD_LONGNAME));
	rootNode->appendRow(driverItem);
	driverItem->appendRow(providerRow);
	driverItem->appendRow(descriptionRow);
	driverItem->appendRow(metadataRow);

	QStandardItem *sizeItem = new QStandardItem("Size");
	QList<QStandardItem *> widthRow = prepareRow("Width", QString::number(m_width));
	QList<QStandardItem *> heightRow = prepareRow("Height", QString::number(m_height));
	QList<QStandardItem *> bandCountRow = prepareRow("Band Counts", QString::number(m_channels));
	rootNode->appendRow(sizeItem);
	sizeItem->appendRow(widthRow);
	sizeItem->appendRow(heightRow);
	sizeItem->appendRow(bandCountRow);

	QStandardItem *projectionItem = new QStandardItem("Projection");
	rootNode->appendRow(projectionItem);
	if (m_dataset->GetProjectionRef() != NULL)
	{
		QList<QStandardItem *> refRow = prepareRow("Projection", m_dataset->GetProjectionRef());
		projectionItem->appendRow(refRow);
	}

	QStandardItem *geoTransItem = new QStandardItem("GeoTransform");
	rootNode->appendRow(geoTransItem);
	if (m_dataset->GetGeoTransform(m_adfGeoTransform) == CE_None)
	{
		QList<QStandardItem *> originRow = prepareRow("Origin", QString::number(m_origin.first) + tr(", ") + QString::number(m_origin.second));
		QList<QStandardItem *> pixelSizeRow = prepareRow("Pixel Size",QString::number(m_pixelSize.first) + tr(", ") + QString::number(m_pixelSize.second));
		geoTransItem->appendRow(originRow);
		geoTransItem->appendRow(pixelSizeRow);
	}
	else
	{
		QList<QStandardItem *> originRow = prepareRow("Origin", "NULL");
		QList<QStandardItem *> pixelSizeRow = prepareRow("Pixel Size", "NULL");
		geoTransItem->appendRow(originRow);
		geoTransItem->appendRow(pixelSizeRow);
	}

	// Getting bands information of the dataset
	for (int i = 0; i < m_dataset->GetRasterCount(); ++i)
	{
		QStandardItem *bandItem = new QStandardItem(tr("Band %1").arg(i+1));
		rootNode->appendRow(bandItem);

		QList<QStandardItem *> blockRow = prepareRow("Block", QString::number(m_block.at(i).first) + " * " + QString::number(m_block.at(i).second));
		QList<QStandardItem *> dataTypeRow = prepareRow("Data Type", GDALGetDataTypeName(m_gdType.at(i)));
		QList<QStandardItem *> colorRow = prepareRow("Color Interpretation", GDALGetColorInterpretationName(m_colorInterp.at(i)));
		bandItem->appendRow(blockRow);
		bandItem->appendRow(dataTypeRow);
		bandItem->appendRow(colorRow);

		QList<QStandardItem *> minMaxRow = prepareRow("Min / Max", 
			QString::number(m_min.at(i)) + " / " + QString::number(m_max.at(i)));
		bandItem->appendRow(minMaxRow);
	}
}

bool MapLayer::readData()
{
	// set the image to zero
	m_image = 0;

	// iterate over each raster band
	// note that OpenCV does bgr rather than rgb

	GDALColorTable* gdalColorTable = NULL;
	if (m_dataset->GetRasterBand(1)->GetColorTable() != NULL) {
		gdalColorTable = m_dataset->GetRasterBand(1)->GetColorTable();
	}

	const GDALDataType gdalType = m_dataset->GetRasterBand(1)->GetRasterDataType();
	
	int nRows, nCols;
	for (int c = 0; c < m_channels; c++) {

		// get the GDAL Band
		GDALRasterBand* band = m_dataset->GetRasterBand(c + 1);

		/* Map palette band and gray band to color index 0 and red, green,
		blue, alpha bands to BGRA indexes. Note: ignoring HSL, CMY,
		CMYK, and YCbCr color spaces, rather than converting them
		to BGR. */
		int color = 0;
		switch (band->GetColorInterpretation()) {
		case GCI_PaletteIndex:
		case GCI_GrayIndex:
		case GCI_RedBand:
			color = 0;
			break;
		case GCI_GreenBand:
			color = 1;
			break;
		case GCI_BlueBand:
			color = 2;
			break;
		case GCI_AlphaBand:
			color = 3;
			break;
		default:
			return false;
		}

		// make sure the image band has the same dimensions as the image
		if (band->GetXSize() != m_width || band->GetYSize() != m_height) { return false; }

		// grab the raster size
		nRows = band->GetYSize();
		nCols = band->GetXSize();

		// create a temporary scanline pointer to store data
		double* scanline = new double[nCols];

		// iterate over each row and column
		for (int y = 0; y < nRows; y++) {

			// get the entire row
			CPLErr err = band->RasterIO(GF_Read, 0, y, nCols, 1, scanline, nCols, 1, GDT_Float64, 0, 0);
			CV_Assert(err == CE_None);

			// set inside the image
			for (int x = 0; x < nCols; x++) {

				// set depending on image types
				//   given boost, I would use enable_if to speed up.  Avoid for now.
				if (hasColorTable == false) {
					write_pixel(scanline[x], gdalType, m_channels, m_image, y, x, color);
				}
				else {
					write_ctable_pixel(scanline[x], gdalType, gdalColorTable, m_image, y, x, color);
				}
			}
		}

		// delete our temp pointer
		delete[] scanline;
	}

	return true;
}

//bool MapLayer::getQImage()
//{
//	if (m_image.channels() == 1)
//	{
//		if (m_image.depth() == CV_8U) {
//			m_imageDraw = QImage(m_image.data, m_image.cols, m_image.rows, m_image.step, QImage::Format_Grayscale8);
//			return true;
//		}
//		else {
//			m_imageDraw = QImage(m_image.data, m_image.cols, m_image.rows, m_image.step, QImage::Format_Grayscale8);
//			/*QMessageBox::information(this, tr("Error!"),
//				tr("Sorry, Qt now only supports displaying 8-bit grayscale image."
//				   "You can use 'tools' to convert the image."));*/
//			
//			return true;
//		}
//	}
//	else if (m_image.channels() == 3)
//	{
//		if (m_image.depth() != CV_8U) {
//			QMessageBox::information(this, tr("Note!"),
//				tr("Formats with more than 8 bit per color channel will only be processed by the raster engine using 8 bit per color."));
//			return false;
//		}
//
//		m_imageDraw = QImage(m_image.data, m_image.cols, m_image.rows, m_image.step, QImage::Format_RGB888);
//		return true;
//	}
//	else if (m_image.channels() == 4)
//	{
//		if (m_image.depth() != CV_8U) {
//			QMessageBox::information(this, tr("Note!"),
//				tr("Formats with more than 8 bit per color channel will only be processed by the raster engine using 8 bit per color."));
//			return false;
//		}
//
//		m_imageDraw = QImage(m_image.data, m_image.cols, m_image.rows, m_image.step, QImage::Format_RGBA8888);
//		return true;
//	}
//	else 
//	{
//		QMessageBox::critical(this, tr("Error!"), tr("Unknown QImage format"));
//		return false;
//	}
//}

QImage MapLayer::getQImage()
{
	QImage imageDraw;
	if (m_image.channels() == 1)
	{
		imageDraw = QImage(m_image.data, m_image.cols, m_image.rows, m_image.step, QImage::Format_Grayscale8);
		return imageDraw;
		//if (image.depth() == CV_8U) {
		//	imageDraw = QImage(image.data, image.cols, image.rows, image.step, QImage::Format_Grayscale8);
		//	return imageDraw;
		//}
		//else {
		//	m_imageDraw = QImage(m_image.data, m_image.cols, m_image.rows, m_image.step, QImage::Format_Grayscale8);
		//	/*QMessageBox::information(this, tr("Error!"),
		//	tr("Sorry, Qt now only supports displaying 8-bit grayscale image."
		//	"You can use 'tools' to convert the image."));*/
		//	return imageDraw;
		//}
	}
	else if (m_image.channels() == 3)
	{
		if (m_image.depth() != CV_8U) {
			QMessageBox::information(this, tr("Note!"),
				tr("Formats with more than 8 bit per color channel will only be processed by the raster engine using 8 bit per color."));
			return QImage();
		}

		imageDraw = QImage(m_image.data, m_image.cols, m_image.rows, m_image.step, QImage::Format_RGB888);
		return imageDraw;
	}
	else if (m_image.channels() == 4)
	{
		if (m_image.depth() != CV_8U) {
			QMessageBox::information(this, tr("Note!"),
				tr("Formats with more than 8 bit per color channel will only be processed by the raster engine using 8 bit per color."));
			return QImage();
		}

		m_imageDraw = QImage(m_image.data, m_image.cols, m_image.rows, m_image.step, QImage::Format_RGBA8888);
		return imageDraw;
	}
	else
	{
		QMessageBox::critical(this, tr("Error!"), tr("Unknown QImage format"));
		return QImage();
	}
}

QImage MapLayer::cvt16bTo8b(cv::Mat &src)
{
	// define table
	/*uchar table[256];
	for (int i = 0; i < 256; ++i)
		table[i] = (uchar)((i / m_max[0]) * 256);*/
	// copy src to dst mat
	Mat dst = src.clone();
	const int channels = dst.channels();
	switch (channels)
	{
	case 1:
	{
		MatIterator_<short> it, end;
		for (it = dst.begin<short>(), end = dst.end<short>(); it != end; ++it)
			*it = (uchar)((*it) / m_max[0] * 255);
		break;
	}
	case 3:
	{
		MatIterator_<Vec3s> it, end;
		for (it = dst.begin<Vec3s>(), end = dst.end<Vec3s>(); it != end; ++it)
		{
			(*it)[0] = (uchar)((*it) / m_max[0] * 255)[0];
			(*it)[1] = (uchar)((*it) / m_max[1] * 255)[1];
			(*it)[2] = (uchar)((*it) / m_max[2] * 255)[2];
		}
	}
	}
	/*imwrite("Data/Output/8bit.jpg", dst);*/
	QImage imageDraw = QImage(dst.data, dst.cols, dst.rows, src.step, QImage::Format_Grayscale8);
	return imageDraw;
}