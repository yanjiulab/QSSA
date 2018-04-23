#include <QtWidgets>
#include <QtPrintSupport/qtprintsupportglobal.h>
#include <QtPrintSupport/QPrintDialog>

#include "QSSA.h"

QSSA::QSSA(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowTitle(tr("QSystem of Submerging Analysis"));
	setWindowState(Qt::WindowMaximized);

	setupCenter();  //Image display area.
	setupMenuBar();  //Create actions
	setupToolBar();
	setupStatusBar();
	setupDockWindows();

	setupConnections();
	// uncategoried slots;
	connect(hillshadePushBtn, &QPushButton::clicked, this, &QSSA::procHillshade);
	connect(colorReliefPushBtn, &QPushButton::clicked, this, &QSSA::procColorRelief);
	connect(gdalinfoPushBtn, &QPushButton::clicked, this, &QSSA::procGDALInfo);
	connect(submergePushBtn , &QPushButton::clicked, this, &QSSA::runSubmerge);
	connect(submerge, &Submerge::submergeProgress, this, &QSSA::runProgress);
	connect(submerge, &Submerge::submergeFinish, this, &QSSA::runFinish);

	connect(demList, SIGNAL(currentIndexChanged(int)), this, SLOT(setDEM()));
	connect(landsatList, SIGNAL(currentIndexChanged(int)), this, SLOT(setLandsat()));
	connect(matchList, SIGNAL(currentIndexChanged(int)), this, SLOT(setMatchMethod()));
	connect(submergeList, SIGNAL(currentIndexChanged(int)), this, SLOT(setSubMethod()));
}

QSSA::~QSSA()
{

}

bool QSSA::loadFile(const QString &fileName)
{
	MapLayer *layer = new MapLayer(fileName);
	if (layer->readHeader())
	{
		layer->setMetaModel();
		layer->initMatData();
		layer->readData();
		layer->getQImage(layer->m_image);

		// add layer to layer manager
		if (!layerManager->addLayer(layer)) 
		{
			QMessageBox::critical(this, tr("Error!"), tr("File %1 Already opened").arg(fileName));
			return false;
		}

		// update processing dock window
		demList->addItem(fileName);
		landsatList->addItem(fileName);

		layerManager->updateLayerModel();//应该是updateLayer（）应该做的事情，先放到这里
		emit layerManager->layerChanged();
		return true;
	}
	else
	{
		QMessageBox::critical(this, tr("Error!"), tr("Can not open file %1""%2").arg(fileName));
		return false;
	}
}

void QSSA::setupCenter()
{
	scene = new QGraphicsScene;
	viewer = new MapViewer(this);
	layerManager = new MapLayerManager;
	submerge = new Submerge;

	viewer->view()->setScene(scene);
	viewer->view()->show();

	setCentralWidget(viewer);

	//connect(viewer->view()->verticalScrollBar(), SIGNAL(valueChanged(int)),
	//	this, SLOT(Viewer->resetView()));
	//connect(viewer->view()->horizontalScrollBar(), SIGNAL(valueChanged(int)),
	//	this, SLOT(setResetButtonEnabled()));
	//connect(viewer, &MapViewer::layerChanged, this, &QSSA::setInfoModel);
}

