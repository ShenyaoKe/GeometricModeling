#include "OGLViewer.h"

OGLViewer::OGLViewer(QWidget *parent)
	: QOpenGLWidget(parent), tcount(0), fps(30)
	, m_selectMode(OBJECT_SELECT), m_Select(-1)
	, curStrokeID(-1), curLayerID(-1)
	, hair_mesh_opacity(1.0)
{
	// Set surface format for current widget
	QSurfaceFormat format;
	format.setDepthBufferSize(32);
	format.setStencilBufferSize(8);
	format.setVersion(4, 5);
	format.setProfile(QSurfaceFormat::CoreProfile);
	this->setFormat(format);

	// Link timer trigger
	/*process_time.start();
	QTimer *timer = new QTimer(this);
	/ *timer->setSingleShot(false);* /
	connect(timer, SIGNAL(timeout()), this, SLOT(update()));
	timer->start(0);*/


	// Read obj file
#ifdef _DEBUG
	charMesh = new HDS_Mesh("../../scene/obj/monsterfrog.obj");
#else
	charMesh = new HDS_Mesh("quad_cube.obj");
#endif
	hairMesh = new HairMesh(charMesh);
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

	initShader();

	// Export vbo for shaders
	charMesh->exportIndexedVBO(&char_verts, nullptr, nullptr, &char_idxs, 1);
	bindCharacter();

	// Enable OpenGL features
	//glEnable(GL_MULTISAMPLE);
	//glEnable(GL_LINE_SMOOTH);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glFrontFace(GL_CCW); // set counter-clock-wise vertex order to mean the front

	/*GLint maxpathvertex;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &maxpathvertex);
	cout << "max patch vertex: " << maxpathvertex << endl;*/
}

void OGLViewer::initShader()
{
	// Create shader files

	// Get uniform variable location
	shader_obj = new GLSLProgram("quad_vs.glsl", "quad_fs.glsl", "quad_gs.glsl");
	shader_obj->add_uniformv("model_matrix");
	shader_obj->add_uniformv("view_matrix");
	shader_obj->add_uniformv("proj_matrix");
	shader_obj->add_uniformv("sel_id");

	shader_hairmesh = new GLSLProgram("hairmesh_vs.glsl", "hairmesh_fs.glsl", "hairmesh_gs.glsl");
	shader_hairmesh->add_uniformv("model_matrix");
	shader_hairmesh->add_uniformv("view_matrix");
	shader_hairmesh->add_uniformv("proj_matrix");
	shader_hairmesh->add_uniformv("selected");
	shader_hairmesh->add_uniformv("color");
	shader_hairmesh->add_uniformv("opacity");

	shader_wireframe = new GLSLProgram("hairmesh_vs.glsl", "wireframe_fs.glsl", "wireframe_gs.glsl");
	shader_wireframe->add_uniformv("view_matrix");
	shader_wireframe->add_uniformv("proj_matrix");

	shader_sel_layer = new GLSLProgram("sel_layer_vs.glsl", "sel_layer_fs.glsl", "sel_layer_gs.glsl");
	shader_sel_layer->add_uniformv("view_matrix");
	shader_sel_layer->add_uniformv("proj_matrix");

	// Shaders for picking
	shader_uid = new GLSLProgram("id_vs.glsl", "id_fs.glsl", "id_gs.glsl");
	shader_uid->add_uniformv("view_matrix");
	shader_uid->add_uniformv("proj_matrix");
	shader_uid->add_uniformv("mode");

	shader_stroke_uid = new GLSLProgram("id_vs.glsl", "id_stroke_fs.glsl", "id_gs.glsl");
	shader_stroke_uid->add_uniformv("view_matrix");
	shader_stroke_uid->add_uniformv("proj_matrix");
	shader_stroke_uid->add_uniformv("strokeid");
	shader_stroke_uid->add_uniformv("mode");

	shader_layer_uid = new GLSLProgram("id_vs.glsl", "id_layer_fs.glsl", "id_layer_gs.glsl");
	shader_layer_uid->add_uniformv("view_matrix");
	shader_layer_uid->add_uniformv("proj_matrix");
	shader_layer_uid->add_uniformv("mode");

	// Hair Strands Shader
	shader_hairstroke = new GLSLProgram("hairstroke_vs.glsl", "hairstroke_fs.glsl",
		"hairstroke_gs.glsl", "hairstroke_tc.glsl", "hairstroke_te.glsl");
	shader_layer_uid->add_uniformv("view_matrix");
	shader_layer_uid->add_uniformv("proj_matrix");
	shader_layer_uid->add_uniformv("degree");
	shader_layer_uid->add_uniformv("segments");
	shader_layer_uid->add_uniformv("NumStrips");
	shader_layer_uid->add_uniformv("pointsize");
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

GLuint OGLViewer::bindHairMesh()
{
	glDeleteVertexArrays(1, &hmsh_vao);
	glDeleteBuffers(1, &hmsh_pts_vbo);
	glDeleteBuffers(1, &hmsh_elb);

	glGenBuffers(1, &hmsh_pts_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, hmsh_pts_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * hmsh_verts.size(), &hmsh_verts[0], GL_STATIC_DRAW);

	glGenBuffers(1, &hmsh_elb);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hmsh_elb);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * hmsh_idxs.size(), &hmsh_idxs[0], GL_STATIC_DRAW);

	// Bind VAO
	glGenVertexArrays(1, &hmsh_vao);
	glBindVertexArray(hmsh_vao);
	glBindBuffer(GL_ARRAY_BUFFER, hmsh_pts_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hmsh_elb);

	//vao_handles.push_back(box_vao);
	return hmsh_vao;
}

