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

#include "vtkMRMLMarkupsStorageNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLScene.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

#include <sstream>


//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsStorageNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsStorageNode::vtkMRMLMarkupsStorageNode()
{
  this->CoordinateSystem = vtkMRMLMarkupsStorageNode::RAS;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsStorageNode::~vtkMRMLMarkupsStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "coordinateSystem"))
      {
      this->SetCoordinateSystem(atoi(attValue));
      }
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);

  os << "CoordinateSystem = " << this->GetCoordinateSystemAsString().c_str() << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkIndent indent(nIndent);

  of << indent << " coordinateSystem=\"" << this->CoordinateSystem << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);

  vtkMRMLMarkupsStorageNode *node = (vtkMRMLMarkupsStorageNode *) anode;
  if (!node)
    {
    return;
    }

  this->SetCoordinateSystem(node->GetCoordinateSystem());
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLMarkupsNode");
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkWarningMacro("Subclasses must implement ReadDataInternal!");
  return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLMarkupsStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkWarningMacro("Subclasses must implement WriteDataInternal!");
  return 0;

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

//----------------------------------------------------------------------------
std::string vtkMRMLMarkupsStorageNode::GetCoordinateSystemAsString()
{
  std::string coordString;
  if (this->CoordinateSystem == vtkMRMLMarkupsStorageNode::RAS)
    {
    coordString = std::string("RAS");
    }
  else if (this->CoordinateSystem == vtkMRMLMarkupsStorageNode::LPS)
    {
    coordString = std::string("LPS");
    }
  else if (this->CoordinateSystem == vtkMRMLMarkupsStorageNode::IJK)
    {
    coordString = std::string("IJK");
    }
  return coordString;
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::UseRASOn()
{
  this->SetCoordinateSystem(vtkMRMLMarkupsStorageNode::RAS);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::GetUseRAS()
{
  if (this->GetCoordinateSystem() == vtkMRMLMarkupsStorageNode::RAS)
    {
    return true;
    }
  else
    {
    return false;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::UseLPSOn()
{
  this->SetCoordinateSystem(vtkMRMLMarkupsStorageNode::LPS);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::GetUseLPS()
{
  if (this->GetCoordinateSystem() == vtkMRMLMarkupsStorageNode::LPS)
    {
    return true;
    }
  else
    {
    return false;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::UseIJKOn()
{
  this->SetCoordinateSystem(vtkMRMLMarkupsStorageNode::IJK);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::GetUseIJK()
{
  if (this->GetCoordinateSystem() == vtkMRMLMarkupsStorageNode::IJK)
    {
    return true;
    }
  else
    {
    return false;
    }
}
