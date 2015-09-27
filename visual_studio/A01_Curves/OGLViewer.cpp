#include "OGLViewer.h"

OGLViewer::OGLViewer(QWidget *parent)
	: QOpenGLWidget(parent),
	cv_op_mode(DRAWING_MODE), cv_type(0),
	curve_degree(3), curve_seg(20),
	curPoint(nullptr), viewScale(1.0)
{
	// Set surface format for current widget
	QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
	format.setVersion(4, 4);
	format.setProfile(QSurfaceFormat::CoreProfile);
	this->setFormat(format);

	// Initialize points
	Point3D *p = new Point3D();
	ctrl_points.push_back(p);
	exportPointVBO(points_verts);

	// Initialize camera
	proj_mat[0] = static_cast<GLfloat>(height()) / static_cast<GLfloat>(width()) / viewScale;
	proj_mat[5] /= viewScale;
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
	
	curve_shaders[0] = new GLSLProgram("curve_vs.glsl", "curve_fs.glsl", nullptr, "curve_tc.glsl", "lagrange_te.glsl");
	curve_shaders[1] = new GLSLProgram("curve_vs.glsl", "curve_fs.glsl", nullptr, "curve_tc.glsl", "bezier_te.glsl");
	curve_shaders[2] = new GLSLProgram("curve_vs.glsl", "curve_fs.glsl", nullptr, "curve_tc.glsl", "bspline_te.glsl");
	curve_shaders[3] = new GLSLProgram("curve_vs.glsl", "curve_fs.glsl", nullptr, "catmull_rom_tc.glsl", "catmull_rom_te.glsl");
	curve_shaders[4] = new GLSLProgram("curve_vs.glsl", "curve_fs.glsl");

	// Export vbo for shaders

	// Get uniform variable location
	point_proj_mat_loc = points_shader->getUniformLocation("proj_matrix"); // WorldToCamera matrix
	win_size_loc = points_shader->getUniformLocation("win_size");
	curve_proj_mat_loc = curve_shaders[cv_type]->getUniformLocation("proj_matrix"); // WorldToCamera matrix
	curve_degree_loc = curve_shaders[cv_type]->getUniformLocation("degree");
	curve_seg_loc = curve_shaders[cv_type]->getUniformLocation("segments");

	cout << "Point Projection matrix location: " << point_proj_mat_loc << endl;
	cout << "Curve Projection matrix location: " << curve_proj_mat_loc << endl;
}
void OGLViewer::keyPressEvent(QKeyEvent *e)
{
	/*if (e->key() == Qt::Key_E)
	{
		cv_op_mode++;
		cv_op_mode %= 3;
	}
	else*/ if (e->key() == Qt::Key_Home)
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
	// Add points
	if (e->buttons() == Qt::LeftButton && cv_op_mode == DRAWING_MODE)
	{
		Point3D *pt = new Point3D(viewScale * (e->x() * 2 - width()) / static_cast<Float>(height()),
			viewScale * (1.0 - 2.0 * e->y() / static_cast<Float>(height())), 0);
		ctrl_points.push_back(pt);

		this->exportPointVBO(points_verts);
		//cout << *pt << endl;
		cout << "Mouse position: " << e->x() << ", " << e->y() << endl;
	}
	// Select closes point
	if (e->buttons() == Qt::LeftButton && cv_op_mode == EDIT_MODE)
	{
		curPoint = nullptr;
		Point3D *np = new Point3D(viewScale * (e->x() * 2 - width()) / static_cast<Float>(height()),
			viewScale * (1.0 - 2.0 * e->y() / static_cast<Float>(height())), 0);
		Float mindist = std::numeric_limits<Float>::infinity();
		for (int i = 0; i < ctrl_points.size(); i++)
		{
			Float curdist = (*np - *ctrl_points[i]).getLenSq();
			if (curdist < mindist && curdist < 0.1)
			{
				curPoint = ctrl_points[i];
				mindist = curdist;
			}
		}
		//curPoint = ctrl_points[0];
	}
	update();
};

void OGLViewer::mouseMoveEvent(QMouseEvent *e)
{
	int dx = e->x() - m_lastMousePos[0];
	int dy = e->y() - m_lastMousePos[1];

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
			curPoint->x = viewScale * (e->x() * 2 - width()) / static_cast<Float>(height());
			curPoint->y = viewScale * (1.0 - 2.0 * e->y() / static_cast<Float>(height()));
			exportPointVBO(points_verts);
		}
		
	}

	m_lastMousePos[0] = e->x();
	m_lastMousePos[1] = e->y();
	update();
}



void OGLViewer::exportPointVBO(GLfloat* &ptsVBO)
{
	if (ctrl_points.size())
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
}

