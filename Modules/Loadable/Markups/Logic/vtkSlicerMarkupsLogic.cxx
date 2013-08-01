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
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsStorageNode.h"

// Annotation MRML includes
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"

// MRML includes
#include "vtkMRMLHierarchyNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkStringArray.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerMarkupsLogic);

//----------------------------------------------------------------------------
// call back to be triggered when the default display node is changed, so that
// changes to it can be linked to a modified event on the logic
// \ingroup 
class vtkSlicerMarkupsLogicCallback : public vtkCommand
{
public:
  static vtkSlicerMarkupsLogicCallback *New()
  { return new vtkSlicerMarkupsLogicCallback; }

  vtkSlicerMarkupsLogicCallback(){}

  virtual void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void*)
  {
    if (event == vtkCommand::ModifiedEvent)
      {
      if (!this->markupsLogic)
        {
        return;
        }
      // trigger a modified event on the logic so that settings panel
      // observers can update
      this->markupsLogic->InvokeEvent(vtkCommand::ModifiedEvent);
      }
  }
  void SetLogic(vtkSlicerMarkupsLogic *logic)
  {
    this->markupsLogic = logic;
  }
  vtkSlicerMarkupsLogic * markupsLogic;
};

//----------------------------------------------------------------------------
vtkSlicerMarkupsLogic::vtkSlicerMarkupsLogic()
{
  this->DefaultMarkupsDisplayNode = vtkMRMLMarkupsDisplayNode::New();
  // link an observation of the modified event on the display node to trigger
  // a modified event on the logic so any settings panel can get updated
  // first, create the callback
  vtkSlicerMarkupsLogicCallback *myCallback = vtkSlicerMarkupsLogicCallback::New();
  myCallback->SetLogic(this);
  this->DefaultMarkupsDisplayNode->AddObserver(vtkCommand::ModifiedEvent, myCallback);
  myCallback->Delete();
}

//----------------------------------------------------------------------------
vtkSlicerMarkupsLogic::~vtkSlicerMarkupsLogic()
{
  this->DefaultMarkupsDisplayNode->RemoveObserver(vtkCommand::ModifiedEvent);
  this->DefaultMarkupsDisplayNode->Delete();
  this->DefaultMarkupsDisplayNode = NULL;
}

//----------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ProcessMRMLNodesEvents(vtkObject *caller,
                                                   unsigned long event,
                                                   void *vtkNotUsed(callData))
{
  vtkDebugMacro("ProcessMRMLNodesEvents: Event " << event);

//  vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*> (callData);

  vtkMRMLMarkupsDisplayNode *markupsDisplayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(caller);
  if (markupsDisplayNode)
    {
    if (event == vtkMRMLMarkupsDisplayNode::ResetToDefaultsEvent)
      {
      vtkDebugMacro("ProcessMRMLNodesEvents: calling SetDisplayNodeToDefaults");
      this->SetDisplayNodeToDefaults(markupsDisplayNode);
      }
    }
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
  vtkMRMLSelectionNode *selectionNode = NULL;
  vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(this->GetSelectionNodeID().c_str());
  if (mrmlNode)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(mrmlNode);
    }
  if (selectionNode)
    {
    // got into batch process mode so that an update on the mouse mode tool
    // bar is triggered when leave it
    this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLMarkupsFiducialNode", ":/Icons/MarkupsMouseModePlace.png", "Fiducial");

    // trigger an upate on the mouse mode toolbar
    this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
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

  vtkMRMLMarkupsFiducialStorageNode* markupsFiducialStorageNode = vtkMRMLMarkupsFiducialStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass(markupsFiducialStorageNode);
  markupsFiducialStorageNode->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node)
    {
    return;
    }
  if (node->IsA("vtkMRMLMarkupsDisplayNode"))
    {
    vtkDebugMacro("OnMRMLSceneNodeAdded: Have a markups display node");
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkMRMLMarkupsDisplayNode::ResetToDefaultsEvent);
    vtkUnObserveMRMLNodeMacro(node);
    vtkObserveMRMLNodeEventsMacro(node, events.GetPointer());
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  // remove observer
  if (!node)
    {
    return;
    }
  if (node->IsA("vtkMRMLMarkupsDisplayNode"))
    {
    vtkDebugMacro("OnMRMLSceneNodeRemoved: Have a markups display node");
    vtkUnObserveMRMLNodeMacro(node);
    }
}

