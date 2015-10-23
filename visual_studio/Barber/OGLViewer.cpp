#include "OGLViewer.h"

OGLViewer::OGLViewer(QWidget *parent)
	: QOpenGLWidget(parent), tcount(0), fps(30)
	, m_selectMode(OBJECT_SELECT), m_Select(0)
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
	box_mesh = new Mesh("../../scene/obj/cube_large.obj");
	model_mesh = new Mesh("../../scene/obj/monkey.obj");
	model_mesh->refine(triangleList);
	mytree = new KdTreeAccel(triangleList);

	resetCamera();

	// Initialize transform matrix
	matrix.setIdentity();// setRotation(20, 0, 0);
	matrix.exportVBO(model_mat);

	//sphere_matrix = setTranslation(Point3D());
	//sphere_matrix.exportVBO(sphere_model_mat);
}

OGLViewer::~OGLViewer()
{
}
void OGLViewer::resetCamera()
{
	Transform cam2w = lookAt(Point3D(3, 2, 3), Point3D(0.0, 0.0, 0.0), Point3D(0, 1, 0));
	Transform pers = Transform(setPerspective(67,
		width() / static_cast<double>(height()), 0.1, 100));
	view_cam = new perspCamera(cam2w, pers);
	view_cam->exportVBO(view_mat, proj_mat, nullptr);
	//update();
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
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CCW); // set counter-clock-wise vertex order to mean the front


	//////////////////////////////////////////////////////////////////////////

	// Create shader files
	shader = new GLSLProgram("vert.glsl", "frag.glsl");
	//shader_transparent = new GLSLProgram("vert.glsl", "transparent_frag.glsl");


	// Export vbo for shaders
	box_mesh->exportVBO(box_vbo_size, box_verts, box_uvs, box_norms, box_idxs);
	model_mesh->exportVBO(model_vbo_size, model_verts, model_uvs, model_norms, model_idxs);

	bindBox();
	bindMesh();

	// Get uniform variable location
	model_mat_loc = shader->getUniformLocation("model_matrix");
	view_mat_loc = shader->getUniformLocation("view_matrix");
	proj_mat_loc = shader->getUniformLocation("proj_matrix");; // WorldToCamera matrix
	cout << "Model matrix location: " << model_mat_loc << endl;
	cout << "View matrix location: " << view_mat_loc << endl;
	cout << "Projection matrix location: " << proj_mat_loc << endl;
}
void OGLViewer::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_W)
	{
		view_cam->zoom(0.0, 0.0, 0.10);
		view_cam->exportVBO(view_mat, nullptr, nullptr);

	}
	else if (e->key() == Qt::Key_S)
	{
		view_cam->zoom(0.0, 0.0, -0.10);
		view_cam->exportVBO(view_mat, nullptr, nullptr);
	}
	else if (e->key() == Qt::Key_Q)
	{
		view_cam->zoom(0.10, 0.0, 0.0);
		view_cam->exportVBO(view_mat, nullptr, nullptr);
	}
	else if (e->key() == Qt::Key_A)
	{
		view_cam->zoom(-0.10, 0.0, 0.0);
		view_cam->exportVBO(view_mat, nullptr, nullptr);
	}
	else if (e->key() == Qt::Key_E)
	{
		view_cam->zoom(0.0, 0.10, 0.0);
		view_cam->exportVBO(view_mat, nullptr, nullptr);
	}
	else if (e->key() == Qt::Key_D)
	{
		view_cam->zoom(0.0, -0.10, 0.0);
		view_cam->exportVBO(view_mat, nullptr, nullptr);
	}
	else if (e->key() == Qt::Key_Home)
	{
		initParas();
	}
	// Selection mode switch
	else if (e->key() == Qt::Key_F8)
	{
		m_selectMode = OBJECT_SELECT;
	}
	else if (e->key() == Qt::Key_F11)
	{
		m_selectMode = FACE_COMPONENT_SELECT;
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
		// Do something here
	}
	if (e->buttons() == Qt::LeftButton && !e->modifiers())
	{
		Ray inRay = view_cam->shootRay(m_lastMousePos[0], height() - m_lastMousePos[1]);
		double tHit = INFINITY, re = 0.001;
		DifferentialGeometry queryPoint;
		if (mytree->hit(inRay, &queryPoint, &tHit, &re))
		{
			m_Select = queryPoint.object->getIndex();
			cout <<"Selected Face ID: "<< m_Select << endl;
		}
		else
		{
			m_Select = 0;
		}
	}
};

void OGLViewer::mouseReleaseEvent(QMouseEvent *e)
{
	m_lastMousePos[0] = e->x();
	m_lastMousePos[1] = e->y();
}

