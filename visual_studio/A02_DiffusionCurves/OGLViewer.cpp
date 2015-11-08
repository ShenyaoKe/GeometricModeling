#include "OGLViewer.h"

OGLViewer::OGLViewer(QWidget *parent)
	: QOpenGLWidget(parent)
	, cv_op_mode(DRAWING_MODE), cv_closed(true)
	, curve_degree(3), curve_seg(200)
	, current_point(nullptr), current_curve(nullptr)
	, viewScale(1.0), viewTx(0), viewTy(0)
	, drawCtrlPts(true), drawCurves(true), drawImage(false)
{
	// Set surface format for current widget
	/*QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
	//format.setVersion(4, 4);
	//format.setProfile(QSurfaceFormat::CoreProfile);
	this->setFormat(format);*/


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
	
	/*GLint bufs, samples;
	glGetIntegerv(GL_SAMPLE_BUFFERS, &bufs);
	glGetIntegerv(GL_SAMPLES, &samples);
	printf("MSAA: buffers = %d samples = %d\n", bufs, samples);*/


	//////////////////////////////////////////////////////////////////////////
	// Create shader files
	points_shader = new GLSLProgram("points_vs.glsl", "points_fs.glsl", "points_gs.glsl");
	curve_shaders = new GLSLProgram("bezier_vs.glsl", "bezier_fs.glsl", "bezier_gs.glsl", "bezier_tc.glsl", "bezier_te.glsl");

	// Export vbo for shaders

	// Get uniform variable location
	point_size_loc = points_shader->getUniformLocation("pointsize");
	point_color_loc = points_shader->getUniformLocation("pointcolor");
	point_proj_mat_loc = points_shader->getUniformLocation("proj_matrix"); // WorldToCamera matrix
	win_size_loc = points_shader->getUniformLocation("win_size");

	curve_proj_mat_loc = curve_shaders->getUniformLocation("proj_matrix"); // WorldToCamera matrix
	curve_degree_loc = curve_shaders->getUniformLocation("degree");
	curve_seg_loc = curve_shaders->getUniformLocation("segments");
	cv_flc_loc = curve_shaders->getUniformLocation("f_lf_color");
	cv_frc_loc = curve_shaders->getUniformLocation("f_rt_color");
	cv_elc_loc = curve_shaders->getUniformLocation("e_lf_color");
	cv_erc_loc = curve_shaders->getUniformLocation("e_rt_color");
/*
	cout << "Point Projection matrix location: " << point_proj_mat_loc << endl;
	cout << "Curve Projection matrix location: " << curve_proj_mat_loc << endl;*/
}
void OGLViewer::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Home)
	{
		initParas();
	}
	else if (e->key() == Qt::Key_P && e->modifiers() == Qt::ControlModifier)
	{
		generateDiffusionCurve();
		saveFrameBuffer();
		//paintGL();
	}
	else if (e->key() == Qt::Key_D)
	{
		drawImage = !drawImage;
		//paintGL();
	}
	else if (e->key() == Qt::Key_Return)
	{
		cv_closed = true;
		current_curve = nullptr;
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
		if (cv_closed && current_curve == nullptr)
		{
			Bezier* newCurve = new Bezier;
			curves.push_back(newCurve);
			current_curve = newCurve;
			cv_closed = false;
		}
		QVector3D *pt = new QVector3D(viewScale * (e->x() * 2 - width()) / static_cast<double>(height()) - viewTx,
			viewScale * (1.0 - 2.0 * e->y() / static_cast<double>(height())) - viewTy, 0);
		current_curve->insertPoint(pt);
		emit selectionChanged();

		cout << "Mouse position: " << e->x() << ", " << e->y() << endl;
	}
	// Select closes point
	if (e->buttons() == Qt::LeftButton && cv_op_mode == EDIT_MODE)
	{
		QVector3D *np = new QVector3D(viewScale * (e->x() * 2 - width()) / static_cast<float>(height()) - viewTx,
			viewScale * (1.0 - 2.0 * e->y() / static_cast<float>(height())) - viewTy, 0);
		selectPoint(np);
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
		if (current_point != nullptr)
		{
			current_point->setX(viewScale * (e->x() * 2 - width()) / static_cast<float>(height()));
			current_point->setY(viewScale * (1.0 - 2.0 * e->y() / static_cast<float>(height())));
			//exportPointVBO(points_verts);
		}
	}
	m_lastMousePos[0] = e->x();
	m_lastMousePos[1] = e->y();
	update();
}

void OGLViewer::clearImageBuffers()
{
	for (int i = 0; i < ds_imgs.size(); i++)
	{
		delete ds_imgs[i];
		ds_imgs[i] = nullptr;
	}
	ds_imgs.clear();
	for (int i = 1; i < us_imgs.size(); i++)
	{
		delete us_imgs[i];
		us_imgs[i] = nullptr;
	}
	us_imgs.clear();
}

