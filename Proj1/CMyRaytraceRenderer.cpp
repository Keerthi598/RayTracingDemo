#include "pch.h"
#include "CMyRaytraceRenderer.h"
#include "graphics/GrTexture.h"

#include <math.h>



CMyRaytraceRenderer::CMyRaytraceRenderer()
{
	m_window = NULL;
}

void CMyRaytraceRenderer::SetImage(BYTE** img, int imgwidth, int imgheight)
{
	m_rayimage = img;
	m_rayimagewidth = imgwidth;
	m_rayimageheight = imgheight;
}


void CMyRaytraceRenderer::SetWindow(CWnd* p_window)
{
	m_window = p_window;
}


bool CMyRaytraceRenderer::RendererStart()
{
	m_intersection.Initialize();
	m_mstack.clear();

	m_material = NULL;

	// We have to do all of the matrix work ourselves.
	// Set up the matrix stack.
	CGrTransform t;
	t.SetLookAt(Eye().X(), Eye().Y(), Eye().Z(),
		Center().X(), Center().Y(), Center().Z(),
		Up().X(), Up().Y(), Up().Z());

	m_mstack.push_back(t);

	double p_angle = ProjectionAngle();
	double p_aspect = ProjectionAspect();
	double n_c = 20.;
	double f_c = 1000.;

	CGrTransform p_mat;
	p_mat.M(0, 0) = 1 / (p_aspect * tan(p_angle / 2 * GR_DTOR));
	p_mat.M(1, 1) = 1 / tan(p_angle / 2 * GR_DTOR);
	p_mat.M(2, 2) = 0 - ((n_c + f_c) / f_c - n_c);
	p_mat.M(2, 3) = 0 - ((2 * n_c * f_c) / (f_c - n_c));
	p_mat.M(3, 2) = 0 - 1;



	for (int i = 0; i < LightCnt(); i++) {
		auto currLight = GetLight(i);
		CGrPoint lightPos = currLight.m_pos;
		auto res = p_mat * t;

		lightPos = res * lightPos;

		m_lightPos.push_back(lightPos);
	}

	return true;
}

void CMyRaytraceRenderer::RendererMaterial(CGrMaterial* p_material)
{
	m_material = p_material;
}

void CMyRaytraceRenderer::RendererPushMatrix()
{
	m_mstack.push_back(m_mstack.back());
}

void CMyRaytraceRenderer::RendererPopMatrix()
{
	m_mstack.pop_back();
}

void CMyRaytraceRenderer::RendererRotate(double a, double x, double y, double z)
{
	CGrTransform r;
	r.SetRotate(a, CGrPoint(x, y, z));
	m_mstack.back() *= r;
}

void CMyRaytraceRenderer::RendererTranslate(double x, double y, double z)
{
	CGrTransform r;
	r.SetTranslate(x, y, z);
	m_mstack.back() *= r;
}

//
// Name : CMyRaytraceRenderer::RendererEndPolygon()
// Description : End definition of a polygon. The superclass has
// already collected the polygon information
//
void CMyRaytraceRenderer::RendererEndPolygon()
{
    const std::list<CGrPoint>& vertices = PolyVertices();
    const std::list<CGrPoint>& normals = PolyNormals();
    const std::list<CGrPoint>& tvertices = PolyTexVertices();

    // Allocate a new polygon in the ray intersection system
    m_intersection.PolygonBegin();
    m_intersection.Material(m_material);

    if (PolyTexture())
    {
        m_intersection.Texture(PolyTexture());
    }

    std::list<CGrPoint>::const_iterator normal = normals.begin();
    std::list<CGrPoint>::const_iterator tvertex = tvertices.begin();

    for (std::list<CGrPoint>::const_iterator i = vertices.begin(); i != vertices.end(); i++)
    {
        if (normal != normals.end())
        {
            m_intersection.Normal(m_mstack.back() * *normal);
            normal++;
        }

        if (tvertex != tvertices.end())
        {
            m_intersection.TexVertex(*tvertex);
            tvertex++;
        }

        m_intersection.Vertex(m_mstack.back() * *i);
    }

    m_intersection.PolygonEnd();
}

