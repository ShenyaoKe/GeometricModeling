#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_update_pending(false), m_animating(false)
{
	m_oglviewer = new OGLViewer;

	ui.setupUi(this);
	ui.ogl_layout->addWidget(m_oglviewer);
	//setWindowTitle(tr("OpenGL Qt Template"));

	m_oglviewer->setFocusPolicy(Qt::StrongFocus);

	connect(ui.actionOpen, &QAction::triggered, m_oglviewer, &OGLViewer::loadOBJ);
	connect(ui.actionSave, &QAction::triggered, m_oglviewer, &OGLViewer::saveOBJ);

	connect(ui.smooth, &QPushButton::clicked, m_oglviewer, &OGLViewer::smoothMesh);
	connect(m_oglviewer, SIGNAL(levelChanged(int)), ui.lv_label, SLOT(setNum(int)));
}

MainWindow::~MainWindow()
{
}

void MainWindow::on_actionAbout_triggered()
{
	about = new QDialog(0,0);
	Ui::about_dialog about_ui;
	about_ui.setupUi(about);
	about->show();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
	Q_UNUSED(e);

	foreach(QWidget *widget, QApplication::topLevelWidgets())
	{
		widget->close();
	}
}
