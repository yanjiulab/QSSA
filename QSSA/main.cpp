#include <QtWidgets/QApplication>
#include "QSSA.h"

//#include "algorithms.h"
//#include "asccgdal.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QSSA *ui = new QSSA;
	ui->show();
	return app.exec();
}