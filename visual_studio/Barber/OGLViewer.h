#pragma once
#ifndef __OGLVIEWER__
#define __OGLVIEWER__

#include "GL/glew.h"

#include <QOpenGLWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QTime>
#include <QString>
#include <QFileDialog>
#include <QOpenGLFramebufferObject>
//#include <QOpenGLVertexArrayObject>
//#include <QGLFunctions>

#include "OpenGL_Utils/GLSLProgram.h"
#include "Math/MathUtil.h"
#include "Image/ImageData.h"
#include "Geometry/TriangleMesh.h"
#include "Math/Matrix4x4.h"
#include "Camera/perspCamera.h"
//#include "Accel/KdTreeAccel.h"
#include "hds_mesh.h"
#include "HairMesh.h"


//static Matrix4D matrix;// Transform matrix

/*
static GLfloat model_mat[16];// Uniform matrix buffer
static GLfloat view_mat[16];
static GLfloat proj_mat[16];*/
//////////////////////////////////////////////////////////////////////////
// Acceleration
//////////////////////////////////////////////////////////////////////////


const double eps = 5e-4;


// OpenGL Window in Qt
class OGLViewer : public QOpenGLWidget
{
	Q_OBJECT
public:
	enum Select_Mode
	{
		OBJECT_SELECT,
		STROKE_SELECT,
		LAYER_SELECT
	};
	enum Operation_Mode
	{
		OP_NULL,
		OP_TRANSLATE,
		OP_ROTATE,
		OP_SCALE
	};
	enum Operation_Axis
	{
		OP_Z_AXIS = 0,
		OP_Y_AXIS = 1,
		OP_X_AXIS = 2
	};
	//OGLViewer();
	OGLViewer(QWidget *parent = nullptr);
	~OGLViewer();

	void resetCamera();
	void initParas();
protected:
	void initializeGL() Q_DECL_OVERRIDE;
	void paintGL() Q_DECL_OVERRIDE;
	void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

	void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
private:
	void initShader();
	GLuint bindCharacter();
	GLuint bindHairMesh();

	void renderUidBuffer();
	void saveFrameBuffer();

	void generateHairCage();
	void extrude(double val);

	void openHMS(const QString &filename);
	void exportHMS(const QString &filename) const;
private: // Simulation part
	void animate();
	void createSimMesh();
	void changeSimStatus();
	void changeSimDrawStatus();
signals:
	void echoHint(const QString &);
public:
	double process_fps;
protected:
	perspCamera* view_cam;
	GLint m_Select;
private:// OpenGL variables
	friend class MainWindow;
	int fps;
	int tcount;

	QTime process_time;
	int m_lastMousePos[2];
	int m_selectMode;
	int m_operationMode;
	int m_operationAxis;
	int display_mode = 0;
	bool m_drawHairMesh;
	bool m_drawHairCurve;
	bool m_drawHairMeshWireframe;

	bool isSimulating = false;
	bool m_drawSimulation = false;
	QTimer *simTimer;
private: //Scene data
	HDS_Mesh* charMesh;
	vector<GLfloat> char_verts;
	vector<GLuint> char_idxs;
	GLuint char_pts_vbo;
	GLuint char_elb;
	GLuint char_vao;

	HairMesh* hairMesh;
	vector<GLfloat> hmsh_verts;
	vector<GLfloat> hmsh_sim_verts;
	vector<GLfloat> hmsh_colors;
	vector<GLuint> hmsh_idxs;
	vector<GLuint> hmsh_vtx_offset;	// Vertex offset, last val is the size
	vector<GLuint> hmsh_idx_offset; // Index component offest, need /4
	GLuint hmsh_pts_vbo;
	GLuint hmsh_elb;
	GLuint hmsh_vao;

	QColor hairRootColor = QColor(0x502800);
	QColor hairTipColor = QColor(0xFFFAC4);
	QColor hairScatterColor;
	GLfloat hair_mesh_opacity;

	int curLayerID;
	int curStrokeID;

private:
	vector<Shape*> triangleList;
	TriangleMesh* collisionMesh;
	KdTreeAccel *mytree;
private: //Shaders
	// Scene Object Shader
	GLSLProgram* shader_obj;// Character Shader
	GLSLProgram* shader_hairmesh;// Hair Mesh Shader
	GLSLProgram* shader_wireframe;// Wireframe Shader
	GLSLProgram* shader_sel_layer;// Selected Hair Layer Shader

	// Framebuffer Shaders
	GLSLProgram* shader_uid;// Picking Shader
	GLSLProgram* shader_stroke_uid;// Picking Shader
	GLSLProgram* shader_layer_uid;// Picking Shader

	// Hair Shader
	GLSLProgram* shader_hairstroke;
};

#endif // __OGLVIEWER__