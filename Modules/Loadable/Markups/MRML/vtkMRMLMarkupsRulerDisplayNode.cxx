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

  // markup ruler display node settings
  this->PointColor1[0] = 0.9;
  this->PointColor1[1] = 0.5;
  this->PointColor1[2] = 0.5;

  this->PointColor2[0] = 0.9;
  this->PointColor2[1] = 0.7;
  this->PointColor2[2] = 0.9;

  this->LineColor[0] = 1.0;
  this->LineColor[1] = 1.0;
  this->LineColor[2] = 1.0;

  this->SelectedColor[0] = 1.0;
  this->SelectedColor[1] = 0.5;
  this->SelectedColor[2] = 0.5;

  this->SelectedPointColor1[0] = 1.0;
  this->SelectedPointColor1[1] = 0.5;
  this->SelectedPointColor1[2] = 0.5;

  this->SelectedPointColor2[0] = 1.0;
  this->SelectedPointColor2[1] = 0.5;
  this->SelectedPointColor2[2] = 0.5;

  this->SelectedLineColor[0] = 1.0;
  this->SelectedLineColor[1] = 0.5;
  this->SelectedLineColor[2] = 0.5;

  this->LineThickness = 1.0;
  this->LabelPosition = 0.2;
  this->TickSpacing = 10.0;
  this->MaxTicks = 99;

  // projection settings
  this->SliceProjection = (vtkMRMLMarkupsDisplayNode::ProjectionOff |
                           vtkMRMLMarkupsRulerDisplayNode::ProjectionDashed |
                           vtkMRMLMarkupsRulerDisplayNode::ProjectionColoredWhenParallel |
                           vtkMRMLMarkupsRulerDisplayNode::ProjectionThickerOnTop |
                           vtkMRMLMarkupsRulerDisplayNode::ProjectionUseRulerColor);

  this->UnderLineThickness = 1.0;
  this->OverLineThickness = 3.0;
  /// bug 2375: don't show the slice intersection until it's correct
  this->SliceIntersectionVisibility = 0;

  // distance annotation format
  this->DistanceMeasurementFormat = NULL;
  this->SetDistanceMeasurementFormat("%-#6.3g mm");
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsRulerDisplayNode::~vtkMRMLMarkupsRulerDisplayNode()
{
  if (this->DistanceMeasurementFormat)
    {
    delete [] this->DistanceMeasurementFormat;
    this->DistanceMeasurementFormat = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  if (this->PointColor1)
    {
    of << " pointColor1=\"" << this->PointColor1[0] << " "
       << this->PointColor1[1] << " "
       << this->PointColor1[2] << "\"";
    }
  if (this->SelectedPointColor1)
    {
    of << " selectedPointColor1=\"" << this->SelectedPointColor1[0] << " "
       << this->SelectedPointColor1[1] << " "
       << this->SelectedPointColor1[2] << "\"";
    }
  if (this->PointColor2)
    {
    of << " pointcolor2=\"" << this->PointColor2[0] << " "
       << this->PointColor2[1] << " "
       << this->PointColor2[2] << "\"";
    }
  if (this->SelectedPointColor2)
    {
    of << " selectedPointColor2=\"" << this->SelectedPointColor2[0] << " "
       << this->SelectedPointColor2[1] << " "
       << this->SelectedPointColor2[2] << "\"";
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

  of << " lineThickness=\"" << this->LineThickness << "\"";
  of << " labelPosition=\"" << this->LabelPosition << "\"";
  of << " tickSpacing=\"" << this->TickSpacing << "\"";
  of << " maxTicks=\"" << this->MaxTicks << "\"";

  of << " underLineThickness=\"" << this->UnderLineThickness << "\"";
  of << " overLineThickness=\"" << this->OverLineThickness << "\"";

  if (this->DistanceMeasurementFormat != NULL)
    {
    of << " distanceMeasurementFormat=\""
       << this->DistanceMeasurementFormat << "\"";
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

    if (!strcmp(attName, "pointColor1"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->PointColor1[0];
      ss >> this->PointColor1[1];
      ss >> this->PointColor1[2];
      }
    else if (!strcmp(attName, "selectedPointColor1"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SelectedPointColor1[0];
      ss >> this->SelectedPointColor1[1];
      ss >> this->SelectedPointColor1[2];
      }
    else if (!strcmp(attName, "pointColor2"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->PointColor2[0];
      ss >> this->PointColor2[1];
      ss >> this->PointColor2[2];
      }
    else if (!strcmp(attName, "selectedPointColor2"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SelectedPointColor2[0];
      ss >> this->SelectedPointColor2[1];
      ss >> this->SelectedPointColor2[2];
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
    else if (!strcmp(attName, "lineThickness"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LineThickness;
      }
    else if (!strcmp(attName, "labelPosition"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LabelPosition;
      }
    else if (!strcmp(attName, "tickSpacing"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->TickSpacing;
      }
    else if (!strcmp(attName, "maxTicks"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MaxTicks;
      }
    else if (!strcmp(attName, "underLineThickness"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->UnderLineThickness;
      }
    else if (!strcmp(attName, "overLineThickness"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->OverLineThickness;
      }
    else if (!strcmp(attName, "distanceMeasurementFormat"))
      {
      this->SetDistanceMeasurementFormat(attValue);
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

  this->SetPointColor1(node->GetPointColor1());
  this->SetSelectedPointColor1(node->GetSelectedPointColor1());

  this->SetPointColor2(node->GetPointColor2());
  this->SetSelectedPointColor2(node->GetSelectedPointColor2());

  this->SetLineColor(node->GetLineColor());
  this->SetSelectedLineColor(node->GetSelectedLineColor());

  this->SetLineThickness(node->LineThickness);
  this->SetLabelPosition(node->LabelPosition);
  this->SetTickSpacing(node->TickSpacing);
  this->SetMaxTicks(node->MaxTicks);

  this->SetUnderLineThickness(node->GetUnderLineThickness());
  this->SetOverLineThickness(node->GetOverLineThickness());

  this->SetDistanceMeasurementFormat(node->GetDistanceMeasurementFormat());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "PointColor1: (";
  os << this->PointColor1[0] << ","
     << this->PointColor1[1] << ","
     << this->PointColor1[2] << ")" << "\n";

  os << indent << "SelectedPointColor1: (";
  os << this->SelectedPointColor1[0] << ","
     << this->SelectedPointColor1[1] << ","
     << this->SelectedPointColor1[2] << ")" << "\n";

  os << indent << "PointColor2: (";
  os << this->PointColor2[0] << ","
     << this->PointColor2[1] << ","
     << this->PointColor2[2] << ")" << "\n";

  os << indent << "SelectedPointColor2: (";
  os << this->SelectedPointColor2[0] << ","
     << this->SelectedPointColor2[1] << ","
     << this->SelectedPointColor2[2] << ")" << "\n";

  os << indent << "LineColor: (";
  os << this->LineColor[0] << ","
     << this->LineColor[1] << ","
     << this->LineColor[2] << ")" << "\n";

  os << indent << "SelectedLineColor: (";
  os << this->SelectedLineColor[0] << ","
     << this->SelectedLineColor[1] << ","
     << this->SelectedLineColor[2] << ")" << "\n";

  os << indent << "Line Thickness   : " << this->LineThickness << "\n";
  os << indent << "Label Position   : " << this->LabelPosition << "\n";
  os << indent << "Tick Spacing     : " << this->TickSpacing << "\n";
  os << indent << "Max Ticks        : " << this->MaxTicks << "\n";

  os << indent << "Under Line Thickness: " << this->UnderLineThickness << "\n";
  os << indent << "Over Line Thickness: " << this->OverLineThickness << "\n";

  os << indent << "DistanceMeasurementFormat: "
     << (this->DistanceMeasurementFormat ?
         this->GetDistanceMeasurementFormat() : "(none)") << "\n";
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
