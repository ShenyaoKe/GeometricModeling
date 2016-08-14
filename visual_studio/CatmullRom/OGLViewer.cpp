#include "OGLViewer.h"

OGLViewer::OGLViewer(QWidget *parent)
	: QOpenGLWidget(parent)
	, cv_op_mode(DRAWING_MODE)
	, curve_degree(2), curve_seg(20)
	, curPoint(nullptr), viewScale(1.0)
	, drawCtrlPts(true), drawCurves(true)
{
	// Set surface format for current widget
	QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
	format.setSamples(4);
	format.setVersion(4, 5);
	format.setProfile(QSurfaceFormat::CoreProfile);
	this->setFormat(format);

	// Initialize camera
	updateCamera();
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
	//glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_LINE_STIPPLE);

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CCW); // set counter-clock-wise vertex order to mean the front
	
	GLint bufs, samples;
	glGetIntegerv(GL_SAMPLE_BUFFERS, &bufs);
	glGetIntegerv(GL_SAMPLES, &samples);
	printf("MSAA: buffers = %d samples = %d\n", bufs, samples);


	//////////////////////////////////////////////////////////////////////////
	// Initialize Buffers
	// DSA
	// Create VAO
	glCreateBuffers(1, &pts_vbo);
	glCreateVertexArrays(1, &pts_vao);
	glEnableVertexArrayAttrib(pts_vao, 0);
	// Setup the formats
	glVertexArrayAttribFormat(pts_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(pts_vao, 0, pts_vbo, 0, sizeof(float) * 2);
	glVertexArrayAttribBinding(pts_vao, 0, 0);

	// Curve VAO
	glCreateBuffers(1, &curve_ibo);
	glCreateVertexArrays(1, &curve_vao);
	glEnableVertexArrayAttrib(curve_vao, 0);

	glVertexArrayAttribFormat(curve_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(curve_vao, 0, pts_vbo, 0, sizeof(GLfloat) * 2);
	glVertexArrayAttribBinding(curve_vao, 0, 0);
	glVertexArrayElementBuffer(curve_vao, curve_ibo);

	// Setup the formats

	updateBufferData();
	//////////////////////////////////////////////////////////////////////////
	// Create shader files
	point_shader.reset(new GLSLProgram("shaders/points_vs.glsl", "shaders/points_fs.glsl", "shaders/points_gs.glsl"));
	curve_shader.reset(new GLSLProgram("shaders/catmull_rom_vs.glsl", "shaders/catmull_rom_fs.glsl", nullptr, "shaders/catmull_rom_tc.glsl", "shaders/catmull_rom_te.glsl"));

	// Get uniform variable location
	point_shader->add_uniformv("proj_mat"); // WorldToCamera matrix
	point_shader->add_uniformv("win_size");
	curve_shader->add_uniformv("proj_mat"); // WorldToCamera matrix
	curve_shader->add_uniformv("degree");
	curve_shader->add_uniformv("segments");
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
	lastMousePos[0] = e->x();
	lastMousePos[1] = e->y();
	
	if ((e->buttons() == Qt::LeftButton) && (e->modifiers() == Qt::AltModifier))
	{
		cout << "Mouse position: " << e->x() << ", " << e->y() << endl;
	}
	// Add points
	if (e->buttons() == Qt::LeftButton && cv_op_mode == DRAWING_MODE)
	{
		/*Point2f *pt = new Point2f(viewScale * (e->x() * 2 - width()) / static_cast<Float>(height()),
			viewScale * (1.0 - 2.0 * e->y() / static_cast<Float>(height())));*/
		
		ctrl_pts.push_back(QVector2D(e->x(), e->y()));
		
		addCurvePatch();

		//cout << *pt << endl;
		/*cout << "Mouse position: " << e->x() << ", " << e->y() << "\tPoint Position: " << ctrl_pts.back() << endl;*/
	}
	// Select closes point
	if (e->buttons() == Qt::LeftButton && cv_op_mode == EDIT_MODE)
	{
		curPoint = nullptr;
		/*Point2f np(viewScale * (e->x() * 2 - width()) / static_cast<Float>(height()),
			viewScale * (1.0 - 2.0 * e->y() / static_cast<Float>(height())));*/
		QVector2D np(e->x(), e->y());
		Float mindist = std::numeric_limits<Float>::max();
		for (int i = 0; i < ctrl_pts.size(); i++)
		{
			Float curdist = (np - ctrl_pts[i]).lengthSquared();
			if (curdist < mindist && curdist < 16)
			{
				curPoint = &ctrl_pts[i];
				mindist = curdist;
			}
		}
		//curPoint = ctrl_points[0];
	}
	updateBufferData();
	update();
};

void OGLViewer::mouseMoveEvent(QMouseEvent *e)
{
	int dx = e->x() - lastMousePos[0];
	int dy = e->y() - lastMousePos[1];

	// Zoom-In
	if ((e->buttons() == Qt::RightButton) && (e->modifiers() == Qt::AltModifier))
	{
		viewScale -= dx * 0.01;
		viewScale = viewScale < 0.001 ? 0.001 : viewScale;
		updateCamera();
	}
	// Drag Points
	if (e->buttons() == Qt::LeftButton && cv_op_mode == EDIT_MODE)
	{
		if (curPoint != nullptr)
		{
			/*curPoint->x = viewScale * (e->x() * 2 - width()) / static_cast<Float>(height());
			curPoint->y = viewScale * (1.0 - 2.0 * e->y() / static_cast<Float>(height()));*/
			curPoint->setX(width());
			curPoint->setY(height());
			addCurvePatch();
		}
		
	}

	lastMousePos[0] = e->x();
	lastMousePos[1] = e->y();
	update();
}



void OGLViewer::addCurvePatch()
{
	size_t sizept = ctrl_pts.size();
	if (sizept > 3)
	{
		ctrl_idx.resize((sizept - curve_degree - 1) << 2);
		size_t idOffset = ctrl_idx.size() - 1;
		ctrl_idx[idOffset--] = sizept - 1;
		ctrl_idx[idOffset--] = sizept - 2;
		ctrl_idx[idOffset--] = sizept - 3;
		ctrl_idx[idOffset--] = sizept - 4;
	}
	else
	{
		ctrl_idx.clear();
		ctrl_idx.shrink_to_fit();
	}
}

void OGLViewer::updateBufferData()
{
	if (ctrl_pts.empty())
	{
		glNamedBufferData(pts_vbo, 0, nullptr, GL_STATIC_DRAW);
	}
	else
	{
		glNamedBufferData(pts_vbo, sizeof(GLfloat) * ctrl_pts.size() * 2,
			&ctrl_pts[0], GL_STATIC_DRAW);
	}
	if (ctrl_idx.empty())
	{
		glNamedBufferData(curve_ibo, 0, nullptr, GL_STATIC_DRAW);
	}
	else
	{
		glNamedBufferData(curve_ibo, sizeof(GLuint) * ctrl_idx.size(), &ctrl_idx[0], GL_STATIC_DRAW);
	}
}

void OGLViewer::paintGL()
{
	// Make current window
	makeCurrent();
	glClearColor(0.64f, 0.64f, 0.64f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (ctrl_pts.size())
	{
		glBindVertexArray(pts_vao);
		if (drawCtrlPts)
		{
			// Bind VBOs
			//pts vbo
			
			glDisable(GL_MULTISAMPLE);
			glDisable(GL_BLEND);
			// Use shader program
			point_shader->use_program();

			// Apply uniform matrix
			glUniformMatrix4fv((*point_shader)("proj_mat"), 1, GL_FALSE, proj_mat);
			//glUniform1f((*points_shader)("pointsize"), 10.0 * viewScale / height());

			glLineWidth(1.0);
			glDrawArrays(GL_POINTS, 0, ctrl_pts.size());
		}
		//////////////////////////////////////////////////////////////////////////
		// Draw straight lines
		if (drawCurves)
		{
			glBindVertexArray(curve_vao);
			glLineWidth(1.6);

			glDisable(GL_MULTISAMPLE);
			glDisable(GL_BLEND);
			
			curve_shader->use_program();

			// Apply uniform matrix
			glUniformMatrix4fv((*curve_shader)("proj_mat"), 1, GL_FALSE, proj_mat);
			glUniform1i((*curve_shader)("degree"), curve_degree);
			glUniform1i((*curve_shader)("segments"), curve_seg);
			if (!ctrl_idx.empty())
			{
				glPatchParameteri(GL_PATCH_VERTICES, curve_degree * 2);
				glDrawElements(GL_PATCHES, ctrl_idx.size(), GL_UNSIGNED_INT, 0);
			}
		}
	}
	
}
//Resize function
void OGLViewer::resizeGL(int w, int h)
{
	updateCamera();
}

void OGLViewer::updateCamera()
{
	proj_mat[0] = 2.0 / static_cast<GLfloat>(width());
	proj_mat[5] = -2.0 / static_cast<GLfloat>(height());
}

void OGLViewer::initParas()
{
	update();
}

void OGLViewer::clearVertex()
{
	ctrl_pts.clear();
	// Clear intersection information
	update();
}

void OGLViewer::changeOperation(int val)
{
	cv_op_mode = val;
}

void OGLViewer::setSegment(int val)
{
	curve_seg = val;
	update();
}

void OGLViewer::setDispCtrlPts(bool mode)
{
	drawCtrlPts = mode;
	update();
}

void OGLViewer::setDispCurves(bool mode)
{
	drawCurves = mode;
	update();
}