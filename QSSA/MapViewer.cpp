//#include <QtWidgets>
//#include "MapViewer.h"
//
//MapViewer::MapViewer(QWidget *parent)
//	: QGraphicsView(parent)
//{
//	
//}
//MapViewer::~MapViewer()
//{
//
//}
//
//void MapViewer::wheelEvent(QWheelEvent * event)
//{
//	if (event->modifiers() & Qt::ControlModifier) {
//		if (event->delta() > 0)
//			zoomIn(6);
//		else
//			zoomOut(6);
//		event->accept();
//	}
//	else {
//		QGraphicsView::wheelEvent(event);
//	}
//}
//
//void MapViewer::mousePressEvent(QMouseEvent * event)
//{
//}
//
//void MapViewer::mouseMoveEvent(QMouseEvent * event)
//{
//}
//
//void MapViewer::mouseReleaseEvent(QMouseEvent * event)
//{
//}
//
//void MapViewer::zoomIn(int level)
//{
//	zoomSlider += level;
//}
//
//
//void MapViewer::zoomOut(int level)
//{
//	zoomSlider->setValue(zoomSlider->value() - level);
//}
//
//void MapViewer::normalSize()
//{
//	m_zoomSlider = 250;
//	m_rotateSlider = 0;
//	setupMatrix();
//	viewer->ensureVisible(QRectF(0, 0, 0, 0));
//
//	normalSizeAct->setEnabled(false);
//}
//
//void MapViewer::rotateLeft()
//{
//	rotateSlider->setValue(rotateSlider->value() - 10);
//}
//
//void MapViewer::rotateRight()
//{
//	rotateSlider->setValue(rotateSlider->value() + 10);
//}
//
//void MapViewer::setupMatrix()
//{
//	qreal scale = qPow(qreal(2), (m_zoomSlider - 250) / qreal(50));
//
//	QMatrix matrix;
//	matrix.scale(scale, scale);
//	matrix.rotate(m_rotateSlider);
//
//	setMatrix(matrix);
//	setResetButtonEnabled();
//}

#include <QtWidgets>
#include "MapViewer.h"
#include <qmath.h>

void GraphicsView::wheelEvent(QWheelEvent *e)
{
	if (e->modifiers() & Qt::ControlModifier) {
		if (e->delta() > 0)
			view->zoomIn(5);
		else
			view->zoomOut(5);
		e->accept();
	}
	else {
		QGraphicsView::wheelEvent(e);
	}
}

MapViewer::MapViewer(QWidget *parent)
	: QFrame(parent)
{
	setFrameStyle(Sunken | StyledPanel);
	graphicsView = new GraphicsView(this);
	graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
	graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
	graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
	graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	graphicsView->setAlignment(Qt::AlignCenter);

	graphicsView->setMouseTracking(true);

	// setup slider 
	zoomSlider = new QSlider;
	zoomSlider->setMinimum(0);
	zoomSlider->setMaximum(500);
	zoomSlider->setValue(250);
	zoomSlider->setTickPosition(QSlider::TicksRight);

	rotateSlider = new QSlider;
	rotateSlider->setOrientation(Qt::Horizontal);
	rotateSlider->setMinimum(-360);
	rotateSlider->setMaximum(360);
	rotateSlider->setValue(0);
	rotateSlider->setTickPosition(QSlider::TicksBelow);

	// setup slider layout
	QGridLayout *topLayout = new QGridLayout;
	topLayout->addWidget(graphicsView, 0, 0);
	topLayout->addWidget(zoomSlider, 0, 1);
	topLayout->addWidget(rotateSlider, 1, 0);
	setLayout(topLayout);

	// Rotate slider layout
	connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
	connect(rotateSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));

	setupMatrix();
}

QGraphicsView *MapViewer::view() const
{
	return static_cast<QGraphicsView *>(graphicsView);
}

void MapViewer::mouseDoubleClickEvent(QMouseEvent * event)
{
	Q_UNUSED(event);
	fullScreen();
}

void MapViewer::keyPressEvent(QKeyEvent * event)
{
	if (event->modifiers() == Qt::ControlModifier) {
		
		if (event->key() == Qt::Key_F)
			zoomNative();
	}
	else QWidget::keyPressEvent(event);   
}

void MapViewer::zoomNative()
{
	zoomSlider->setValue(250);
	rotateSlider->setValue(0);
	setupMatrix();
	graphicsView->ensureVisible(QRectF(0, 0, 0, 0));
	//graphicsView->centerOn(0, 0);
}

void MapViewer::fullScreen()
{
	if (this->isFullScreen()) {      //if full screen, then convert to non-full screen
		this->setWindowFlags(Qt::SubWindow);
		this->showNormal();
	}
	else {                                 //if non-full screen, then convert to full screen
		this->setWindowFlags(Qt::Dialog);
		this->setWindowFlags(Qt::Window);
		this->showFullScreen();
	}
}


void MapViewer::setupMatrix()
{
	qreal scale = qPow(qreal(2), (zoomSlider->value() - 250) / qreal(50));

	QMatrix matrix;
	matrix.scale(scale, scale);
	matrix.rotate(rotateSlider->value());

	graphicsView->setMatrix(matrix);

}

void MapViewer::print()
{

	//QPrinter printer;
	//QPrintDialog dialog(&printer, this);
	//if (dialog.exec() == QDialog::Accepted) {
	//	QPainter painter(&printer);
	//	graphicsView->render(&painter);
	//}

}

void MapViewer::zoomIn(int level)
{
	level = 5;
	zoomSlider->setValue(zoomSlider->value() + level);
}

void MapViewer::zoomOut(int level)
{
	level = 5;
	zoomSlider->setValue(zoomSlider->value() - level);
}

void MapViewer::rotateLeft()
{
	rotateSlider->setValue(rotateSlider->value() - 10);
}

void MapViewer::rotateRight()
{
	rotateSlider->setValue(rotateSlider->value() + 10);
}
