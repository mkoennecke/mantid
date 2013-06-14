#include "GLViewport.h"
#include <math.h>
#include <iostream>
#include "MantidGeometry/Rendering/OpenGL_Headers.h"
#include "MantidKernel/V3D.h"
#include "OpenGLError.h"

GLViewport::GLViewport(int w, int h):mWidth(w),mHeight(h)
{
  mProjection=GLViewport::ORTHO;
  mZoomFactor=1.0;
  mXTrans=0.0;
  mYTrans=0.0;
  mLeft = -1;
  mRight = 1;
  mBottom = -1;
  mTop = 1;
  mNear = -1;
  mFar = 1;
}

GLViewport::~GLViewport()
{
}
void GLViewport::resize(int w,int h)
{
    mWidth=w;
    mHeight=h;
}
void GLViewport::getViewport(int* w, int* h) const
{
    *w=mWidth;
    *h=mHeight;
}
/**
 * This will set the projection to Ortho
 *
 * @param l :: left side of the Ortho projection (xmin)
 * @param r :: right side of the Ortho projection (xmax)
 * @param b :: bottom side of the Ortho projection (ymin)
 * @param t :: top side of the Ortho projection (ymax)
 * @param nearz :: near side of the Ortho Projection (zmin)
 * @param farz :: far side of the Ortho Projection (zmax)
 * @param useZ :: If set to false near and far will not be used. It is a quick fix of a problem with viewing
 *          selected instrument pixels
 */
void GLViewport::setOrtho(double l,double r,double b,double t,double nearz,double farz,bool useZ)
{
	mLeft=l;
	mRight=r;
	if (mLeft>mRight) std::swap(mLeft,mRight); // Make sure that it is only from back to front otherwise the
	mBottom=b;
	mTop=t;
	if (mBottom>mTop) std::swap(mBottom,mTop);

        if (useZ)
        {
          mNear=nearz;
          mFar=farz;
          if (mNear>mFar) std::swap(mNear,mFar);
        }

	mProjection=GLViewport::ORTHO;
  mZoomFactor=1.0;
  mXTrans=0.0;
  mYTrans=0.0;
}

/**
 * This will set the projection to perspective.
 * UNUSED! as of 2010-11-01.
 *
 * @param l :: left side of the perspective projection (xmin)
 * @param r :: right side of the perspective projection (xmax)
 * @param b :: bottom side of the perspective projection (ymin)
 * @param t :: top side of the perspective projection (ymax)
 * @param nearz :: near side of the perspective Projection (zmin)
 * @param farz :: far side of the perspective Projection (zmax)
 */
void GLViewport::setPrespective(double l,double r,double b,double t,double nearz,double farz)
{
	mLeft=l;
	mRight=r;
	mBottom=b;
	mTop=t;
	mNear=nearz;
	mFar=farz;
	mProjection=GLViewport::PERSPECTIVE;
}

GLViewport::ProjectionType GLViewport::getProjectionType()const
{
	return mProjection;
}

void GLViewport::getProjection(double& xmin,double& xmax,double& ymin,double& ymax,double& zmin,double& zmax)
{
	xmin=mLeft;
	xmax=mRight;
	ymin=mBottom;
	ymax=mTop;
	zmin=mNear;
	zmax=mFar;
}

void GLViewport::getInstantProjection(double& xmin,double& xmax,double& ymin,double& ymax,double& zmin,double& zmax)const
{
  Mantid::Kernel::V3D center((mRight+mLeft)/2.0,(mTop+mBottom)/2.0,(mNear+mFar)/2.0);

  Mantid::Kernel::V3D distance(mRight-mLeft,mTop-mBottom,mNear-mFar);
  //Window Aspect ratio
  GLdouble windowAspect= mHeight > 0 ? (GLdouble)mWidth/(GLdouble)mHeight : 1.0;
  //Adjust width and height to show the dimensions correct
  //Adjust window aspect ratio
  if(windowAspect<1.0) //window height is higher than width (x<y)
  {
    if(distance[0]<distance[1]&&distance[0]/windowAspect<distance[1])///TESTING
    {
      distance[0]=distance[1];
      distance[0]*=windowAspect;
    }
    else
    {
      distance[1]=distance[0];
      distance[1]/=windowAspect;
    }
  }
  else
  {
    if(distance[0]<distance[1])
    {
      distance[0]=distance[1];
      distance[0]*=windowAspect;
    }
    else if(distance[0]/windowAspect<distance[1])
    {
      distance[0]=distance[1];
      distance[0]*=windowAspect;
    }
    else
    {
      distance[1]=distance[0];
      distance[1]/=windowAspect;
    }
  }
  //use zoom now
  distance*=mZoomFactor/2.0;
  
  double tmpNear = center[2]+distance[2];
  double nearVal = tmpNear < mNear ? tmpNear : mNear;//center[2]+distance[2];
  double farVal = mFar;

	xmin=center[0]-distance[0]-mXTrans;
	xmax=center[0]+distance[0]-mXTrans;
	ymin=center[1]-distance[1]-mYTrans;
	ymax=center[1]+distance[1]-mYTrans;
	zmin=nearVal;
	zmax=farVal;

}

void GLViewport::setZoomFactor(double val)
{
	mZoomFactor=val;
	issueGL();
}

double GLViewport::getZoomFactor()
{
	return mZoomFactor;
}

void GLViewport::setTranslation(double xval,double yval)
{
	mXTrans=xval;
	mYTrans=yval;
	issueGL();
}
void GLViewport::getTranslation(double& xval,double& yval)
{
	xval=mXTrans;
	yval=mYTrans;
}

/** Issue the OpenGL commands that define the viewport and projection. */
void GLViewport::issueGL() const
{
  double xmin,xmax,ymin,ymax,zmin,zmax;
  getInstantProjection(xmin,xmax,ymin,ymax,zmin,zmax);
  if (xmin == xmax)
  {
    xmin -= 0.001;
    xmax += 0.001;
  }
  if (ymin == ymax)
  {
    ymin -= 0.001;
    ymax += 0.001;
  }

  if (zmin == zmax)
  {
    zmin -= 0.001;
    zmax += 0.001;
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, mWidth, mHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  OpenGLError::check("GLViewport::issueGL()");

  if(mProjection==GLViewport::PERSPECTIVE)
  {
    glFrustum(xmin,xmax,ymin,ymax,zmin,zmax);
  }
  else
  {
    glOrtho(xmin,xmax,ymin,ymax,zmin,zmax);

    if (OpenGLError::hasError("GLViewport::issueGL()"))
    {
      OpenGLError::log() << "Arguments to glOrtho:\n";
      OpenGLError::log() 
                         << xmin << ' ' << xmax << '\n'
                         << ymin << ' ' << ymax << '\n'
                         << zmin << ' ' << zmax << "\n\n";
    }
  }
  glMatrixMode(GL_MODELVIEW);
}