void OGLViewer::saveFrameBuffer()
{
	QString filepath = "../../scene/texture/";
	ds_imgs[0]->save(filepath + "origin.png");
	
	for (int i = 1; i < ds_imgs.size(); i++)
	{
		ds_imgs[i]->save(filepath + "downsample_" + QString::number(i) + ".png");
	}
	for (int i = 0; i < us_imgs.size() - 2; i++)
	{
		us_imgs[i]->save(filepath + "upsample_" + QString::number(i) + ".png");
	}

	us_imgs.back()->save(filepath + "output.png");
}

void OGLViewer::selectPoint(const QVector3D *cursor)
{
	current_point = nullptr;
	current_curve = nullptr;

	float mindist = std::numeric_limits<float>::infinity();
	for (auto curve : curves)
	{
		for (auto pt : curve->ctrlPts)
		{
			float curdist = (*cursor - *pt).lengthSquared();
			if (curdist < mindist && curdist < 0.1)
			{
				current_point = pt;
				current_curve = curve;
				mindist = curdist;
			}
		}
	}
	if (current_curve != nullptr)
	{
		emit selectionChanged();
	}
}

void OGLViewer::paintGL()
{
	// Make curent window
	makeCurrent();
	// Clear background and color buffer
	//glClearColor(0.26, 0.72, 0.94, 1.0);
	//glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//if (drawImage && us_imgs.size() > 0)
	/*if (drawImage)
	{
		//painter.begin(this);
		QPainter painter(this);
		painter.beginNativePainting();
		QImage paintImg = *us_imgs[us_imgs.size() - 1];
		//QImage paintImg("D:/shenyao_ke/Courses/GeometricModeling/scene/texture/upsample_0.png");
		painter.drawImage(rect(), paintImg, paintImg.rect());
		painter.endNativePainting();
		//painter.end();
	/// *
	}
	else*/
	{
		if (curves.size() == 0)
		{
			return;
		}
		for (auto curve : curves)
		{
			curve->exportVBO(curve_degree, curve_seg,
				points_size, points_verts, points_colors);
			if (points_size == 0)
			{
				continue;
			}
			if (drawCtrlPts)
			{
				// Bind VBOs
				//pts vbo
				glDisable(GL_MULTISAMPLE);
				glDisable(GL_BLEND);
				GLuint pts_vbo;
				glGenBuffers(1, &pts_vbo);
				glBindBuffer(GL_ARRAY_BUFFER, pts_vbo);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * points_size, points_verts, GL_STATIC_DRAW);

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
				if (curve == current_curve)
				{
					glUniform3fv(point_color_loc, 1, sel_pt_color);
				}
				else
				{
					glUniform3fv(point_color_loc, 1, uns_pt_color);
				}
				glUniformMatrix4fv(point_proj_mat_loc, 1, GL_FALSE, proj_mat);
				glUniform1f(point_size_loc, 10.0 * viewScale / height());

				glLineWidth(1.0);
				glDrawArrays(GL_POINTS, 0, points_size);
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
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * points_size, points_verts, GL_STATIC_DRAW);

				// Bind VAO
				GLuint curve_vao;
				glGenVertexArrays(1, &curve_vao);
				glBindVertexArray(curve_vao);
				glBindBuffer(GL_ARRAY_BUFFER, curve_vbo);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
				glEnableVertexAttribArray(0);

				if (points_size > curve_degree)
				{
					curve_shaders->use_program();

					// Apply uniform matrix
					glUniform3fv(cv_flc_loc, 1, points_colors);
					glUniform3fv(cv_frc_loc, 1, points_colors + 3);
					glUniform3fv(cv_elc_loc, 1, points_colors + 6);
					glUniform3fv(cv_erc_loc, 1, points_colors + 9);
					glUniformMatrix4fv(curve_proj_mat_loc, 1, GL_FALSE, proj_mat);
					glUniform1i(curve_degree_loc, curve_degree);
					glUniform1i(curve_seg_loc, curve_seg);

					glPatchParameteri(GL_PATCH_VERTICES, curve_degree + 1);
					for (int i = 0; i < (points_size - 1) / curve_degree; i++)
					{
						glDrawArrays(GL_PATCHES, i * curve_degree, curve_degree + 1);
					}
				}
			}
		}
	}
}
// Redraw function
void OGLViewer::paintEvent(QPaintEvent *e)
{
	/*if (drawImage)
	{
		QPainter painter(this);
		painter.begin(this);
		painter.beginNativePainting();
		//QImage paintImg = *us_imgs[us_imgs.size() - 1];
		QImage paintImg("D:/shenyao_ke/Courses/GeometricModeling/scene/texture/upsample_0.png");
		painter.drawImage(rect(), paintImg, paintImg.rect());
		painter.endNativePainting();
		painter.end();
	} 
	else
	{*/
		// Draw current frame
		paintGL();
	//}
	
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
	for (auto curve : curves)
	{
		delete curve;
	}
	curves.clear();
	current_curve = nullptr;
	cv_closed = true;
	update();
}

void OGLViewer::changeOperation(int val)
{
	cv_op_mode = val;
	cv_closed = true;
}

