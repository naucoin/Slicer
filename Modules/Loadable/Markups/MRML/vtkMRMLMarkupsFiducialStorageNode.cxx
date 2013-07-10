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

#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerVersionConfigure.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsFiducialStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialStorageNode::vtkMRMLMarkupsFiducialStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialStorageNode::~vtkMRMLMarkupsFiducialStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialStorageNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsFiducialStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLMarkupsFiducialNode");
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsFiducialStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  if (!refNode)
    {
    vtkErrorMacro("ReadDataInternal: null reference node!");
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();

  if (fullName == std::string(""))
    {
    vtkErrorMacro("vtkMRMLMarkupsFiducialStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLMarkupsNode *markupsNode =
    vtkMRMLMarkupsNode::SafeDownCast(refNode);
  if (!markupsNode)
    {
    return 0;
    }

  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = NULL;
  vtkMRMLDisplayNode * mrmlNode = markupsNode->GetDisplayNode();
  if (mrmlNode)
    {
    displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(mrmlNode);
    }
  if (!displayNode)
    {
    vtkWarningMacro("ReadDataInternal: no display node!");
    if (this->GetScene())
      {
      vtkWarningMacro("ReadDataInternal: adding a new display node.");
      displayNode = vtkMRMLMarkupsDisplayNode::New();
      this->GetScene()->AddNode(displayNode);
      markupsNode->SetAndObserveDisplayNodeID(displayNode->GetID());
      displayNode->Delete();
      }
    }

  // open the file for reading input
  fstream fstr;

  fstr.open(fullName.c_str(), fstream::in);

  if (fstr.is_open())
    {
    if (markupsNode->GetNumberOfMarkups() > 0)
      {
      // clear out the list
      markupsNode->RemoveAllMarkups();
      }

    char line[1024];

    // save the valid lines in a vector, parse them once know the max id
    std::vector<std::string>lines;
    int thisMarkupNumber = 0;

    // check for the version
    std::string version;
    // only print out the warning once
    bool printedVersionWarning = false;

    // coordinate system
    int coordinateSystemFlag = 0;

    while (fstr.good())
      {
      fstr.getline(line, 1024);

      // does it start with a #?
      if (line[0] == '#')
        {
        vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");

        // if there's a space after the hash, check for the version
        if (line[1] == ' ')
          {
          vtkDebugMacro("Have a possible option in line " << line);
          std::string lineString = std::string(line);
          if (lineString.find("# Markups fiducial file version = ") != std::string::npos)
            {
            version = lineString.substr(34,std::string::npos);
            vtkDebugMacro("Version = " << version);
            }
          else if (lineString.find("# CoordinateSystem = ") != std::string::npos)
            {
            std::string str = lineString.substr(21,std::string::npos);
            coordinateSystemFlag = atoi(str.c_str());
            vtkDebugMacro("CoordinateSystem = " << coordinateSystemFlag);
            this->SetCoordinateSystem(coordinateSystemFlag);
            }
          else if (lineString.find("# columns = ") != std::string::npos)
            {
            // the markups header, fixed
            }
          }
        }
      else
        {
        // is it empty?
        if (line[0] == '\0')
          {
          vtkDebugMacro("Empty line, skipping:\n\"" << line << "\"");
          }
        else
          {
          if (version.size() == 0)
            {
            if (!printedVersionWarning)
              {
              vtkWarningMacro("Have an unversioned file, assuming Slicer 3 format .fcsv");
              printedVersionWarning = true;
              }
            // point line format = label,x,y,z,sel,vis
            std::string label = std::string("");
            double x = 0.0, y = 0.0, z = 0.0;
            int sel = 1, vis = 1;

            markupsNode->AddMarkupWithNPoints(1);

            std::stringstream ss(line);
            std::string component;

            // label
            getline(ss, component, ',');
            if (component.size())
              {
              vtkDebugMacro("Got label = " << component.c_str());
              label = component;
              markupsNode->SetNthMarkupLabel(thisMarkupNumber,label);
              }

            // x,y,z
            getline(ss, component, ',');
            x = atof(component.c_str());
            getline(ss, component, ',');
            y = atof(component.c_str());
            getline(ss, component, ',');
            z = atof(component.c_str());
            markupsNode->SetMarkupPoint(thisMarkupNumber,0,x,y,z);

            // selected
            getline(ss, component, ',');
            sel = atoi(component.c_str());
            markupsNode->SetNthMarkupSelected(thisMarkupNumber,sel);

            // visibility
            getline(ss, component, ',');
            vtkDebugMacro("component = " << component.c_str());
            vis = atoi(component.c_str());
            markupsNode->SetNthMarkupVisibility(thisMarkupNumber,vis);

            thisMarkupNumber++;
            }
          else
            {
            // Slicer 4 markups fiducial file
            vtkDebugMacro("\n\n\n\nVersion = " << version << ", got a line: \n\"" << line << "\"");
            std::string id = std::string("");
            double x = 0.0, y = 0.0, z = 0.0;
            double ow = 0.0, ox = 0.0, oy = 0.0, oz = 1.0;
            int sel = 1, vis = 1, lock = 0;
            std::string associatedNodeID = std::string("");
            std::string label = std::string("");
            std::string desc = std::string("");

            std::stringstream ss(line);
            int numPoints = 1;
            markupsNode->AddMarkupWithNPoints(numPoints);

            std::string component;

            // id
            getline(ss, component, ',');
            if (component.size())
              {
              vtkDebugMacro("Got id = " << component.c_str());
              id = component;
              markupsNode->SetNthMarkupID(thisMarkupNumber,id);
              }
            else
              {
              vtkDebugMacro("No ID");
              if (this->GetScene())
                {
                markupsNode->SetNthMarkupID(thisMarkupNumber,this->GetScene()->GenerateUniqueName(this->GetID()));
                }
              }

            // x,y,z
            getline(ss, component, ',');
            x = atof(component.c_str());
            getline(ss, component, ',');
            y = atof(component.c_str());
            getline(ss, component, ',');
            z = atof(component.c_str());
            if (this->GetCoordinateSystem() == vtkMRMLMarkupsFiducialStorageNode::RAS)
              {
              markupsNode->SetMarkupPoint(thisMarkupNumber,0,x,y,z);
              }
            else if (this->GetCoordinateSystem() == vtkMRMLMarkupsFiducialStorageNode::LPS)
              {
              markupsNode->SetMarkupPointLPS(thisMarkupNumber,0,x,y,z);
              }
            else
              {
              // IJK not implemented yet, assume RAS
              markupsNode->SetMarkupPoint(thisMarkupNumber,0,x,y,z);
              }

            // orientatation
            getline(ss, component, ',');
            ow = atof(component.c_str());
            getline(ss, component, ',');
            ox = atof(component.c_str());
            getline(ss, component, ',');
            oy = atof(component.c_str());
            getline(ss, component, ',');
            oz = atof(component.c_str());
            markupsNode->SetNthMarkupOrientation(thisMarkupNumber, ow, ox, oy, oz);

            // visibility
            getline(ss, component, ',');
            vtkDebugMacro("component = " << component.c_str());
            vis = atoi(component.c_str());
            markupsNode->SetNthMarkupVisibility(thisMarkupNumber,vis);

            // selected
            getline(ss, component, ',');
            sel = atoi(component.c_str());
            markupsNode->SetNthMarkupSelected(thisMarkupNumber,sel);

            // locked
            getline(ss, component, ',');
            lock = atoi(component.c_str());
            markupsNode->SetNthMarkupLocked(thisMarkupNumber,lock);

            // label
            getline(ss, component, ',');
            if (component.size())
              {
              vtkDebugMacro("Got label = " << component.c_str());
              label = component;
              markupsNode->SetNthMarkupLabel(thisMarkupNumber,label);
              }
            else
              {
              vtkDebugMacro("No label");
              markupsNode->SetNthMarkupLabel(thisMarkupNumber,"");
              }

            // description
            getline(ss, component, ',');
            if (component.size())
              {
              vtkDebugMacro("Got desc = " << component.c_str());
              desc = component;
              markupsNode->SetNthMarkupDescription(thisMarkupNumber,desc);
              }
            else
              {
              vtkDebugMacro("No description");
              markupsNode->SetNthMarkupDescription(thisMarkupNumber,"");
              }

            // in case the file was written by hand, the associated node id
            // might be empty
            getline(ss, component, ',');
            if (component.size())
              {
              vtkDebugMacro("Got associated node id = " << component.c_str());
              associatedNodeID = component;
              markupsNode->SetNthMarkupAssociatedNodeID(thisMarkupNumber,associatedNodeID);
              }
            else
              {
              vtkDebugMacro("no associated node id");
              markupsNode->SetNthMarkupAssociatedNodeID(thisMarkupNumber,"");
              }

            thisMarkupNumber++;
            vtkDebugMacro("got id = " << id << ", vis = " << vis << ", sel = " << sel
                          << ", associatedNodeID = " << associatedNodeID.c_str()
                          << ", label = '" << label.c_str() << "', markup number is now " << thisMarkupNumber);
            } // point line
          }
        }
      }

//    markupsNode->SetDisableModifiedEvent(modFlag);
//    markupsNode->InvokeEvent(vtkMRMLScene::NodeAddedEvent, markupsNode);//vtkMRMLMarkupsNode::DisplayModifiedEvent);
    fstr.close();

//    this->DebugOff();
    }
  else
    {
    vtkErrorMacro("ERROR opening markups file " << this->FileName << endl);
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsFiducialStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
    {
    vtkErrorMacro("vtkMRMLMarkupsFiducialStorageNode: File name not specified");
    return 0;
    }
  vtkDebugMacro("WriteDataInternal: have file name " << fullName.c_str());

  // cast the input node
  vtkMRMLMarkupsNode *markupsNode = NULL;
  if ( refNode->IsA("vtkMRMLMarkupsNode") )
    {
    markupsNode = dynamic_cast <vtkMRMLMarkupsNode *> (refNode);
    }

  if (markupsNode == NULL)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known markups node");
    return 0;
    }

  // open the file for writing
  fstream of;

  of.open(fullName.c_str(), fstream::out);

  if (!of.is_open())
    {
    vtkErrorMacro("WriteData: unable to open file " << fullName.c_str() << " for writing");
    return 0;
    }
  int numberOfMarkups = markupsNode->GetNumberOfMarkups();

  // put down a header
  of << "# Markups fiducial file version = " << Slicer_VERSION << endl;
  of << "# CoordinateSystem = " << this->GetCoordinateSystem() << endl;

  // label the columns
  // id,x,y,z,ow,ox,oy,oz,vis,sel,lock,label,desc,associatedNodeID
  // orientation is a quaternion, angle and axis
  // associatedNodeID and description and label can be empty strings
  // id,x,y,z,ow,ox,oy,oz,vis,sel,lock,,,
  // label can have spaces, everything up to next comma is used, no quotes
  // necessary, same with the description
  of << "# columns = id,x,y,z,ow,ox,oy,oz,vis,sel,lock,label,desc,associatedNodeID" << endl;
  for (int i = 0; i < numberOfMarkups; i++)
    {
    std::string id = markupsNode->GetNthMarkupID(i);
    of << id.c_str();
    vtkDebugMacro("WriteDataInternal: wrote id " << id.c_str());

    int p = 0;
    double xyz[3];
    if (this->GetCoordinateSystem() == vtkMRMLMarkupsFiducialStorageNode::RAS)
      {
      markupsNode->GetMarkupPoint(i,p,xyz);
      }
    else if (this->GetCoordinateSystem() == vtkMRMLMarkupsFiducialStorageNode::LPS)
      {
      markupsNode->GetMarkupPointLPS(i,p,xyz);
      }
    else if (this->GetCoordinateSystem() == vtkMRMLMarkupsFiducialStorageNode::IJK)
      {
      // not implemented yet, use RAS
//      markupsNode->GetMarkupPointIJK(i,p,xyz);
      markupsNode->GetMarkupPoint(i,p,xyz);
      }
    of << "," << xyz[0] << "," << xyz[1] << "," << xyz[2];

    double orientation[4];
    markupsNode->GetNthMarkupOrientation(i, orientation);
    bool vis = markupsNode->GetNthMarkupVisibility(i);
    bool sel = markupsNode->GetNthMarkupSelected(i);
    bool lock = markupsNode->GetNthMarkupLocked(i);

    std::string label = markupsNode->GetNthMarkupLabel(i);
    std::string desc = markupsNode->GetNthMarkupDescription(i);
    if (desc.size() == 0)
      {
      desc = std::string("");
      }

    std::string associatedNodeID = markupsNode->GetNthMarkupAssociatedNodeID(i);
    if (associatedNodeID.size() == 0)
      {
      associatedNodeID = std::string("");
      }
    of << "," << orientation[0] << "," << orientation[1] << "," << orientation[2] << "," << orientation[3];
    of << "," << vis << "," << sel << "," << lock;
    of << "," << label;
    of << "," << desc;
    of << "," << associatedNodeID;

    of << endl;
    }

  of.close();

  return 1;

}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Markups Fiducial CSV (.fcsv)");
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Markups Fiducial CSV (.fcsv)");
}

//----------------------------------------------------------------------------
const char* vtkMRMLMarkupsFiducialStorageNode::GetDefaultWriteFileExtension()
{
  return "fcsv";
}
