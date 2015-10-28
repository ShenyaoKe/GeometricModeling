#include "MainWindow.h"

#include <QFileDialog>
#include <QColorDialog>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_update_pending(false), m_animating(false)
	, m_fgColor(255, 255, 255), m_bgColor(0, 0, 0)
{
	m_oglviewer = new OGLViewer;

	ui.setupUi(this);
	ui.ogl_layout->addWidget(m_oglviewer);
	//setWindowTitle(tr("OpenGL Qt Template"));

	m_oglviewer->setFocusPolicy(Qt::StrongFocus);
	connect(ui.clear_button, SIGNAL(clicked()), m_oglviewer, SLOT(clearVertex()));
	
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

	connect(ui.disp_ctrl_pts, SIGNAL(toggled(bool)), m_oglviewer, SLOT(setDispCtrlPts(bool)));
	connect(ui.disp_curves, SIGNAL(toggled(bool)), m_oglviewer, SLOT(setDispCurves(bool)));
	

	ui.foreground_color->setStyleSheet("QPushButton { background-color : #FFFFFF;}");
	ui.background_color->setStyleSheet("QPushButton { background-color : #000000;}");
	connect(ui.foreground_color, SIGNAL(clicked()), this, SLOT(pickColor()));
	
}

MainWindow::~MainWindow()
{

}

void MainWindow::readPoints()
{
	QString filename = QFileDialog::getOpenFileName(
		this, "Open control points file...", "",tr("CPS files(*.cps)"));
	m_oglviewer->readPoints(filename.toUtf8().constData());

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

void MainWindow::pickColor()
{
	QColor curColor = QColorDialog::getColor(m_fgColor);
	if (!curColor.isValid())
	{
		return;
	}
	m_fgColor = curColor;
	QPalette pal = ui.foreground_color->palette();
	pal.setColor(QPalette::Background, m_fgColor);

	const QString COLOR_STYLE("QPushButton { background-color : %1;}");
	ui.foreground_color->setStyleSheet(COLOR_STYLE.arg(m_fgColor.name()));
	ui.foreground_color->setPalette(pal);
}
