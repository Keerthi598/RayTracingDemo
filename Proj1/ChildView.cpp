
// ChildView.cpp : implementation of the CChildView class
//

#include "pch.h"
#include "framework.h"
#include "Proj1.h"
#include "ChildView.h"
#include "graphics/OpenGLRenderer.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const float PI = 3.14159265358979f;

// CChildView

CChildView::CChildView()
{
	m_raytrace = false;

	m_camera.Set(70, 40, -30, 0, 0, 0, 0, 0, 0);
	//m_camera.Set(0, 30, 1, 0, 0, 0, 0, 0, 0);

	m_rayimage = NULL;


	CGrPtr<CGrComposite> scene = new CGrComposite;
	m_scene = scene;

	// A red box
	CGrPtr<CGrMaterial> yellowpaint = new CGrMaterial;
	yellowpaint->AmbientAndDiffuse(0.8f, 0.8f, 0.0f);
	scene->Child(yellowpaint);

	CGrPtr<CGrComposite> yellowbox = new CGrComposite;
	yellowpaint->Child(yellowbox);
	yellowbox->Box(-15, -4, 0, 5, 5, 5);

	// A white box
	CGrPtr<CGrMaterial> pinkpaint = new CGrMaterial;
	pinkpaint->AmbientAndDiffuse(0.8f, 0.f, 0.8f);
	scene->Child(pinkpaint);

	CGrPtr<CGrComposite> pinkbox = new CGrComposite;
	pinkpaint->Child(pinkbox);
	pinkbox->Box(-10, -5, 10, 5, 5, 5);
	

	// Floor
	CGrPtr<CGrMaterial> floorPaint = new CGrMaterial;
	floorPaint->AmbientAndDiffuse(0.8f, 0.8f, 0.8f);
	scene->Child(floorPaint);

	CGrPtr<CGrTexture> floorTex = new CGrTexture;
	floorTex->LoadFile(L"textures/plank01.BMP");

	CGrPtr<CGrPolygon> floor = new CGrPolygon;
	floorPaint->Child(floor);
	floor->Texture(floorTex);

	floor->AddNormal3d(0, 1, 0);
	floor->AddTex2d(0, 0);
	floor->AddVertex3d(-30, -10,  20);
	floor->AddTex2d(1, 0);
	floor->AddVertex3d(  0, -10,  20);
	floor->AddTex2d(1, 1);
	floor->AddVertex3d(  0, -10, -10);
	floor->AddTex2d(0, 1);
	floor->AddVertex3d(-30, -10, -10);




	// Pyramid
	CGrPtr<CGrMaterial> pyramidPaint = new CGrMaterial;
	pyramidPaint->AmbientAndDiffuse(0.f, 1.f, 1.f);
	scene->Child(pyramidPaint);

	CGrPtr<CGrPolygon> pyramid = new CGrPolygon;
	pyramidPaint->Child(pyramid);

	double temp_a[] = { -2, -6, -2};
	double temp_b[] = { -8, -6, -2};
	double temp_c[] = { -8, -6, -8};
	double temp_d[] = { -2, -6, -8};
	double temp_h[] = { -5,  0, -4 };
	
	pyramid->AddVertices3(temp_h, temp_a, temp_d, true);
	pyramid->AddVertices3(temp_h, temp_d, temp_c, true);
	pyramid->AddVertices3(temp_h, temp_c, temp_b, true);
	pyramid->AddVertices3(temp_h, temp_b, temp_a, true);

}

CChildView::~CChildView()
{
	if (m_rayimage)
		delete m_rayimage[0];

	delete m_rayimage;
}


BEGIN_MESSAGE_MAP(CChildView, COpenGLWnd)
	ON_WM_PAINT()
	ON_UPDATE_COMMAND_UI(ID_RAYTRACE_RENDER, &CChildView::OnUpdateRenderRaytrace)
	ON_COMMAND(ID_RAYTRACE_RENDER, &CChildView::OnRenderRaytrace)
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!COpenGLWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}