void OGLViewer::renderUidBuffer()
{
	makeCurrent();
	QOpenGLFramebufferObject qfbo(width(), height(), QOpenGLFramebufferObject::CombinedDepthStencil, GL_TEXTURE_2D);
	qfbo.bind();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK); // cull back face
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	bindCharacter();
	//glBindVertexArray(char_vao);
	shader_uid->use_program();

	// Apply uniform matrix
	glUniform1i((*shader_uid)("mode"), m_selectMode);
	//glUniformMatrix4fv((*shader_id)("model_matrix"), 1, GL_FALSE, model_mat);
	glUniformMatrix4fv((*shader_uid)("view_matrix"), 1, GL_FALSE, view_mat);
	glUniformMatrix4fv((*shader_uid)("proj_matrix"), 1, GL_FALSE, proj_mat);
	glDrawElements(GL_LINES_ADJACENCY, char_idxs.size(), GL_UNSIGNED_INT, 0);

	if (m_selectMode != OBJECT_SELECT && !hairMesh->empty())
	{
		bindHairMesh();

		// Draw each stroke
		shader_stroke_uid->use_program();
		glUniform1i((*shader_stroke_uid)("mode"), m_selectMode);
		glUniformMatrix4fv((*shader_stroke_uid)("view_matrix"), 1, GL_FALSE, view_mat);
		glUniformMatrix4fv((*shader_stroke_uid)("proj_matrix"), 1, GL_FALSE, proj_mat);
		for (int i = 0; i < hmsh_idx_offset.size() - 1; i++)
		{
			glUniform1i((*shader_stroke_uid)("strokeid"), i);
			int size = hmsh_idx_offset[i + 1] - hmsh_idx_offset[i];
			glDrawElements(GL_LINES_ADJACENCY,
				hmsh_idx_offset[i + 1] - hmsh_idx_offset[i],
				GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * hmsh_idx_offset[i]));
		}
		if (m_selectMode == LAYER_SELECT)
		{
			if (curStrokeID < 0)
			{
				echoHint(tr("Select Stroke first!"));
			}
			else
			{
				shader_layer_uid->use_program();
				glUniform1i((*shader_layer_uid)("mode"), m_selectMode);
				glUniformMatrix4fv((*shader_layer_uid)("view_matrix"), 1, GL_FALSE, view_mat);
				glUniformMatrix4fv((*shader_layer_uid)("proj_matrix"), 1, GL_FALSE, proj_mat);
				glLineWidth(10);
				glDrawArrays(GL_LINES_ADJACENCY, hmsh_vtx_offset[curStrokeID],
					hmsh_idx_offset[curStrokeID + 1] - hmsh_idx_offset[curStrokeID]);
			}
		}
	}
	glBindVertexArray(0);
	shader_uid->unuse();
	qfbo.release();
	/*QImage bufferImg = qfbo.toImage();
	bufferImg.save("buffer.png");*/
	QRgb pixel = qfbo.toImage().pixel(m_lastMousePos[0], m_lastMousePos[1]);

	int res = (pixel >> 24) & 0xFF ? pixel & 0xFFFFFF : -1;
	switch (m_selectMode)
	{
	case OBJECT_SELECT:
		m_Select = res;
		curStrokeID = -1;
		curLayerID = -1;
		cout << "Select ID: " << res << endl;
		break;
	case STROKE_SELECT:
		curStrokeID = res;
		cout << "Stroke ID: " << res << endl;
		break;
	case LAYER_SELECT:
		curLayerID = res;
		cout << "Layer ID: " << res << endl;
		break;
	default:
		break;
	}
}

