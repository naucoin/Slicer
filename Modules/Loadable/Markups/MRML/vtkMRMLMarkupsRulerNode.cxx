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

// MRML includes
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsRulerNode.h"
#include "vtkMRMLMarkupsRulerStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsRulerNode);


//----------------------------------------------------------------------------
vtkMRMLMarkupsRulerNode::vtkMRMLMarkupsRulerNode()
{

}

//----------------------------------------------------------------------------
vtkMRMLMarkupsRulerNode::~vtkMRMLMarkupsRulerNode()
{

}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->RemoveAllMarkups();

  Superclass::ReadXMLAttributes(atts);
  const char* attName;
  const char* attValue;

  int rulerID = 0;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    // backward compatibility reading of annotation rulers
    if (!strcmp(attName, "ctrlPtsCoord"))
      {
      std::string valStr(attValue);
      std::replace(valStr.begin(), valStr.end(), '|', ' ');

      std::stringstream ss;
      ss << valStr;
      float d;
      int i = 0;
      double twoPoints[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
      while (ss >> d && i < 6)
        {
        twoPoints[i] = d;
        i++;
        }
      rulerID = this->AddRulerFromArray(twoPoints);
      }
    else if (!strcmp(attName, "ctrlPtsSelected"))
      {
      std::stringstream ss;
      int selected;
      ss << attValue;
      ss >> selected;
      this->SetNthRulerSelected(rulerID, (selected == 1 ? true : false));
      }
    else if (!strcmp(attName, "ctrlPtsVisible"))
      {
      std::stringstream ss;
      int visible;
      ss << attValue;
      ss >> visible;
      this->SetNthRulerVisibility(rulerID, (visible == 1 ? true : false));
      }
    else if (!strcmp(attName, "ctrlPtsNumberingScheme"))
      {
      // ignore
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}


//-----------------------------------------------------------
void vtkMRMLMarkupsRulerNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//-------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLMarkupsRulerNode::CreateDefaultStorageNode()
{
  return vtkMRMLStorageNode::SafeDownCast(vtkMRMLMarkupsRulerStorageNode::New());
}

//-------------------------------------------------------------------------
vtkMRMLMarkupsRulerDisplayNode *vtkMRMLMarkupsRulerNode::GetMarkupsRulerDisplayNode()
{
  vtkMRMLDisplayNode *displayNode = this->GetDisplayNode();
  if (displayNode &&
      displayNode->IsA("vtkMRMLMarkupsRulerDisplayNode"))
    {
    return vtkMRMLMarkupsRulerDisplayNode::SafeDownCast(displayNode);
    }
  return NULL;
}

//-------------------------------------------------------------------------
int vtkMRMLMarkupsRulerNode::AddRuler(double x1, double y1, double z1, double x2, double y2, double z2)
{
  int rulerIndex = this->AddMarkupWithNPoints(2);
  this->SetMarkupPoint(rulerIndex, 0, x1, y1, z1);
  this->SetMarkupPoint(rulerIndex, 1, x2, y2, z2);
  return rulerIndex;
}

//-------------------------------------------------------------------------
int vtkMRMLMarkupsRulerNode::AddRulerFromArray(double pos[6])
{
  return this->AddRuler(pos[0], pos[1], pos[2], pos[3], pos[4], pos[5]);
}

//-------------------------------------------------------------------------
int vtkMRMLMarkupsRulerNode::AddRulerFromArrays(double pos1[3], double pos2[3])
{
  return this->AddRuler(pos1[0], pos1[1], pos1[2],
                        pos2[0], pos2[1], pos2[2]);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::GetNthRulerPosition(int n, double pos[6])
{
  vtkVector3d point1 = this->GetMarkupPointVector(n, 0);
  vtkVector3d point2 = this->GetMarkupPointVector(n, 1);
#if (VTK_MAJOR_VERSION <= 5)
  pos[0] = point1.X();
  pos[1] = point1.Y();
  pos[2] = point1.Z();
  pos[3] = point2.X();
  pos[4] = point2.Y();
  pos[5] = point2.Z();
#else
  pos[0] = point1.GetX();
  pos[1] = point1.GetY();
  pos[2] = point1.GetZ();
  pos[3] = point2.GetX();
  pos[4] = point2.GetY();
  pos[5] = point2.GetZ();
#endif
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::GetNthRulerPositionInArrays(int n, double pos1[3],
                                                          double pos2[3])
{
  vtkVector3d point1 = this->GetMarkupPointVector(n, 0);
  vtkVector3d point2 = this->GetMarkupPointVector(n, 1);
#if (VTK_MAJOR_VERSION <= 5)
  pos1[0] = point1.X();
  pos1[1] = point1.Y();
  pos1[2] = point1.Z();
  pos2[0] = point2.X();
  pos2[1] = point2.Y();
  pos2[2] = point2.Z();
#else
  pos1[0] = point1.GetX();
  pos1[1] = point1.GetY();
  pos1[2] = point1.GetZ();
  pos2[0] = point2.GetX();
  pos2[1] = point2.GetY();
  pos2[2] = point2.GetZ();
#endif
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::GetNthRulerPosition1(int n, double pos[3])
{
  vtkVector3d point1 = this->GetMarkupPointVector(n, 0);
#if (VTK_MAJOR_VERSION <= 5)
  pos[0] = point1.X();
  pos[1] = point1.Y();
  pos[2] = point1.Z();
#else
  pos[0] = point1.GetX();
  pos[1] = point1.GetY();
  pos[2] = point1.GetZ();
#endif
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::GetNthRulerPosition2(int n, double pos[3])
{
  vtkVector3d point2 = this->GetMarkupPointVector(n, 1);
#if (VTK_MAJOR_VERSION <= 5)
  pos[0] = point2.X();
  pos[1] = point2.Y();
  pos[2] = point2.Z();
#else
  pos[0] = point2.GetX();
  pos[1] = point2.GetY();
  pos[2] = point2.GetZ();
#endif
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerPosition(int n,
    double x1, double y1, double z1,
    double x2, double y2, double z2)
{
  this->SetMarkupPoint(n, 0, x1, y1, z1);
  this->SetMarkupPoint(n, 1, x2, y2, z2);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerPosition1(int n,
    double x, double y, double z)
{
  this->SetMarkupPoint(n, 0, x, y, z);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerPosition2(int n,
    double x, double y, double z)
{
  this->SetMarkupPoint(n, 1, x, y, z);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerPositionFromArray(int n, double pos[6])
{
  this->SetMarkupPoint(n, 0, pos[0], pos[1], pos[2]);
  this->SetMarkupPoint(n, 1, pos[3], pos[4], pos[5]);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerPositionFromArrays(int n,
    double pos1[3], double pos2[3])
{
  this->SetMarkupPoint(n, 0, pos1[0], pos1[1], pos1[2]);
  this->SetMarkupPoint(n, 1, pos2[0], pos2[1], pos2[2]);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerPosition1FromArray(int n, double pos[3])
{
  this->SetMarkupPoint(n, 0, pos[0], pos[1], pos[2]);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerPosition2FromArray(int n, double pos[3])
{
  this->SetMarkupPoint(n, 1, pos[0], pos[1], pos[2]);
}

//-------------------------------------------------------------------------
bool vtkMRMLMarkupsRulerNode::GetNthRulerSelected(int n)
{
  return this->GetNthMarkupSelected(n);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerSelected(int n, bool flag)
{
  this->SetNthMarkupSelected(n, flag);
}

//-------------------------------------------------------------------------
bool vtkMRMLMarkupsRulerNode::GetNthRulerVisibility(int n)
{
  return this->GetNthMarkupVisibility(n);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerVisibility(int n, bool flag)
{
  this->SetNthMarkupVisibility(n, flag);
}

//-------------------------------------------------------------------------
std::string vtkMRMLMarkupsRulerNode::GetNthRulerLabel(int n)
{
  return this->GetNthMarkupLabel(n);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerLabel(int n, std::string label)
{
  this->SetNthMarkupLabel(n, label);
}

//-------------------------------------------------------------------------
std::string vtkMRMLMarkupsRulerNode::GetNthRulerAssociatedNodeID(int n)
{
  return this->GetNthMarkupAssociatedNodeID(n);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerAssociatedNodeID(int n, const char* id)
{
  this->SetNthMarkupAssociatedNodeID(n, std::string(id));
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerWorldCoordinates(int n, double coords[8])
{
  this->SetMarkupPointWorld(n, 0, coords[0], coords[1], coords[2]);
  this->SetMarkupPointWorld(n, 1, coords[4], coords[5], coords[6]);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerWorldCoordinatesFromArrays(int n,
    double coords1[4], double coords2[4])
{
  this->SetMarkupPointWorld(n, 0, coords1[0], coords1[1], coords1[2]);
  this->SetMarkupPointWorld(n, 1, coords2[0], coords2[1], coords2[2]);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerWorldCoordinates1(int n,
    double coords[4])
{
  this->SetMarkupPointWorld(n, 0, coords[0], coords[1], coords[2]);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::SetNthRulerWorldCoordinates2(int n,
    double coords[4])
{
  this->SetMarkupPointWorld(n, 1, coords[0], coords[1], coords[2]);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::GetNthRulerWorldCoordinates(int n, double coords[8])
{
  double point1[4], point2[4];

  this->GetMarkupPointWorld(n, 0, point1);
  this->GetMarkupPointWorld(n, 1, point2);

  coords[0] = point1[0];
  coords[1] = point1[1];
  coords[2] = point1[2];
  coords[3] = point1[3];

  coords[4] = point2[0];
  coords[5] = point2[1];
  coords[6] = point2[2];
  coords[7] = point2[3];
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsRulerNode::GetNthRulerWorldCoordinatesInArrays(int n,
    double coords1[4], double coords2[4])
{
  this->GetMarkupPointWorld(n, 0, coords1);
  this->GetMarkupPointWorld(n, 1, coords2);
}
