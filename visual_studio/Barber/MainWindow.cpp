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
	
	// Extrusion Operation
	connect(ui.extrude_button, &QPushButton::clicked,
		[&](){ m_oglviewer->extrude(ui.extrude_val->value()); });
	
	connect(ui.hmsh_opacity,
		static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[&](double val) { m_oglviewer->hair_mesh_opacity = val; update(); });

	// Color Picking UI
	ui.root_color->setStyleSheet(COLOR_STYLE.arg(m_oglviewer->hairRootColor.name()));
	ui.tip_color->setStyleSheet(COLOR_STYLE.arg(m_oglviewer->hairTipColor.name()));
	ui.scatter_color->setStyleSheet(COLOR_STYLE.arg(m_oglviewer->hairScatterColor.name()));
	connect(ui.root_color, &QPushButton::clicked, [&]() {
		pickColor(&m_oglviewer->hairRootColor, ui.root_color);
	});
	connect(ui.tip_color, &QPushButton::clicked, [&]() {
		pickColor(&m_oglviewer->hairTipColor, ui.tip_color);
	});
	connect(ui.scatter_color, &QPushButton::clicked, [&]() {
		pickColor(&m_oglviewer->hairScatterColor, ui.scatter_color);
	});

	// Save/ Open file
	connect(ui.actionOpen, &QAction::triggered, [&](){
		QString filename = QFileDialog::getOpenFileName(
			this, "Open hair mesh file...", "", tr("Hair Mesh Files(*.hms)"));
		m_oglviewer->openHMS(filename); });
	connect(ui.actionSave, &QAction::triggered, [&](){
		QString filename = QFileDialog::getSaveFileName(
			this, "Save hair mesh to...", "untitled.hms", tr("Hair Mesh Files(*.hms)"));
		m_oglviewer->exportHMS(filename); });

	// Simulation Option
	connect(ui.gen_dynamic_hair, &QPushButton::clicked, m_oglviewer, &OGLViewer::createSimMesh);
	connect(ui.simulate, &QPushButton::clicked, m_oglviewer, &OGLViewer::changeSimStatus);
	connect(ui.play_button, &QPushButton::clicked, m_oglviewer, &OGLViewer::changeSimDrawStatus);
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

void MainWindow::pickColor(QColor* targ, QPushButton* src)
{
	QColor curColor = QColorDialog::getColor(*targ);
	if (!curColor.isValid())
	{
		src->setStyleSheet(COLOR_STYLE.arg(targ->name()));
		return;
	}
	*targ = curColor;
	src->setStyleSheet(COLOR_STYLE.arg(curColor.name()));
}
