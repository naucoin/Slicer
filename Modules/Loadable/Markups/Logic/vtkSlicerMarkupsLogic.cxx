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

// Markups includes
#include "vtkSlicerMarkupsLogic.h"

// Markups MRML includes
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsStorageNode.h"

// MRML includes
#include "vtkMRMLSelectionNode.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerMarkupsLogic);

//----------------------------------------------------------------------------
vtkSlicerMarkupsLogic::vtkSlicerMarkupsLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerMarkupsLogic::~vtkSlicerMarkupsLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ObserveMRMLScene()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  // add known markup types to the selection node
  vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton");
  if (mrmlNode)
    {
    vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(mrmlNode);
    if (selectionNode)
      {
      selectionNode->AddNewMarkupsIDToList("vtkMRMLMarkupsFiducialNode", ":/Icons/MarkupsMouseModePlace.png");
      }
    }
 this->Superclass::ObserveMRMLScene();
}

//-----------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);

  // Nodes
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::New();
  this->GetMRMLScene()->RegisterNodeClass(markupsNode);
  markupsNode->Delete();
  
  vtkMRMLMarkupsFiducialNode* fidNode = vtkMRMLMarkupsFiducialNode::New();
  this->GetMRMLScene()->RegisterNodeClass(fidNode);
  fidNode->Delete();
  
  // Display nodes
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = vtkMRMLMarkupsDisplayNode::New();
  this->GetMRMLScene()->RegisterNodeClass(markupsDisplayNode);
  markupsDisplayNode->Delete();

  // Storage Nodes
  vtkMRMLMarkupsStorageNode* markupsStorageNode = vtkMRMLMarkupsStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass(markupsStorageNode);
  markupsStorageNode->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
std::string vtkSlicerMarkupsLogic::AddNewMarkupsNode()
{
  std::string id;
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("AddNewMarkupsNode: no scene to add a markups node to!");
    }
  else
    {
    // create and add the node
    vtkMRMLMarkupsNode *mnode = vtkMRMLMarkupsNode::New();
    this->GetMRMLScene()->AddNode(mnode);
    // create and add the display node
    vtkMRMLMarkupsDisplayNode *dnode = vtkMRMLMarkupsDisplayNode::New();
    this->GetMRMLScene()->AddNode(dnode);

    // observe the display node
    mnode->DisableModifiedEventOn();
    mnode->AddAndObserveDisplayNodeID(dnode->GetID());
    mnode->DisableModifiedEventOff();

    // get the node id to return
    id = std::string(mnode->GetID());

    // clean up
    dnode->Delete();
    mnode->Delete();
    }
  return id;
}
