#pragma once
#ifndef QTOGL_WINDOW_H
#define QTOGL_WINDOW_H

#include <QMainWindow>
#include "OGLViewer.h"
//#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions>
#include <QSignalMapper>
#include <QColor>

#include "ui_MainWindow.h"

class MainWindow : public QMainWindow, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
private slots :
	void updateCurveProp();
	void readPoints();
	void savePoints();
	void exportSVG();
	void pickColor(int idx);
protected:

private:
	bool m_update_pending;
	bool m_animating;

	QSignalMapper *signalMapper;
	OGLViewer *m_oglviewer;
	QColor m_colors[4];
	QSignalMapper *colorPickingSignals;
	Ui::MainWindowClass ui;
};

#endif // QTOGL_WINDOW_H
