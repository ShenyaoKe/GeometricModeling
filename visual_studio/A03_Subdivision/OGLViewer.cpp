#include "OGLViewer.h"

OGLViewer::OGLViewer(QWidget *parent)
	: QOpenGLWidget(parent), tcount(0), fps(30)
	, subd_lv(0)
	, m_selectMode(OBJECT_SELECT)
{
	// Set surface format for current widget
	QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
	format.setVersion(4, 5);
	format.setProfile(QSurfaceFormat::CoreProfile);
	this->setFormat(format);

	// Read obj file
	//box_mesh = new Mesh("../../scene/obj/cube_large.obj");
	//model_mesh = new Mesh("../../scene/obj/monkey.obj");
#ifdef _DEBUG
	hds_model = new HDS_Mesh("../../scene/obj/monsterfrog.obj");
#else
	hds_model = new HDS_Mesh("quad_cube.obj");
#endif
	//hds_box->reIndexing();
	//hds_box->validate();
	subd_mesh = new Subdivision(0, hds_model);

	resetCamera();
}

OGLViewer::~OGLViewer()
{
	delete model_mesh;
	model_mesh = nullptr;
	delete view_cam;
	view_cam = nullptr;
}
/************************************************************************/
/* OpenGL Rendering Modules                                             */
/************************************************************************/
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
	glEnable(GL_BLEND);
	/*glEnable(GL_MULTISAMPLE);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);*/
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_STENCIL_TEST);
	glBlendEquation(GL_FUNC_ADD_EXT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CCW); // set counter-clock-wise vertex order to mean the front


	//////////////////////////////////////////////////////////////////////////

	// Create shader files
	point_shader = new GLSLProgram("point_vs.glsl", "point_fs.glsl");
	wireframe_shader = new GLSLProgram("wireframe_vs.glsl", "wireframe_fs.glsl", "wireframe_gs.glsl");
	mesh_shader = new GLSLProgram("quad_vs.glsl", "quad_fs.glsl", "quad_gs.glsl");

	// Export vbo for shaders
	hds_model->exportIndexedVBO(&mesh_verts, nullptr, nullptr, &mesh_idxs);
	subd_mesh->exportIndexedVBO(subd_lv, &mesh_verts, nullptr, nullptr, &mesh_idxs);

	bindMesh();

	// Get uniform variable location
	point_shader->add_uniformv("view_matrix");
	point_shader->add_uniformv("proj_matrix");; // WorldToCamera matrix
	wireframe_shader->add_uniformv("view_matrix");
	wireframe_shader->add_uniformv("proj_matrix");
	mesh_shader->add_uniformv("view_matrix");
	mesh_shader->add_uniformv("proj_matrix");
}

void OGLViewer::loadOBJ()
{
	QString filename = QFileDialog::getOpenFileName(
		this, "Load OBJ file...", "dragon.obj", tr("Object Files(*.obj)"));
	if (filename.size() > 0)
	{
		delete hds_model;
		delete subd_mesh;
		hds_model = new HDS_Mesh(filename.toUtf8().constData());
		subd_lv = 0;
		subd_mesh = new Subdivision(0, hds_model);
		subd_mesh->exportIndexedVBO(0, &mesh_verts, nullptr, nullptr, &mesh_idxs);
		//bindMesh();
		emit levelChanged(subd_lv);
		update();
	}
}

void OGLViewer::saveOBJ()
{
	QString filename = QFileDialog::getSaveFileName(
		this, "Save OBJ file...", "default", "Object File(*.obj)");
	if (filename.size() > 0)
	{
		filename.append("_"+QString::number(subd_lv)+".obj");
		subd_mesh->saveAsOBJ(subd_lv, filename.toUtf8().constData());
	}
}

void OGLViewer::smoothMesh()
{
	if (subd_lv >= 4)
	{
		int ret = QMessageBox::warning(this, tr("Warning"),
			tr("Current mesh is smoothed more than level 4!\n"
			"Do you still want to smooth it?"),
			QMessageBox::Yes | QMessageBox::Cancel);
		if (ret == QMessageBox::Cancel)
		{
			return;
		}
	}
	subd_mesh->subdivide();
	subd_lv = subd_mesh->getLevel();
	subd_mesh->exportIndexedVBO(subd_lv, &mesh_verts, nullptr, nullptr, &mesh_idxs);

	bindMesh();

	emit levelChanged(subd_lv);
	update();
}