void OGLViewer::paintGL()
{
	// Make curent window
	makeCurrent();
	// Clear background and color buffer
	//glClearColor(0.26, 0.72, 0.94, 1.0);
	glClearColor(0.64, 0.64, 0.64, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT); // cull back face
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (ctrl_points.size() && points_verts != nullptr)
	{
		// Bind VBOs
		//pts vbo
		GLuint pts_vbo;
		glGenBuffers(1, &pts_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, pts_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * ctrl_points.size(), points_verts, GL_STATIC_DRAW);

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
		glUniformMatrix4fv(point_proj_mat_loc, 1, GL_FALSE, proj_mat);
		glUniform1f(points_shader->getUniformLocation("pointsize"), 10.0 * viewScale / height());
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

		if (ctrl_points.size() > curve_degree)
		{
			curve_shaders[cv_type]->use_program();

			// Apply uniform matrix
			glUniformMatrix4fv(curve_proj_mat_loc, 1, GL_FALSE, proj_mat);
			glUniform1i(curve_degree_loc, curve_degree);
			glUniform1i(curve_seg_loc, curve_seg);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			//glDrawArrays(GL_LINE_STRIP, 0, ctrl_points.size());

			if (cv_type == 2)// B-Spline
			{
				glPatchParameteri(GL_PATCH_VERTICES, curve_degree + 1);
				for (int i = 0; i < ctrl_points.size() - curve_degree; i++)
				{
					glDrawArrays(GL_PATCHES, i, curve_degree + 1);
				}
			}
			else if (cv_type == 3)// Catmull-Rom Spline
			{
				if (ctrl_points.size() >= (curve_degree * 2))
				{
					glPatchParameteri(GL_PATCH_VERTICES, curve_degree * 2);
					for (int i = 0; i <= ctrl_points.size() - curve_degree * 2; i++)
					{
						glDrawArrays(GL_PATCHES, i, 2 * curve_degree);
					}
				}
				
			}
			else// Bezier and Lagrange
			{
				glPatchParameteri(GL_PATCH_VERTICES, curve_degree + 1);
				for (int i = 0; i < (ctrl_points.size() - 1) / curve_degree; i++)
				{
					glDrawArrays(GL_PATCHES, i * curve_degree, curve_degree + 1);
				}
			}
		} 
		else
		{
			curve_shaders[4]->use_program();
			glUniformMatrix4fv(curve_proj_mat_loc, 1, GL_FALSE, proj_mat);
			glDrawArrays(GL_LINE_STRIP, 0, ctrl_points.size());
		}
		
		
		//glDrawArrays(GL_PATCHES, 0, 4);
		//glDrawArrays(GL_PATCHES, 4, 4);
	}
	
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
	updateCamera();

	/*glViewport(0, 0, width(), height());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();*/
}

void OGLViewer::updateCamera()
{
	proj_mat[0] = static_cast<GLfloat>(height()) / static_cast<GLfloat>(width()) / viewScale;
	proj_mat[5] = 1.0 / viewScale;
}

void OGLViewer::initParas()
{
	update();
}

void OGLViewer::clearVertex()
{
	for (int i = 0; i < ctrl_points.size(); i++)
	{
		delete ctrl_points[i];
	}
	ctrl_points.clear();
	delete[] points_verts;
	points_verts = nullptr;
	exportPointVBO(points_verts);
	update();
}

void OGLViewer::changeOperation(int val)
{
	cv_op_mode = val;
}

void OGLViewer::changeCurveType(int new_cv_type)
{
	// Update curve type
	cv_type = new_cv_type;

	// Switch shader location
	curve_proj_mat_loc = curve_shaders[cv_type]->getUniformLocation("proj_matrix"); // WorldToCamera matrix
	curve_degree_loc = curve_shaders[cv_type]->getUniformLocation("degree");
	curve_seg_loc = curve_shaders[cv_type]->getUniformLocation("segments");

	cout << "Curve Projection matrix location: " << curve_proj_mat_loc << endl;
	update();
}

void OGLViewer::setDegree(int val)
{
	curve_degree = val;
	update();
}

void OGLViewer::setSegment(int val)
{
	curve_seg = val;
	update();
}

void OGLViewer::writePoints(const char *filename)
{
	FILE *VEC_File;
	errno_t err = fopen_s(&VEC_File, filename, "w");
	if (err)
	{
		printf("Can't write to file %s!\n", filename);
		return;
	}
	for (int i = 0; i < ctrl_points.size(); i++)
	{
		fprintf(VEC_File, "v %lf %lf %lf\n",
			ctrl_points[i]->x, ctrl_points[i]->y, ctrl_points[i]->z);
	}
	fclose(VEC_File);
}
