#include "ObjCompAssemblyActor.h"
#include "ObjComponentActor.h"
#include "InstrumentActor.h"
#include "OpenGLError.h"

#include "MantidKernel/V3D.h"
#include "MantidGeometry/Objects/Object.h"
#include "MantidGeometry/ICompAssembly.h"
#include "MantidGeometry/IObjComponent.h"
#include "MantidGeometry/IDetector.h"
#include "MantidGeometry/Instrument/ObjCompAssembly.h"
#include "MantidKernel/Exception.h"
#include <cfloat>
using namespace Mantid;
using namespace Geometry;

ObjCompAssemblyActor::ObjCompAssemblyActor(const InstrumentActor& instrActor,Mantid::Geometry::ComponentID compID):
  ICompAssemblyActor(instrActor,compID),
  m_idData(0),
  m_idPick(0),
  m_n(getObjCompAssembly()->nelements()),
  m_pick_data()
{

  ObjCompAssembly_const_sptr objAss = getObjCompAssembly();
  mNumberOfDetectors = objAss->nelements();
  assert(m_n == mNumberOfDetectors);
  m_data = new unsigned char[m_n*3];
  m_pick_data = new unsigned char[m_n*3];
  for(int i=0;i<getNumberOfDetectors();++i)
  {
    IDetector_const_sptr det = boost::dynamic_pointer_cast<const IDetector>(objAss->getChild(i));
    assert(det);
    detid_t id = det->getID();
    m_detIDs.push_back(id);
    size_t pickID = instrActor.push_back_detid(id);
    setDetectorColor(m_pick_data,i,GLActor::makePickColor(pickID));
  }
  Mantid::Geometry::BoundingBox boundBox;
  objAss->getBoundingBox(boundBox);
  minBoundBox[0]=boundBox.xMin(); minBoundBox[1]=boundBox.yMin(); minBoundBox[2]=boundBox.zMin();
  maxBoundBox[0]=boundBox.xMax(); maxBoundBox[1]=boundBox.yMax(); maxBoundBox[2]=boundBox.zMax();

  setColors();
  generateTexture(m_pick_data,m_idPick);
}

/**
* Destructor which removes the actors created by this object
*/
ObjCompAssemblyActor::~ObjCompAssemblyActor()
{
  if (m_data)
  {
    delete[] m_data;
    delete[] m_pick_data;
  }
}

/**
* This function is concrete implementation that renders the Child ObjComponents and Child CompAssembly's
*/
void ObjCompAssemblyActor::draw(bool picking)const
{
  OpenGLError::check("ObjCompAssemblyActor::draw(0)");
  ObjCompAssembly_const_sptr objAss = getObjCompAssembly();
  glPushMatrix();

  unsigned int texID = picking? m_idPick : m_idData;
  // Because texture colours are combined with the geometry colour
  // make sure the current colour is white
  glColor3f(1.0f,1.0f,1.0f);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texID);
  objAss->draw();
  glBindTexture(GL_TEXTURE_2D, 0);
  OpenGLError::check("ObjCompAssemblyActor::draw()");

  glPopMatrix();
}

void ObjCompAssemblyActor::generateTexture(unsigned char* data, unsigned int& id)
{
  if (id > 0)
  {
    glDeleteTextures(1,&id);
    OpenGLError::check("TexObject::generateTexture()[delete texture] ");
  }
  bool vertical = true; // depends on the tex coordinates of the shape object

  int width = m_n;
  int height = 1;
  if (vertical)
  {
    width = 1;
    height = m_n;
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenTextures(1, &id);					// Create The Texture
  OpenGLError::check("TexObject::generateTexture()[generate] ");
  glBindTexture(GL_TEXTURE_2D, id);
  OpenGLError::check("TexObject::generateTexture()[bind] ");

  GLint texParam = GL_NEAREST;
  glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  OpenGLError::check("TexObject::generateTexture()[set data] ");
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,texParam);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,texParam);
  OpenGLError::check("TexObject::generateTexture()[parameters] ");
}

/**
  * Set colour to a detector.
  * @param data :: pointer to color array
  * @param i :: Index of the detector in ObjCompAssembly
  * @param c :: The colour
  */
void ObjCompAssemblyActor::setDetectorColor(unsigned char* data,size_t i,GLColor c)
{
    size_t pos = 3*i;
    float r,g,b,a;
    c.get(r,g,b,a);
    data[pos]   = (unsigned char)(r*255);
    data[pos+1] = (unsigned char)(g*255);
    data[pos+2] = (unsigned char)(b*255);
}

void ObjCompAssemblyActor::swap()
{
  if (!m_pick_data)
  {
    m_pick_data = new  unsigned char[m_n*3];
  }
  unsigned char* tmp = m_data;
  m_data = m_pick_data;
  m_pick_data = tmp;
}

const unsigned char* ObjCompAssemblyActor::getColor(int i)const
{
  return &m_data[3*i];
}

void ObjCompAssemblyActor::setColors()
{
  for(size_t i = 0; i < size_t(m_n); ++i)
  {
    GLColor c = m_instrActor.getColor(m_detIDs[i]);
    setDetectorColor(m_data,i,c);
  }
  generateTexture(m_data,m_idData);
}

bool ObjCompAssemblyActor::accept(GLActorVisitor &visitor, GLActor::VisitorAcceptRule )
{
    return visitor.visit(this);
}

bool ObjCompAssemblyActor::accept(GLActorConstVisitor &visitor, GLActor::VisitorAcceptRule) const
{
    return visitor.visit(this);
}

