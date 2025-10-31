#pragma once
#include <list>
#include <vector>

#include "graphics/GrRenderer.h"
#include "graphics/RayIntersection.h"
#include "graphics/GrTransform.h"


class CMyRaytraceRenderer :
    public CGrRenderer
{
private:
    int m_rayimagewidth;
    int m_rayimageheight;
    BYTE** m_rayimage;

    CWnd* m_window;
    CRayIntersection m_intersection;
    std::list<CGrTransform> m_mstack;

    CGrMaterial* m_material;

    std::vector<CGrPoint> m_lightPos;

    void RayColor(const CRay& p_ray, float& red, float& green, float& blue, const CRayIntersection::Object* p_ignore);

public:
    CMyRaytraceRenderer();

    void SetImage(BYTE** img, int imgwidth, int imgheight);
    void SetWindow(CWnd* p_window);

    bool RendererStart();
    void RendererMaterial(CGrMaterial* p_material);

    void RendererPushMatrix();
    void RendererPopMatrix();
    void RendererRotate(double a, double x, double y, double z);
    void RendererTranslate(double x, double y, double z);
    
    void RendererEndPolygon();
    bool RendererEnd();
};

