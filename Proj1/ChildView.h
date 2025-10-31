
// ChildView.h : interface of the CChildView class
//


#pragma once

#include "graphics/OpenGLWnd.h"
#include "graphics/GrCamera.h"
#include "graphics/GrObject.h"
#include "graphics/GrTexture.h"
#include "CMyRaytraceRenderer.h"

// CChildView window

class CChildView : public COpenGLWnd
{
// Construction
public:
	CChildView();

// Attributes
public:

private:
	CGrCamera m_camera;
	CGrPtr<CGrObject> m_scene;

	bool m_raytrace;

	BYTE** m_rayimage;
	int m_rayimagewidth;
	int m_rayimageheight;


// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual void OnGLDraw(CDC* pDC);
	void ConfigureRenderer(CGrRenderer* p_renderer);
	afx_msg void OnUpdateRenderRaytrace(CCmdUI* pCmdUI);
	afx_msg void OnRenderRaytrace();
};