void QSSA::setupMenuBar()
{
	// File
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

	QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &QSSA::open);
	openAct->setShortcut(QKeySequence::Open);

	saveAsAct = fileMenu->addAction(tr("&Save As..."), this, &QSSA::saveAs);
	saveAsAct->setEnabled(false);

	printAct = fileMenu->addAction(tr("&Print..."), this, &QSSA::print);
	printAct->setShortcut(QKeySequence::Print);
	printAct->setEnabled(false);

	fileMenu->addSeparator();

	QAction *exitAct = fileMenu->addAction(tr("E&xit QSSA"), this, &QWidget::close);
	exitAct->setShortcut(tr("Ctrl+Q"));

	// Edit
	QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

	// View
	QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

	zoomInAct = viewMenu->addAction(tr("Zoom &In"), viewer, &MapViewer::zoomIn);
	zoomInAct->setIcon(QIcon(":/Icons/zoomin.png"));
	zoomInAct->setAutoRepeat(true);
	zoomInAct->setEnabled(false);
	
	zoomOutAct = viewMenu->addAction(tr("Zoom &Out"), viewer, &MapViewer::zoomOut);
	zoomOutAct->setIcon(QIcon(":/Icons/zoomout.png"));
	zoomOutAct->setAutoRepeat(true);
	zoomOutAct->setEnabled(false);

	normalSizeAct = viewMenu->addAction(tr("&Zoom to Native Resolution (100%)"), viewer, &MapViewer::zoomNative);
	normalSizeAct->setIcon(QIcon(":/Icons/normalsize.png"));
	normalSizeAct->setShortcut(tr("Ctrl+F"));
	normalSizeAct->setEnabled(false);

	rotateLeftAct = viewMenu->addAction(tr("&Rotate Left"), viewer, &MapViewer::rotateLeft);
	rotateLeftAct->setIcon(QIcon(":/Icons/rotateleft.png"));
	rotateLeftAct->setEnabled(false);

	rotateRightAct = viewMenu->addAction(tr("&Rotate Left"), viewer, &MapViewer::rotateRight);
	rotateRightAct->setIcon(QIcon(":/Icons/rotateright.png"));
	rotateRightAct->setEnabled(false);

	fullScreenAct = viewMenu->addAction(tr("&Full Scene"), viewer, &MapViewer::fullScreen);
	fullScreenAct->setIcon(QIcon(":/Icons/fullscreen.png"));
	fullScreenAct->setShortcut(QKeySequence::FullScreen);
	//fullScreenAct->setEnabled(false);

	// Layer
	QMenu *layerMenu = menuBar()->addMenu(tr("&Layers"));

	closeCurAct = layerMenu->addAction(tr("&Close Current Layer"), this, &QSSA::closeCurLayer);
	closeCurAct->setEnabled(false);

	closeAllAct = layerMenu->addAction(tr("&Close All Layers"), this, &QSSA::closeAllLayers);
	closeAllAct->setEnabled(false);

	//setAsDEMAct = layerMenu->addAction(tr("&Set As DEM Dataset"), this, &QSSA::);
	//setAsDEMAct->setEnabled(false);

	//setAsLandsatAct = layerMenu->addAction(tr("&Set As Landsat Dataset"), viewer, &QSSA::closeAllLayers);
	//setAsLandsatAct->setEnabled(false);

	/// Settings
	QMenu *settingMenu = menuBar()->addMenu(tr("&Settings"));

	/// Processing
	QMenu *processMenu = menuBar()->addMenu(tr("&Processing"));

	/// Help
	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

	helpMenu->addAction(tr("&About"), this, &QSSA::about);
	helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);
}

void QSSA::setupToolBar()
{
	
	QToolBar *viewToolBar = addToolBar(tr("View"));

	viewToolBar->addAction(zoomInAct);
	viewToolBar->addAction(zoomOutAct);
	viewToolBar->addAction(normalSizeAct);
	viewToolBar->addAction(rotateLeftAct);
	viewToolBar->addAction(rotateRightAct);
	viewToolBar->addAction(fullScreenAct);
	// pass
}

void QSSA::setupConnections()
{
	connect(viewer, &MapViewer::xyCoordinates, this, &QSSA::saveLastMousePosition);
}

void QSSA::setupStatusBar()
{
	statusBar()->showMessage(tr("Welcome to QSystem of Submerging Analysis"));
}

void QSSA::setupDockBrowserWindow()
{
	/* Setup dock directory window */
	dockDirWindow = new QDockWidget(tr("Browser"), this);
	dockDirWindow->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	QFileSystemModel *model = new QFileSystemModel;
	model->setRootPath("");
	dirTree = new QTreeView(dockDirWindow);
	dirTree->setModel(model);
	dirTree->setRootIndex(model->index(""));
	dirTree->setColumnHidden(1, true);
	dirTree->setColumnHidden(2, true);
	dirTree->setColumnHidden(3, true);

	dockDirWindow->setWidget(dirTree);
	addDockWidget(Qt::LeftDockWidgetArea, dockDirWindow);
}

