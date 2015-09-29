#include "OGLViewer.h"

OGLViewer::OGLViewer(QWidget *parent)
	: QOpenGLWidget(parent),
	cv_op_mode(DRAWING_MODE), cv_type(0),
	curve_degree(3), curve_seg(200),
	curPoint(nullptr), viewScale(1.0),
	drawCtrlPts(true), drawCurves(true), drawIntersection(true)
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
	intersection_shader = new GLSLProgram("intersection_vs.glsl", "intersection_fs.glsl", "intersection_gs.glsl");
	
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
			if (drawIntersection)
			{
				findIntersections();
			}
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

bool OGLViewer::intersect(const vector<Point3D*> &cv0, const vector<Point3D*> &cv1)
{
	bool res = false;
	BBox bound0(cv0), bound1(cv1);
	double threshold = 0.001;
	
	/*cout << "Size of cv0: " << cv0.size() << " cv1: " << cv1.size() << endl;*/
	//cout << "Bound 0\n\tPmin: " << bound0.pMin << "\n\tPmax: " << bound0.pMax << endl;
	//cout << "Bound 1\n\tPmin: " << bound1.pMin << "\n\tPmax: " << bound1.pMax << endl;
	if (covered(bound0, bound1))
	{
		//return true;
		if (bound0.getDiagnal().getLength() < threshold
			&& bound1.getDiagnal().getLength() < threshold
			&& (bound0.getMidPoint() - bound1.getMidPoint()).getLength() < threshold)
		{
			/*if (cv0.front() == cv1.front() || cv0.front() == cv1.back() ||
				cv0.back() == cv1.front() || cv0.back() == cv1.back())
			{
				cout << "haha" << endl;
				
			}
			else*/
			Point3D *interPt = new Point3D((bound1.getMidPoint() + bound0.getMidPoint())*0.5);
			intersections.push_back(interPt);
			return true;
		}
		bool isLine0 = isLine(cv0);
		bool isLine1 = isLine(cv1);
		vector<Point3D*> cv0_0, cv0_1, cv1_0, cv1_1;
		subdivBezier(cv0, cv0_0, cv0_1);
		subdivBezier(cv1, cv1_0, cv1_1);

		res |= intersect(cv0_0, cv1_0);
		res |= intersect(cv0_0, cv1_1);
		res |= intersect(cv0_1, cv1_0);
		res |= intersect(cv0_1, cv1_1);
		//vector<Point3D>
	}
	return res;
}

bool OGLViewer::intersect(const vector<Point3D*> cv, const Point3D* p0, const Point3D* p1)
{
	return false;
}

bool OGLViewer::intersect(const Point3D* p0, const Point3D* p1, const Point3D* q0, const Point3D* q1)
{
	return false;
}

bool OGLViewer::internalIntersect(const vector<Point3D*> &cv)
{
	vector<Point3D*> cv0, cv1;
	subdivBezier(cv, cv0, cv1);
	intersect(cv0, cv1);
	return false;
}

void OGLViewer::subdivBezier(const vector<Point3D*> &cvs, vector<Point3D*> &lf_cvs, vector<Point3D*> &rt_cvs)
{
	vector<Point3D*> tmpCVS(cvs);
	lf_cvs.push_back(cvs.front());
	rt_cvs.push_back(cvs.back());
	for (int j = cvs.size() - 1; j > 0; j--)
	{
		for (int i = 0; i < j; i++)
		{
			Point3D* curPt = new Point3D((*tmpCVS[i] + *tmpCVS[i + 1]) * 0.5);
			tmpCVS[i] = curPt;
		}
		lf_cvs.push_back(tmpCVS[0]);
		rt_cvs.push_back(tmpCVS[j - 1]);
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

	
	if (ctrl_points.size() && points_verts != nullptr)
	{
		if (drawCtrlPts)
		{
			// Bind VBOs
			//pts vbo
			glDisable(GL_MULTISAMPLE);
			glDisable(GL_BLEND);
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
		}
		
		
		//////////////////////////////////////////////////////////////////////////
		// Draw straight lines
		if (drawCurves)
		{
			glEnable(GL_MULTISAMPLE);
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
				curve_shaders[cv_type]->use_program();

				// Apply uniform matrix
				glUniformMatrix4fv(curve_proj_mat_loc, 1, GL_FALSE, proj_mat);
				glUniform1i(curve_degree_loc, curve_degree);
				glUniform1i(curve_seg_loc, curve_seg);

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
		}
		
		//////////////////////////////////////////////////////////////////////////
		// Draw Intersections
		if (drawIntersection && intersections.size() > 0)
		{
			GLfloat *intersection_verts = new GLfloat[intersections.size() * 3];
			for (int i = 0; i < intersections.size(); i++)
			{
				intersection_verts[i * 3] = static_cast<GLfloat>(intersections[i]->x);
				intersection_verts[i * 3 + 1] = static_cast<GLfloat>(intersections[i]->y);
				intersection_verts[i * 3 + 2] = static_cast<GLfloat>(intersections[i]->z);
			}

			GLuint intersection_vbo;
			glGenBuffers(1, &intersection_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, intersection_vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * intersections.size(), intersection_verts, GL_STATIC_DRAW);

			// Bind VAO
			GLuint intersection_vao;
			glGenVertexArrays(1, &intersection_vao);
			glBindVertexArray(intersection_vao);
			glBindBuffer(GL_ARRAY_BUFFER, intersection_vbo);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
			glEnableVertexAttribArray(0);

			// Use shader program
			intersection_shader->use_program();

			// Apply uniform matrix
			glUniformMatrix4fv(intersection_shader->getUniformLocation("proj_matrix"), 1, GL_FALSE, proj_mat);
			glUniform1f(intersection_shader->getUniformLocation("pointsize"), 10.0 * viewScale / height());
			glDrawArrays(GL_POINTS, 0, intersections.size());
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

void OGLViewer::findIntersections()
{
	//drawIntersection = dispMode;
	cv_type = BEZIER_CURVE;
	// Clear intersection information
	for (int i = 0; i < intersections.size(); i++)
	{
		delete intersections[i];
	}
	intersections.clear();

	//draw_intersection = true;
	int segNum = (ctrl_points.size() - 1) / curve_degree;
	if (segNum > 0)
	{
		vector<Point3D*> *segs = new vector<Point3D*>[segNum];
		for (int i = 0; i < segNum; i++)
		{
			for (int j = 0; j < curve_degree + 1; j++)
			{
				segs[i].push_back(ctrl_points[i * curve_degree + j]);
			}
			internalIntersect(segs[i]);
		}
		if (segNum > 1)
		{
			for (int i = 0; i < segNum; i++)
			{
				for (int j = i + 1; j < segNum; j++)
				{
					intersect(segs[i], segs[j]);
				}
			}
		}
		cout << "Intersection numbers" << intersections.size() << endl;
	}
	else
	{
		cout << "Not enough points for intersection!" << endl;
	}
	
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

void OGLViewer::setDispIntersections(bool mode)
{
	drawIntersection = mode;
	update();
}