//---------------------------------------------------------------------------
std::string vtkSlicerMarkupsLogic::GetSelectionNodeID()
{
  std::string selectionNodeID = std::string("");

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("GetSelectionNodeID: no scene defined!");
    return selectionNodeID;
    }

  // try the application logic first
  vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
  if (mrmlAppLogic)
    {
    vtkMRMLSelectionNode *selectionNode = mrmlAppLogic->GetSelectionNode();
    if (selectionNode)
      {
      char *id = selectionNode->GetID();
      if (id)
        {
        selectionNodeID = std::string(id);
        }
      }
    }
  else
    {
    // try a default string
    selectionNodeID = std::string("vtkMRMLSelectionNodeSingleton");
    // check if it's in the scene
    if (this->GetMRMLScene()->GetNodeByID(selectionNodeID.c_str()) == NULL)
      {
      vtkErrorMacro("GetSelectionNodeID: no selection node in scene with id " << selectionNodeID);
      // reset it
      selectionNodeID = std::string("");
      }
    }
  return selectionNodeID;
}

//---------------------------------------------------------------------------
std::string vtkSlicerMarkupsLogic::GetActiveListID()
{
  std::string listID = std::string("");

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("GetActiveListID: no scene defined!");
    return listID;
    }

  // get the selection node
  vtkMRMLSelectionNode *selectionNode = NULL;;
  std::string selectionNodeID = this->GetSelectionNodeID();
  vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(selectionNodeID.c_str());
  if (!node)
    {
    vtkErrorMacro("GetActiveListID: no selection node to govern active lists.");
    return listID;
    }
  selectionNode = vtkMRMLSelectionNode::SafeDownCast(node);

  if (!selectionNode)
    {
    vtkErrorMacro("GetActiveListID: unable to get the selection node that governs active lists.");
    return listID;
    }

  char *activePlaceNodeID = selectionNode->GetActivePlaceNodeID();
  // is there no active fiducial list?
  if (activePlaceNodeID == NULL)
    {
    vtkDebugMacro("GetListID: no active place node");
    return listID;
    }

  listID = std::string(activePlaceNodeID);
  return listID;
}

//---------------------------------------------------------------------------
std::string vtkSlicerMarkupsLogic::AddNewDisplayNodeForMarkupsNode(vtkMRMLNode *mrmlNode)
{
  std::string id;
  if (!mrmlNode || !mrmlNode->GetScene())
    {
    vtkErrorMacro("AddNewDisplayNodeForMarkupsNode: unable to add a markups display node!");
    }
  else
    {
    // create the display node
    vtkMRMLMarkupsDisplayNode *displayNode = vtkMRMLMarkupsDisplayNode::New();
    // set it from the defaults
    this->SetDisplayNodeToDefaults(displayNode);
    vtkDebugMacro("AddNewDisplayNodeForMarkupsNode: set display node to defaults");

    // add it to the scene
    //mrmlNode->GetScene()->AddNode(displayNode);
    vtkMRMLNode *n = mrmlNode->GetScene()->InsertBeforeNode(mrmlNode, displayNode);
    if (!n)
      {
      vtkErrorMacro("AddNewDisplayNodeForMarkupsNode: error on insert before node");
      return id;
      }

    // get the node id to return
    id = std::string(displayNode->GetID());

    // cast to markups node
    vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    if (markupsNode)
      {
      // observe the display node
      markupsNode->DisableModifiedEventOn();
      markupsNode->AddAndObserveDisplayNodeID(id.c_str());
      markupsNode->DisableModifiedEventOff();
      }

    // clean up
    displayNode->Delete();
    }
  return id;
}

