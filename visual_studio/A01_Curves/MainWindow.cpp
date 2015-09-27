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

	connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(exportToFile()));

	signalMapper = new QSignalMapper(this);
	connect(signalMapper, SIGNAL(mapped(int)), m_oglviewer, SLOT(changeOperation(int)));
	signalMapper->setMapping(ui.actionInsert, 0);
	signalMapper->setMapping(ui.actionMove, 1);
	connect(ui.actionInsert, SIGNAL(triggered()), signalMapper, SLOT(map()));
	connect(ui.actionMove, SIGNAL(triggered()), signalMapper, SLOT(map()));
}

MainWindow::~MainWindow()
{

}

void MainWindow::exportToFile()
{
	QString filename = QFileDialog::getSaveFileName(this, "Save file to...");
	m_oglviewer->writePoints(filename.toUtf8().constData());
}