void OGLViewer::bindMesh()
{
	glDeleteVertexArrays(1, &subd_vao);
	glDeleteBuffers(1, &subd_vbo);
	glDeleteBuffers(1, &subd_ibo);

	// Bind VAO
	glGenVertexArrays(1, &subd_vao);
	glBindVertexArray(subd_vao);

	glGenBuffers(1, &subd_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, subd_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mesh_verts.size(), &mesh_verts[0], GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &subd_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subd_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh_idxs.size(), &mesh_idxs[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OGLViewer::paintGL()
{
	// Make curent window
	makeCurrent();
	// Clear background and color buffer
	glClearColor(0.6, 0.6, 0.6, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	// Model
	/*if (drawWireFrame)
	{
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{*/
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // cull back face
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//}
	bindMesh();
	glBindVertexArray(subd_vao);
	
	/*if (showPiece)
	{
		glDrawElements(GL_LINES_ADJACENCY, 4, GL_UNSIGNED_INT, 0);
		
	} 
	else*/
	{
		mesh_shader->use_program();
		glUniformMatrix4fv((*mesh_shader)("view_matrix"), 1, GL_FALSE, view_mat);
		glUniformMatrix4fv((*mesh_shader)("proj_matrix"), 1, GL_FALSE, proj_mat);
		glDrawElements(GL_LINES_ADJACENCY, mesh_idxs.size(), GL_UNSIGNED_INT, 0);
		mesh_shader->unuse();

		if (drawWireFrame)
		{
			wireframe_shader->use_program();
			glUniformMatrix4fv((*wireframe_shader)("view_matrix"), 1, GL_FALSE, view_mat);
			glUniformMatrix4fv((*wireframe_shader)("proj_matrix"), 1, GL_FALSE, proj_mat);
			glLineWidth(2.f);
			glDrawElements(GL_LINES_ADJACENCY, mesh_idxs.size(), GL_UNSIGNED_INT, 0);
			wireframe_shader->unuse();
		}
		
		if (drawPoint)
		{
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, subd_vbo);

			point_shader->use_program();
			glUniformMatrix4fv((*point_shader)("view_matrix"), 1, GL_FALSE, view_mat);
			glUniformMatrix4fv((*point_shader)("proj_matrix"), 1, GL_FALSE, proj_mat);
			glPointSize(6.0);
			glDrawArrays(GL_POINTS, 0, mesh_verts.size());
			point_shader->unuse();
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

	}
}
// Redraw function
void OGLViewer::paintEvent(QPaintEvent *e)
{
	// Draw current frame
	paintGL();
}
// Resize function
void OGLViewer::resizeGL(int w, int h)
{
	// Widget resize operations
	view_cam->resizeViewport(width() / static_cast<double>(height()));
	view_cam->setResolution(width(), height());
	view_cam->exportVBO(nullptr, proj_mat, nullptr);
}
/************************************************************************/
/* Qt User Operation Functions                                          */
/************************************************************************/
void OGLViewer::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Home)
	{
		initParas();
	}
	// Subdiv
	else if (e->key() == Qt::Key_Plus)
	{
		//offset = min(++offset, (int)box_idxs.size() / 3 - 1);
		subd_lv = min(++subd_lv, (int)subd_mesh->getLevel());
		subd_mesh->exportIndexedVBO(subd_lv, &mesh_verts, nullptr, nullptr, &mesh_idxs);
		bindMesh();

		emit levelChanged(subd_lv);
	}
	else if (e->key() == Qt::Key_Minus)
	{
		subd_lv = max(--subd_lv, 0);
		subd_mesh->exportIndexedVBO(subd_lv, &mesh_verts, nullptr, nullptr, &mesh_idxs);
		bindMesh();
		emit levelChanged(subd_lv);
	}
	else if (e->key() == Qt::Key_Left)
	{
		offset = max(--offset, 0);
	}
	else if (e->key() == Qt::Key_Right)
	{
		offset = min(++offset, (int)mesh_idxs.size() / 3 - 1);
	}
	else if (e->key() == Qt::Key_1)
	{
		showPiece = !showPiece;
	}
	else if (e->key() == Qt::Key_2)
	{
		drawPoint = !drawPoint;
	}
	else if (e->key() == Qt::Key_3)
	{
		drawWireFrame = !drawWireFrame;
	}
	// Save frame buffer
	else if (e->key() == Qt::Key_P && e->modifiers() == Qt::ControlModifier)
	{
		this->saveFrameBuffer();
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

	if ((e->buttons() == Qt::LeftButton) && (e->modifiers() == Qt::AltModifier))
	{
		view_cam->rotate(dy * 0.25, -dx * 0.25, 0.0);
		view_cam->exportVBO(view_mat, nullptr, nullptr);
		update();
	}
	else if ((e->buttons() == Qt::RightButton) && (e->modifiers() == Qt::AltModifier))
	{
		if (dx != e->x() && dy != e->y())
		{
			view_cam->zoom(0.0, 0.0, dx * 0.05);
			view_cam->exportVBO(view_mat, nullptr, nullptr);
			update();
		}
	}
	else if ((e->buttons() == Qt::MidButton) && (e->modifiers() == Qt::AltModifier))
	{
		if (dx != e->x() && dy != e->y())
		{
			view_cam->zoom(dx * 0.05, dy * 0.05, 0.0);
			view_cam->exportVBO(view_mat, nullptr, nullptr);
			update();
		}
	}
	/*else
	{
	QOpenGLWidget::mouseMoveEvent(e);
	}*/

	m_lastMousePos[0] = e->x();
	m_lastMousePos[1] = e->y();
	
}
/************************************************************************/
/* Application Functions                                                */
/************************************************************************/
void OGLViewer::resetCamera()
{
	Transform cam2w = lookAt(Point3D(30, 10, 30), Point3D(0.0, 0.0, 0.0), Point3D(0, 1, 0));
	Transform pers = Transform(setPerspective(54.3,
		width() / static_cast<double>(height()), 0.1, 500));
	view_cam = new perspCamera(cam2w, pers);
	view_cam->exportVBO(view_mat, proj_mat, nullptr);
	//update();
}
void OGLViewer::initParas()
{
	update();
}

void OGLViewer::saveFrameBuffer()
{
	QString filename = QFileDialog::getSaveFileName(
		this, "Save Screenshot ...", "default", "PNG(*.png)");
	this->grab().save(filename);
}