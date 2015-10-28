#include "OGLViewer.h"

OGLViewer::OGLViewer(QWidget *parent)
	: QOpenGLWidget(parent)
	, cv_op_mode(DRAWING_MODE)
	, curve_degree(3), curve_seg(200)
	, curPoint(nullptr)
	, viewScale(1.0), viewTx(0), viewTy(0)
	, drawCtrlPts(true), drawCurves(true)
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
	glEnable(GL_LINE_STIPPLE);

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
	// Create shader files
	points_shader = new GLSLProgram("points_vs.glsl", "points_fs.glsl", "points_gs.glsl");
	curve_shaders = new GLSLProgram("bezier_vs.glsl", "bezier_fs.glsl", "bezier_gs.glsl", "bezier_tc.glsl", "bezier_te.glsl");

	// Export vbo for shaders

	// Get uniform variable location
	point_proj_mat_loc = points_shader->getUniformLocation("proj_matrix"); // WorldToCamera matrix
	win_size_loc = points_shader->getUniformLocation("win_size");
	curve_proj_mat_loc = curve_shaders->getUniformLocation("proj_matrix"); // WorldToCamera matrix
	curve_degree_loc = curve_shaders->getUniformLocation("degree");
	curve_seg_loc = curve_shaders->getUniformLocation("segments");

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
		Point3D *pt = new Point3D(viewScale * (e->x() * 2 - width()) / static_cast<Float>(height()) - viewTx,
			viewScale * (1.0 - 2.0 * e->y() / static_cast<Float>(height())) - viewTy, 0);
		ctrl_points.push_back(pt);

		this->exportPointVBO(points_verts);
		//cout << *pt << endl;
		cout << "Mouse position: " << e->x() << ", " << e->y() << endl;
	}
	// Select closes point
	if (e->buttons() == Qt::LeftButton && cv_op_mode == EDIT_MODE)
	{
		curPoint = nullptr;
		Point3D *np = new Point3D(viewScale * (e->x() * 2 - width()) / static_cast<Float>(height()) - viewTx,
			viewScale * (1.0 - 2.0 * e->y() / static_cast<Float>(height())) - viewTy, 0);
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
	if (e->buttons() == Qt::MidButton && e->modifiers() == Qt::AltModifier)
	{
		viewTx += viewScale * dx / static_cast<GLfloat>(height());// *0.01;
		viewTy -= viewScale * dy / static_cast<GLfloat>(height());//* 0.01;
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
	//glClearColor(0.64, 0.64, 0.64, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	if (ctrl_points.size() && points_verts != nullptr)
	{
		if (drawCtrlPts)
		{
			// Bind VBOs
			//pts vbo
			glDisable(GL_MULTISAMPLE);
			//glDisable(GL_BLEND);
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

			glLineWidth(1.0);
			glDrawArrays(GL_POINTS, 0, ctrl_points.size());
		}
		
		
		//////////////////////////////////////////////////////////////////////////
		// Draw straight lines
		if (drawCurves)
		{
			glLineWidth(1.6);

			//glEnable(GL_MULTISAMPLE);
			glEnable(GL_BLEND);
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
				curve_shaders->use_program();

				// Apply uniform matrix
				glUniformMatrix4fv(curve_proj_mat_loc, 1, GL_FALSE, proj_mat);
				glUniform1i(curve_degree_loc, curve_degree);
				glUniform1i(curve_seg_loc, curve_seg);

				glPatchParameteri(GL_PATCH_VERTICES, curve_degree + 1);
				for (int i = 0; i < (ctrl_points.size() - 1) / curve_degree; i++)
				{
					glDrawArrays(GL_PATCHES, i * curve_degree, curve_degree + 1);
				}
			}
		}
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
}

void OGLViewer::updateCamera()
{
	proj_mat[0] = static_cast<GLfloat>(height()) / static_cast<GLfloat>(width()) / viewScale;
	proj_mat[5] = 1.0 / viewScale;
	proj_mat[12] = viewTx;	proj_mat[13] = viewTy;
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
	// Clear intersection information
	for (int i = 0; i < intersections.size(); i++)
	{
		delete intersections[i];
	}
	intersections.clear();
	update();
}

void OGLViewer::changeOperation(int val)
{
	cv_op_mode = val;
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
	// Write curve type
	fprintf(VEC_File, "#Bezier Curve\nc %d\n", BEZIER_CURVE);
	
	// Write curve degree
	fprintf(VEC_File, "d %d\n", curve_degree);
	// Write curve segments
	fprintf(VEC_File, "s %d\n", curve_seg);

	for (int i = 0; i < ctrl_points.size(); i++)
	{
		fprintf(VEC_File, "v %lf %lf %lf\n",
			ctrl_points[i]->x, ctrl_points[i]->y, ctrl_points[i]->z);
	}
	fclose(VEC_File);
}

void OGLViewer::readPoints(const char *filename)
{
	FILE *VEC_File;
	errno_t err = fopen_s(&VEC_File, filename, "r");
	if (err)
	{
		printf("Can't read file %s!\n", filename);
		return;
	}
	clearVertex();
	while (true)
	{
		char lineHeader[128];
		int res = fscanf_s(VEC_File, "%s", &lineHeader, _countof(lineHeader));
		if (res == EOF)
		{
			break;
		}

		if (strcmp(lineHeader, "c") == 0)
		{
			fscanf_s(VEC_File, " %d", BEZIER_CURVE);
		}
		else if (strcmp(lineHeader, "d") == 0)
		{
			fscanf_s(VEC_File, " %d", &curve_degree);
		}
		else if (strcmp(lineHeader, "s") == 0)
		{
			fscanf_s(VEC_File, " %d", &curve_seg);
		}
		else if (strcmp(lineHeader, "v") == 0)
		{
			Point3D* vtx = new Point3D;
			fscanf_s(VEC_File, " %lf %lf %lf\n", &vtx->x, &vtx->y, &vtx->z);
			ctrl_points.push_back(vtx);
		}
		else//if (strcmp(lineHeader, "#") == 0)
		{
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, VEC_File);
		}
	}
	fclose(VEC_File);
	exportPointVBO(points_verts);
	update();
}