void QSSA::setupDockLayerWindow()
{
	/* Setup image layer dock window */
	dockImgLayerWindow = new QDockWidget(tr("Layers"), this);
	dockImgLayerWindow->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	layerTree = new QTreeView(dockImgLayerWindow);
	layerTree->setEditTriggers(0);
	layerTree->setModel(layerManager->layerModel);
	dockImgLayerWindow->setWidget(layerTree);
	addDockWidget(Qt::LeftDockWidgetArea, dockImgLayerWindow);

	QItemSelectionModel *selectionModel = layerTree->selectionModel();

	connect(layerManager, &MapLayerManager::layerChanged, this, &QSSA::updateLayer);
	connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
		this, SLOT(selectionChangedSlot(const QItemSelection &, const QItemSelection &)));
}

void QSSA::setupDockInfoWindow()
{
	/* Setup image information dock window */
	dockImgInfoWindow = new QDockWidget(tr("Image Information"), this);
	dockImgInfoWindow->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	infoTree = new QTreeView(dockImgInfoWindow);
	infoTree->setEditTriggers(0);

	dockImgInfoWindow->setWidget(infoTree);
	addDockWidget(Qt::LeftDockWidgetArea, dockImgInfoWindow);
}

void QSSA::setupDockProcessWindow()
{
	/* Setup image processing dock window */
	dockImgProcessWindow = new QDockWidget(tr("Processing"), this);
	dockImgProcessWindow->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	// Define processing base widgets
	QWidget *GDALGroupBox = new QGroupBox;
	QVBoxLayout *GDALLayout = new QVBoxLayout(GDALGroupBox);
	
	QWidget *generalGroupBox = new QGroupBox;
	QVBoxLayout *generalLayout = new QVBoxLayout(generalGroupBox);
	
	QWidget *submergeGroupBox = new QGroupBox;
	QVBoxLayout *submergeLayout = new QVBoxLayout(submergeGroupBox);

	// Define processing button widgets
	hillshadePushBtn = new QPushButton(GDALGroupBox);
	hillshadePushBtn->setText(QStringLiteral("Hillshade"));
	hillshadePushBtn->setEnabled(false);

	slopePushBtn = new QPushButton(GDALGroupBox);
	slopePushBtn->setText(QStringLiteral("Slope"));
	slopePushBtn->setEnabled(false);

	aspectPushBtn = new QPushButton(GDALGroupBox);
	aspectPushBtn->setText(QStringLiteral("Aspect"));
	aspectPushBtn->setEnabled(false);

	colorReliefPushBtn = new QPushButton(GDALGroupBox);
	colorReliefPushBtn->setText(QStringLiteral("Color-relief"));
	colorReliefPushBtn->setEnabled(false);

	TRIPushBtn = new QPushButton(GDALGroupBox);
	TRIPushBtn->setText(QStringLiteral("TRI"));
	TRIPushBtn->setEnabled(false);

	TPIPushBtn = new QPushButton(GDALGroupBox);
	TPIPushBtn->setText(QStringLiteral("TPI"));
	TPIPushBtn->setEnabled(false);

	roughnessPushBtn = new QPushButton(GDALGroupBox);
	roughnessPushBtn->setText(QStringLiteral("Roughness"));
	roughnessPushBtn->setEnabled(false);

	gdalinfoPushBtn = new QPushButton(GDALGroupBox);
	gdalinfoPushBtn->setEnabled(false);
	gdalinfoPushBtn->setText(QStringLiteral("GDAL Info"));

	demList = new QComboBox(submergeGroupBox);
	demList->setEnabled(false);

	landsatList = new QComboBox(submergeGroupBox);
	landsatList->setEnabled(false);

	matchList = new QComboBox(submergeGroupBox);
	matchList->addItem(QStringLiteral("Based GEOGCS"));
	matchList->addItem(QStringLiteral("Based PROJCS"));
	matchList->addItem(QStringLiteral("Based SIFT"));
	matchList->addItem(QStringLiteral("Based SURF"));
	matchList->setEnabled(false);

	submergeList = new QComboBox(submergeGroupBox);
	submergeList->addItem(QStringLiteral("Passive Submerging"));
	submergeList->addItem(QStringLiteral("Active Submerging"));
	submergeList->setEnabled(false);

	submergePushBtn = new QPushButton(submergeGroupBox);
	submergePushBtn->setEnabled(false);
	submergePushBtn->setText(QStringLiteral("Submerging"));
	// Construct panel
	GDALLayout->addWidget(new QLabel(QStringLiteral("GDAL DEM")));
	GDALLayout->addWidget(hillshadePushBtn, 0, Qt::AlignTop);
	GDALLayout->addWidget(slopePushBtn, 0, Qt::AlignTop);
	GDALLayout->addWidget(aspectPushBtn, 0, Qt::AlignTop);
	GDALLayout->addWidget(colorReliefPushBtn, 0, Qt::AlignTop);
	GDALLayout->addWidget(TRIPushBtn, 0, Qt::AlignTop);
	GDALLayout->addWidget(TPIPushBtn, 0, Qt::AlignTop);
	GDALLayout->addWidget(roughnessPushBtn, 0, Qt::AlignTop);
	GDALLayout->addWidget(new QLabel(QStringLiteral("GDAL Info")));
	GDALLayout->addWidget(gdalinfoPushBtn, 0, Qt::AlignTop);
	GDALLayout->addStretch();

	generalLayout->addWidget(new QLabel(QStringLiteral("Rotate horizontally")));
	generalLayout->addWidget(new QLabel(QStringLiteral("Rotate horizontally")));
	generalLayout->addStretch();

	submergeLayout->addWidget(new QLabel(QStringLiteral("DEM File")));
	submergeLayout->addWidget(demList);
	submergeLayout->addWidget(new QLabel(QStringLiteral("Landsat File")));
	submergeLayout->addWidget(landsatList);
	submergeLayout->addWidget(new QLabel(QStringLiteral("Match Method")));
	submergeLayout->addWidget(matchList);
	submergeLayout->addWidget(new QLabel(QStringLiteral("Submerging Method")));
	submergeLayout->addWidget(submergeList);
	submergeLayout->addWidget(new QLabel(QStringLiteral("Run Analysis")));
	submergeLayout->addWidget(submergePushBtn);
	submergeLayout->addStretch();

	// Combine various processing panels to a ToolBox
	QToolBox *processBox = new QToolBox(dockImgProcessWindow);
	processBox->addItem(generalGroupBox, tr("General Processing"));
	processBox->addItem(GDALGroupBox, tr("GDAL Processing"));
	processBox->addItem(submergeGroupBox, tr("Submerging Analysis"));

	dockImgProcessWindow->setWidget(processBox);
	addDockWidget(Qt::RightDockWidgetArea, dockImgProcessWindow);
}