void CChildView::OnGLDraw(CDC* pDC)
{
	if (m_raytrace)
	{
		// Clear the color buffer
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set up for parallel projection
		int width, height;
		GetSize(width, height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, width, 0, height, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		

		// If we got it, draw it
		if (m_rayimage)
		{
			glRasterPos3i(0, 0, 0);
			glDrawPixels(m_rayimagewidth, m_rayimageheight,
				GL_RGB, GL_UNSIGNED_BYTE, m_rayimage[0]);
		}

		glFlush();
	}
	else
	{
		//
		// Instantiate a renderer
		//

		COpenGLRenderer renderer;

		// Configure the renderer
		ConfigureRenderer(&renderer);

		//
		// Render the scene
		//

		renderer.Render(m_scene);
	}
}


//
// Name :         CChildView::ConfigureRenderer()
// Description :  Configures our renderer so it is able to render the scene.
//                Indicates how we'll do our projection, where the camera is,
//                and where any lights are located.
//

void CChildView::ConfigureRenderer(CGrRenderer* p_renderer)
{
	// Determine the screen size so we can determine the aspect ratio
	int width, height;
	GetSize(width, height);
	double aspectratio = double(width) / double(height);

	//
	// Set up the camera in the renderer
	//

	p_renderer->Perspective(m_camera.FieldOfView(),
		aspectratio, // The aspect ratio.
		20., // Near clipping
		1000.); // Far clipping

	// m_camera.FieldOfView is the vertical field of view in degrees.

	//
	// Set the camera location
	//

	p_renderer->LookAt(m_camera.Eye()[0], m_camera.Eye()[1], m_camera.Eye()[2],
						m_camera.Center()[0], m_camera.Center()[1], m_camera.Center()[2],
						m_camera.Up()[0], m_camera.Up()[1], m_camera.Up()[2]);

	//
	// Set the light locations and colors
	//

	float dimd = 0.5f;
	GLfloat dim[] = { dimd, dimd, dimd, 1.0f };
	GLfloat brightwhite[] = { 1.f, 1.f, 1.f, 1.0f };

	p_renderer->AddLight(CGrPoint(1, 0.5, 1.2, 0), dim, brightwhite, brightwhite);
	p_renderer->AddLight(CGrPoint(-1, 0.5, 1.2, 0), dim, brightwhite, brightwhite);
}

void CChildView::OnUpdateRenderRaytrace(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_raytrace);
}


void CChildView::OnRenderRaytrace()
{
	m_raytrace = !m_raytrace;

	Invalidate();
	if (!m_raytrace)
		return;

	if (m_rayimage)
		delete m_rayimage[0];

	delete m_rayimage;

	GetSize(m_rayimagewidth, m_rayimageheight);
	m_rayimage = new BYTE * [m_rayimageheight];

	int rowwid = m_rayimagewidth * 3;
	while (rowwid % 4)
		rowwid++;

	m_rayimage[0] = new BYTE[m_rayimageheight * rowwid];
	
	for (int i = 1; i < m_rayimageheight; i++)
	{
		m_rayimage[i] = m_rayimage[0] + i * rowwid;
	}

	for (int i = 0; i < m_rayimageheight; i++)
	{
		for (int j = 0; j < m_rayimagewidth; j++)
		{
			m_rayimage[i][j * 3] = 0;
			m_rayimage[i][j * 3 + 1] = 0;
			m_rayimage[i][j * 3 + 2] = BYTE(255);
		}
	}

	// Instantiate a raytrace object
	CMyRaytraceRenderer raytrace;

	// Generic configurations for all renderers
	ConfigureRenderer(&raytrace);

	//
	// Render the Scene
	//
	raytrace.SetImage(m_rayimage, m_rayimagewidth, m_rayimageheight);
	raytrace.SetWindow(this);
	raytrace.Render(m_scene);
	Invalidate();

}