inline float dotProd(CGrPoint a, CGrPoint b) {
	float dot = a.X() * b.X();
	dot += a.Y() * b.Y();
	dot += a.Z() * b.Z();

	return dot;
}
bool CMyRaytraceRenderer::RendererEnd()
{
    m_intersection.LoadingComplete();

	auto buo = m_mstack;

    double ymin = -tan(ProjectionAngle() / 2 * GR_DTOR);
    double yhit = -ymin * 2;

    double xmin = ymin * ProjectionAspect();
    double xwid = -xmin * 2;


	for (int r = 0; r < m_rayimageheight; r++)
	{
		for (int c = 0; c < m_rayimagewidth; c++)
		{
			double x = xmin + (c + 0.5) / m_rayimagewidth * xwid;
			double y = ymin + (r + 0.5) / m_rayimageheight * yhit;


			// Construct a Ray
			CRay ray(CGrPoint(0, 0, 0), Normalize3(CGrPoint(x, y, -1, 0)));

			double t;                                   // Will be distance to intersection
			CGrPoint intersect;                         // Will by x,y,z location of intersection
			const CRayIntersection::Object* nearest;    // Pointer to intersecting object
			if (m_intersection.Intersect(ray, 1e20, NULL, nearest, t, intersect))
			{
				// We hit something...
				// Determine information about the intersection
				CGrPoint N;
				CGrMaterial* material;
				CGrTexture* texture;
				CGrPoint texcoord;

				m_intersection.IntersectInfo(ray, nearest, t,
					N, material, texture, texcoord);

				if (material != NULL)
				{
					float tempR, tempG, tempB, tempX = 0, tempY = 0, tempZ = 0;

					if (texture) {
						int width = texture->Width();
						int height = texture->Height();

						float x = width * texcoord.X();
						float y = height * texcoord.Y();

						x -= 1;
						y -= 1;

						float fx = fmod(x, 1);
						float fy = fmod(y, 1);

						BYTE* c_y = texture->Row(int(y));
						BYTE r = c_y[int(x)]++;
						BYTE g = c_y[int(x)]++;
						BYTE b = c_y[int(x)]++;


						BYTE rt, gt, bt;
						
						rt = ((*texture)[int(y) + 1][int(x)*3] * (1 - fx)) + ((*texture)[int(y) + 1][(int(x)+1)*3] * (fx));
						gt = ((*texture)[int(y) + 1][int(x)*3 + 1] * (1 - fx)) + ((*texture)[int(y) + 1][(int(x) + 1)*3 + 1] * (fx));
						bt = ((*texture)[int(y) + 1][int(x)*3 + 2] * (1 - fx)) + ((*texture)[int(y) + 1][(int(x)+1)*3 + 2] * (fx));
						
						BYTE rb = ((*texture)[int(y)][int(x)*3] * (1 - fx)) + ((*texture)[int(y)][(int(x) + 1) * 3] * (fx));
						BYTE gb = ((*texture)[int(y)][int(x)*3 + 1] * (1 - fx)) + ((*texture)[int(y)][(int(x) + 1) * 3 + 1] * (fx));
						BYTE bb = ((*texture)[int(y)][int(x)*3 + 2] * (1 - fx)) + ((*texture)[int(y)][(int(x) + 1) * 3 + 2] * (fx));

						r = BYTE((rt * fy) + (rb * (1 - fy)));
						g = BYTE((gt * fy) + (gb * (1 - fy)));
						b = BYTE((bt * fy) + (bb * (1 - fy)));


						tempR = (material->Diffuse(0) * r);
						tempG = (material->Diffuse(1) * g);
						tempB = (material->Diffuse(2) * b);

						tempX = material->Ambient(0) * 0.2 * r;
						tempY = material->Ambient(1) * 0.2 * g;
						tempZ = material->Ambient(2) * 0.2 * b;
					}

					else {
						tempR = (material->Diffuse(0) * 255);
						tempG = (material->Diffuse(1) * 255);
						tempB = (material->Diffuse(2) * 255);

						tempX = material->Ambient(0) * 0.2 * 255;
						tempY = material->Ambient(1) * 0.2 * 255;
						tempZ = material->Ambient(2) * 0.2 * 255;
					}
					
					for (int k = 0; k < m_lightPos.size(); k++)
					{
						auto currLight = GetLight(k);
						CGrPoint lightPos = m_lightPos[k];

						//
						// Check for shadows
						//
						double t_temp;
						CGrPoint intersect_temp;
						const CRayIntersection::Object* nearest_temp;
						CGrPoint Ntemp;

						CRay sRay(intersect, Normalize3(lightPos - intersect));
						if (m_intersection.Intersect(sRay, t, nearest, nearest_temp, t_temp, intersect_temp)) {
							m_intersection.IntersectInfo(sRay, nearest_temp, t_temp, Ntemp, material, texture, texcoord);

							if (t_temp > 5.2)
								continue;
						}

						CGrPoint lDir = Normalize3(lightPos - intersect);
						CGrPoint vDir = Normalize3(ray.Origin() - intersect);
						CGrPoint hVec = Normalize3(lDir + vDir);
						CGrPoint norm = Normalize3(N);
						

						float diff = (currLight.m_diffuse[3]) * max(0.0, dotProd(lDir, norm));
						
						tempX += currLight.m_diffuse[0] * diff * tempR;
						tempY += currLight.m_diffuse[1] * diff * tempG;
						tempZ += currLight.m_diffuse[2] * diff * tempB;

						float spec = currLight.m_specular[3]* 0.2 * pow(max(0.0, dotProd(norm, hVec)), 3.0);

						tempX += spec * currLight.m_specular[0] * 255;
						tempY += spec * currLight.m_specular[1] * 255;
						tempZ += spec * currLight.m_specular[2] * 255;

					}

					float refR = 0, refG = 0, refB = 0;
					CGrPoint ref = Normalize3(N) - Normalize3(ray.Origin() - intersect);
					float refDot = 2 * dotProd(Normalize3(N), Normalize3(ray.Origin() - intersect));

					ref.X() *= refDot;
					ref.Y() *= refDot;
					ref.Z() *= refDot;

					CRay p_ray(intersect, Normalize3(ref));
					RayColor(p_ray, refR, refG, refB, nearest);

					tempX += 0.2 * refR;
					tempY += 0.2 * refG;
					tempZ += 0.2 * refB;


					tempX = (tempX >= 0) ? tempX : 0;
					tempY = (tempY >= 0) ? tempY : 0;
					tempZ = (tempZ >= 0) ? tempZ : 0;

					tempX = (tempX <= 255) ? tempX : 255;
					tempY = (tempY <= 255) ? tempY : 255;
					tempZ = (tempZ <= 255) ? tempZ : 255;

					m_rayimage[r][c * 3] = GLbyte(tempX);
					m_rayimage[r][c * 3 + 1] = GLbyte(tempY);
					m_rayimage[r][c * 3 + 2] = GLbyte(tempZ);

				}
			}
			else
			{
				// We hit nothing...
				m_rayimage[r][c * 3] = 0;
				m_rayimage[r][c * 3 + 1] = 0;
				m_rayimage[r][c * 3 + 2] = 0;
			}
		}
		if ((r % 50) == 0)
		{
			m_window->Invalidate();
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				DispatchMessage(&msg);
		}
	}
	
    return true;
}