void QSSA::setupDockWindows()
{
	setupDockBrowserWindow();
	setupDockLayerWindow();
	setupDockInfoWindow();
	setupDockProcessWindow();

	/* Set layout*/
	tabifyDockWidget(dockImgLayerWindow, dockImgInfoWindow);
	dockImgLayerWindow->raise();
}

void QSSA::updateLayer()
{
	bool has_layer = !layerManager->allLayers.isEmpty();

	if (has_layer) 
	{
		// update information dock window
		infoTree->setModel(layerManager->getCurLayer()->imgMetaModel);
		infoTree->expandAll();

		// update layer dock window
		//closeAllAct->setEnabled(true);
		//closeCurAct->setEnabled(true);

		// update central display window --> setImage()
		scene->clear();
		pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(layerManager->getCurLayer()->m_imageDraw));
		//pixmapItem->setFlags(QGraphicsPixmapItem::ItemIsSelectable | QGraphicsPixmapItem::ItemIsMovable);
		pixmapItem->setAcceptHoverEvents(true);
		scene->addItem(pixmapItem);

		//viewer
	}
	else
	{
		// destructed something ...
	}

	/// update pushbutton
	hillshadePushBtn->setEnabled(has_layer);
	colorReliefPushBtn->setEnabled(has_layer);
	gdalinfoPushBtn->setEnabled(has_layer);
	
	submergePushBtn->setEnabled(has_layer);
	demList->setEnabled(has_layer);
	landsatList->setEnabled(has_layer);
	matchList->setEnabled(has_layer);
	submergeList->setEnabled(has_layer);

	/// update actions
	saveAsAct->setEnabled(layerManager->getCurLayer());
	//copyAct->setEnabled(!image.isNull());

	zoomInAct->setEnabled(has_layer);
	zoomOutAct->setEnabled(has_layer);
	normalSizeAct->setEnabled(has_layer);
	rotateLeftAct->setEnabled(has_layer);
	rotateRightAct->setEnabled(has_layer);
}

