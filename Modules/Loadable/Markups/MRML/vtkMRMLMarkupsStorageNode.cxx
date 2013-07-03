#include <sstream>

#include "vtkMRMLMarkupsStorageNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLScene.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"


//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsStorageNode::vtkMRMLMarkupsStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsStorageNode::~vtkMRMLMarkupsStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLMarkupsNode");
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  if (!refNode)
    {
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();

  if (fullName == std::string("")) 
    {
    vtkErrorMacro("vtkMRMLMarkupsStorageNode: File name not specified");
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
//      mrmlGlyphType = markupsNode->GetGlyphType();
      // clear out the list
      markupsNode->RemoveAllMarkups();
      }

    // turn off modified events
//    int modFlag = markupsNode->GetDisableModifiedEvent();
//    markupsNode->DisableModifiedEventOn();
    char line[1024];

    // save the valid lines in a vector, parse them once know the max id
    std::vector<std::string>lines;
    int numberOfMarkups = 0;
    int thisMarkupNumber = 0;

//    this->DebugOn();

    while (fstr.good())
      {
      fstr.getline(line, 1024);
      
      // does it start with a #?
      if (line[0] == '#')
        {
        vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");
        // if there's a space after the hash, try to find options
        if (line[1] == ' ')
          {
          vtkDebugMacro("Have a possible option in line " << line);
          std::string lineString = std::string(line);
          if (lineString.find("# numPoints = ") != std::string::npos)
            {
            std::string str = lineString.substr(14,std::string::npos);
            numberOfMarkups = atof(str.c_str());
            vtkDebugMacro("Skipping using numPoints, number of markups = " << numberOfMarkups);
            }
          else if (lineString.find("# name = ") != std::string::npos)
            {
            std::string str = lineString.substr(9,std::string::npos);
            vtkDebugMacro("Getting name, substr = " << str);
            markupsNode->SetName(str.c_str());
            }
          else if (lineString.find("# locked = ") != std::string::npos)
            {
            std::string str = lineString.substr(10, std::string::npos);
            vtkDebugMacro("Getting locked, substr = " << str.c_str());
            int val = atoi(str.c_str());
            markupsNode->SetLocked(val);
            }
          else if (lineString.find("# visibility = ") != std::string::npos)
            {
            std::string str = lineString.substr(15,std::string::npos);
            vtkDebugMacro("Getting visibility, substr = " << str);
            int vis = atoi(str.c_str());
            if (displayNode)
              {
              displayNode->SetVisibility(vis);
              }
            }
          else if (lineString.find("# symbolScale = ") != std::string::npos)
            {
            std::string str = lineString.substr(16,std::string::npos);
            vtkDebugMacro("Getting symbolScale, substr = " << str);
            float scale = atof(str.c_str());
            if (displayNode)
              {
              displayNode->SetGlyphScale(scale);
              }
            }
          else if (lineString.find("# symbolType = ") != std::string::npos)
            {
            std::string str = lineString.substr(15,std::string::npos);
            vtkDebugMacro("Getting symbolType, substr = " << str);
            if (displayNode)
              {
              displayNode->SetGlyphTypeFromString(str.c_str());
              }
            }
          else if (lineString.find("# textScale = ") != std::string::npos)
            {
            std::string str = lineString.substr(14, std::string::npos);
            vtkDebugMacro("Getting textScale, substr = " << str.c_str());
            float scale = atof(str.c_str());
            if (displayNode)
              {
              displayNode->SetTextScale(scale);
              }
            }
          else if (lineString.find("# color = ") != std::string::npos ||
                   lineString.find("# selectedColor = ") != std::string::npos)
            {
            std::string str;
            if (lineString.find("# color = ") != std::string::npos)
              {
              str = lineString.substr(10, std::string::npos);
              }
            else
              {
              str = lineString.substr(18, std::string::npos);
              }
            vtkDebugMacro("Getting colors, substr = " << str.c_str());
            // the rgb values are separated by commas
            float r = 0.0, g = 0.0, b = 0.0;
            char *ptr;
            char *colours = (char *)(str.c_str());
            ptr = strtok(colours, ",");
            if (ptr != NULL)
              {
              r = atof(ptr);
              }
            ptr = strtok(NULL, ",");
            if (ptr != NULL)
              {
              g = atof(ptr);
              }
            ptr = strtok(NULL, ",");
            if (ptr != NULL)
              {
              b = atof(ptr);
              }
            // now set the correct value
            if (displayNode)
              {
              if (lineString.find("# color = ") != std::string::npos)
                {
                displayNode->SetColor(r,g,b);
                }
              else
                {
                displayNode->SetSelectedColor(r,g,b);
                }
              }
            }
          else if (lineString.find("# opacity = ") != std::string::npos)
            {
            std::string str = lineString.substr(12, std::string::npos);
            vtkDebugMacro("Getting opacity, substr = " << str.c_str());
            float val = atof(str.c_str());
            if (displayNode)
              {
              displayNode->SetOpacity(val);
              }
            }
          else if (lineString.find("# ambient = ") != std::string::npos)
            {
            std::string str = lineString.substr(12, std::string::npos);
            vtkDebugMacro("Getting ambient, substr = " << str.c_str());
            float val = atof(str.c_str());
            if (displayNode)
              {
              displayNode->SetAmbient(val);
              }
            }
          else if (lineString.find("# diffuse = ") != std::string::npos)
            {
            std::string str = lineString.substr(12, std::string::npos);
            vtkDebugMacro("Getting diffuse, substr = " << str.c_str());
            float val = atof(str.c_str());
            if (displayNode)
              {
              displayNode->SetDiffuse(val);
              }
            }
          else if (lineString.find("# specular = ") != std::string::npos)
            {
            std::string str = lineString.substr(13, std::string::npos);
            vtkDebugMacro("Getting specular, substr = " << str.c_str());
            float val = atof(str.c_str());
            if (displayNode)
              {
              displayNode->SetSpecular(val);
              }
            }
          else if (lineString.find("# power = ") != std::string::npos)
            {
            std::string str = lineString.substr(10, std::string::npos);
            vtkDebugMacro("Getting power, substr = " << str.c_str());
            float val = atof(str.c_str());
            if (displayNode)
              {
              displayNode->SetPower(val);
              }
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
          vtkDebugMacro("\n\n\n\ngot a line: \n\"" << line << "\"");
          int numPoints = 0;
          double x = 0.0, y = 0.0, z = 0.0;
          int sel = 1, vis = 1, lock = 0;
          std::string id = std::string("");
          std::string associatedNodeID = std::string("");
          std::string label = std::string("");
          std::string desc = std::string("");

            std::stringstream ss(line);
            int lineElementIndex = 0;
            std::string numPointsStr;
            getline(ss, numPointsStr, ',');
            vtkDebugMacro("component = " << numPointsStr.c_str());
            numPoints = atof(numPointsStr.c_str());
            markupsNode->AddMarkupWithNPoints(numPoints);
            int pointsLineElementLastIndex = numPoints*3;
            int currentPoint = 0;
            vtkDebugMacro("pointsLineElementLastIndex = " << pointsLineElementLastIndex);
            while (ss.good() && lineElementIndex < pointsLineElementLastIndex)
              {
              std::string component;
              getline(ss, component, ',');
              vtkDebugMacro("component = " << component.c_str() );
              int pointIndex = lineElementIndex % 3;
              vtkDebugMacro("lineElementIndex = " << lineElementIndex << ", pointIndex = " << pointIndex);
              switch (pointIndex)
                {
                case 0:
                  vtkDebugMacro("x = " << component.c_str());
                  x = atof(component.c_str());
                  break;
                case 1:
                  vtkDebugMacro("y = " << component.c_str());
                  y = atof(component.c_str());
                  break;
                case 2:
                  vtkDebugMacro("z = " << component.c_str() << ", setting markup " << thisMarkupNumber << " point " << currentPoint);
                  z = atof(component.c_str());
                  // now set this point
                  markupsNode->SetMarkupPoint(thisMarkupNumber,currentPoint,x,y,z);
                  currentPoint++;
                  break;
                }
              lineElementIndex++;
              }
            // parse the end of the line
            std::string component;
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
            // ID
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
            // desc
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
          vtkDebugMacro("got vis = " << vis << ", sel = " << sel
                        << ", associatedNodeID = " << associatedNodeID.c_str()
                        << ", label = '" << label.c_str() << "', id = "
                        << id << ", markup number is now " << thisMarkupNumber);
          } // point line
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
int vtkMRMLMarkupsStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
    {
    vtkErrorMacro("vtkMRMLMarkupsStorageNode: File name not specified");
    return 0;
    }

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

  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = NULL;
  vtkMRMLDisplayNode *mrmlNode = markupsNode->GetDisplayNode();
  if (mrmlNode)
    {
    displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(mrmlNode);
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
  of << "# Markups file " << (this->GetFileName() != NULL ? this->GetFileName() : "null") << endl;
  of << "# name = " << markupsNode->GetName() << endl;
  of << "# numPoints = " << numberOfMarkups << endl;
  of << "# locked = " << markupsNode->GetLocked() << endl;
  if (displayNode)
    {
    of << "# visibility = " << displayNode->GetVisibility() << endl;
    of << "# symbolScale = " << displayNode->GetGlyphScale() << endl;
    of << "# symbolType = " << displayNode->GetGlyphTypeAsString() << endl;
    of << "# textScale = " << displayNode->GetTextScale() << endl;
    double *colour = displayNode->GetColor();
    of << "# color = " << colour[0] << "," << colour[1] << "," << colour[2] << endl;
    colour = displayNode->GetSelectedColor();
    of << "# selectedColor = " << colour[0] << "," << colour[1] << "," << colour[2] << endl;
    of << "# opacity = " << displayNode->GetOpacity() << endl;
    of << "# ambient = " << displayNode->GetAmbient() << endl;
    of << "# diffuse = " << displayNode->GetDiffuse() << endl;
    of << "# specular = " << displayNode->GetSpecular() << endl;
    of << "# power = " << displayNode->GetPower() << endl;
    }
  else
    {
    vtkWarningMacro("WriteDataInternal: no display node, only recording point information");
    }
  // label the columns, there can be any number of points associated with a
  // markup, so start by saying how many there are, for a fiducial:
  // 1,x,y,z,vis,sel,lock,label,id,desc,associatedNodeID
  // for a ruler:
  // 2,x,y,z,x,y,z,vis,sel,lock,label,id,desc,associatedNodeID
  // associatedNodeID and description can be empty strings
  // 1,x,y,z,vis,sel,lock,label,id,,
  // label can have spaces, everything up to next comma is used, no quotes
  // necessary, same with the description
  of << "# columns = numPoints,([x,y,z]*numPoints),vis,sel,lock,label,id,desc,associatedNodeID" << endl;
  for (int i = 0; i < numberOfMarkups; i++)
    {
    int numberOfPoints = markupsNode->GetNumberOfPointsInNthMarkup(i);
    of << numberOfPoints;
    for (int p = 0; p < numberOfPoints; p++)
      {
      double xyz[3];
      markupsNode->GetMarkupPoint(i,p,xyz);
      of << "," << xyz[0] << "," << xyz[1] << "," << xyz[2];
      }
    bool vis = markupsNode->GetNthMarkupVisibility(i);
    bool sel = markupsNode->GetNthMarkupSelected(i);
    bool lock = markupsNode->GetNthMarkupLocked(i);

    std::string label = markupsNode->GetNthMarkupLabel(i);
    std::string id = markupsNode->GetNthMarkupID(i);
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
    
    of << "," << vis << "," << sel << "," << lock;
    of << "," << label;
    of << "," << id;
    of << "," << desc;
    of << "," << associatedNodeID;

    of << endl;
    }

  of.close();

  return 1;
  
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Markups CSV (.mcsv)");
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Markups CSV (.mcsv)");
}

//----------------------------------------------------------------------------
const char* vtkMRMLMarkupsStorageNode::GetDefaultWriteFileExtension()
{
  return "mcsv";
}
