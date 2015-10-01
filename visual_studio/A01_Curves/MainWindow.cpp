#include "MainWindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_update_pending(false), m_animating(false)
{
	m_oglviewer = new OGLViewer;

	ui.setupUi(this);
	ui.ogl_layout->addWidget(m_oglviewer);
	//setWindowTitle(tr("OpenGL Qt Template"));

	m_oglviewer->setFocusPolicy(Qt::StrongFocus);
	connect(ui.clear_button, SIGNAL(clicked()), m_oglviewer, SLOT(clearVertex()));
	connect(ui.curve_type, SIGNAL(currentIndexChanged(int)), m_oglviewer, SLOT(changeCurveType(int)));
	connect(ui.degree_val, SIGNAL(valueChanged(int)), m_oglviewer, SLOT(setDegree(int)));
	connect(ui.seg_val, SIGNAL(valueChanged(int)), m_oglviewer, SLOT(setSegment(int)));

	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(readPoints()));
	connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(savePoints()));
	connect(ui.actionExport, SIGNAL(triggered()), this, SLOT(exportSVG()));

	signalMapper = new QSignalMapper(this);
	connect(signalMapper, SIGNAL(mapped(int)), m_oglviewer, SLOT(changeOperation(int)));
	signalMapper->setMapping(ui.actionInsert, 0);
	signalMapper->setMapping(ui.actionMove, 1);
	connect(ui.actionInsert, SIGNAL(triggered()), signalMapper, SLOT(map()));
	connect(ui.actionMove, SIGNAL(triggered()), signalMapper, SLOT(map()));

	connect(ui.intersection_button, SIGNAL(clicked()), m_oglviewer, SLOT(findIntersections()));

	connect(ui.disp_ctrl_pts, SIGNAL(toggled(bool)), m_oglviewer, SLOT(setDispCtrlPts(bool)));
	connect(ui.disp_curves, SIGNAL(toggled(bool)), m_oglviewer, SLOT(setDispCurves(bool)));
	connect(ui.disp_intersections, SIGNAL(toggled(bool)), m_oglviewer, SLOT(setDispIntersections(bool)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::readPoints()
{
	QString filename = QFileDialog::getOpenFileName(
		this, "Open control points file...", "",tr("CPS files(*.cps)"));
	m_oglviewer->readPoints(filename.toUtf8().constData());

	ui.curve_type->setCurrentIndex(m_oglviewer->cv_type);
	ui.degree_val->setValue(m_oglviewer->curve_degree);
	ui.seg_val->setValue(m_oglviewer->curve_seg);
}

void MainWindow::savePoints()
{
	QString filename = QFileDialog::getSaveFileName(
		this, "Save control points to...", "untitled.cps", tr("Control Points Files(*.cps)"));
	m_oglviewer->writePoints(filename.toUtf8().constData());
}

void MainWindow::exportSVG()
{
	QString filename = QFileDialog::getSaveFileName(
		this, "Export Bezier Curve to...", "untitled.svg", tr("SVG files(*.svg)"));
	m_oglviewer->exportSVG(filename.toUtf8().constData());

	
}
