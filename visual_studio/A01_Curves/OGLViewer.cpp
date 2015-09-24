#include "OGLViewer.h"

OGLViewer::OGLViewer(QWidget *parent)
	: QOpenGLWidget(parent), fps(30)
{
	// Set surface format for current widget
	QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
	format.setVersion(4, 5);
	format.setProfile(QSurfaceFormat::CoreProfile);
	this->setFormat(format);

	// Link timer trigger
	process_time.start();
	QTimer *timer = new QTimer(this);
	/*timer->setSingleShot(false);*/
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	timer->start(0);


	// Read obj file
	Point3D *p = new Point3D();// width() / 2, height() / 2, 0.0);
	ctrl_points.push_back(p);
	//delete[] points_verts;
	//points_verts = new GLfloat[ctrl_points.size() * 3];
	exportPointVBO(points_verts);

	// Init camera
	proj_mat[0] = 2.0 / static_cast<GLfloat>(width());
	proj_mat[5] = 2.0 / static_cast<GLfloat>(height());
/*
	Transform cam2w = lookAt();
	Transform ortho(setOrthographic(width(), height()));
	view_cam = new orthoCamera(cam2w, ortho);
	view_cam->exportVBO(nullptr, proj_mat, nullptr);*/
}

OGLViewer::~OGLViewer()
{
}

void OGLViewer::initializeGL()
{
	// OpenGL extention initialization
	glewInit();

	// Print OpenGL vertion
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	// Enable OpenGL features
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CCW); // set counter-clock-wise vertex order to mean the front


	//////////////////////////////////////////////////////////////////////////
	// Create shader files
	points_shader = new GLSLProgram("points_vs.glsl", "points_fs.glsl", "points_gs.glsl");
	curve_shader = new GLSLProgram("curve_vs.glsl", "curve_fs.glsl");


	// Export vbo for shaders

	// Get uniform variable location
	//vertex_position = points_shader->getUniformLocation("vertex_position");
	//vertex_colour = points_shader->getUniformLocation("vertex_colour");
	point_proj_mat_loc = points_shader->getUniformLocation("proj_matrix"); // WorldToCamera matrix
	curve_proj_mat_loc = curve_shader->getUniformLocation("proj_matrix"); // WorldToCamera matrix

	/*cout << "vertex_position location: " << vertex_position << endl;
	cout << "vertex_colour location: " << vertex_colour << endl;*/
	cout << "Point Projection matrix location: " << point_proj_mat_loc << endl;
	cout << "Curve Projection matrix location: " << curve_proj_mat_loc << endl;
}
void OGLViewer::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_E)
	{
		cv_op_mode++;
		cv_op_mode %= 3;
	}
	else if (e->key() == Qt::Key_Home)
	{
		initParas();
	}
	else
	{
		QOpenGLWidget::keyPressEvent(e);
	}
	update();
}

void OGLViewer::mousePressEvent(QMouseEvent *e)
{
	m_lastMousePos[0] = e->x();
	m_lastMousePos[1] = e->y();
		
	if ((e->buttons() == Qt::LeftButton) && (e->modifiers() == Qt::AltModifier))
	{
		cout << "Mouse position: " << e->x() << ", " << e->y() << endl;
	}
	if (e->buttons() == Qt::LeftButton && cv_op_mode == DRAWING_MODE)
	{
		Point3D *pt = new Point3D(e->x() - width() / 2, height() / 2 - e->y(), 0);
			/*new Point3D(
			(e->x() - width() / 2) * 2.0 / height(),
			1.0 - e->y() * 2.0 / height(),
			0.0);*/
		ctrl_points.push_back(pt);

		this->exportPointVBO(points_verts);
		cout << "Mouse position: " << e->x() << ", " << e->y() << endl;
	}
	update();
};

