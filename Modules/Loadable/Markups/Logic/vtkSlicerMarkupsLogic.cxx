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
#include "vtkMRMLSliceNode.h"

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
      // got into batch process mode so that an update on the mouse mode tool
      // bar is triggered when leave it
      this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

      selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLMarkupsFiducialNode", ":/Icons/MarkupsMouseModePlace.png", "MFiducial");

      // trigger an upate on the mouse mode toolbar
      this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
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

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::JumpSlicesToLocation(double x, double y, double z)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("JumpSlicesToLocation: No scene defined");
    return;
    }

 vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNthNodeByClass(0,"vtkMRMLSliceNode");
  if (!mrmlNode)
    {
    vtkErrorMacro("JumpSlicesToLocation: could not get first slice node from scene");
    return;
    }
  vtkMRMLSliceNode *sliceNode = vtkMRMLSliceNode::SafeDownCast(mrmlNode);
  if (sliceNode)
    {
    this->GetMRMLScene()->SaveStateForUndo();
    sliceNode->JumpAllSlices(x,y,z);
    // JumpAllSlices jumps all the other slices, not self, so JumpSlice on
    // this node as well
    sliceNode->JumpSlice(x,y,z);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::JumpSlicesToNthPointInMarkup(const char *id, int n)
{
  if (!id)
    {
    return;
    }
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("JumpSlicesToLocation: No scene defined");
    return;
    }
  // get the markups node
  vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(id);
  if (mrmlNode == NULL)
    {
    return;
    }
  vtkMRMLMarkupsNode *markup = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  if (markup)
    {
    double point[4];
    // get the first point for now
    markup->GetMarkupPointWorld(n, 0, point);
    this->JumpSlicesToLocation(point[0], point[1], point[2]);
    }
}

//---------------------------------------------------------------------------
char * vtkSlicerMarkupsLogic::LoadMarkupsFiducials(const char *fileName, const char *fidsName)
{
  char *nodeID = NULL;
  std::string idList;
  if (!fileName)
    {
    vtkErrorMacro("LoadMarkupsFiducials: null file name, cannot load");
    return nodeID;
    }

  vtkDebugMacro("LoadMarkupsFiducials, file name = " << fileName << ", fidsName = " << (fidsName ? fidsName : "null"));
  
  // turn on batch processing
  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

  // make a storage node and fiducial node and set the file name
  vtkSmartPointer<vtkMRMLMarkupsStorageNode> storageNode = vtkSmartPointer<vtkMRMLMarkupsStorageNode>::New();
  storageNode->SetFileName(fileName);
  vtkSmartPointer<vtkMRMLMarkupsFiducialNode> fidNode = vtkSmartPointer<vtkMRMLMarkupsFiducialNode>::New();
  fidNode->SetName(fidsName);

  // add the nodes to the scene and set up the observation on the storage node
  this->GetMRMLScene()->AddNode(storageNode);
  this->GetMRMLScene()->AddNode(fidNode);
  fidNode->SetAndObserveStorageNodeID(storageNode->GetID());

  // read the file
  if (storageNode->ReadData(fidNode))
    {
    nodeID = fidNode->GetID();
    }

  // turn off batch processing
  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);

  return nodeID;

}