void QSSA::selectionChangedSlot(const QItemSelection & newSelection, const QItemSelection & oldSelection)
{
	//get the text of the selected item
	const QModelIndex index = layerTree->selectionModel()->currentIndex();
	QString selectedText = index.data(Qt::DisplayRole).toString();

	if (layerManager->setCurLayer(selectedText)) {
		emit layerManager->layerChanged();
	}

	//show status bar
	statusBar()->showMessage(selectedText);
}

void QSSA::saveLastMousePosition(const QPoint p)
{

	statusBar()->showMessage(tr("Coordinate: (%1, %2)")
		.arg(p.x())
		.arg(p.y()));
}

void QSSA::setDEM()
{
	submerge->m_dem = layerManager->allLayers.value(demList->currentText());

	statusBar()->showMessage(tr("Set DEM file to %1").arg(demList->currentText()));
	/*QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
		tr("dem: %1")
		.arg(demLayer));*/
}

void QSSA::setLandsat()
{
	submerge->m_landsat = layerManager->allLayers.value(landsatList->currentText());

	statusBar()->showMessage(tr("Set landsat file to %1").arg(landsatList->currentText()));
	/*QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
		tr("landsatLayer: %1")
		.arg(landsatLayer));*/
}

void QSSA::setMatchMethod()
{
	int method = matchList->currentIndex();
	switch (method)
	{
	case 0:
		submerge->m_matchMethod = Submerge::BASE_GEOGCS;
		statusBar()->showMessage(tr("Set submerge match method to 'Geographics Coordinate System Based'."));
		break;
	case 1:
		submerge->m_matchMethod = Submerge::BASE_PROJCS;
		statusBar()->showMessage(tr("Set submerge match method to 'Projected Coordinate System Based'."));
		break;
	case 2:
		submerge->m_matchMethod = Submerge::BASE_SIFT;
		statusBar()->showMessage(tr("Set submerge match method to 'SIFT feature detector'."));
		break;
	case 3:
		submerge->m_matchMethod = Submerge::BASE_SURF;
		statusBar()->showMessage(tr("Set submerge match method to 'SURF feature detector'."));
		break;
	default:
		submerge->m_matchMethod = Submerge::BASE_GEOGCS;
		statusBar()->showMessage(tr("Set submerge match method to 'Geographics Coordinate System Based'."));
		break;
	}
	/*QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
		tr("match: %1")
		.arg(matchMethod));*/
}

void QSSA::setSubMethod()
{
	int method = submergeList->currentIndex();
	switch (method)
	{
	case 0:
		submerge->m_submergeMethod = submerge->PASSIVE_SUBMERGING;
		statusBar()->showMessage(tr("Set submerge method to 'Passive Submerging'."));
		break;
	case 1:
		submerge->m_submergeMethod = submerge->ACTIVE_SUBMERGING;
		statusBar()->showMessage(tr("Set submerge method to 'Active Submerging'."));
		break;
	default:
		submerge->m_submergeMethod = submerge->PASSIVE_SUBMERGING;
		statusBar()->showMessage(tr("Set submerge method to 'Passive Submerging'."));
		break;
	}
	/*QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
	tr("sub: %1")
	.arg(submergeMethod));*/
}

void QSSA::procHillshade()
{
	GDALDatasetH hillpoDataset = GDALDEMProcessing(
		"Data/out_hillshade.tif",
		GDALDatasetH(GDALDatasetH(layerManager->getCurLayer()->m_dataset)),
		"hillshade",
		NULL,
		GDALDEMProcessingOptionsNew(NULL, NULL),
		NULL
	);
	GDALClose(hillpoDataset);
	loadFile("Data/out_hillshade.tif");
}

void QSSA::procColorRelief()
{
	//char *papszArgv[] = { "-s", "111200", NULL };
	/*GDALDatasetH slopepoDataset = GDALDEMProcessing(
		"Data/out_slope.tif",
		GDALDatasetH(poDataset),
		"slope",
		NULL,
		GDALDEMProcessingOptionsNew(NULL, NULL),
		NULL
	);
	GDALClose(slopepoDataset);*/

	GDALDatasetH colorDataset = GDALDEMProcessing(
		"Data/out_color_relief.tif",
		GDALDatasetH(layerManager->getCurLayer()->m_dataset),
		"color-relief",
		"Config/color-relief.txt",
		GDALDEMProcessingOptionsNew(NULL, NULL),
		NULL
	);
	GDALClose(colorDataset);
	loadFile("Data/out_color_relief.tif");
}