void OGLViewer::mouseMoveEvent(QMouseEvent *e)
{
	int dx = e->x() - m_lastMousePos[0];
	int dy = e->y() - m_lastMousePos[1];

	printf("dx: %d, dy: %d\n", dx, dy);
	/*if ((e->buttons() == Qt::LeftButton) && (e->modifiers() == Qt::AltModifier))
	{
		view_cam->rotate(-dy / 4, dx / 4, 0);
		view_cam->exportVBO(view_mat, nullptr, nullptr);
	}
	else if ((e->buttons() == Qt::RightButton) && (e->modifiers() == Qt::AltModifier))
	{
		if (dx != e->x() && dy != e->y())
		{
			view_cam->zoom(0.0, 0.0, dx * 0.10);
			view_cam->exportVBO(view_mat, nullptr, nullptr);
		}
	}*/
	/*else
	{
	QOpenGLWidget::mouseMoveEvent(e);
	}*/

	m_lastMousePos[0] = e->x();
	m_lastMousePos[1] = e->y();
	//update();
}



void OGLViewer::exportPointVBO(GLfloat* &ptsVBO)
{

	delete[] ptsVBO;
	ptsVBO = new GLfloat[ctrl_points.size() * 3];

	for (int i = 0; i < ctrl_points.size(); i++)
	{
		ptsVBO[i * 3] = static_cast<GLfloat>(ctrl_points[i]->x);
		ptsVBO[i * 3 + 1] = static_cast<GLfloat>(ctrl_points[i]->y);
		ptsVBO[i * 3 + 2] = static_cast<GLfloat>(ctrl_points[i]->z);
	}
}

void OGLViewer::paintGL()
{
	// Make curent window
	makeCurrent();
	// Clear background and color buffer
	glClearColor(0.26, 0.72, 0.94, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT); // cull back face
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Bind VBOs
	//pts vbo
	GLuint pts_vbo;
	glGenBuffers(1, &pts_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pts_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * ctrl_points.size(), points_verts, GL_STATIC_DRAW);

	// Bind normal value as color
	/*GLuint box_color_vbo;
	glGenBuffers(1, &box_color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, box_color_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9 * box_vbo_size, box_norms, GL_STATIC_DRAW);*/

	// Bind VAO
	GLuint pts_vao;
	glGenVertexArrays(1, &pts_vao);
	glBindVertexArray(pts_vao);
	glBindBuffer(GL_ARRAY_BUFFER, pts_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	// Use shader program
	points_shader->use_program();
	
	// Apply uniform matrix
	//glUniformMatrix4fv(vertex_position, 1, GL_FALSE, points_verts);
	glUniformMatrix4fv(point_proj_mat_loc, 1, GL_FALSE, proj_mat);
	glDrawArrays(GL_POINTS, 0, ctrl_points.size());


	//////////////////////////////////////////////////////////////////////////
	// Draw straight lines
	GLuint curve_vbo;
	glGenBuffers(1, &curve_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, curve_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * ctrl_points.size(), points_verts, GL_STATIC_DRAW);

	// Bind VAO
	GLuint curve_vao;
	glGenVertexArrays(1, &curve_vao);
	glBindVertexArray(curve_vao);
	glBindBuffer(GL_ARRAY_BUFFER, curve_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	curve_shader->use_program();

	// Apply uniform matrix
	glUniformMatrix4fv(curve_proj_mat_loc, 1, GL_FALSE, proj_mat);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_LINE_STRIP, 0, ctrl_points.size());
}
// Redraw function
void OGLViewer::paintEvent(QPaintEvent *e)
{
	// Draw current frame
	paintGL();
}
//Resize function
void OGLViewer::resizeGL(int w, int h)
{
	// Widget resize operations
	/*view_cam->resizeViewport(width() / static_cast<double>(height()));
	view_cam->exportVBO(nullptr, proj_mat, nullptr);*/

	proj_mat[0] = 2.0 / static_cast<GLfloat>(width());
	proj_mat[5] = 2.0 / static_cast<GLfloat>(height());
	/*glViewport(0, 0, width(), height());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();*/
}

void OGLViewer::initParas()
{
	update();
}