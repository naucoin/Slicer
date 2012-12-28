// MRML includes
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes


//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsFiducialNode);


//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode::vtkMRMLMarkupsFiducialNode()
{

}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode::~vtkMRMLMarkupsFiducialNode()
{

}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
 
  vtkIndent indent(nIndent);
 
  //of << indent << " locked=\"" << this->Locked << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->RemoveAllMarkups();

  Superclass::ReadXMLAttributes(atts);
  const char* attName;
  const char* attValue;

  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    /*
  if (!strncmp(attName, "textList", 9))
      {
      this->AddText(attValue);
      }  
    else if (!strcmp(attName, "locked"))
      {
      this->SetLocked(atof(attValue));
      }
    */
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
/*
  vtkMRMLMarkupsFiducialNode *node = (vtkMRMLMarkupsFiducialNode *) anode;
  if (!node)
    {
    return;
    }
*/
}


//-----------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);


}

//-------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLMarkupsFiducialNode::CreateDefaultStorageNode()
{
  return vtkMRMLStorageNode::SafeDownCast(vtkMRMLMarkupsFiducialStorageNode::New());
}

//-------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode *vtkMRMLMarkupsFiducialNode::GetMarkupsDisplayNode()
{
  vtkMRMLDisplayNode *displayNode = this->GetDisplayNode();
  if (displayNode &&
      displayNode->IsA("vtkMRMLMarkupsDisplayNode"))
    {
    return vtkMRMLMarkupsDisplayNode::SafeDownCast(displayNode);
    }
  return NULL;
}

//-------------------------------------------------------------------------
int vtkMRMLMarkupsFiducialNode::AddFiducial(double x, double y, double z)
{
  vtkVector3d point;
  point.SetX(x);
  point.SetY(y);
  point.SetZ(z);
  return this->AddPointToNewMarkup(point);
}

//-------------------------------------------------------------------------
int vtkMRMLMarkupsFiducialNode::AddFiducialFromArray(double pos[3])
{
  return this->AddFiducial(pos[0], pos[1], pos[2]);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::GetNthFiducialPosition(int n, double pos[3])
{
  vtkVector3d point= this->GetMarkupPointVector(n, 0);
  pos[0] = point.X();
  pos[1] = point.Y();
  pos[2] = point.Z();
}

//-------------------------------------------------------------------------
bool vtkMRMLMarkupsFiducialNode::GetNthFiducialSelected(int n)
{
  return this->GetNthMarkupSelected(n);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::SetNthFiducialSelected(int n, bool flag)
{
  this->SetNthMarkupSelected(n, flag);
}

//-------------------------------------------------------------------------
bool vtkMRMLMarkupsFiducialNode::GetNthFiducialVisibility(int n)
{
  return this->GetNthMarkupVisibility(n);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::SetNthFiducialVisibility(int n, bool flag)
{
  this->SetNthMarkupVisibility(n, flag);
}

//-------------------------------------------------------------------------
std::string vtkMRMLMarkupsFiducialNode::GetNthFiducialLabel(int n)
{
  return this->GetNthMarkupLabel(n);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::SetNthFiducialLabel(int n, std::string label)
{
  this->SetNthMarkupLabel(n, label);
}

//-------------------------------------------------------------------------
std::string vtkMRMLMarkupsFiducialNode::GetNthFiducialAssociatedNodeID(int n)
{
  return this->GetNthMarkupAssociatedNodeID(n);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::SetNthFiducialAssociatedNodeID(int n, const char* id)
{
  this->SetNthMarkupAssociatedNodeID(n, std::string(id));
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::SetNthFiducialWorldCoordinates(int n, double coords[4])
{
  this->SetMarkupPointWorld(n, 0, coords[0], coords[1], coords[2]);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::GetNthFiducialWorldCoordinates(int n, double coords[4])
{
  this->GetMarkupPointWorld(n, 0, coords);
}
