#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_update_pending(false), m_animating(false)
{
	m_oglviewer = new OGLViewer;

	ui.setupUi(this);
	ui.ogl_layout->addWidget(m_oglviewer);
	//connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(on_actionAbout_triggered()));
	setWindowTitle(tr("Barber"));

	m_oglviewer->setFocusPolicy(Qt::StrongFocus);

	connect(m_oglviewer, &OGLViewer::echoHint,
		[&](const QString &msg) { ui.statusBar->showMessage(msg, 2000); });
	connect(ui.hmsh_opacity,
		static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[&](double val) { m_oglviewer->hair_mesh_opacity = val; update(); });
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