//---------------------------------------------------------------------------
std::string vtkSlicerMarkupsLogic::AddNewFiducialNode(const char *name)
{
  std::string id;
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("AddNewMarkupsNode: no scene to add a markups node to!");
    }
  else
    {
    // create and add the node
    vtkMRMLMarkupsFiducialNode *mnode = vtkMRMLMarkupsFiducialNode::New();
    this->GetMRMLScene()->AddNode(mnode);

    // add a display node
    std::string displayID = this->AddNewDisplayNodeForMarkupsNode(mnode);

    if (displayID.compare("") != 0)
      {
      // get the node id to return
      id = std::string(mnode->GetID());
      if (name != NULL)
        {
        mnode->SetName(name);
        }
      // make it active so mouse mode tool bar clicks will add new fids to
      // this list
      vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(this->GetSelectionNodeID());
      vtkMRMLSelectionNode *selectionNode = NULL;
      if (node)
        {
        selectionNode = vtkMRMLSelectionNode::SafeDownCast(node);
        }
      if (selectionNode)
        {
        // call the set reference to make sure the event is invoked
        selectionNode->SetReferenceActivePlaceNodeClassName(mnode->GetClassName());
        selectionNode->SetActivePlaceNodeID(mnode->GetID());
        }
      }
    // clean up
    mnode->Delete();
    }
  return id;
}

