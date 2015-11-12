#include "MainWindow.h"

#include <QtWidgets/QApplication>
#include <QFile>

int main(int argc, char *argv[])
{

	QApplication qtApp(argc, argv);
	QFile File("stylesheet.qss");
	File.open(QFile::ReadOnly);

	QString StyleSheet = QLatin1String(File.readAll());
	qtApp.setStyleSheet(StyleSheet);
	MainWindow w;
	w.show();

	return qtApp.exec();
}
