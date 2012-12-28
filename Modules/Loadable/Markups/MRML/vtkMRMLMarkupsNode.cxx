// MRML includes
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsStorageNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include <vtkAbstractTransform.h>
#include <vtkCommand.h>
#include <vtkBitArray.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkStringArray.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsNode);


//----------------------------------------------------------------------------
vtkMRMLMarkupsNode::vtkMRMLMarkupsNode()
{
  this->TextList = vtkStringArray::New();
  this->Locked = 0;
  this->UseListNameForMarkups = 1;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsNode::~vtkMRMLMarkupsNode()
{
  this->TextList->Delete();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkIndent indent(nIndent);

  of << indent << " locked=\"" << this->Locked << "\"";
  of << indent << " useListNameForMarkups=\"" << this->UseListNameForMarkups << "\"";

  int textLength = this->TextList->GetNumberOfValues();

  for (int i = 0 ; i < textLength; i++)
    {
    of << " textList" << i << "=\"" << this->TextList->GetValue(i) << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::ReadXMLAttributes(const char** atts)
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

    if (!strncmp(attName, "textList", 9))
      {
      this->AddText(attValue);
      }
    else if (!strcmp(attName, "locked"))
      {
      this->SetLocked(atof(attValue));
      }
    else if (!strcmp(attName, "useListNameForMarkups"))
      {
      this->SetUseListNameForMarkups(atof(attValue));
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);

  vtkMRMLMarkupsNode *node = (vtkMRMLMarkupsNode *) anode;
  if (!node)
    {
    return;
    }

  this->SetLocked(node->GetLocked());
  this->SetUseListNameForMarkups(node->GetUseListNameForMarkups());
  this->TextList->DeepCopy(node->TextList);

  this->Markups.clear();
  int numMarkups = node->GetNumberOfMarkups();
  for (int n = 0; n < numMarkups; n++)
    {
    Markup *markup = node->GetNthMarkup(n);
    this->AddMarkup(*markup);
    }
}


//-----------------------------------------------------------
void vtkMRMLMarkupsNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Locked: " << this->Locked << "\n";
  os << indent << "UseListNameForMarkups: " << this->UseListNameForMarkups << "\n";

  for (int i = 0; i < this->GetNumberOfMarkups(); i++)
    {
    os << indent << "Markup " << i << ":\n";
    int numPoints = this->GetNumberOfPointsInNthMarkup(i);
    Markup *markup = this->GetNthMarkup(i);
    if (markup)
      {
      os << indent.GetNextIndent() << "Label = " << markup->Label.c_str() << "\n";
      os << indent.GetNextIndent() << "Associated node id = " << markup->AssociatedNodeID.c_str() << "\n";
      os << indent.GetNextIndent() << "Selected = " << markup->Selected << "\n";
      os << indent.GetNextIndent() << "Visibility = " << markup->Visibility << "\n";
      for (int p = 0; p < numPoints; p++)
        {
        vtkVector3d point = markup->points[p];
        os << indent.GetNextIndent() << "p" << p << ": " << point.X() << ", " << point.Y() << ", " << point.Z() << "\n";
        }
      }
    }
  
  os << indent << "textList: "; 
  if  (!this->TextList || !this->GetNumberOfTexts()) 
    {
    os << indent << "None"  << endl;
    }
  else 
    {
    os << endl;
    for (int i = 0 ; i < this->GetNumberOfTexts() ; i++) 
      {
      os << indent << "  " << i <<": " <<  (TextList->GetValue(i) ? TextList->GetValue(i) : "(none)") << endl;
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsNode::RemoveAllMarkups()
{
  // remove all markups and points
  this->Markups.clear();
  
  // remove all text 
  this->TextList->Initialize(); 

  this->Locked = 0;
  this->UseListNameForMarkups = 1;
}



//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetText(int id, const char *newText)
{
  if (id < 0) 
    {
    vtkErrorMacro("SetText: Invalid ID");
    return;
    }
  if (!this->TextList) 
    {
    vtkErrorMacro("SetText: TextList is NULL");
    return;
    }

  vtkStdString newString;
  if (newText)
    {
    newString = vtkStdString(newText);
    }

  // check if the same as before
  if (((this->TextList->GetNumberOfValues() == 0) && (newText == NULL || newString == "")) ||
      ((this->TextList->GetNumberOfValues() > id) && 
       (this->TextList->GetValue(id) == newString)
        ) )
    {
    return;
    }

  this->TextList->InsertValue(id,newString);

  if(!this->GetDisableModifiedEvent())
    {
    // invoke a modified event
    this->InvokeEvent(vtkCommand::ModifiedEvent);
    }
}




//-------------------------------------------------------------------------
int vtkMRMLMarkupsNode::AddText(const char *newText)
{
  if (!this->TextList)
    {
    vtkErrorMacro("Markups: For node " << this->GetName() << " text is not defined");
    return -1 ;
    }
  int n = this->GetNumberOfTexts();
  this->SetText(n,newText);

  return n;
}

//-------------------------------------------------------------------------
vtkStdString vtkMRMLMarkupsNode::GetText(int n)
{
  if ((this->GetNumberOfTexts() <= n) || n < 0 )
    {
      return vtkStdString();
    }
  return this->TextList->GetValue(n);
}

//-------------------------------------------------------------------------
int  vtkMRMLMarkupsNode::DeleteText(int id)
{
  if (!this->TextList)
    {
    return -1;
    }

  int n = this->GetNumberOfTexts();
  if (id < 0 || id >= n)
    {
      return -1;
    }

  for (int i = id; id < n-1; i++ )
    {
      this->TextList->SetValue(i,this->GetText(i+1));
    }

  this->TextList->Resize(n-1);

  return 1;
}


//-------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetNumberOfTexts()
{
  if (!this->TextList)
    {
    return -1;
    }
  return this->TextList->GetNumberOfValues();
}


//-------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLMarkupsNode::CreateDefaultStorageNode()
{
  return vtkMRMLStorageNode::SafeDownCast(vtkMRMLMarkupsStorageNode::New());
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetLocked(int locked)
{
  if (this->Locked == locked)
    {
    return;
    }
  this->Locked = locked;
  if(!this->GetDisableModifiedEvent())
    {
    // invoke a lock modified event
    this->InvokeEvent(vtkMRMLMarkupsNode::LockModifiedEvent);
    this->Modified();
    }
//  this->ModifiedSinceReadOn();
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::MarkupExists(int n)
{
  if (n < 0)
    {
    vtkErrorMacro("MarkupExists: n of " << n << " must be greater than or equal to zero.");
    return false;
    }
  if (n >= this->GetNumberOfMarkups())
    {
    vtkErrorMacro("MarkupExists: n of " << n << " must be less than the current number of markups, " << this->GetNumberOfMarkups());
    return false;
    }
  else
    {
    return true;
    }
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode::GetNumberOfMarkups()
{
  return this->Markups.size();
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::PointExistsInMarkup(int p, int n)
{
  if (!this->MarkupExists(n))
    {
    return false;
    }
  if (p < 0)
    {
    vtkErrorMacro("PointExistsInMarkup: point index of " << p << " is less than 0");
    return false;
    }
  int numPoints = this->GetNumberOfPointsInNthMarkup(n);

  if (p >=  numPoints )
    {
    vtkErrorMacro("PointExistsInMarkup: point index of " << p << " must be less than the current number of points in markup " << n << ", " << numPoints);
    return false;
    }
  return true;
}

//---------------------------------------------------------------------------
Markup *vtkMRMLMarkupsNode::GetNthMarkup(int n)
{
  if (this->MarkupExists(n))
    {
    return &(this->Markups[n]);
    }

  return NULL;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsNode:: GetNumberOfPointsInNthMarkup(int n)
{
  vtkDebugMacro("GetNumberOfPointsInNthMarkup: n = " << n << ", number of marksups = " << this->GetNumberOfMarkups());
  if (!this->MarkupExists(n))
    {
    return 0;
    }
  Markup *markupN = this->GetNthMarkup(n);
  if (markupN)
    {
    return markupN->points.size();
    }
  else
    {
    return 0;
    }
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::InitMarkup(Markup *markup)
{
  if (!markup)
    {
    vtkErrorMacro("InitMarkup: null markup!");
    return;
    }

  // set a default label
  int numberOfMarkups = this->GetNumberOfMarkups();
  // the markup hasn't been added to the list yet, so increment by one so as not to start with 0
  numberOfMarkups++;
  std::stringstream ss;
  ss << numberOfMarkups;
  std::string numberString;
  ss >> numberString;
  if (this->GetUseListNameForMarkups())
    {
    if (this->GetName() != NULL)
      {
      markup->Label = std::string(this->GetName()) + numberString;
      }
    else
      {
      markup->Label = numberString;
      }
    }
  else
    {
    markup->Label = std::string("M") + numberString;
    }

  // set the flags
  markup->Selected = true;
  markup->Visibility = true;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::AddMarkup(Markup markup)
{
  this->Markups.push_back(markup);
  if (!this->GetDisableModifiedEvent())
    {
    this->InvokeEvent(vtkMRMLMarkupsNode::MarkupAddedEvent);
    }
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddMarkupWithNPoints(int n)
{
  int markupIndex = -1;
  if (n < 0)
    {
    vtkErrorMacro("AddMarkupWithNPoints: invalid number of points " << n);
    return markupIndex;
    }
  Markup markup;
  this->InitMarkup(&markup);
  for (int i = 0; i < n; i++)
    {
    vtkVector3d p;
    p.SetX(0.0);
    p.SetY(0.0);
    p.SetZ(0.0);
    markup.points.push_back(p);
    }
  this->Markups.push_back(markup);
  markupIndex = this->GetNumberOfMarkups() - 1;
  if (!this->GetDisableModifiedEvent())
    {
    this->InvokeEvent(vtkMRMLMarkupsNode::MarkupAddedEvent, (void*)&markupIndex);
    }
  
  return markupIndex;
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddPointToNewMarkup(vtkVector3d point)
{
  int markupIndex = 0;

  Markup newmarkup;
  this->InitMarkup(&newmarkup);
  newmarkup.points.push_back(point);
  this->Markups.push_back(newmarkup);

  markupIndex = this->Markups.size() - 1;
  
  if (!this->GetDisableModifiedEvent())
    {
    this->InvokeEvent(vtkMRMLMarkupsNode::MarkupAddedEvent, (void*)&markupIndex);
    }

 
  
  return markupIndex;
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::AddPointToNthMarkup(vtkVector3d point, int n)
{
  int pointIndex = 0;
  if (this->MarkupExists(n))
    {
    this->Markups[n].points.push_back(point);
    }
  return pointIndex;
}

//-----------------------------------------------------------
vtkVector3d vtkMRMLMarkupsNode::GetMarkupPointVector(int markupIndex, int pointIndex)
{
  vtkVector3d point;
  point.SetX(0.0);
  point.SetY(0.0);
  point.SetZ(0.0);
  if (!PointExistsInMarkup(pointIndex, markupIndex))
    {
    return point;
    }
  point = this->GetNthMarkup(markupIndex)->points[pointIndex];
  return point;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::GetMarkupPoint(int markupIndex, int pointIndex, double point[3])
{
  vtkVector3d vectorPoint = this->GetMarkupPointVector(markupIndex, pointIndex);
  point[0] = vectorPoint.GetX();
  point[1] = vectorPoint.GetY();
  point[2] = vectorPoint.GetZ();
}

//-----------------------------------------------------------
int vtkMRMLMarkupsNode::GetMarkupPointWorld(int markupIndex, int pointIndex, double worldxyz[4])
{
  vtkVector3d vectorPoint = this->GetMarkupPointVector(markupIndex, pointIndex);
  double xyz[3];
  xyz[0] = vectorPoint.GetX();
  xyz[1] = vectorPoint.GetY();
  xyz[2] = vectorPoint.GetZ();
  // get the markup's transform node
  vtkMRMLTransformNode* tnode = this->GetParentTransformNode();
  vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
  transformToWorld->Identity();
  if (tnode != NULL && tnode->IsLinear())
    {
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    lnode->GetMatrixTransformToWorld(transformToWorld);
    }
  // convert by the parent transform
  double  xyzw[4];
  xyzw[0] = xyz[0];
  xyzw[1] = xyz[1];
  xyzw[2] = xyz[2];
  xyzw[3] = 1.0;

  transformToWorld->MultiplyPoint(xyzw, worldxyz);

  return 1;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::RemoveMarkup(int m)
{
  if (this->MarkupExists(m))
    {
    std::cout << "RemoveMarkup: m = " << m << ", markups size = " << this->Markups.size() << std::endl;
    this->Markups.erase(this->Markups.begin() + m);
    }
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SwapMarkups(int m1, int m2)
{
  if (!this->MarkupExists(m1) ||
      !this->MarkupExists(m1))
    {
    vtkErrorMacro("SwapMarkups: one of the markup indices is out of range 0-" << this->GetNumberOfMarkups() -1 << ", m1 = " << m1 << ", m2 = " << m2);
    return;
    }
  vtkErrorMacro("SwapMarkups: not implemented yet");
//  this->Markups[m1].swap(this->Markups[m2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetMarkupPointFromPointer(const int markupIndex, const int pointIndex,
                                        const double * pos)
{
  if (!pos)
    {
    vtkErrorMacro("SetMarkupPointFromPointer: invalid position pointer!");
    return;
    }
  this->SetMarkupPoint(markupIndex, pointIndex, pos[0], pos[1], pos[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetMarkupPointFromArray(const int markupIndex, const int pointIndex,
                                        const double pos[3])
{
  this->SetMarkupPoint(markupIndex, pointIndex, pos[0], pos[1], pos[2]);
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetMarkupPoint(const int markupIndex, const int pointIndex,
                                        const double x, const double y, const double z)
{
  if (!this->PointExistsInMarkup(pointIndex, markupIndex))
    {
    return;
    }
  Markup *markup = this->GetNthMarkup(markupIndex);
  if (markup)
    {
    markup->points[pointIndex].SetX(x);
    markup->points[pointIndex].SetY(y);
    markup->points[pointIndex].SetZ(z);
    }
  else
    {
    vtkErrorMacro("SetMarkupPoint: unable to get markup " << markupIndex);
    }
  // throw an event to let listeners know teh position has changed
  if(!this->GetDisableModifiedEvent())
    {
    this->InvokeEvent(vtkMRMLMarkupsNode::PointModifiedEvent, (void*)&markupIndex);
    }
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetMarkupPointWorld(const int markupIndex, const int pointIndex,
                                             const double x, const double y, const double z)
{
  if (!this->PointExistsInMarkup(pointIndex, markupIndex))
    {
    return;
    }
  // get the markup's transform node
  vtkMRMLTransformNode* tnode = this->GetParentTransformNode();
  vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
  transformToWorld->Identity();
  if (tnode != NULL)
  {
	  if (tnode->IsLinear())
    {
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    lnode->GetMatrixTransformToWorld(transformToWorld);
    }
  }
  // convert by the inverted parent transform
  transformToWorld->Invert();
  double  xyzw[4];
  xyzw[0] = x;
  xyzw[1] = y;
  xyzw[2] = z;
  xyzw[3] = 1.0;
  double worldxyz[4], *worldp = &worldxyz[0];
  transformToWorld->MultiplyPoint(xyzw, worldp);

  tnode = NULL;

  this->SetMarkupPoint(markupIndex, pointIndex, worldxyz[0], worldxyz[1], worldxyz[2]);
}
//-----------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetNthMarkupAssociatedNodeID(int n)
{
  std::string id = std::string("");
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      id = markup->AssociatedNodeID;
      }
    }
  else
    {
    std::cerr << "GetNthMarkupAssociatedNodeID: markup " << n << " doesn't exist" << std::endl;
    }
  return id;
}

//-----------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupAssociatedNodeID(int n, std::string id)
{
  vtkDebugMacro("SetNthMarkupAssociatedNodeID: n = " << n << ", id = '" << id.c_str() << "'");
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      std::cout << "Changing markup " << n << " associated node id from " << markup->AssociatedNodeID.c_str() << " to " << id.c_str() << std::endl;
      markup->AssociatedNodeID = std::string(id.c_str());
      }
    }
  else
    {
    std::cerr << "SetNthMarkupAssociatedNodeID: markup " << n << " doesn't exist, can't set id to " << id.c_str() << std::endl;
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetNthMarkupSelected(int n)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      return markup->Selected;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupSelected(int n, bool flag)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      if (markup->Selected != flag)
        {
        markup->Selected = flag;
        int markupIndex = n;
        this->InvokeEvent(vtkMRMLMarkupsNode::NthMarkupModifiedEvent, (void*)&markupIndex);
        }
      }
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::GetNthMarkupVisibility(int n)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      return markup->Visibility;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupVisibility(int n, bool flag)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      if (markup->Visibility != flag)
        {
        markup->Visibility = flag;
        int markupIndex = n;
        this->InvokeEvent(vtkMRMLMarkupsNode::NthMarkupModifiedEvent, (void*)&markupIndex);
        }
      }
    }
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsNode::GetNthMarkupLabel(int n)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      return markup->Label;
      }
    }
  return std::string("");
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::SetNthMarkupLabel(int n, std::string label)
{
  if (this->MarkupExists(n))
    {
    Markup *markup = this->GetNthMarkup(n);
    if (markup)
      {
      if (markup->Label.compare(label))
        {
        markup->Label = label;
        int markupIndex = n;
        this->InvokeEvent(vtkMRMLMarkupsNode::NthMarkupModifiedEvent, (void*)&markupIndex);
        }
      }
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsNode::CanApplyNonLinearTransforms()const
{
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::ApplyTransformMatrix(vtkMatrix4x4* transformMatrix)
{
  int numMarkups = this->GetNumberOfMarkups();
  double (*matrix)[4] = transformMatrix->Element;
  double xyzIn[3];
  double xyzOut[3];
  for (int m = 0; m < numMarkups; m++)
    {
    int numPoints = this->GetNumberOfPointsInNthMarkup(m);
    for (int n=0; n<numPoints; n++)
      {
      this->GetMarkupPoint(m, n, xyzIn);
      xyzOut[0] = matrix[0][0]*xyzIn[0] + matrix[0][1]*xyzIn[1] + matrix[0][2]*xyzIn[2] + matrix[0][3];
      xyzOut[1] = matrix[1][0]*xyzIn[0] + matrix[1][1]*xyzIn[1] + matrix[1][2]*xyzIn[2] + matrix[1][3];
      xyzOut[2] = matrix[2][0]*xyzIn[0] + matrix[2][1]*xyzIn[1] + matrix[2][2]*xyzIn[2] + matrix[2][3];
      std::cout << "ApplyTransformMatrix: xyzIn = " << xyzIn[0] << "," << xyzIn[1] << "," << xyzIn[2] << ", xyzOut = " << xyzOut[0] << "," << xyzOut[1] << "," << xyzOut[2] << std::endl;
      this->SetMarkupPointFromArray(m, n, xyzOut);
      }
    }

  this->StorableModifiedTime.Modified();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::ApplyTransform(vtkAbstractTransform* transform)
{
  int numMarkups = this->GetNumberOfMarkups();
  double xyzIn[3];
  double xyzOut[3];
  for (int m=0; m<numMarkups; m++)
    {
    int numPoints = this->GetNumberOfPointsInNthMarkup(m);
    for (int n=0; n<numPoints; n++)
      {
      this->GetMarkupPoint(m, n, xyzIn);
      transform->TransformPoint(xyzIn,xyzOut);
      this->SetMarkupPointFromArray(m, n, xyzOut);
      }
    }
  this->StorableModifiedTime.Modified();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsNode::WriteCLI(std::ostringstream& ss, std::string prefix)
{
  Superclass::WriteCLI(ss, prefix);

  int numMarkups = this->GetNumberOfMarkups();

  // loop over the markups
  for (int m=0; m<numMarkups; m++)
    {
    // only use selected markups
    if (this->GetNthMarkupSelected(m))
      {
      int numPoints = this->GetNumberOfPointsInNthMarkup(m);
      // loop over the points
      for (int n=0; n<numPoints; n++)
        {
        double point[3];
        this->GetMarkupPoint(m, n, point);
        // write
        if (prefix.compare("") != 0)
          {
          ss << prefix << " ";
          }
        // avoid scientific notation
        //ss.precision(5);
        //ss << std::fixed << point[0] << "," <<  point[1] << "," <<  point[2] ;
        ss << point[0] << "," <<  point[1] << "," <<  point[2];
        if (n < numPoints-1) 
          {
          // put a space after each point if there's another point to print
          ss << " ";
          }
        }
      // add a space if there are more markups to print
      if (m < numMarkups-1)
        {
         ss << " ";
        }
      }
    }
}

