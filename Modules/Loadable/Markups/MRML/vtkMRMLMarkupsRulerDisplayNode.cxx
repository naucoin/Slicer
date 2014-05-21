/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRMLMarkups includes
#include "vtkMRMLMarkupsRulerDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STL includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsRulerDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsRulerDisplayNode::vtkMRMLMarkupsRulerDisplayNode()
{
    // model display node settings
  this->SetVisibility(1);
  this->SetVectorVisibility(0);
  this->SetScalarVisibility(0);
  this->SetTensorVisibility(0);

  this->Color[0] = 0.4;
  this->Color[1] = 1.0;
  this->Color[2] = 1.0;

  this->Color2[0] = 0.4;
  this->Color2[1] = 1.0;
  this->Color2[2] = 1.0;

  this->LineColor[0] = 0.4;
  this->LineColor[1] = 1.0;
  this->LineColor[2] = 1.0;

  this->SelectedColor[0] = 1.0;
  this->SelectedColor[1] = 0.5;
  this->SelectedColor[2] = 0.5;

  this->SelectedColor2[0] = 1.0;
  this->SelectedColor2[1] = 0.5;
  this->SelectedColor2[2] = 0.5;

  this->SelectedLineColor[0] = 1.0;
  this->SelectedLineColor[1] = 0.5;
  this->SelectedLineColor[2] = 0.5;

  this->SetName("");
  this->Opacity = 1.0;
  this->Ambient = 0;
  this->Diffuse = 1.0;
  this->Specular = 0;
  this->Power = 1;

  // markup display node settings
  this->TextScale = 3.4;
  this->GlyphType = vtkMRMLMarkupsRulerDisplayNode::Sphere3D;
  this->GlyphScale = 2.1;

  // projection settings
  this->SliceProjection = (vtkMRMLMarkupsRulerDisplayNode::ProjectionOff);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsRulerDisplayNode::~vtkMRMLMarkupsRulerDisplayNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

 if (this->Color2)
    {
    of << " color2=\"" << this->Color2[0] << " "
       << this->Color2[1] << " "
       << this->Color2[2] << "\"";
    }
 if (this->SelectedColor2)
    {
    of << " selectedColor2=\"" << this->SelectedColor2[0] << " "
       << this->SelectedColor2[1] << " "
       << this->SelectedColor2[2] << "\"";
    }

 if (this->LineColor)
    {
    of << " lineColor=\"" << this->LineColor[0] << " "
       << this->LineColor[1] << " "
       << this->LineColor[2] << "\"";
    }
 if (this->SelectedLineColor)
    {
    of << " selectedLineColor=\"" << this->SelectedLineColor[0] << " "
       << this->SelectedLineColor[1] << " "
       << this->SelectedLineColor[2] << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "color2"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Color2[0];
      ss >> this->Color2[1];
      ss >> this->Color2[2];
      }
    else if (!strcmp(attName, "selectedColor2"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SelectedColor2[0];
      ss >> this->SelectedColor2[1];
      ss >> this->SelectedColor2[2];
      }
    else if (!strcmp(attName, "lineColor"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LineColor[0];
      ss >> this->LineColor[1];
      ss >> this->LineColor[2];
      }
    else if (!strcmp(attName, "selectedLineColor"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SelectedLineColor[0];
      ss >> this->SelectedLineColor[1];
      ss >> this->SelectedLineColor[2];
      }
    }

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLMarkupsRulerDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLMarkupsRulerDisplayNode *node = (vtkMRMLMarkupsRulerDisplayNode *)anode;

  this->SetColor2(node->GetColor2());
  this->SetSelectedColor2(node->GetSelectedColor2());

  this->SetLineColor(node->GetLineColor());
  this->SetSelectedLineColor(node->GetSelectedLineColor());

  this->EndModify(disabledModify);
}



//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Color2: (";
  os << this->Color2[0] << ","
     << this->Color2[1] << ","
     << this->Color2[2] << ")" << "\n";

  os << indent << "SelectedColor2: (";
  os << this->SelectedColor2[0] << ","
     << this->SelectedColor2[1] << ","
     << this->SelectedColor2[2] << ")" << "\n";

  os << indent << "LineColor: (";
  os << this->LineColor[0] << ","
     << this->LineColor[1] << ","
     << this->LineColor[2] << ")" << "\n";

  os << indent << "SelectedLineColor: (";
  os << this->SelectedLineColor[0] << ","
     << this->SelectedLineColor[1] << ","
     << this->SelectedLineColor[2] << ")" << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
}