void CMyRaytraceRenderer::RayColor(const CRay& p_ray, float& red, float& green, float& blue, const CRayIntersection::Object* p_ignore)
{
	const CRayIntersection::Object* obj;
	double t;
	CGrPoint intersect;
	if (m_intersection.Intersect(p_ray, 1e5, p_ignore, obj, t, intersect)) {
		CGrPoint N;
		CGrMaterial* material;
		CGrTexture* texture;
		CGrPoint texcoord;

		m_intersection.IntersectInfo(p_ray, obj, t,
			N, material, texture, texcoord);

		if (material != NULL)
		{
			float tempR, tempG, tempB, tempX = 0, tempY = 0, tempZ = 0;

			if (texture) {
				int width = texture->Width();
				int height = texture->Height();

				float x = width * texcoord.X();
				float y = height * texcoord.Y();

				x -= 1;
				y -= 1;

				float fx = fmod(x, 1);
				float fy = fmod(y, 1);

				BYTE* c_y = texture->Row(int(y));
				BYTE r = c_y[int(x)]++;
				BYTE g = c_y[int(x)]++;
				BYTE b = c_y[int(x)]++;


				BYTE rt, gt, bt;

				rt = ((*texture)[int(y) + 1][int(x) * 3] * (1 - fx)) + ((*texture)[int(y) + 1][(int(x) + 1) * 3] * (fx));
				gt = ((*texture)[int(y) + 1][int(x) * 3 + 1] * (1 - fx)) + ((*texture)[int(y) + 1][(int(x) + 1) * 3 + 1] * (fx));
				bt = ((*texture)[int(y) + 1][int(x) * 3 + 2] * (1 - fx)) + ((*texture)[int(y) + 1][(int(x) + 1) * 3 + 2] * (fx));

				BYTE rb = ((*texture)[int(y)][int(x) * 3] * (1 - fx)) + ((*texture)[int(y)][(int(x) + 1) * 3] * (fx));
				BYTE gb = ((*texture)[int(y)][int(x) * 3 + 1] * (1 - fx)) + ((*texture)[int(y)][(int(x) + 1) * 3 + 1] * (fx));
				BYTE bb = ((*texture)[int(y)][int(x) * 3 + 2] * (1 - fx)) + ((*texture)[int(y)][(int(x) + 1) * 3 + 2] * (fx));

				r = BYTE((rt * fy) + (rb * (1 - fy)));
				g = BYTE((gt * fy) + (gb * (1 - fy)));
				b = BYTE((bt * fy) + (bb * (1 - fy)));


				tempR = (material->Diffuse(0) * r);
				tempG = (material->Diffuse(1) * g);
				tempB = (material->Diffuse(2) * b);

				tempX = material->Ambient(0) * 0.2 * r;
				tempY = material->Ambient(1) * 0.2 * g;
				tempZ = material->Ambient(2) * 0.2 * b;
			}

			else {
				tempR = (material->Diffuse(0) * 255);
				tempG = (material->Diffuse(1) * 255);
				tempB = (material->Diffuse(2) * 255);

				tempX = material->Ambient(0) * 0.2 * 255;
				tempY = material->Ambient(1) * 0.2 * 255;
				tempZ = material->Ambient(2) * 0.2 * 255;
			}
			for (int k = 0; k < m_lightPos.size(); k++)
			{
				auto currLight = GetLight(k);
				CGrPoint lightPos = m_lightPos[k];

				//
				// Check for shadows
				//
				double t_temp;
				CGrPoint intersect_temp;
				const CRayIntersection::Object* nearest_temp;
				CGrPoint Ntemp;

				CRay sRay(intersect, Normalize3(lightPos - intersect));
				if (m_intersection.Intersect(sRay, t, obj, nearest_temp, t_temp, intersect_temp)) {
					m_intersection.IntersectInfo(sRay, nearest_temp, t_temp, Ntemp, material, texture, texcoord);

					if (t_temp > 5.2)
						continue;
				}

				CGrPoint lDir = Normalize3(lightPos - intersect);
				CGrPoint vDir = Normalize3(p_ray.Origin() - intersect);
				CGrPoint hVec = Normalize3(lDir + vDir);
				CGrPoint norm = Normalize3(N);


				float diff = (currLight.m_diffuse[3]) * max(0.0, dotProd(lDir, norm));

				tempX += currLight.m_diffuse[0] * diff * tempR;
				tempY += currLight.m_diffuse[1] * diff * tempG;
				tempZ += currLight.m_diffuse[2] * diff * tempB;

				float spec = currLight.m_specular[3] * 0.2 * pow(max(0.0, dotProd(norm, hVec)), 3.0);


				tempX += spec * currLight.m_specular[0] * 255;
				tempY += spec * currLight.m_specular[1] * 255;
				tempZ += spec * currLight.m_specular[2] * 255;

			}

			tempX = (tempX >= 0) ? tempX : 0;
			tempY = (tempY >= 0) ? tempY : 0;
			tempZ = (tempZ >= 0) ? tempZ : 0;

			tempX = (tempX <= 255) ? tempX : 255;
			tempY = (tempY <= 255) ? tempY : 255;
			tempZ = (tempZ <= 255) ? tempZ : 255;

			red = tempX;
			blue = tempY;
			green = tempZ;
		}
	}
}
