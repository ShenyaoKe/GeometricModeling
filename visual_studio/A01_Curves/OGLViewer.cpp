#include "OGLViewer.h"

OGLViewer::OGLViewer(QWidget *parent)
	: QOpenGLWidget(parent)
	, cv_op_mode(DRAWING_MODE), cv_type(0)
	, curve_degree(3), curve_seg(200)
	, curPoint(nullptr), viewScale(1.0)
	, drawCtrlPts(true), drawCurves(true), drawIntx(true)
{
	// Set surface format for current widget
	QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
	format.setSamples(4);
	format.setVersion(4, 5);
	format.setProfile(QSurfaceFormat::CoreProfile);
	this->setFormat(format);

	// Initialize points
	/*Point2f *p = new Point2f(width() * 0.5, height() * 0.5);
	ctrl_points.push_back(p);
	exportPointVBO(points_verts);*/

	// Initialize camera
	/*proj_mat[0] = static_cast<GLfloat>(height()) / static_cast<GLfloat>(width()) / viewScale;
	proj_mat[5] /= viewScale;*/
	updateCamera();
}

OGLViewer::~OGLViewer()
{
	for (auto pt : ctrl_pts)
	{
		delete pt;
	}
	// Clear intersection information
	for (auto intx : intersects)
	{
		delete intx;
	}
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

	// Intersection Buffers
	glCreateBuffers(1, &intx_vbo);
	glCreateVertexArrays(1, &intx_vao);
	glEnableVertexArrayAttrib(intx_vao, 0);

	// Setup the formats
	glVertexArrayAttribFormat(intx_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayVertexBuffer(intx_vao, 0, intx_vbo, 0, sizeof(float) * 2);
	glVertexArrayAttribBinding(intx_vao, 0, 0);

	updateBufferData();
	//////////////////////////////////////////////////////////////////////////
	// Create shader files
	points_shader.reset(new GLSLProgram("points_vs.glsl", "points_fs.glsl", "points_gs.glsl"));
	intx_shader.reset(new GLSLProgram("intersection_vs.glsl", "intersection_fs.glsl", "intersection_gs.glsl"));
	
	cvShaders[0].reset(new GLSLProgram("curve_vs.glsl", "curve_fs.glsl", nullptr, "curve_tc.glsl", "lagrange_te.glsl"));
	cvShaders[1].reset(new GLSLProgram("curve_vs.glsl", "curve_fs.glsl", nullptr, "curve_tc.glsl", "bezier_te.glsl"));
	cvShaders[2].reset(new GLSLProgram("curve_vs.glsl", "curve_fs.glsl", nullptr, "curve_tc.glsl", "bspline_te.glsl"));
	cvShaders[3].reset(new GLSLProgram("curve_vs.glsl", "curve_fs.glsl", nullptr, "catmull_rom_tc.glsl", "catmull_rom_te.glsl"));
	cvShaders[4].reset(new GLSLProgram("curve_vs.glsl", "curve_fs.glsl"));

	// Get uniform variable location
	points_shader->add_uniformv("proj_mat"); // WorldToCamera matrix
	points_shader->add_uniformv("win_size");
	for (int i = 0; i < 5 ; i++)
	{
		cvShaders[i]->add_uniformv("proj_mat"); // WorldToCamera matrix
		cvShaders[i]->add_uniformv("degree");
		cvShaders[i]->add_uniformv("segments");
	}
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
		
		ctrl_pts.push_back(new Point2f(static_cast<Float>(e->x()), static_cast<Float>(e->y())));

		this->exportPointVBO(points_verts);

		//cout << *pt << endl;
		cout << "Mouse position: " << e->x() << ", " << e->y() << "\tPoint Position: " << *ctrl_pts.back() << endl;
	}
	// Select closes point
	if (e->buttons() == Qt::LeftButton && cv_op_mode == EDIT_MODE)
	{
		curPoint = nullptr;
		/*Point2f np(viewScale * (e->x() * 2 - width()) / static_cast<Float>(height()),
			viewScale * (1.0 - 2.0 * e->y() / static_cast<Float>(height())));*/
		Point2f np(e->x(), e->y());
		Float mindist = std::numeric_limits<Float>::max();
		for (int i = 0; i < ctrl_pts.size(); i++)
		{
			Float curdist = (np - *ctrl_pts[i]).lengthSquared();
			if (curdist < mindist && curdist < 16)
			{
				curPoint = ctrl_pts[i];
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
			curPoint->x = static_cast<Float>(height());
			curPoint->y = static_cast<Float>(height());
			exportPointVBO(points_verts);
			if (drawIntx)
			{
				findIntersections();
			}
		}
		
	}

	lastMousePos[0] = e->x();
	lastMousePos[1] = e->y();
	update();
}



void OGLViewer::exportPointVBO(vector<GLfloat> &ptsVBO)
{
	if (ctrl_pts.size())
	{
		ptsVBO.resize(ctrl_pts.size() * 2);

		for (int i = 0; i < ctrl_pts.size(); i++)
		{
			ptsVBO[i * 2] = static_cast<GLfloat>(ctrl_pts[i]->x);
			ptsVBO[i * 2 + 1] = static_cast<GLfloat>(ctrl_pts[i]->y);
		}
	}
}

void OGLViewer::updateBufferData()
{
	if (points_verts.empty())
	{
		glNamedBufferData(pts_vbo, 0, nullptr, GL_STATIC_DRAW);
	}
	else
	{
		glNamedBufferData(pts_vbo, sizeof(GLfloat) * points_verts.size(),
			&points_verts[0], GL_STATIC_DRAW);
	}
	if (intx_verts.empty())
	{
		glNamedBufferData(intx_vbo, 0, nullptr, GL_STATIC_DRAW);
	}
	else
	{
		glNamedBufferData(intx_vbo, sizeof(GLfloat) * intx_verts.size(),
			&intx_verts[0], GL_STATIC_DRAW);
	}
}

bool OGLViewer::intersect(const vector<Point2f*> &cv0, const vector<Point2f*> &cv1)
{
	bool ret = false;
	Bounds2f bound0(cv0), bound1(cv1);
	
	if (covered(bound0, bound1))
	{
		bool isLine0 = isLine(cv0);
		bool isLine1 = isLine(cv1);
		if (isLine0 && isLine1)// both curve can be lines
		{
			ret = intersect(cv0.front(), cv0.back(), cv1.front(), cv1.back());
		}
		else if (isLine0 && !isLine1)
		{
			ret |= intersect(cv1, cv0.front(), cv0.back());
		}
		else if (!isLine0 && isLine1)
		{
			ret |= intersect(cv0, cv1.front(), cv1.back());
		}
		else
		{
			vector<Point2f*> cv0_0, cv0_1, cv1_0, cv1_1;
			subdivBezier(cv0, cv0_0, cv0_1);
			subdivBezier(cv1, cv1_0, cv1_1);

			ret |= intersect(cv0_0, cv1_0);
			ret |= intersect(cv0_0, cv1_1);
			ret |= intersect(cv0_1, cv1_0);
			ret |= intersect(cv0_1, cv1_1);
		}
	}
	return ret;
}
// Box-Line Intersection
bool OGLViewer::intersect(const vector<Point2f*> cv,
	const Point2f* p0, const Point2f* p1)
{
	bool isIntersect = false;
	Bounds2f bound(cv);

	if (!covered(bound, p0, p1)) // Line !cover bound
	{
		return false;
	}
	else
	{
		vector<Point2f*> cv0, cv1;
		subdivBezier(cv, cv0, cv1);

		bool isLine0 = isLine(cv0);
		bool isLine1 = isLine(cv1);
		if (isLine0)
		{
			isIntersect |= intersect(cv0.front(), cv0.back(), p0, p1);
		}
		else
		{
			isIntersect |= intersect(cv0, p0, p1);
		}
		if (isLine1)
		{
			isIntersect |= intersect(cv1.front(), cv1.back(), p0, p1);
		}
		else
		{
			isIntersect |= intersect(cv1, p0, p1);
		}
	}
	return isIntersect;
}
// Line-Line Intersection
bool OGLViewer::intersect(const Point2f* p0, const Point2f* p1, const Point2f* q0, const Point2f* q1)
{
	Vector2f dp(p1->x - p0->x, p1->y - p0->y), dq(q1->x - q0->x, q1->y - q0->y);
	Vector2f pq0(q0->x - p0->x, q0->y - p0->y), pq1(q1->x - p0->x, q1->y - p0->y);

	if (Cross(dp, dq) == 0.0)
	{
		// Parallel
		if (Cross(pq0, pq1) != 0.0) return false;

		// Colinear
		Vector2f unitDp = Normalize(dp);
		double lenP = Dot(dp, unitDp);
		double distPQ0 = Dot(pq0, unitDp), distPQ1 = Dot(pq1, unitDp);
		
		// make distPQ0 smaller value
		if (distPQ0 > distPQ1) swap(distPQ0, distPQ1);
		if (distPQ0 > lenP || distPQ1 < lenP) return false;

		if (distPQ0 < 0) distPQ0 = 0;
		if (distPQ1 > lenP) distPQ1 = lenP;

		Point2f* iPt0 = new Point2f(p0->x + unitDp.x * distPQ0, p0->y + unitDp.y * distPQ0);
		Point2f* iPt1 = new Point2f(p0->x + unitDp.x * distPQ1, p0->y + unitDp.y * distPQ1);
		intersects.push_back(iPt0);
		intersects.push_back(iPt1);
		return true;
	}

	//
	Vector2f np(dp.y, -dp.x), nq(dq.y, -dq.x);

	double sign0 = Dot(pq0, np), sign1 = Dot(pq1, np);
	if ((sign0 <= 0 && sign1 <= 0) || (sign0 >= 0 && sign1 >= 0))
	{
		return false;
	}

	Vector2f qp0(p0->x - q0->x, p0->y - q0->y), qp1(p1->x - q0->x, p1->y - q0->y);

	double sign2 = Dot(qp0, nq), sign3 = Dot(qp1, nq);
	if ((sign2 <= 0 && sign3 <= 0) || (sign2 >= 0 && sign3 >= 0))
	{
		return false;
	}

	double t = (qp0.x * dq.y - qp0.y * dq.x) / (dp.y * dq.x - dp.x * dq.y);
	Point2f* iPt = new Point2f(p0->x + dp.x * t, p0->y + dp.y * t);
	intersects.push_back(iPt);
	
	return true;
}

bool OGLViewer::internalIntersect(const vector<Point2f*> &cv)
{
	vector<Point2f*> cv0, cv1;
	subdivBezier(cv, cv0, cv1);
	intersect(cv0, cv1);
	return false;
}


void OGLViewer::paintGL()
{
	// Make curent window
	makeCurrent();
	// Clear background and color buffer
	//glClearColor(0.26, 0.72, 0.94, 1.0);
	glClearColor(0.64, 0.64, 0.64, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	if (ctrl_pts.size() && !points_verts.empty())
	{
		glBindVertexArray(pts_vao);
		if (drawCtrlPts)
		{
			// Bind VBOs
			//pts vbo
			glDisable(GL_MULTISAMPLE);
			glDisable(GL_BLEND);


			// Use shader program
			points_shader->use_program();

			// Apply uniform matrix
			glUniformMatrix4fv((*points_shader)("proj_mat"), 1, GL_FALSE, proj_mat);
			//glUniform1f((*points_shader)("pointsize"), 10.0 * viewScale / height());

			glLineWidth(1.0);
			glDrawArrays(GL_POINTS, 0, ctrl_pts.size());
		}
		
		
		//////////////////////////////////////////////////////////////////////////
		// Draw straight lines
		if (drawCurves)
		{
			glLineWidth(1.6);

			glEnable(GL_MULTISAMPLE);
			glEnable(GL_BLEND);

			
			cvShader = cvShaders[cv_type];
			cvShader->use_program();

			// Apply uniform matrix
			glUniformMatrix4fv((*cvShader)("proj_mat"), 1, GL_FALSE, proj_mat);
			glUniform1i((*cvShader)("degree"), curve_degree);
			glUniform1i((*cvShader)("segments"), curve_seg);
			if (ctrl_pts.size() > curve_degree)
			{

				if (cv_type == 2)// B-Spline
				{
					glPatchParameteri(GL_PATCH_VERTICES, curve_degree + 1);
					for (int i = 0; i < ctrl_pts.size() - curve_degree; i++)
					{
						glDrawArrays(GL_PATCHES, i, curve_degree + 1);
					}
				}
				else if (cv_type == 3)// Catmull-Rom Spline
				{
					if (ctrl_pts.size() >= (curve_degree * 2))
					{
						glPatchParameteri(GL_PATCH_VERTICES, curve_degree * 2);
						for (int i = 0; i <= ctrl_pts.size() - curve_degree * 2; i++)
						{
							glDrawArrays(GL_PATCHES, i, 2 * curve_degree);
						}
					}

				}
				else// Bezier and Lagrange
				{
					glPatchParameteri(GL_PATCH_VERTICES, curve_degree + 1);
					for (int i = 0; i < (ctrl_pts.size() - 1) / curve_degree; i++)
					{
						glDrawArrays(GL_PATCHES, i * curve_degree, curve_degree + 1);
					}
				}
			}
			else
			{
				cvShaders[4]->use_program();
				//glUniformMatrix4fv(curve_proj_mat_loc, 1, GL_FALSE, proj_mat);
				glDrawArrays(GL_LINE_STRIP, 0, ctrl_pts.size());
			}
		}
		
		//////////////////////////////////////////////////////////////////////////
		// Draw Intersections
		if (drawIntx && intersects.size() > 0)
		{

			glLineWidth(1.0);
			//GLfloat *intersection_verts = new GLfloat[intersections.size() * 2];
			

			glBindVertexArray(intx_vao);

			// Use shader program
			intx_shader->use_program();

			// Apply uniform matrix
			glUniformMatrix4fv(intx_shader->getUniformLocation("proj_mat"), 1, GL_FALSE, proj_mat);
			glUniform1f(intx_shader->getUniformLocation("pointsize"), 10.0 * viewScale / height());
			glDrawArrays(GL_POINTS, 0, intersects.size());
		}
		//glDrawArrays(GL_PATCHES, 0, 4);
		//glDrawArrays(GL_PATCHES, 4, 4);
	}
	
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
	/*proj_mat[0] = static_cast<GLfloat>(height()) / static_cast<GLfloat>(width()) / viewScale;
	proj_mat[5] = 1.0 / viewScale;*/
	proj_mat[0] = 2.0 / static_cast<GLfloat>(width());
	proj_mat[5] = -2.0 / static_cast<GLfloat>(height());
}

void OGLViewer::initParas()
{
	update();
}

void OGLViewer::clearVertex()
{
	for (auto pt : ctrl_pts)
	{
		delete pt;
	}	
	ctrl_pts.clear(); ctrl_pts.shrink_to_fit();
	points_verts.clear(); points_verts.shrink_to_fit();
	// Clear intersection information
	for (auto intx : intersects)
	{
		delete intx;
	}
	intersects.clear(); intersects.shrink_to_fit();
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
	for (int i = 0; i < intersects.size(); i++)
	{
		delete intersects[i];
	}
	intersects.clear();

	//draw_intersection = true;
	int segNum = (ctrl_pts.size() - 1) / curve_degree;
	if (segNum > 0)
	{
		vector<Point2f*> *segs = new vector<Point2f*>[segNum];
		for (int i = 0; i < segNum; i++)
		{
			for (int j = 0; j < curve_degree + 1; j++)
			{
				segs[i].push_back(ctrl_pts[i * curve_degree + j]);
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
		intx_verts.resize(intersects.size() * 2);
		for (int i = 0; i < intersects.size(); i++)
		{
			intx_verts[i * 2] = static_cast<GLfloat>(intersects[i]->x);
			intx_verts[i * 2 + 1] = static_cast<GLfloat>(intersects[i]->y);
		}
		cout << "Intersection numbers" << intersects.size() << endl;
	}
	else
	{
		cout << "Not enough points for intersection!" << endl;
	}
	updateBufferData();

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
	switch (cv_type)
	{
	case 0:
		fprintf(VEC_File, "#Lagrange Curve\nc %d\n", cv_type);
		break;
	case 1:
		fprintf(VEC_File, "#Bezier Curve\nc %d\n", cv_type);
		break;
	case 2:
		fprintf(VEC_File, "#B-Spline\nc %d\n", cv_type);
		break;
	case 3:
		fprintf(VEC_File, "#Catmull-Rom Curve\nc %d\n", cv_type);
		break;
	default:
		break;
	}
	// Write curve degree
	fprintf(VEC_File, "d %d\n", curve_degree);
	// Write curve segments
	fprintf(VEC_File, "s %d\n", curve_seg);

	for (int i = 0; i < ctrl_pts.size(); i++)
	{
		fprintf(VEC_File, "v %lf %lf\n",
			ctrl_pts[i]->x, ctrl_pts[i]->y);
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
			fscanf_s(VEC_File, " %d", &cv_type);
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
			Point2f* vtx = new Point2f;
			fscanf_s(VEC_File, " %lf %lf\n", &vtx->x, &vtx->y);
			ctrl_pts.push_back(vtx);
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
	for (int i = 0; i < (ctrl_pts.size() - 1) / 3; i++)
	{
		int idx = i * 3;

		fprintf(SVG_File,
			"\t<path d=\"M%lf,%lf C%lf,%lf %lf,%lf %lf,%lf\" stroke=\"rgb(102,255,153)\" " \
			"stroke-width=\"2\" fill=\"none\"/>\n",
			ctrl_pts[idx]->x * coef + halfW, halfH - ctrl_pts[idx]->y * coef,
			ctrl_pts[idx + 1]->x * coef + halfW, halfH - ctrl_pts[idx + 1]->y * coef,
			ctrl_pts[idx + 2]->x * coef + halfW, halfH - ctrl_pts[idx + 2]->y * coef,
			ctrl_pts[idx + 3]->x * coef + halfW, halfH - ctrl_pts[idx + 3]->y * coef
			);
	}
	fprintf(SVG_File,
		"</g>\n<g stroke=\"rgb(133,51,255)\" stroke-width=\"2\" fill=\"rgb(133,51,255)\">\n");
  
	for (int i = 0; i < ctrl_pts.size(); i++)
	{
		fprintf(SVG_File,
			"\t<circle cx=\"%lf\" cy=\"%lf\" r=\"3\" />\n",
			ctrl_pts[i]->x * coef + halfW, halfH - ctrl_pts[i]->y * coef);
	}
	fprintf(SVG_File, "</g>\n");

	fprintf(SVG_File, "</svg>");
	fclose(SVG_File);
	cout << "SVG file saved successfully!\n";
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
	drawIntx = mode;
	update();
}