//---------------------------------------------------------------------------
int vtkSlicerMarkupsLogic::AddFiducial(double r, double a, double s)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("AddFiducial: no scene defined!");
    return -1;
    }

  // get the active list id
  std::string listID = this->GetActiveListID();

  // is there no active fiducial list?
  if (listID.size() == 0)
    {
    vtkDebugMacro("AddFiducial: no list is active, adding one first!");
    std::string newListID = this->AddNewFiducialNode();
    if (newListID.size() == 0)
      {
      vtkErrorMacro("AddFiducial: failed to add a new fiducial list to the scene.");
      return -1;
      }
    // try to get the id again
    listID = this->GetActiveListID();
    if (listID.size() == 0)
      {
      vtkErrorMacro("AddFiducial: failed to create a new list to add to!");
      return -1;
      }
    }

  // get the active list
  vtkMRMLNode *listNode = this->GetMRMLScene()->GetNodeByID(listID.c_str());
  if (!listNode)
    {
    vtkErrorMacro("AddFiducial: failed to get the active list with id " << listID);
    return -1;
    }
  vtkMRMLMarkupsFiducialNode *fiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(listNode);
  if (!fiducialNode)
    {
    vtkErrorMacro("AddFiducial: active list is not a fiducial list: " << listNode->GetClassName());
    return -1;
    }
  vtkDebugMacro("AddFiducial: adding a fiducial to the list " << listID);
  // add the point to the active fiducial list
  return fiducialNode->AddFiducial(r,a,s);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::JumpSlicesToLocation(double x, double y, double z, bool centered)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("JumpSlicesToLocation: No scene defined");
    return;
    }

  // save the whole state as iterating over all slice nodes
  this->GetMRMLScene()->SaveStateForUndo();

  // jump all the slice nodes in the scene
  int numSliceNodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  for (int n = 0; n < numSliceNodes; ++n)
    {
    vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNthNodeByClass(n,"vtkMRMLSliceNode");
    if (!mrmlNode)
      {
      vtkErrorMacro("JumpSlicesToLocation: could not get slice node " << n << " from scene");
      return;
      }
    vtkMRMLSliceNode *sliceNode = vtkMRMLSliceNode::SafeDownCast(mrmlNode);
    if (sliceNode)
      {
      if (centered)
        {
        sliceNode->JumpSliceByCentering(x,y,z);
        }
      else
        {
        sliceNode->JumpSliceByOffsetting(x,y,z);
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::JumpSlicesToNthPointInMarkup(const char *id, int n, bool centered)
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
    this->JumpSlicesToLocation(point[0], point[1], point[2], centered);
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
  vtkSmartPointer<vtkMRMLMarkupsFiducialStorageNode> storageNode = vtkSmartPointer<vtkMRMLMarkupsFiducialStorageNode>::New();
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

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetAllMarkupsVisibility(vtkMRMLMarkupsNode *node, bool flag)
{
  if (!node)
    {
    vtkDebugMacro("SetAllMarkupsVisibility: No list");
    return;
    }
  int numMarkups = node->GetNumberOfMarkups();
  for (int i = 0; i < numMarkups; i++)
    {
    node->SetNthMarkupVisibility(i, flag);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetAllMarkupsLocked(vtkMRMLMarkupsNode *node, bool flag)
{
  if (!node)
    {
    vtkDebugMacro("SetAllMarkupsLocked: No list");
    return;
    }
  int numMarkups = node->GetNumberOfMarkups();
  for (int i = 0; i < numMarkups; i++)
    {
    node->SetNthMarkupLocked(i, flag);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetAllMarkupsSelected(vtkMRMLMarkupsNode *node, bool flag)
{
  if (!node)
    {
    vtkDebugMacro("SetAllMarkupsSelected: No list");
    return;
    }
  int numMarkups = node->GetNumberOfMarkups();
  for (int i = 0; i < numMarkups; i++)
    {
    node->SetNthMarkupSelected(i, flag);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeGlyphType(int glyphType)
{
  this->DefaultMarkupsDisplayNode->SetGlyphType(glyphType);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeGlyphTypeFromString(const char *glyphType)
{
  if (glyphType == NULL)
    {
    vtkErrorMacro("SetDefaultMarkupsDisplayNodeGlyphTypeFromString: null glyph type string!");
    return;
    }
  this->DefaultMarkupsDisplayNode->SetGlyphTypeFromString(glyphType);
}

//---------------------------------------------------------------------------
int vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeGlyphType()
{
  return this->DefaultMarkupsDisplayNode->GetGlyphType();
}

//---------------------------------------------------------------------------
std::string vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeGlyphTypeAsString()
{
  std::string glyphString;
  const char *glyphType = this->DefaultMarkupsDisplayNode->GetGlyphTypeAsString();
  if (glyphType)
    {
    glyphString = std::string(glyphType);
    }
  return glyphString;
}

//---------------------------------------------------------------------------
double vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeGlyphScale()
{
  return this->DefaultMarkupsDisplayNode->GetGlyphScale();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeGlyphScale(double scale)
{
  this->DefaultMarkupsDisplayNode->SetGlyphScale(scale);
}

//---------------------------------------------------------------------------
double vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeTextScale()
{
  return this->DefaultMarkupsDisplayNode->GetTextScale();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeTextScale(double scale)
{
  this->DefaultMarkupsDisplayNode->SetTextScale(scale);
}

//---------------------------------------------------------------------------
double vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeOpacity()
{
  return this->DefaultMarkupsDisplayNode->GetOpacity();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeOpacity(double opacity)
{
  this->DefaultMarkupsDisplayNode->SetOpacity(opacity);
}

//---------------------------------------------------------------------------
double *vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeColor()
{
  return this->DefaultMarkupsDisplayNode->GetColor();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeColor(double *color)
{
  if (!color)
    {
    return;
    }
  this->DefaultMarkupsDisplayNode->SetColor(color);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeColor(double r, double g, double b)
{
  this->DefaultMarkupsDisplayNode->SetColor(r,g,b);
}

//---------------------------------------------------------------------------
double *vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeSelectedColor()
{
  return this->DefaultMarkupsDisplayNode->GetSelectedColor();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeSelectedColor(double *color)
{
  if (!color)
    {
    return;
    }
  this->DefaultMarkupsDisplayNode->SetSelectedColor(color);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeSelectedColor(double r, double g, double b)
{
  this->DefaultMarkupsDisplayNode->SetSelectedColor(r,g,b);
}

//---------------------------------------------------------------------------
int vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeSliceProjection()
{
  return this->DefaultMarkupsDisplayNode->GetSliceProjection();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeSliceProjection(int projection)
{
  this->DefaultMarkupsDisplayNode->SetSliceProjection(projection);
}

//---------------------------------------------------------------------------
double *vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeSliceProjectionColor()
{
  return this->DefaultMarkupsDisplayNode->GetSliceProjectionColor();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeSliceProjectionColor(double *color)
{
  if (!color)
    {
    return;
    }
  this->DefaultMarkupsDisplayNode->SetSliceProjectionColor(color);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeSliceProjectionColor(double r, double g, double b)
{
  this->DefaultMarkupsDisplayNode->SetSliceProjectionColor(r,g,b);
}

//---------------------------------------------------------------------------
double vtkSlicerMarkupsLogic::GetDefaultMarkupsDisplayNodeSliceProjectionOpacity()
{
  return this->DefaultMarkupsDisplayNode->GetSliceProjectionOpacity();
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDefaultMarkupsDisplayNodeSliceProjectionOpacity(double opacity)
{
  this->DefaultMarkupsDisplayNode->SetSliceProjectionOpacity(opacity);
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::SetDisplayNodeToDefaults(vtkMRMLMarkupsDisplayNode *displayNode)
{
  if (!displayNode)
    {
    return;
    }

  displayNode->SetSelectedColor(this->GetDefaultMarkupsDisplayNodeSelectedColor());
  displayNode->SetColor(this->GetDefaultMarkupsDisplayNodeColor());
  displayNode->SetOpacity(this->GetDefaultMarkupsDisplayNodeOpacity());
  displayNode->SetGlyphType(this->GetDefaultMarkupsDisplayNodeGlyphType());
  displayNode->SetGlyphScale(this->GetDefaultMarkupsDisplayNodeGlyphScale());
  displayNode->SetTextScale(this->GetDefaultMarkupsDisplayNodeTextScale());
  displayNode->SetSliceProjection(this->GetDefaultMarkupsDisplayNodeSliceProjection());
  displayNode->SetSliceProjectionColor(this->GetDefaultMarkupsDisplayNodeSliceProjectionColor());
  displayNode->SetSliceProjectionOpacity(this->GetDefaultMarkupsDisplayNodeSliceProjectionOpacity());
}

//---------------------------------------------------------------------------
bool vtkSlicerMarkupsLogic::MoveNthMarkupToNewListAtIndex(int n, vtkMRMLMarkupsNode *markupsNode,
                                                          vtkMRMLMarkupsNode *newMarkupsNode, int newIndex)
{
  if (!markupsNode || !newMarkupsNode)
    {
    vtkErrorMacro("MoveNthMarkupToNewListAtIndex: at least one of the markup list nodes are null!");
    return false;
    }

  if (n < 0 || n >= markupsNode->GetNumberOfMarkups())
    {
    vtkErrorMacro("MoveNthMarkupToNewListAtIndex: source index n " << n
                  << " is not in list of size " << markupsNode->GetNumberOfMarkups());
    return false;
    }

  // get the markup
  Markup newMarkup;
  markupsNode->CopyMarkup(markupsNode->GetNthMarkup(n), &newMarkup);

  // add it to the destination list
  bool insertVal = newMarkupsNode->InsertMarkup(newMarkup, newIndex);
  if (!insertVal)
    {
    vtkErrorMacro("MoveNthMarkupToNewListAtIndex: failed to insert new markup at " << newIndex << ", markup is still on source list.");
    return false;
    }

  // remove it from the source list
  markupsNode->RemoveMarkup(n);

  return true;
}

//---------------------------------------------------------------------------
void vtkSlicerMarkupsLogic::ConvertAnnotationFiducialsToMarkups()
{
  if (!this->GetMRMLScene())
    {
    return;
    }

  vtkCollection *col = this->GetMRMLScene()->GetNodesByClass("vtkMRMLAnnotationFiducialNode");
  int numberOfAnnotationFids = col->GetNumberOfItems();
  if (numberOfAnnotationFids == 0)
    {
    col->RemoveAllItems();
    col->Delete();
    return;
    }

  // go through all the annotation fiducials and collect their hierarchies
  vtkStringArray *hierarchyNodeIDs = vtkStringArray::New();

  for (unsigned int n = 0; n < numberOfAnnotationFids; ++n)
    {
    vtkMRMLNode *mrmlNode = vtkMRMLNode::SafeDownCast(col->GetItemAsObject(n));
    if (!mrmlNode)
      {
      continue;
      }
    vtkMRMLHierarchyNode *oneToOneHierarchyNode =
      vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(mrmlNode->GetScene(),
                                                       mrmlNode->GetID());
    if (!oneToOneHierarchyNode)
      {
      continue;
      }
    char * parentNodeID = oneToOneHierarchyNode->GetParentNodeID();
    char * hierarchyName = oneToOneHierarchyNode->GetParentNode()->GetName();
    // is it not already in the list of annotation hierarchy node ids?
    vtkIdType id = hierarchyNodeIDs->LookupValue(parentNodeID);
    if (id == -1)
      {
      vtkDebugMacro("Found unique annotation hierarchy node, id = " << parentNodeID << ", name = " << hierarchyName);
      hierarchyNodeIDs->InsertNextValue(parentNodeID);
      }
    }

  col->RemoveAllItems();
  col->Delete();

  if (hierarchyNodeIDs->GetNumberOfValues() == 0)
    {
    hierarchyNodeIDs->Delete();
    return;
    }
  else
    {
    vtkDebugMacro("Converting " << hierarchyNodeIDs->GetNumberOfValues()
                  << " annotation hierarchies to markup lists");
    }
  // now iterate over the hierarchies that have fiducials in them and convert
  // them to markups lists
  for (int i = 0; i < hierarchyNodeIDs->GetNumberOfValues(); ++i)
    {
    vtkMRMLNode *mrmlNode = NULL;
    vtkMRMLHierarchyNode *hierarchyNode = NULL;
    mrmlNode = this->GetMRMLScene()->GetNodeByID(hierarchyNodeIDs->GetValue(i));
    if (!mrmlNode)
      {
      continue;
      }
    hierarchyNode = vtkMRMLHierarchyNode::SafeDownCast(mrmlNode);
    if (!hierarchyNode)
      {
      continue;
      }

    // create a markups fiducial list with this name
    std::string markupsListID = this->AddNewFiducialNode(hierarchyNode->GetName());
    vtkMRMLMarkupsFiducialNode *markupsNode = NULL;
    mrmlNode = this->GetMRMLScene()->GetNodeByID(markupsListID.c_str());
    if (!mrmlNode)
      {
      continue;
      }
    markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(mrmlNode);
    if (!markupsNode)
      {
      continue;
      }
    // now get the fiducials in this annotation hierarchy
    vtkCollection *children = vtkCollection::New();
    hierarchyNode->GetAssociatedChildrendNodes(children, "vtkMRMLAnnotationFiducialNode");
    vtkDebugMacro("Found " << children->GetNumberOfItems() << " annot fids in this hierarchy");
    for (unsigned int c = 0; c < children->GetNumberOfItems(); ++c)
      {
      vtkMRMLAnnotationFiducialNode *annotNode;
      annotNode = vtkMRMLAnnotationFiducialNode::SafeDownCast(children->GetItemAsObject(c));
      if (!annotNode)
        {
        continue;
        }
      double coord[3];
      annotNode->GetFiducialCoordinates(coord);
      int fidIndex = markupsNode->AddFiducial(coord[0], coord[1], coord[2]);
      vtkDebugMacro("Added a fiducial at index " << fidIndex);
      markupsNode->SetNthMarkupLabel(fidIndex, std::string(annotNode->GetName()));
      char *desc = annotNode->GetDescription();
      if (desc)
        {
        markupsNode->SetNthMarkupDescription(fidIndex,std::string(desc));
        }
      markupsNode->SetNthMarkupSelected(fidIndex, annotNode->GetSelected());
      markupsNode->SetNthMarkupVisibility(fidIndex,
                                          annotNode->GetDisplayVisibility());
      markupsNode->SetNthMarkupLocked(fidIndex, annotNode->GetLocked());
      const char *assocNodeID = annotNode->GetAttribute("AssociatedNodeID");
      if (assocNodeID)
        {
        markupsNode->SetNthMarkupAssociatedNodeID(fidIndex, assocNodeID);
        }

      // get the display nodes
      vtkMRMLAnnotationPointDisplayNode *pointDisplayNode = NULL;
      vtkMRMLAnnotationTextDisplayNode *textDisplayNode = NULL;
      pointDisplayNode = annotNode->GetAnnotationPointDisplayNode();
      textDisplayNode = annotNode->GetAnnotationTextDisplayNode();

      if (c == 0)
        {
        // use the first display node to get display settings
        vtkMRMLMarkupsDisplayNode *markupDisplayNode = markupsNode->GetMarkupsDisplayNode();
        if (!markupDisplayNode || !pointDisplayNode || !textDisplayNode)
          {
          continue;
          }
        markupDisplayNode->SetColor(pointDisplayNode->GetColor());
        markupDisplayNode->SetSelectedColor(pointDisplayNode->GetSelectedColor());
        markupDisplayNode->SetGlyphScale(pointDisplayNode->GetGlyphScale());
        markupDisplayNode->SetTextScale(textDisplayNode->GetTextScale());
        markupDisplayNode->SetOpacity(pointDisplayNode->GetOpacity());
        markupDisplayNode->SetPower(pointDisplayNode->GetPower());
        markupDisplayNode->SetAmbient(pointDisplayNode->GetAmbient());
        markupDisplayNode->SetDiffuse(pointDisplayNode->GetDiffuse());
        markupDisplayNode->SetSpecular(pointDisplayNode->GetSpecular());
        markupDisplayNode->SetSliceProjection(pointDisplayNode->GetSliceProjection());
        markupDisplayNode->SetSliceProjectionColor(pointDisplayNode->GetProjectedColor());
        markupDisplayNode->SetSliceProjectionOpacity(pointDisplayNode->GetProjectedOpacity());
        }
      //
      // clean up the no longer needed annotation nodes
      //
      // remove the 1:1 hierarchy node
      vtkMRMLHierarchyNode *oneToOneHierarchyNode =
        vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(annotNode->GetScene(),
                                                         annotNode->GetID());
      if (oneToOneHierarchyNode)
        {
        this->GetMRMLScene()->RemoveNode(oneToOneHierarchyNode);
        }

      // remove the display nodes
      if (pointDisplayNode)
        {
        this->GetMRMLScene()->RemoveNode(pointDisplayNode);
        }
      if (textDisplayNode)
        {
        this->GetMRMLScene()->RemoveNode(textDisplayNode);
        }
      // is there a storage node?
      vtkMRMLStorageNode *storageNode = annotNode->GetStorageNode();
      if (storageNode)
        {
        this->GetMRMLScene()->RemoveNode(storageNode);
        }
      // now remove the annotation node
      this->GetMRMLScene()->RemoveNode(annotNode);
      }
    children->RemoveAllItems();
    children->Delete();
    }
  hierarchyNodeIDs->Delete();
}