void OGLViewer::exportSVG(const char *filename)
{
	FILE *SVG_File;
	errno_t err = fopen_s(&SVG_File, filename, "w");
	if (err)
	{
		printf("Can't write to files!\n");
		return;
	}
	//SVG file head
	fprintf(SVG_File,
		"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n" \
		"<svg width=\"%d\" height=\"%d\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n" \
		"<g opacity=\"0.8\">\n",
		width(), height());// define the size of export graph
	double coef = height() * 0.5 / viewScale;
	double halfW = width() * 0.5, halfH = height() * 0.5;
	for (int i = 0; i < (ctrl_points.size() - 1) / 3; i++)
	{
		int idx = i * 3;

		fprintf(SVG_File,
			"\t<path d=\"M%lf,%lf C%lf,%lf %lf,%lf %lf,%lf\" stroke=\"rgb(102,255,153)\" " \
			"stroke-width=\"2\" fill=\"none\"/>\n",
			ctrl_points[idx]->x * coef + halfW, halfH - ctrl_points[idx]->y * coef,
			ctrl_points[idx + 1]->x * coef + halfW, halfH - ctrl_points[idx + 1]->y * coef,
			ctrl_points[idx + 2]->x * coef + halfW, halfH - ctrl_points[idx + 2]->y * coef,
			ctrl_points[idx + 3]->x * coef + halfW, halfH - ctrl_points[idx + 3]->y * coef
			);
	}
	fprintf(SVG_File,
		"</g>\n<g stroke=\"rgb(133,51,255)\" stroke-width=\"2\" fill=\"rgb(133,51,255)\">\n");
  
	for (int i = 0; i < ctrl_points.size(); i++)
	{
		fprintf(SVG_File,
			"\t<circle cx=\"%lf\" cy=\"%lf\" r=\"3\" />\n",
			ctrl_points[i]->x * coef + halfW, halfH - ctrl_points[i]->y * coef);
	}
	fprintf(SVG_File, "</g>\n");

	fprintf(SVG_File, "</svg>");
	fclose(SVG_File);
	cout << "SVG file saved successfully!" << endl;
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