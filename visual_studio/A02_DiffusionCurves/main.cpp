#include "MainWindow.h"

#include <QtWidgets/QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
	QApplication qtApp(argc, argv);
	QString apppath = qtApp.applicationDirPath();
	QFile File("stylesheet.qss");
	File.open(QFile::ReadOnly);

	QString StyleSheet = QLatin1String(File.readAll());
	qtApp.setStyleSheet(StyleSheet);

	QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
	format.setVersion(4, 4);
	format.setProfile(QSurfaceFormat::CoreProfile);
	QSurfaceFormat::setDefaultFormat(format);

	MainWindow w;
	w.show();
	/*QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
	format.setVersion(4, 5);
	format.setProfile(QSurfaceFormat::CoreProfile);
	QSurfaceFormat::setDefaultFormat(format);*/
	return qtApp.exec();
}
