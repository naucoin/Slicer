#include <sstream>

// MRMLMarkups includes
#include "vtkMRMLMarkupsDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

const char *vtkMRMLMarkupsDisplayNode::GlyphTypesNames[GlyphMax+2] = 
{
  "GlyphMin",
  "Vertex2D",
  "Dash2D",
  "Cross2D",
  "ThickCross2D",
  "Triangle2D",
  "Square2D",
  "Circle2D",
  "Diamond2D",
  "Arrow2D",
  "ThickArrow2D",
  "HookedArrow2D",
  "StarBurst2D",
  "Sphere3D"
};

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode::vtkMRMLMarkupsDisplayNode()
{
    // model display node settings
  this->SetVisibility(1);
  this->SetVectorVisibility(0);
  this->SetScalarVisibility(0);
  this->SetTensorVisibility(0);
  
  this->Color[0] = 0.7373;
  this->Color[1] = 0.2549;
  this->Color[2] = 0.1098;
  
  this->SelectedColor[0] = 0.2667;
  this->SelectedColor[1] = 0.6745;
  this->SelectedColor[2] = 0.3922;

  this->SetName("");
  this->Opacity = 1.0;
  this->Ambient = 0;
  this->Diffuse = 1.0;
  this->Specular = 0;
  this->Power = 1;

  // markup display node settings
  this->TextScale = 4.5;
  this->GlyphType = vtkMRMLMarkupsDisplayNode::Sphere3D;
  this->GlyphScale = 5.0;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode::~vtkMRMLMarkupsDisplayNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << " textScale=\"" << this->TextScale << "\"";
  of << " glyphScale=\"" << this->GlyphScale << "\"";
  of << " glyphType=\"" << this->GlyphType << "\"";
}



//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  
  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "textScale"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->TextScale;
      }
    else if (!strcmp(attName, "glyphType"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->GlyphType;
      }        
    else if (!strcmp(attName, "glyphScale"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->GlyphScale;
      }
    }

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLMarkupsDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLMarkupsDisplayNode *node = (vtkMRMLMarkupsDisplayNode *)anode;
  
  this->SetTextScale(node->TextScale);
  this->SetGlyphType(node->GlyphType);
  this->SetGlyphScale(node->GlyphScale);

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
const char* vtkMRMLMarkupsDisplayNode::GetGlyphTypeAsString()
{
  return this->GetGlyphTypeAsString(this->GlyphType);
}

//----------------------------------------------------------------------------
const char* vtkMRMLMarkupsDisplayNode::GetGlyphTypeAsString(int glyphType)
{
  if (glyphType < GlyphMin || (glyphType > GlyphMax)) 
    {
      return "UNKNOWN";
    }
    return this->GlyphTypesNames[glyphType];
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SetGlyphTypeFromString(const char *glyphString)
{
  for (int ID = GlyphMin; ID <= GlyphMax; ID++)
    {
      if (!strcmp(glyphString,GlyphTypesNames[ID]))
      {
      this->SetGlyphType(ID);
      return;
      }
    }
  vtkErrorMacro("Invalid glyph type string: " << glyphString);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Text scale: " << this->TextScale << "\n";
  os << indent << "Glyph scale: (";
  os << this->GlyphScale << ")\n";
  os << indent << "Glyph type: ";
  os << this->GetGlyphTypeAsString() << " (" << this->GlyphType << ")\n";
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
int  vtkMRMLMarkupsDisplayNode::GlyphTypeIs3D(int glyphType)
{
  if (glyphType >= vtkMRMLMarkupsDisplayNode::Sphere3D)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}
                              
//---------------------------------------------------------------------------
void  vtkMRMLMarkupsDisplayNode::SetGlyphType(int type)
{
  if (this->GlyphType == type)
    {
    return;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting GlyphType to " << type);
  this->GlyphType = type;

  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayNode::SetGlyphScale(double scale)
{
  if (this->GlyphScale == scale)
    {
    return;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting GlyphScale to " << scale);
  this->GlyphScale = scale;
  this->Modified();
}