void OGLViewer::saveFrameBuffer()
{
	this->grab().save("../../scene/texture/screenshot.png");
}


void OGLViewer::paintGL()
{
	// Make curent window
	makeCurrent();
	// Clear background and color buffer
	glClearColor(0.6, 0.6, 0.6, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//////////////////////////////////////////////////////////////////////////
	// Character

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK); // cull back face
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	bindCharacter();
	//glBindVertexArray(char_vao);
	shader_obj->use_program();

	// Apply uniform matrix
	//glUniformMatrix4fv((*shader)("model_matrix"), 1, GL_FALSE, sphere_model_mat);
	glUniformMatrix4fv((*shader_obj)("view_matrix"), 1, GL_FALSE, view_mat);
	glUniformMatrix4fv((*shader_obj)("proj_matrix"), 1, GL_FALSE, proj_mat);
	int highlightID = m_selectMode == OBJECT_SELECT ? m_Select : -1;
	glUniform1i((*shader_obj)("sel_id"), highlightID);
	
	glDrawElements(GL_LINES_ADJACENCY, char_idxs.size(), GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(char_vao);
	//////////////////////////////////////////////////////////////////////////
	// Hair Mesh
	if (!hairMesh->empty())
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		bindHairMesh();
		
		/************************************************************************/
		/* Draw Hair Mesh                                                       */
		/************************************************************************/
		shader_hairmesh->use_program();

		glUniform1f((*shader_hairmesh)("opacity"), hair_mesh_opacity);
		glUniformMatrix4fv((*shader_hairmesh)("view_matrix"), 1, GL_FALSE, view_mat);
		glUniformMatrix4fv((*shader_hairmesh)("proj_matrix"), 1, GL_FALSE, proj_mat);

		for (int i = 0; i < hmsh_idx_offset.size() - 1; i++)
		{
			glUniform3fv((*shader_hairmesh)("color"), 1, &hmsh_colors[i * 3]);
			glUniform1i((*shader_hairmesh)("selected"), curStrokeID == i);
			glDrawElements(GL_LINES_ADJACENCY,
				hmsh_idx_offset[i + 1] - hmsh_idx_offset[i],
				GL_UNSIGNED_INT, (void*)(sizeof(GLuint) * hmsh_idx_offset[i]));
		}

		/************************************************************************/
		/* Draw Hair Mesh Wireframe                                             */
		/************************************************************************/
		glLineWidth(1);
		shader_wireframe->use_program();
		glUniformMatrix4fv((*shader_wireframe)("view_matrix"), 1, GL_FALSE, view_mat);
		glUniformMatrix4fv((*shader_wireframe)("proj_matrix"), 1, GL_FALSE, proj_mat);
		glDrawElements(GL_LINES_ADJACENCY, hmsh_idxs.size(), GL_UNSIGNED_INT, 0);


		/************************************************************************/
		/* Draw Selected Hair Mesh Layer                                        */
		/************************************************************************/
		if (curLayerID > 0 && curStrokeID >= 0 && m_selectMode == LAYER_SELECT)
		{
			//draw
			//glDisable(GL_BLEND);
			int startID = hmsh_vtx_offset[curStrokeID] + curLayerID * 4;

			shader_sel_layer->use_program();
			glLineWidth(2);
			glUniformMatrix4fv((*shader_sel_layer)("view_matrix"), 1, GL_FALSE, view_mat);
			glUniformMatrix4fv((*shader_sel_layer)("proj_matrix"), 1, GL_FALSE, proj_mat);
			glDrawArrays(GL_LINES_ADJACENCY, startID, 4);
		}
		if (m_drawHair)
		{
			shader_hairstroke->use_program();
			for (int i = 0; i < hmsh_vtx_offset.size() - 1; i++)
			{
				int patchSize = hmsh_vtx_offset[i + 1] - hmsh_vtx_offset[i];
				if (patchSize <= 4)
				{
					continue;
				}
				glPatchParameteri(GL_PATCH_VERTICES, patchSize);
				glDrawArrays(GL_PATCHES, hmsh_vtx_offset[i], patchSize);
			}
		}
		shader_sel_layer->unuse();
	}
	glBindVertexArray(0);
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
}
void OGLViewer::generateHairCage()
{
	if (m_Select >= 0)
	{
		auto face = charMesh->faceMap.at(m_Select);
		LayeredHairMesh* newStroke = new LayeredHairMesh(face);
		
		hairMesh->push_back(newStroke);
		curStrokeID = hairMesh->layers.size() - 1;
		hairMesh->exportIndexedVBO(&hmsh_verts, &hmsh_idxs,
			&hmsh_vtx_offset, &hmsh_idx_offset, &hmsh_colors);

		bindHairMesh();

		emit echoHint(tr("Ctrl+E to extrude from current mesh."));
	}
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
	case Qt::Key_F9:
	{
		m_selectMode = STROKE_SELECT;
		break;
	}
	case Qt::Key_F10:
	{
		m_selectMode = LAYER_SELECT;
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
		//charMesh->exportIndexedVBO(&char_verts, nullptr, nullptr, &char_idxs, 1);
		//bindCharacter();
		break;
	}
	case Qt::Key_E:
	{

		if (e->modifiers() == Qt::ControlModifier)
		{
			auto curLayer = hairMesh->layers[curStrokeID];
			curLayer->extrude(5);
			hairMesh->exportIndexedVBO(
				&hmsh_verts, &hmsh_idxs, &hmsh_vtx_offset, &hmsh_idx_offset);
			//bindHairMesh();
		}
		break;
	}
	case Qt::Key_Delete:
	{
		if (curStrokeID >= 0)
		{
			if (m_selectMode == STROKE_SELECT)
			{
				auto& layers = hairMesh->layers;
				layers.erase(layers.begin() + curStrokeID);
				curStrokeID = -1;
			}
			if (m_selectMode == LAYER_SELECT && curLayerID > 0)
			{
				hairMesh->layers[curStrokeID]->remove(curLayerID);
			}
			
			hairMesh->exportIndexedVBO(
				&hmsh_verts, &hmsh_idxs, &hmsh_vtx_offset, &hmsh_idx_offset);
			curLayerID = -1;
			
		}
		break;
	}
	case Qt::Key_W://scale
	{
		auto curLayer = hairMesh->layers[curStrokeID];
		curLayer->shrink(curLayerID, 1.2);
		hairMesh->exportIndexedVBO(
			&hmsh_verts, &hmsh_idxs, &hmsh_vtx_offset, &hmsh_idx_offset);
		//bindHairMesh();
		break;
	}
	case Qt::Key_S://scale
	{
		auto curLayer = hairMesh->layers[curStrokeID];
		curLayer->shrink(curLayerID, 0.8);
		hairMesh->exportIndexedVBO(
			&hmsh_verts, &hmsh_idxs, &hmsh_vtx_offset, &hmsh_idx_offset);
		//bindHairMesh();
		break;
	}
	case Qt::Key_Q:
	{
		auto curLayer = hairMesh->layers[curStrokeID];
		curLayer->twist(curLayerID, 0.5);
		hairMesh->exportIndexedVBO(
			&hmsh_verts, &hmsh_idxs, &hmsh_vtx_offset, &hmsh_idx_offset);
		//bindHairMesh();
		break;
	}
	case Qt::Key_A:
	{
		auto curLayer = hairMesh->layers[curStrokeID];
		curLayer->twist(curLayerID, -0.5);
		hairMesh->exportIndexedVBO(
			&hmsh_verts, &hmsh_idxs, &hmsh_vtx_offset, &hmsh_idx_offset);
		//bindHairMesh();
		break;
	}
	case Qt::Key_Up:
	{
		if (!hairMesh->empty())
		{
			curLayerID++;
			curLayerID = min(curLayerID, hairMesh->sizeAtStroke(curStrokeID));
			cout << "Layer: " << curLayerID << endl;
		}
		
		break;
	}
	case Qt::Key_Down:
	{
		curLayerID = max(--curLayerID, 1);
		cout << "Layer: " << curLayerID << endl;
		break;
	}
	case Qt::Key_Left:
	{
		curStrokeID = min(++curStrokeID, (int)hairMesh->layers.size());
		cout << "Stroke: " << curStrokeID << endl;
		break;
	}
	case Qt::Key_Right:
	{
		curStrokeID = max(--curStrokeID, 0);
		cout << "Stroke: " << curStrokeID << endl;
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