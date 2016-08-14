#include "MainWindow.h"

#include <QFileDialog>
#include <QColorDialog>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_update_pending(false), m_animating(false)
	, m_fgColor(255, 255, 255), m_bgColor(0, 0, 0)
	, m_oglviewer(new OGLViewer)
{
	ui.setupUi(this);
	ui.ogl_layout->addWidget(m_oglviewer);

	m_oglviewer->setFocusPolicy(Qt::StrongFocus);
	connect(ui.clear_button, SIGNAL(clicked()), m_oglviewer, SLOT(clearVertex()));
	connect(ui.seg_val, SIGNAL(valueChanged(int)), m_oglviewer, SLOT(setSegment(int)));

	signalMapper = new QSignalMapper(this);
	connect(signalMapper, SIGNAL(mapped(int)), m_oglviewer, SLOT(changeOperation(int)));
	signalMapper->setMapping(ui.actionInsert, 0);
	signalMapper->setMapping(ui.actionMove, 1);
	connect(ui.actionInsert, SIGNAL(triggered()), signalMapper, SLOT(map()));
	connect(ui.actionMove, SIGNAL(triggered()), signalMapper, SLOT(map()));

	connect(ui.disp_ctrl_pts, SIGNAL(toggled(bool)), m_oglviewer, SLOT(setDispCtrlPts(bool)));
	connect(ui.disp_curves, SIGNAL(toggled(bool)), m_oglviewer, SLOT(setDispCurves(bool)));
}

MainWindow::~MainWindow()
{
}