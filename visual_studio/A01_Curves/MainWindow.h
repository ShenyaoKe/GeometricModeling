#pragma once
#ifndef QTOGL_WINDOW_H
#define QTOGL_WINDOW_H

#include <QMainWindow>
#include "OGLViewer.h"
//#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions>
#include <QSignalMapper>

#include "ui_MainWindow.h"

class MainWindow : public QMainWindow, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

	/*virtual void render(QPainter *painter);
	virtual void render();

	virtual void initialize();

	void setAnimating(bool animating);*/

public slots:
	/*void renderLater();
	void renderNow();*/
private slots:
	void exportToFile();

protected:
	//bool event(QEvent *event) Q_DECL_OVERRIDE;
	//void exposeEvent(QExposeEvent *event) Q_DECL_OVERRIDE;

private:
	bool m_update_pending;
	bool m_animating;

	QSignalMapper *signalMapper;
	OGLViewer *m_oglviewer;
	Ui::MainWindowClass ui;
};

#endif // QTOGL_WINDOW_H