void QSSA::procGDALInfo()
{
	//char *papszArgv[] = { "-stats" };
	/*const char *info = GDALInfo(GDALDatasetH(viewer->layerManager->getCurLayer()->m_dataset),
		GDALInfoOptionsNew(papszArgv, NULL));*/
	const char *info = GDALInfo(GDALDatasetH(layerManager->getCurLayer()->m_dataset), NULL);
	QMessageBox::information(this, tr("GDAL Information"), tr(info));
}

void QSSA::runSubmerge()
{
	submerge->run();
}

void QSSA::runProgress(int line)
{
	int total = submerge->m_landsat->m_height;
	statusBar()->showMessage(tr("Processing line = %1 , total = %2").arg(line).arg(total));
}

void QSSA::runFinish()
{
	statusBar()->showMessage(tr("Submerging analysis finished, write results to 'Data' folder."));
}

bool QSSA::saveFile(const QString &fileName)
{
	Mat imageWrite;
	cvtColor(layerManager->getCurLayer()->m_image, imageWrite, CV_RGB2BGR);
	bool is_write = imwrite(fileName.toStdString(), imageWrite);

	if (!is_write) 
	{
		QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
			tr("Cannot write %1")
			.arg(QDir::toNativeSeparators(fileName)));
		return false;
	}
	
	const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
	statusBar()->showMessage(message);
	return true;
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
	static bool firstDialog = true;

	if (firstDialog) {
		firstDialog = false;
		//const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
		const QStringList picturesLocations;
		dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
	}

	QStringList mimeTypeFilters;
	const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
		? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
	
	foreach(const QByteArray &mimeTypeName, supportedMimeTypes)
		mimeTypeFilters.append(mimeTypeName);
	mimeTypeFilters << "application/octet-stream"; // will show "All files (*)"
	mimeTypeFilters.sort();
	dialog.setMimeTypeFilters(mimeTypeFilters);
	dialog.selectMimeTypeFilter("application/octet-stream");
	if (acceptMode == QFileDialog::AcceptSave)
		dialog.setDefaultSuffix("jpg");
}

void QSSA::open()
{
	QFileDialog dialog(this, tr("Open File"));
	initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

	while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void QSSA::saveAs()
{
	QFileDialog dialog(this, tr("Save File As"));
	initializeImageFileDialog(dialog, QFileDialog::AcceptSave);

	while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
}

void QSSA::print()
{
	//not finish
}

void QSSA::closeCurLayer()
{
	layerManager->removeLayer(layerManager->getCurLayer()->m_filename);
	layerManager->updateLayerModel();
	scene->clear();
	emit layerManager->layerChanged();
}

void QSSA::closeAllLayers()
{
	layerManager->removeAllLayers();
	layerManager->updateLayerModel();
	scene->clear();
	emit layerManager->layerChanged();
}

void QSSA::about()
//! [15] //! [16]
{
	QMessageBox::about(this, tr("About Image Viewer"),
		tr("<p>The <b>Image Viewer</b> example shows how to combine QLabel "
			"and QScrollArea to display an image. QLabel is typically used "
			"for displaying a text, but it can also display an image. "
			"QScrollArea provides a scrolling view around another widget. "
			"If the child widget exceeds the size of the frame, QScrollArea "
			"automatically provides scroll bars. </p><p>The example "
			"demonstrates how QLabel's ability to scale its contents "
			"(QLabel::scaledContents), and QScrollArea's ability to "
			"automatically resize its contents "
			"(QScrollArea::widgetResizable), can be used to implement "
			"zooming and scaling features. </p><p>In addition the example "
			"shows how to use QPainter to print an image.</p>"));
}



void QSSA::updateActions()
{
	////saveAsAct->setEnabled(!image.isNull());
	////copyAct->setEnabled(!image.isNull());
	//bool has_layer = layerManager->allLayers.isEmpty();

	//zoomInAct->setEnabled(!has_layer);
	//zoomOutAct->setEnabled(!has_layer);
	//normalSizeAct->setEnabled(!has_layer);
	//rotateLeftAct->setEnabled(!has_layer);
	//rotateRightAct->setEnabled(!has_layer);

}


