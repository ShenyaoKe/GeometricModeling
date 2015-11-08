#include "MainWindow.h"

#include <QFileDialog>
#include <QColorDialog>

const QString COLOR_STYLE("QPushButton { background-color : %1;}");

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
	
	// Curve Properties control
	connect(ui.degree_val, SIGNAL(valueChanged(int)), m_oglviewer, SLOT(setDegree(int)));
	connect(ui.seg_val, SIGNAL(valueChanged(int)), m_oglviewer, SLOT(setSegment(int)));
	connect(m_oglviewer, &OGLViewer::selectionChanged, this, &MainWindow::updateCurveProp);

	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(readPoints()));
	connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(savePoints()));
	connect(ui.actionExport, SIGNAL(triggered()), this, SLOT(exportSVG()));

	//////////////////////////////////////////////////////////////////////////
	// Point control actions
	signalMapper = new QSignalMapper(this);
	connect(signalMapper, SIGNAL(mapped(int)), m_oglviewer, SLOT(changeOperation(int)));
	signalMapper->setMapping(ui.actionInsert, 0);
	signalMapper->setMapping(ui.actionMove, 1);
	connect(ui.actionInsert, SIGNAL(triggered()), signalMapper, SLOT(map()));
	connect(ui.actionMove, SIGNAL(triggered()), signalMapper, SLOT(map()));

	connect(ui.disp_ctrl_pts, SIGNAL(toggled(bool)), m_oglviewer, SLOT(setDispCtrlPts(bool)));
	connect(ui.disp_curves, SIGNAL(toggled(bool)), m_oglviewer, SLOT(setDispCurves(bool)));
	

	//////////////////////////////////////////////////////////////////////////
	// Color actions
	m_colors[0] = QColor(255, 255, 255);
	m_colors[1] = QColor(255, 255, 255);
	m_colors[2] = QColor(255, 255, 255);
	m_colors[3] = QColor(255, 255, 255);
	colorPickingSignals = new QSignalMapper(this);
	connect(colorPickingSignals, SIGNAL(mapped(int)), this, SLOT(pickColor(int)));
	colorPickingSignals->setMapping(ui.fc_lf_color, 0);
	colorPickingSignals->setMapping(ui.fc_rt_color, 1);
	colorPickingSignals->setMapping(ui.ec_lf_color, 2);
	colorPickingSignals->setMapping(ui.ec_rt_color, 3);
	connect(ui.fc_lf_color, SIGNAL(clicked()), colorPickingSignals, SLOT(map()));
	connect(ui.fc_rt_color, SIGNAL(clicked()), colorPickingSignals, SLOT(map()));
	connect(ui.ec_lf_color, SIGNAL(clicked()), colorPickingSignals, SLOT(map()));
	connect(ui.ec_rt_color, SIGNAL(clicked()), colorPickingSignals, SLOT(map()));
	ui.fc_lf_color->setStyleSheet("QPushButton { background-color : #FFFFFF;}");
	ui.fc_rt_color->setStyleSheet("QPushButton { background-color : #FFFFFF;}");
	ui.ec_lf_color->setStyleSheet("QPushButton { background-color : #FFFFFF;}");
	ui.ec_rt_color->setStyleSheet("QPushButton { background-color : #FFFFFF;}");

	//////////////////////////////////////////////////////////////////////////
	// Image operations
	connect(ui.proc_dfcv, SIGNAL(clicked()), m_oglviewer, SLOT(generateDiffusionCurve()));
	connect(ui.save_all_img, SIGNAL(clicked()), m_oglviewer, SLOT(saveFrameBuffer()));
}

MainWindow::~MainWindow()
{

}

void MainWindow::updateCurveProp()
{
	auto curve = m_oglviewer->current_curve;
	ui.degree_val->setValue(curve->degree);
	ui.seg_val->setValue(curve->segments);

	ui.fc_lf_color->setStyleSheet(COLOR_STYLE.arg(curve->colors[0].name()));
	ui.fc_rt_color->setStyleSheet(COLOR_STYLE.arg(curve->colors[1].name()));
	ui.ec_lf_color->setStyleSheet(COLOR_STYLE.arg(curve->colors[2].name()));
	ui.ec_rt_color->setStyleSheet(COLOR_STYLE.arg(curve->colors[3].name()));
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

void MainWindow::pickColor(int idx)
{
	QColor curColor = QColorDialog::getColor(m_colors[idx]);
	if (!curColor.isValid())
	{
		return;
	}
	QPushButton* curButton = dynamic_cast<QPushButton*>(colorPickingSignals->mapping(idx));

	m_colors[idx] = curColor;
	QPalette pal = curButton->palette();
	pal.setColor(QPalette::Background, m_colors[idx]);

	
	curButton->setStyleSheet(COLOR_STYLE.arg(m_colors[idx].name()));
	curButton->setPalette(pal);

	if (m_oglviewer->current_curve != nullptr)
	{
		m_oglviewer->current_curve->colors[idx] = curColor;
	}
}
