#ifndef QSSA_H
#define QSSA_H

#include <QMainWindow>
#include <QImage>
#ifndef QT_NO_PRINTER
#include <QtPrintSupport\QPrinter>
#endif

#include "MapViewer.h"
#include "MapLayerManager.h"
#include "Submerge.h"

QT_BEGIN_NAMESPACE
class QAction;
class QGroupBox;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
class QTreeView;
class QTableView;
class QGraphicsScene;
class QGraphicsView;
class QGraphicsPixmapItem;
QT_END_NAMESPACE

class QSSA : public QMainWindow
{
	Q_OBJECT

public:
	QSSA(QWidget *parent = 0);
	~QSSA();
	bool loadFile(const QString &fileName);

private slots:
	// file and edit
	void open();
	void saveAs();
	void print();
	// view and other menu
	void closeCurLayer();
	void closeAllLayers();
	void about();
	// file browser
	void fileSelected(const QModelIndex &index);
	// layer manager
	void updateLayer();
	void selectionChangedSlot(const QItemSelection & newSelection, const QItemSelection & oldSelection);
	// viewer 
	void saveLastMousePosition(const QPoint p);
	// processing
	void procHillshade();
	void procColorRelief();
	void procGDALInfo();
	void procGDALWarp();
	void procGDALTrans();
	// submerge
	void setDEM();
	void setLandsat();
	void setMatchMethod();
	void setSubMethod();
	void runSubmerge();
	void runProgress(int line);
	void runFinish();

private:
	void setupCenter();
	void setupMenuBar();
	void setupToolBar();
	void setupConnections();
	void setupStatusBar();
	void setupDockWindows();
	void setupDockBrowserWindow();
	void setupDockLayerWindow();
	void setupDockInfoWindow();
	void setupDockProcessWindow();

	void updateActions();
	bool saveFile(const QString &fileName);

	MapViewer *viewer = nullptr;
	QGraphicsScene *scene = nullptr;
	QGraphicsPixmapItem *pixmapItem = nullptr;
	MapLayerManager *layerManager = nullptr;

	QDockWidget *dockDirWindow = nullptr;
	QDockWidget *dockImgLayerWindow = nullptr;
	QDockWidget *dockImgInfoWindow = nullptr;
	QDockWidget *dockImgProcessWindow = nullptr;
	QFileSystemModel *fileModel = nullptr;
	QTreeView *dirTree = nullptr;
	QTreeView *infoTree = nullptr;
	QTreeView *layerTree = nullptr;
	// center view

	// processing buttons
	/// submerge
	Submerge *submerge = nullptr;
	/// General
	/// GDAL
	QPushButton *hillshadePushBtn = nullptr;
	QPushButton *slopePushBtn = nullptr;
	QPushButton *aspectPushBtn = nullptr;
	QPushButton *colorReliefPushBtn = nullptr;
	QPushButton *TRIPushBtn = nullptr;
	QPushButton *TPIPushBtn = nullptr;
	QPushButton *roughnessPushBtn = nullptr;

	QPushButton *gdalinfoPushBtn = nullptr;
	QPushButton *gdalwarpPushBtn = nullptr;
	QPushButton *gdaltransPushBtn = nullptr;
	/// Submerge 
	QComboBox *demList = nullptr;
	QComboBox *landsatList = nullptr;
	QComboBox *matchList = nullptr;
	QComboBox *submergeList = nullptr;
	QPushButton *submergePushBtn = nullptr;

#ifndef QT_NO_PRINTER
	QPrinter printer;
#endif

	QAction *saveAsAct = nullptr;
	QAction *printAct = nullptr;

	QAction *copyAct = nullptr;

	QAction *zoomInAct = nullptr;
	QAction *zoomOutAct = nullptr;
	QAction *normalSizeAct = nullptr;
	QAction *rotateLeftAct = nullptr;
	QAction *rotateRightAct = nullptr;
	QAction *fullScreenAct = nullptr;

	QAction *closeAllAct = nullptr;
	QAction *closeCurAct = nullptr;
	QAction *setAsDEMAct = nullptr;
	QAction *setAsLandsatAct = nullptr;
};

#endif // QSSA_H

