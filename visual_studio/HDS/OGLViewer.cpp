#include "OGLViewer.h"

OGLViewer::OGLViewer(QWidget *parent)
	: QOpenGLWidget(parent), tcount(0), fps(30)
	, subd_lv(0)
	, m_selectMode(OBJECT_SELECT)
	, loader("../../scene/obj/happy.obj")
{
	// Set surface format for current widget
	QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
	format.setVersion(4, 5);
	format.setProfile(QSurfaceFormat::CoreProfile);
	this->setFormat(format);

	

	resetCamera();
}

OGLViewer::~OGLViewer()
{
	delete view_cam;
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
	triangle_shader = new GLSLProgram("triangle_vs.glsl", "triangle_fs.glsl", "triangle_gs.glsl");
	//mesh_shader = new GLSLProgram("quad_vs.glsl", "quad_fs.glsl", "quad_gs.glsl");

	// Export vbo for shaders
	loader.exportVBO(&mesh_verts, nullptr, nullptr, &mesh_idxs);
	offset = mesh_idxs.size() - 3;
	//hds_model->exportIndexedVBO(&mesh_verts, nullptr, nullptr, &mesh_idxs);
	
	bindMesh();

	// Get uniform variable location
	triangle_shader->add_uniformv("view_matrix");
	triangle_shader->add_uniformv("proj_matrix");
}

/*
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
}*/


void OGLViewer::bindMesh()
{
	glDeleteVertexArrays(1, &mesh_vao);
	glDeleteBuffers(1, &mesh_vbo);
	glDeleteBuffers(1, &mesh_ibo);

	// Bind VAO
	glGenVertexArrays(1, &mesh_vao);
	glBindVertexArray(mesh_vao);

	glGenBuffers(1, &mesh_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mesh_verts.size(), &mesh_verts[0], GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &mesh_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_ibo);
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
	glBindVertexArray(mesh_vao);
	
	triangle_shader->use_program();
	glUniformMatrix4fv((*triangle_shader)("view_matrix"), 1, GL_FALSE, view_mat);
	glUniformMatrix4fv((*triangle_shader)("proj_matrix"), 1, GL_FALSE, proj_mat);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, mesh_idxs.size(), GL_UNSIGNED_INT, 0);
	/*glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(offset * sizeof(GLuint)));*/
	triangle_shader->unuse();
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
	if (e->key() == Qt::Key_Left)
	{
		offset -= 3;
		offset = max(offset, 0);

		cout << "indices:\t" << mesh_idxs[offset] << ", " << mesh_idxs[offset + 1] << ", " << mesh_idxs[offset + 2] << endl;
	}
	else if (e->key() == Qt::Key_Right)
	{
		offset += 3;
		offset = min(offset, (int)mesh_idxs.size() - 3);
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
	Transform cam2w = Matrix4D::LookAt(Point3D(30, 10, 30), Point3D(0.0, 0.0, 0.0), Point3D(0, 1, 0));
	Transform pers = Transform(Matrix4D::Perspective(54.3,
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