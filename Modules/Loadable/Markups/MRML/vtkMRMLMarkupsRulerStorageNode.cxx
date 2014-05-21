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

#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsRulerStorageNode.h"
#include "vtkMRMLMarkupsNode.h"

#include "vtkMRMLScene.h"
#include "vtkSlicerVersionConfigure.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include <vtksys/SystemTools.hxx>

#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsRulerStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsRulerStorageNode::vtkMRMLMarkupsRulerStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsRulerStorageNode::~vtkMRMLMarkupsRulerStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerStorageNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsRulerStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLMarkupsRulerNode");
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsRulerStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  if (!refNode)
    {
    vtkErrorMacro("ReadDataInternal: null reference node!");
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();

  if (fullName == std::string(""))
    {
    vtkErrorMacro("vtkMRMLMarkupsRulerStorageNode: File name not specified");
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

  // check if it's an annotation csv file
  bool parseAsAnnotationRuler = false;
  std::string ext = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if (ext.compare(".acsv") == 0)
    {
    parseAsAnnotationRuler = true;
    }

  // open the file for reading input
  fstream fstr;

  fstr.open(fullName.c_str(), fstream::in);

  if (!fstr.is_open())
    {
    vtkErrorMacro("ERROR opening markups file " << this->FileName << endl);
    return 0;
    }

  if (markupsNode->GetNumberOfMarkups() > 0)
    {
    // clear out the list
    markupsNode->RemoveAllMarkups();
    }

  char line[MARKUPS_BUFFER_SIZE];

  int thisMarkupNumber = 0;

  // check for the version
  std::string version;

  // coordinate system
  int coordinateSystemFlag = 0;

  while (fstr.good())
    {
    fstr.getline(line, MARKUPS_BUFFER_SIZE);

    if (line[0] == '#')
      {
      // comment line
      vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");

      // if there's a space after the hash, check for the version
      if (line[1] == ' ')
        {
        vtkDebugMacro("Have a possible option in line " << line);
        std::string lineString = std::string(line);
        if (lineString.find("# Markups ruler file version = ") != std::string::npos)
          {
          version = lineString.substr(31,std::string::npos);
          vtkWarningMacro("Version = " << version);
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
    // is it empty?
    else if (line[0] == '\0')
      {
      vtkDebugMacro("Empty line, skipping:\n\"" << line << "\"");
      }
    else
      {
      vtkWarningMacro("Valid line checking:\n\"" << line << "\"");
      // hopefully have a line defining a ruler
      if (version.size() == 0 && !parseAsAnnotationRuler)
        {
        vtkErrorMacro("Have an invalid ruler file, unable to parse "
                      << fullName);
        fstr.close();
        return 0;
        }
      if (version.size() == 0 && parseAsAnnotationRuler)
        {
        std::string label = std::string("");
        double x = 0.0, y = 0.0, z = 0.0;
        int sel = 1, vis = 1;

        std::stringstream ss(line);
        std::string component;

        // annotation ruler point line format = point|x|y|z|sel|vis
        getline(ss, component, '|');
        if (component.compare("point") == 0)
          {
          vtkWarningMacro("Got point string = " << component.c_str());
          if (!markupsNode->MarkupExists(thisMarkupNumber))
            {
            thisMarkupNumber = markupsNode->AddMarkupWithNPoints(2);
            // use the file name for the ruler name
            std::string filenameName = vtksys::SystemTools::GetFilenameName(this->GetFileName());
            label = vtksys::SystemTools::GetFilenameWithoutExtension(filenameName);
            markupsNode->SetName(label.c_str());
            }

          // x1,y1,z1
          getline(ss, component, '|');
          x = atof(component.c_str());
          getline(ss, component, '|');
          y = atof(component.c_str());
          getline(ss, component, '|');
          z = atof(component.c_str());
          markupsNode->SetMarkupPoint(thisMarkupNumber,0,x,y,z);

          // selected
          getline(ss, component, '|');
          sel = atoi(component.c_str());
          markupsNode->SetNthMarkupSelected(thisMarkupNumber,sel);

          // visibility
          getline(ss, component, '|');
          vtkDebugMacro("component = " << component.c_str());
          vis = atoi(component.c_str());
          markupsNode->SetNthMarkupVisibility(thisMarkupNumber,vis);

          // get the second point line
          fstr.getline(line, MARKUPS_BUFFER_SIZE);
          std::stringstream ss2(line);
          getline(ss2, component, '|');
          if (component.compare("point") == 0)
            {
            // x2,y2,z2
            getline(ss2, component, '|');
            x = atof(component.c_str());
            getline(ss2, component, '|');
            y = atof(component.c_str());
            getline(ss2, component, '|');
            z = atof(component.c_str());
            markupsNode->SetMarkupPoint(thisMarkupNumber,1,x,y,z);
            // keep selected and visibility from the first point line
            }
          else
            {
            vtkErrorMacro("Second point missing for markup "
                          << thisMarkupNumber);
            fstr.close();
            return 0;
            }
          }
        else
          {
          // should only be a line or ruler entry
          if (component.compare("line") != 0 &&
              component.compare("ruler") != 0)
            {
            vtkErrorMacro("Unexpected line start: "
                          << component.c_str());
            fstr.close();
            return 0;
            }
          }
        }
      else
        // Slicer 4 markups ruler file
        {
        vtkDebugMacro("\n\n\n\nVersion = " << version
                      << ", got a line: \n\"" << line << "\"");
        std::string id = std::string("");
        double x1 = 0.0, y1 = 0.0, z1 = 0.0;
        double x2 = 0.0, y2 = 0.0, z2 = 0.0;
        double ow = 0.0, ox = 0.0, oy = 0.0, oz = 1.0;
        int sel = 1, vis = 1, lock = 0;
        std::string associatedNodeID = std::string("");
        std::string label = std::string("");
        std::string desc = std::string("");

        std::stringstream ss(line);
        int numPoints = 2;
        thisMarkupNumber = markupsNode->AddMarkupWithNPoints(numPoints);

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

        // x1,y1,z1
        getline(ss, component, ',');
        x1 = atof(component.c_str());
        getline(ss, component, ',');
        y1 = atof(component.c_str());
        getline(ss, component, ',');
        z1 = atof(component.c_str());
        if (this->GetCoordinateSystem() == vtkMRMLMarkupsRulerStorageNode::RAS)
          {
          markupsNode->SetMarkupPoint(thisMarkupNumber,0,x1,y1,z1);
          }
        else if (this->GetCoordinateSystem() == vtkMRMLMarkupsRulerStorageNode::LPS)
          {
          markupsNode->SetMarkupPointLPS(thisMarkupNumber,0,x1,y1,z1);
          }
        else
          {
          // IJK not implemented yet, assume RAS
          markupsNode->SetMarkupPoint(thisMarkupNumber,0,x1,y1,z1);
          }

        // x2,y2,z2
        getline(ss, component, ',');
        x2 = atof(component.c_str());
        getline(ss, component, ',');
        y2 = atof(component.c_str());
        getline(ss, component, ',');
        z2 = atof(component.c_str());
        if (this->GetCoordinateSystem() == vtkMRMLMarkupsRulerStorageNode::RAS)
          {
          markupsNode->SetMarkupPoint(thisMarkupNumber,1,x2,y2,z2);
          }
        else if (this->GetCoordinateSystem() == vtkMRMLMarkupsRulerStorageNode::LPS)
          {
          markupsNode->SetMarkupPointLPS(thisMarkupNumber,1,x2,y2,z2);
          }
        else
          {
          // IJK not implemented yet, assume RAS
          markupsNode->SetMarkupPoint(thisMarkupNumber,1,x2,y2,z2);
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
        // the label may have quotes around it, look for the end quote and comma
        // std::cout << "Line ss = '" << ss.str().c_str() << "'" << std::endl;
        std::string labelDescID;
        getline(ss, labelDescID);
        // if there's no quote at the start of the line, the label was
        // checked to be sure that there are no commas in it, so extract
        // to the next comma
        size_t endCommaPos;
        if (labelDescID[0] != '"')
          {
          endCommaPos = labelDescID.find(",");
          component = labelDescID.substr(0, endCommaPos);
          }
        else
          {
          component = this->GetFirstQuotedString(labelDescID, &endCommaPos);
          }
        if (component.size())
          {
          vtkDebugMacro("Got label = " << component.c_str());
          label = component;
          markupsNode->SetNthMarkupLabelFromStorage(thisMarkupNumber,label);
          }
        else
          {
          vtkDebugMacro("No label");
          markupsNode->SetNthMarkupLabel(thisMarkupNumber,"");
          }

        // description
        // get the rest of the string after the label
        std::string descID = labelDescID.substr(endCommaPos + 1);
        // the description may have quotes around it as well
        if (descID[0] != '"')
          {
          endCommaPos = descID.find(",");
          component = descID.substr(0, endCommaPos);
          }
        else
          {
          component = this->GetFirstQuotedString(descID, &endCommaPos);
          }
        if (component.size())
          {
          vtkDebugMacro("Got desc = " << component.c_str());
          desc = component;
          markupsNode->SetNthMarkupDescriptionFromStorage(thisMarkupNumber,desc);
          }
        else
          {
          vtkDebugMacro("No description");
          markupsNode->SetNthMarkupDescription(thisMarkupNumber,"");
          }

        // in case the file was written by hand, the associated node id
        // might be empty
        size_t associatedNodeIDPos = ss.str().find_last_of(',');
        if (associatedNodeIDPos != std::string::npos)
          {
          component = ss.str().substr(associatedNodeIDPos + 1);
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
          }
        else
          {
          markupsNode->SetNthMarkupAssociatedNodeID(thisMarkupNumber,"");
          }

        vtkDebugMacro("Line parsed, got id = " << id << ", vis = " << vis
                      << ", sel = " << sel
                      << ", associatedNodeID = " << associatedNodeID.c_str()
                      << ", label = '" << label.c_str()
                      << "', markup number is " << thisMarkupNumber);
        } // non comment line
      }
    }
  fstr.close();

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsRulerStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
    {
    vtkErrorMacro("vtkMRMLMarkupsRulerStorageNode: File name not specified");
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
  of << "# Markups ruler file version = " << Slicer_VERSION << endl;
  of << "# CoordinateSystem = " << this->GetCoordinateSystem() << endl;

  // label the columns
  // id,x1,y1,z1,x2,y2,z2,ow,ox,oy,oz,vis,sel,lock,label,desc,associatedNodeID
  // orientation is a quaternion, angle and axis
  // associatedNodeID and description and label can be empty strings
  // id,x1,y1,z1,x2,y2,z2,ow,ox,oy,oz,vis,sel,lock,,,
  // label can have spaces, everything up to next comma is used, no quotes
  // necessary, same with the description
  of << "# columns = id,x1,y1,z1,x2,y2,z2,ow,ox,oy,oz,vis,sel,lock,label,desc,associatedNodeID" << endl;
  for (int i = 0; i < numberOfMarkups; i++)
    {
    std::string id = markupsNode->GetNthMarkupID(i);
    of << id.c_str();
    vtkDebugMacro("WriteDataInternal: wrote id " << id.c_str());

    for (int p = 0; p < 2; ++p)
      {
      double xyz[3];
      if (this->GetCoordinateSystem() == vtkMRMLMarkupsRulerStorageNode::RAS)
        {
        markupsNode->GetMarkupPoint(i,p,xyz);
        }
      else if (this->GetCoordinateSystem() == vtkMRMLMarkupsRulerStorageNode::LPS)
        {
        markupsNode->GetMarkupPointLPS(i,p,xyz);
        }
      else if (this->GetCoordinateSystem() == vtkMRMLMarkupsRulerStorageNode::IJK)
        {
        // not implemented yet, use RAS
//      markupsNode->GetMarkupPointIJK(i,p,xyz);
        markupsNode->GetMarkupPoint(i,p,xyz);
        }
      of << "," << xyz[0] << "," << xyz[1] << "," << xyz[2];
      }

    double orientation[4];
    markupsNode->GetNthMarkupOrientation(i, orientation);
    bool vis = markupsNode->GetNthMarkupVisibility(i);
    bool sel = markupsNode->GetNthMarkupSelected(i);
    bool lock = markupsNode->GetNthMarkupLocked(i);

    std::string label = markupsNode->GetNthMarkupLabelForStorage(i);
    std::string desc = markupsNode->GetNthMarkupDescriptionForStorage(i);
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
void vtkMRMLMarkupsRulerStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Markups Ruler CSV (.rcsv)");
  this->SupportedReadFileTypes->InsertNextValue("Annotation Ruler CSV (.acsv)");
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Markups Ruler CSV (.rcsv)");
}

//----------------------------------------------------------------------------
const char* vtkMRMLMarkupsRulerStorageNode::GetDefaultWriteFileExtension()
{
  return "rcsv";
}
