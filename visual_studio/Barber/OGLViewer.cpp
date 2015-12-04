#include "OGLViewer.h"

OGLViewer::OGLViewer(QWidget *parent)
	: QOpenGLWidget(parent), tcount(0), fps(30)
	, m_selectMode(OBJECT_SELECT), m_Select(-1)
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
#ifdef _DEBUG
	charMesh = new HDS_Mesh("../../scene/obj/monsterfrog.obj");
#else
	charMesh = new HDS_Mesh("quad_cube.obj");
#endif

	//////////////////////////////////////////////////////////////////////////

	//ImageData* img = new ImageData("../../scene/texture/goldfish.png");
	//img->writeFile("../../scene/texture/framebuffer.png");

	resetCamera();
}

OGLViewer::~OGLViewer()
{
}
void OGLViewer::resetCamera()
{
	Transform cam2w = lookAt(Point3D(10, 6, 10), Point3D(0.0, 0.0, 0.0), Point3D(0, 1, 0));
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
	shader_obj = new GLSLProgram("quad_vs.glsl", "quad_fs.glsl", "quad_gs.glsl");
	shader_uid = new GLSLProgram("id_vs.glsl", "id_fs.glsl", "id_gs.glsl");
	
	// Export vbo for shaders
	charMesh->exportIndexedVBO(&char_verts, nullptr, nullptr, &char_idxs);

	vao_handles.push_back(bindCharacter());

	// Get uniform variable location
	shader_obj->add_uniformv("model_matrix");
	shader_obj->add_uniformv("view_matrix");
	shader_obj->add_uniformv("proj_matrix");// WorldToCamera matrix
	shader_obj->add_uniformv("sel_id");
	shader_uid->add_uniformv("model_matrix");
	shader_uid->add_uniformv("view_matrix");
	shader_uid->add_uniformv("proj_matrix");

}
GLuint OGLViewer::bindCharacter()
{
	glDeleteVertexArrays(1, &char_vao);
	glDeleteBuffers(1, &char_pts_vbo);
	glDeleteBuffers(1, &char_elb);

	glGenBuffers(1, &char_pts_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, char_pts_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * char_verts.size(), &char_verts[0], GL_STATIC_DRAW);

	glGenBuffers(1, &char_elb);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, char_elb);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * char_idxs.size(), &char_idxs[0], GL_STATIC_DRAW);

	// Bind VAO
	glGenVertexArrays(1, &char_vao);
	glBindVertexArray(char_vao);
	glBindBuffer(GL_ARRAY_BUFFER, char_pts_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, char_elb);

	//vao_handles.push_back(box_vao);
	return char_vao;
}

void OGLViewer::renderUidBuffer()
{
	makeCurrent();
	QOpenGLFramebufferObject qfbo(width(), height(), QOpenGLFramebufferObject::CombinedDepthStencil, GL_TEXTURE_2D);
	qfbo.bind();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // cull back face
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	bindCharacter();
	//glBindVertexArray(char_vao);
	shader_uid->use_program();

	// Apply uniform matrix
	//glUniformMatrix4fv((*shader_id)("model_matrix"), 1, GL_FALSE, model_mat);
	glUniformMatrix4fv((*shader_uid)("view_matrix"), 1, GL_FALSE, view_mat);
	glUniformMatrix4fv((*shader_uid)("proj_matrix"), 1, GL_FALSE, proj_mat);
	glDrawElements(GL_LINES_ADJACENCY, char_idxs.size(), GL_UNSIGNED_INT, 0);
	shader_uid->unuse();
	qfbo.release();
	QRgb pixel = qfbo.toImage().pixel(m_lastMousePos[0], m_lastMousePos[1]);

	if ((pixel >> 24) & 0xFF) m_Select = pixel & 0xFFFFFF;
	else m_Select = -1;
	cout << "Select ID: " << m_Select << endl;
}

void OGLViewer::saveFrameBuffer()
{
	this->grab().save("../../scene/texture/framebuffer.png");
	
}

void OGLViewer::generateHairCage()
{
	if (m_Select >= 0)
	{
		auto face = charMesh->faceMap.at(m_Select);
		auto fn = face->computeNormal();
		auto curHE = face->he;
		do 
		{
			curHE->v->pos += fn * 5;
			curHE = curHE->next;
		} while (curHE != face->he);
	}
}

void OGLViewer::paintGL()
{
	// Make curent window
	makeCurrent();
	// Clear background and color buffer
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//////////////////////////////////////////////////////////////////////////
	// Model
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // cull back face
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
	glBindVertexArray(vao_handles[0]);
	shader_obj->use_program();

	// Apply uniform matrix
	//glUniformMatrix4fv((*shader)("model_matrix"), 1, GL_FALSE, sphere_model_mat);
	glUniformMatrix4fv((*shader_obj)("view_matrix"), 1, GL_FALSE, view_mat);
	glUniformMatrix4fv((*shader_obj)("proj_matrix"), 1, GL_FALSE, proj_mat);
	glUniform1i((*shader_obj)("sel_id"), m_Select);
	
	glDrawElements(GL_LINES_ADJACENCY, char_idxs.size(), GL_UNSIGNED_INT, 0);
}
// Redraw function
void OGLViewer::paintEvent(QPaintEvent *e)
{
	// Draw current frame
	paintGL();
	
	process_fps = 1000.0 / process_time.elapsed();

	process_time.start();
}
// Resize function
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
void OGLViewer::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_Home:
	{
		initParas();
		break;
	}
	case Qt::Key_F8:
	{
		m_selectMode = OBJECT_SELECT;
		break;
	}
	case Qt::Key_F11:
	{
		m_selectMode = FACE_COMPONENT_SELECT;
		break;
	}
	case Qt::Key_P:
	{
		if (e->modifiers() == Qt::ControlModifier)
		{
			this->saveFrameBuffer();
		}
		break;
	}
	case Qt::Key_Return:
	{
		generateHairCage();
		charMesh->exportIndexedVBO(&char_verts, nullptr, nullptr, &char_idxs);
		//bindCharacter();
		break;
	}
	default:
	{
		QOpenGLWidget::keyPressEvent(e);
		break;
	}
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
	if (e->buttons() == Qt::LeftButton && e->modifiers() == Qt::NoModifier)
	{
		renderUidBuffer();
		update();
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

	//printf("dx: %d, dy: %d\n", dx, dy);

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


void OGLViewer::initParas()
{
	update();
}