void OGLViewer::mouseMoveEvent(QMouseEvent *e)
{
	int dx = e->x() - m_lastMousePos[0];
	int dy = e->y() - m_lastMousePos[1];

	printf("dx: %d, dy: %d\n", dx, dy);
	
	if ((e->buttons() == Qt::LeftButton) && (e->modifiers() == Qt::AltModifier))
	{
		view_cam->rotate(dy * 0.25, -dx * 0.25, 0.0);
		view_cam->exportVBO(view_mat, nullptr, nullptr);
	}
	else if ((e->buttons() == Qt::RightButton) && (e->modifiers() == Qt::AltModifier))
	{
		if (dx != e->x() && dy != e->y())
		{
			view_cam->zoom(0.0, 0.0, dx * 0.05);
			view_cam->exportVBO(view_mat, nullptr, nullptr);
		}
	}
	else if ((e->buttons() == Qt::MidButton) && (e->modifiers() == Qt::AltModifier))
	{
		if (dx != e->x() && dy != e->y())
		{
			view_cam->zoom(dx * 0.05, dy * 0.05, 0.0);
			view_cam->exportVBO(view_mat, nullptr, nullptr);
		}
	}
	/*else
	{
	QOpenGLWidget::mouseMoveEvent(e);
	}*/

	m_lastMousePos[0] = e->x();
	m_lastMousePos[1] = e->y();
	update();
}



void OGLViewer::bindBox()
{
	GLuint box_pts_vbo;
	glGenBuffers(1, &box_pts_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, box_pts_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9 * box_vbo_size, box_verts, GL_STATIC_DRAW);

	// Bind normal value as color
	GLuint box_color_vbo;
	glGenBuffers(1, &box_color_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, box_color_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9 * box_vbo_size, box_norms, GL_STATIC_DRAW);

	// Bind VAO
	GLuint box_vao;
	glGenVertexArrays(1, &box_vao);
	glBindVertexArray(box_vao);
	glBindBuffer(GL_ARRAY_BUFFER, box_pts_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, box_color_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);

	vao_handles.push_back(box_vao);
}

void OGLViewer::bindMesh()
{
	GLuint model_pts_vbo;
	glGenBuffers(1, &model_pts_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model_pts_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9 * model_vbo_size, model_verts, GL_STATIC_DRAW);

	// Bind normal value as color
	GLuint model_normal_vbo;
	glGenBuffers(1, &model_normal_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model_normal_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9 * model_vbo_size, model_norms, GL_STATIC_DRAW);

	// Bind normal value as color
	GLuint model_uv_vbo;
	glGenBuffers(1, &model_uv_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model_uv_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * model_vbo_size, model_uvs, GL_STATIC_DRAW);

	GLuint model_idx_vbo;
	glGenBuffers(1, &model_idx_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model_idx_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLint) * model_vbo_size, model_idxs, GL_STATIC_DRAW);

	// Bind VAO
	GLuint model_vao;
	glGenVertexArrays(1, &model_vao);
	glBindVertexArray(model_vao);
	glBindBuffer(GL_ARRAY_BUFFER, model_pts_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, model_normal_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, model_uv_vbo);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, model_idx_vbo);
	glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(3);

	vao_handles.push_back(model_vao);
}

void OGLViewer::paintGL()
{
	// Make curent window
	makeCurrent();
	// Clear background and color buffer
	glClearColor(0.25, 0.4, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*if (display_mode == 0)// Wireframe
	{
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT); // cull back face
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//glEnable(GL_BLEND);
	}
	// Bind Box VAOs
	glBindVertexArray(vao_handles[0]);
	// Use shader program
	if (display_mode == 0)
	{
		shader->use_program();
	}
	else
	{
		shader_transparent->use_program();
	}
	// Apply uniform matrix
	glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_mat);
	glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, view_mat);
	glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, proj_mat);
	glDrawArrays(GL_TRIANGLES, 0, box_vbo_size * 3);*/
	//////////////////////////////////////////////////////////////////////////
	// Sphere
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // cull back face
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glEnable(GL_BLEND);
	
	//
	
	glBindVertexArray(vao_handles[1]);
	shader->use_program();

	// Apply uniform matrix
	//glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, sphere_model_mat);
	glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, view_mat);
	glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, proj_mat);
	glUniform1i(shader->getUniformLocation("sel_id"), m_Select);
	glDrawArrays(GL_TRIANGLES, 0, model_vbo_size * 3);


}
// Redraw function
void OGLViewer::paintEvent(QPaintEvent *e)
{
	// Draw current frame
	paintGL();
	
	process_fps = 1000.0 / process_time.elapsed();

	process_time.start();
}
//Resize function
void OGLViewer::resizeGL(int w, int h)
{
	// Widget resize operations
	view_cam->resizeViewport(width() / static_cast<double>(height()));
	view_cam->setResolution(width(), height());
	view_cam->exportVBO(nullptr, proj_mat, nullptr);
	glViewport(0, 0, width(), height());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void OGLViewer::initParas()
{
	/*sphere_matrix = setTranslation(Vector3D());
	sphere_matrix.exportVBO(sphere_model_mat);*/
	update();
}