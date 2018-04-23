#ifndef MAPVIEWER_H
#define MAPVIEWER_H

#include <QFrame>
#include <QGraphicsView>

QT_BEGIN_NAMESPACE
class QSlider;
QT_END_NAMESPACE

class MapViewer;

class GraphicsView : public QGraphicsView
{
	Q_OBJECT
public:
	GraphicsView(MapViewer *v) : QGraphicsView(), view(v) { }

protected:
	void wheelEvent(QWheelEvent *) override;

private:
	MapViewer *view;
};

class MapViewer : public QFrame
{
	Q_OBJECT
public:
	explicit MapViewer(QWidget *parent = 0);

	QGraphicsView *view() const;

protected:
	void mouseDoubleClickEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);

public slots:
	void zoomIn(int level = 5);
	void zoomOut(int level = 5);
	void zoomNative();
	void fullScreen();
	void setupMatrix();
	void print();
	void rotateLeft();
	void rotateRight();

private:
	GraphicsView *graphicsView;
	QSlider *zoomSlider;
	QSlider *rotateSlider;
signals:
	void xyCoordinates(const QPoint &p);
};

#endif // MAPVIEWER_H