void OGLViewer::setDegree(int val)
{
	if (current_curve != nullptr)
	{
		current_curve->degree = val;
	}
	update();
}

void OGLViewer::setSegment(int val)
{
	if (current_curve != nullptr)
	{
		current_curve->segments = val;
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
	int i = 0;
	for (int i = 0; i < curves.size(); i++)
	{
		auto curve = curves[i];
	/*}
	for (auto curve : curves)
	{*/
		// Write curve type
		fprintf(VEC_File, "#Bezier Curve\ncid %d\n", i);

		// Write curve degree
		fprintf(VEC_File, "d %d\n", curve->degree);
		// Write curve segments
		fprintf(VEC_File, "s %d\n", curve_seg);
		//Write curve colors
		fprintf(VEC_File, "color %s %s %s %s\n",
			curve->colors[0].name().toStdString().c_str(),
			curve->colors[1].name().toStdString().c_str(),
			curve->colors[2].name().toStdString().c_str(),
			curve->colors[3].name().toStdString().c_str());

		for (int i = 0; i < curve->ctrlPts.size(); i++)
		{
			fprintf(VEC_File, "v %f %f %f\n",
				curve->ctrlPts[i]->x(), curve->ctrlPts[i]->y(), curve->ctrlPts[i]->z());
		}
		//i++;
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
	Bezier* curCV = nullptr;
	while (true)
	{
		char lineHeader[128];
		int res = fscanf_s(VEC_File, "%s", &lineHeader, _countof(lineHeader));
		if (res == EOF)
		{
			break;
		}
		if (strcmp(lineHeader, "cid") == 0)
		{
			int id;
			fscanf_s(VEC_File, " %d", &id);
			curCV = new Bezier;
			curves.push_back(curCV);
		}
		else if (strcmp(lineHeader, "d") == 0)
		{
			fscanf_s(VEC_File, " %d", &curCV->degree);
		}
		else if (strcmp(lineHeader, "s") == 0)
		{
			fscanf_s(VEC_File, " %d", &curCV->segments);
		}
		else if (strcmp(lineHeader, "v") == 0)
		{
			float x, y, z;
			fscanf_s(VEC_File, " %f %f %f\n", &x, &y, &z);
			curCV->insertPoint(new QVector3D(x, y, z));
		}
		else if (strcmp(lineHeader, "color") == 0)
		{
			char flc[8];
			char frc[8];
			char elc[8];
			char erc[8];
			fscanf_s(VEC_File, " %s", &flc, _countof(flc));
			fscanf_s(VEC_File, " %s", &frc, _countof(frc));
			fscanf_s(VEC_File, " %s", &elc, _countof(elc));
			fscanf_s(VEC_File, " %s\n", &erc, _countof(erc));
			cout << flc;
			//fscanf_s(VEC_File, " %s %s %s %s\n", flc, frc, elc, erc);
			QColor colors[4] = { QColor(flc), QColor(frc), QColor(elc), QColor(erc) };
			curCV->insertColor(colors);
		}
		else//if (strcmp(lineHeader, "#") == 0)
		{
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, VEC_File);
		}
	}
	fclose(VEC_File);
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
			ctrl_points[idx]->x()* coef + halfW, halfH - ctrl_points[idx]->y() * coef,
			ctrl_points[idx + 1]->x()* coef + halfW, halfH - ctrl_points[idx + 1]->y() * coef,
			ctrl_points[idx + 2]->x()* coef + halfW, halfH - ctrl_points[idx + 2]->y() * coef,
			ctrl_points[idx + 3]->x()* coef + halfW, halfH - ctrl_points[idx + 3]->y() * coef
			);
	}
	fprintf(SVG_File,
		"</g>\n<g stroke=\"rgb(133,51,255)\" stroke-width=\"2\" fill=\"rgb(133,51,255)\">\n");
  
	for (int i = 0; i < ctrl_points.size(); i++)
	{
		fprintf(SVG_File,
			"\t<circle cx=\"%lf\" cy=\"%lf\" r=\"3\" />\n",
			ctrl_points[i]->x() * coef + halfW, halfH - ctrl_points[i]->y() * coef);
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

void OGLViewer::generateDiffusionCurve()
{
	// Hide points
	bool old_draw = drawCtrlPts;
	drawCtrlPts = false;
	paintGL();

	// Image processing
	clearImageBuffers();
	drawImage = true;
	QImage* originImg = new QImage(this->grab().toImage());
	
	int len = min(originImg->width(), originImg->height()) + 1;
	int count = 1;
	// Down sample images
	ds_imgs.push_back(originImg);
	while (len >= 1)
	{
		QImage* newimg = downscale(ds_imgs.back());
		ds_imgs.push_back(newimg);

		len >>= 1;
		count++;
	}

	us_imgs.push_back(ds_imgs.back());
	for (int i = ds_imgs.size() - 2; i >= 0; i--)
	{
		QImage* newImg = upscale(ds_imgs[i], us_imgs.back());
		us_imgs.push_back(newImg);
	}

	// Revert to original draw mode
	drawCtrlPts = old_draw;
}
