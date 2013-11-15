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

// MarkupsModule/MRML includes
#include <vtkMRMLMarkupsDisplayNode.h>
#include <vtkMRMLMarkupsNode.h>

// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsDisplayableManager2D.h"

// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLModelDisplayableManager.h>

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkHandleRepresentation.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkPropCollection.h>
#include <vtkProperty2D.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSeedRepresentation.h>
#include <vtkSeedWidget.h>
#include <vtkWeakPointer.h>
#include <vtkWidgetRepresentation.h>

// STD includes
#include <algorithm>
#include <map>
#include <vector>

typedef void (*fp)(void);

#define NUMERIC_ZERO 0.001

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLMarkupsDisplayableManager2D);
vtkCxxRevisionMacro (vtkMRMLMarkupsDisplayableManager2D, "$Revision: 1.2 $");

//---------------------------------------------------------------------------
class vtkMRMLMarkupsDisplayableManager2D::vtkInternal
{
public:
  vtkInternal(vtkMRMLMarkupsDisplayableManager2D * external);
  ~vtkInternal();

  vtkObserverManager* GetMRMLNodesObserverManager();
  void Modified();

  // Slice
  void UpdateSliceNode();
  // Slice Composite
  vtkMRMLSliceCompositeNode* FindSliceCompositeNode();
  void SetSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode);

  vtkMRMLMarkupsDisplayableManager2D*        External;
  vtkWeakPointer<vtkMRMLSliceCompositeNode>  SliceCompositeNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLMarkupsDisplayableManager2D::vtkInternal
::vtkInternal(vtkMRMLMarkupsDisplayableManager2D * external)
{
  this->External = external;
  this->SliceCompositeNode = 0;
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsDisplayableManager2D::vtkInternal::~vtkInternal()
{
  this->SetSliceCompositeNode(0);
}

//---------------------------------------------------------------------------
vtkObserverManager* vtkMRMLMarkupsDisplayableManager2D::vtkInternal::GetMRMLNodesObserverManager()
{
  return this->External->GetMRMLNodesObserverManager();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::vtkInternal::Modified()
{
  return this->External->Modified();
}
//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::vtkInternal::UpdateSliceNode()
{
  if (!this->External)
    {
    return;
    }
  assert(!this->External->GetMRMLSliceNode() || this->External->GetMRMLSliceNode()->GetLayoutName());
  // search the scene for a matching slice composite node
  if (!this->SliceCompositeNode.GetPointer() || // the slice composite has been deleted
      !this->SliceCompositeNode->GetLayoutName() || // the slice composite points to a diff slice node
      strcmp(this->SliceCompositeNode->GetLayoutName(),
             this->External->GetMRMLSliceNode()->GetLayoutName()))
    {
    vtkMRMLSliceCompositeNode* sliceCompositeNode =
      this->FindSliceCompositeNode();
    this->SetSliceCompositeNode(sliceCompositeNode);
    }
}

//---------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* vtkMRMLMarkupsDisplayableManager2D::vtkInternal
::FindSliceCompositeNode()
{
  if (this->External->GetMRMLSliceNode() == 0 ||
      this->External->GetMRMLApplicationLogic() == 0)
    {
    return 0;
    }

  vtkMRMLSliceLogic *sliceLogic = NULL;
  vtkMRMLApplicationLogic *mrmlAppLogic = this->External->GetMRMLApplicationLogic();
  if (mrmlAppLogic)
    {
    sliceLogic = mrmlAppLogic->GetSliceLogic(this->External->GetMRMLSliceNode());
    }
  if (sliceLogic)
    {
    return sliceLogic->GetSliceCompositeNode(this->External->GetMRMLSliceNode());
    }
  // no matching slice composite node is found
  return 0;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::vtkInternal
::SetSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode)
{
  if (this->SliceCompositeNode == compositeNode)
    {
    return;
    }
  vtkSetAndObserveMRMLNodeMacro(this->SliceCompositeNode, compositeNode);
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsDisplayableManager2D::vtkMRMLMarkupsDisplayableManager2D()
{
  this->Helper = vtkMRMLMarkupsDisplayableManagerHelper::New();
  this->ClickCounter = vtkMRMLMarkupsClickCounter::New();
  this->DisableInteractorStyleEventsProcessing = 0;
  this->Updating = 0;

  this->Focus = "vtkMRMLMarkupsNode";

  // by default, multiply the display node scale by this when setting scale on elements in 2d windows
  this->ScaleFactor2D = 0.00333;

  this->LastClickWorldCoordinates[0]=0.0;
  this->LastClickWorldCoordinates[1]=0.0;
  this->LastClickWorldCoordinates[2]=0.0;
  this->LastClickWorldCoordinates[3]=1.0;

  this->Internal = new vtkInternal(this);

  this->LayoutChanging = false;
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsDisplayableManager2D::~vtkMRMLMarkupsDisplayableManager2D()
{

  this->DisableInteractorStyleEventsProcessing = 0;
  this->Updating = 0;
  this->Focus = 0;

  this->Helper->Delete();
  this->ClickCounter->Delete();

  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "DisableInteractorStyleEventsProcessing = " << this->DisableInteractorStyleEventsProcessing << std::endl;
  if (this->GetMRMLSliceNode() &&
      this->GetMRMLSliceNode()->GetID())
    {
    os << indent << "Slice node id = " << this->GetMRMLSliceNode()->GetID() << std::endl;
    }
  else
    {
    os << indent << "No slice node" << std::endl;
    }
  if (this->Focus)
    {
    os << indent << "Focus = " << this->Focus << std::endl;
    }
  os << indent << "ScaleFactor2D = " << this->ScaleFactor2D << std::endl;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::SetAndObserveNode(vtkMRMLMarkupsNode *markupsNode)
{
  if (!markupsNode)
    {
    return;
    }
  vtkNew<vtkIntArray> nodeEvents;
  nodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLMarkupsNode::PointModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLMarkupsNode::NthMarkupModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLMarkupsNode::MarkupAddedEvent);
  nodeEvents->InsertNextValue(vtkMRMLMarkupsNode::MarkupRemovedEvent);
  nodeEvents->InsertNextValue(vtkMRMLMarkupsNode::LockModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);

 if (markupsNode)// && !markupsNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent))
   {
   vtkUnObserveMRMLNodeMacro(markupsNode);
   vtkObserveMRMLNodeEventsMacro(markupsNode, nodeEvents.GetPointer());
   }
}
//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::SetAndObserveNodes()
{



  // run through all associated nodes
  vtkMRMLMarkupsDisplayableManagerHelper::MarkupsNodeListIt it;
  for(it = this->Helper->MarkupsNodeList.begin();
      it != this->Helper->MarkupsNodeList.end();
      ++it)
    {
    vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast((*it));
    this->SetAndObserveNode(markupsNode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::AddObserversToInteractionNode()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  // also observe the interaction node for changes
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (interactionNode)
    {
    vtkDebugMacro("AddObserversToInteractionNode: interactionNode found");
    vtkIntArray *interactionEvents = vtkIntArray::New();
    interactionEvents->InsertNextValue(vtkMRMLInteractionNode::InteractionModeChangedEvent);
    interactionEvents->InsertNextValue(vtkMRMLInteractionNode::InteractionModePersistenceChangedEvent);
    interactionEvents->InsertNextValue(vtkMRMLInteractionNode::EndPlacementEvent);
    vtkObserveMRMLNodeEventsMacro(interactionNode, interactionEvents);
    interactionEvents->Delete();
    }
  else { vtkDebugMacro("AddObserversToInteractionNode: No interaction node!"); }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::RemoveObserversFromInteractionNode()
{
  if (!this->GetMRMLScene())
    {
    return;
    }

  // find the interaction node
  vtkMRMLInteractionNode *interactionNode =  this->GetInteractionNode();
  if (interactionNode)
    {
    vtkUnObserveMRMLNodeMacro(interactionNode);
    }
}

//---------------------------------------------------------------------------
vtkMRMLLayoutNode *vtkMRMLMarkupsDisplayableManager2D::GetLayoutNode()
{
  if (!this->GetMRMLScene())
    {
    return NULL;
    }
  vtkMRMLLayoutNode *layoutNode = NULL;
  vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLLayoutNode");
  if (mrmlNode)
    {
    layoutNode = vtkMRMLLayoutNode::SafeDownCast(mrmlNode);
    }
  return layoutNode;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::AddObserversToLayoutNode()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  // also observe the layout node for changes
  vtkMRMLLayoutNode *layoutNode = this->GetLayoutNode();
  if (layoutNode)
    {
    vtkIntArray *layoutEvents = vtkIntArray::New();
    vtkNew<vtkFloatArray> priorities;
    float normalPriority = 0.0;
    float lowPriority = -5.0;
    layoutEvents->InsertNextValue(vtkMRMLLayoutNode::LayoutChangeStartEvent);
    priorities->InsertNextValue(normalPriority);
    layoutEvents->InsertNextValue(vtkMRMLLayoutNode::LayoutChangeEndEvent);
    priorities->InsertNextValue(lowPriority);
    vtkObserveMRMLNodeEventsPrioritiesMacro(layoutNode, layoutEvents, priorities.GetPointer());
    layoutEvents->Delete();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::RemoveObserversFromLayoutNode()
{
  if (!this->GetMRMLScene())
    {
    return;
    }

  // find the layout node
  vtkMRMLLayoutNode *layoutNode =  this->GetLayoutNode();
  if (layoutNode)
    {
    vtkUnObserveMRMLNodeMacro(layoutNode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::RequestRender()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  if (!this->GetMRMLScene()->IsBatchProcessing())
    {
    this->Superclass::RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::RemoveMRMLObservers()
{
  // run through all associated nodes
  vtkMRMLMarkupsDisplayableManagerHelper::MarkupsNodeListIt it;
  it = this->Helper->MarkupsNodeList.begin();
  while(it != this->Helper->MarkupsNodeList.end())
    {
    vtkUnObserveMRMLNodeMacro(*it);
    ++it;
    }

  if (this->GetMRMLScene())
    {
    this->Internal->SetSliceCompositeNode(0);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::UpdateFromMRML()
{
  // this gets called from RequestRender, so make sure to jump out quickly if possible
  if (this->GetMRMLScene() == NULL || this->Focus == NULL)
    {
    return;
    }

  this->Internal->UpdateSliceNode();

  if (this->LayoutChanging)
    {
    return;
    }

  // check if there are any of these nodes in the scene
  if (this->GetMRMLScene()->GetNumberOfNodesByClass(this->Focus) < 1)
    {
    return;
    }
  // loop over the nodes for which this manager provides widgets
  this->GetMRMLScene()->InitTraversal();
  vtkMRMLNode *node = this->GetMRMLScene()->GetNextNodeByClass(this->Focus);
  // turn off update from mrml requested, as we're doing it now, and create
  // widget requests a render which checks this flag before calling update
  // from mrml again
  this->SetUpdateFromMRMLRequested(0);
  while (node != NULL)
    {
    vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
    if (markupsNode)
      {
      // do we  have a widget for it?
      if (this->GetWidget(markupsNode) == NULL)
        {
        vtkDebugMacro("UpdateFromMRML: creating a widget for node " << markupsNode->GetID());
        // has a renderer been set yet?
        if (this->GetRenderer() == NULL)
          {
          vtkErrorMacro("UpdateFromMRML: no renderer!");
          return;
          }
        vtkAbstractWidget *widget = this->AddWidget(markupsNode);
        if (widget)
          {
          // update the new widget from the node
          //this->PropagateMRMLToWidget(markupsNode, widget);
          }
        else
          {
          vtkErrorMacro("UpdateFromMRML: failed to create a widget for node " << markupsNode->GetID());
          }
        }
      else
        {
        // update the widget?
        // this->PropagateMRMLToWidget(markupsNode, this->GetWidget(markupsNode));
        }
      }
    node = this->GetMRMLScene()->GetNextNodeByClass(this->Focus);
    }
  // set up observers on all the nodes
//  this->SetAndObserveNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  Superclass::SetMRMLSceneInternal(newScene);

  // after a new scene got associated, we want to make sure everything old is gone
  this->OnMRMLSceneEndClose();

  if (newScene)
    {
    this->AddObserversToInteractionNode();
    this->AddObserversToLayoutNode();
    }
  else
    {
    // there's no scene to get the interaction node from, so this won't do anything
    this->RemoveObserversFromInteractionNode();
    this->RemoveObserversFromLayoutNode();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D
::ProcessMRMLNodesEvents(vtkObject *caller,unsigned long event,void *callData)
{

  vtkMRMLMarkupsNode * markupsNode = vtkMRMLMarkupsNode::SafeDownCast(caller);
  vtkMRMLMarkupsDisplayNode *displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(caller);
  vtkMRMLInteractionNode * interactionNode = vtkMRMLInteractionNode::SafeDownCast(caller);
  vtkMRMLSliceCompositeNode * sliceCompositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(caller);
  vtkMRMLLayoutNode * layoutNode = vtkMRMLLayoutNode::SafeDownCast(caller);

  int *nPtr = NULL;
  int n = -1;
  if (callData != 0)
    {
    nPtr = reinterpret_cast<int *>(callData);
    if (nPtr)
      {
      n = *nPtr;
      }
    }
  if (markupsNode)
    {
    switch(event)
      {
      case vtkCommand::ModifiedEvent:
        this->OnMRMLMarkupsNodeModifiedEvent(markupsNode);
        break;
      case vtkMRMLMarkupsNode::PointModifiedEvent:
        this->OnMRMLMarkupsPointModifiedEvent(markupsNode, n);
        break;
      case vtkMRMLMarkupsNode::NthMarkupModifiedEvent:
        this->OnMRMLMarkupsNodeNthMarkupModifiedEvent(markupsNode, n);
        break;
      case vtkMRMLMarkupsNode::MarkupAddedEvent:
        this->OnMRMLMarkupsNodeMarkupAddedEvent(markupsNode);
        break;
      case vtkMRMLMarkupsNode::MarkupRemovedEvent:
        this->OnMRMLMarkupsNodeMarkupRemovedEvent(markupsNode);
        break;
      case vtkMRMLTransformableNode::TransformModifiedEvent:
        this->OnMRMLMarkupsNodeTransformModifiedEvent(markupsNode);
        break;
      case vtkMRMLMarkupsNode::LockModifiedEvent:
        this->OnMRMLMarkupsNodeLockModifiedEvent(markupsNode);
        break;
      }
    }
  else if (displayNode)
    {
    switch(event)
      {
      case vtkCommand::ModifiedEvent:
        this->OnMRMLMarkupsDisplayNodeModifiedEvent(displayNode);
        break;
      }
    }
  else if (interactionNode)
    {
    if (event == vtkMRMLInteractionNode::EndPlacementEvent)
      {
      // remove all seeds and reset the clickcounter
      this->ClickCounter->Reset();
      this->Helper->RemoveSeeds();
      return;
      }
    else if (event == vtkMRMLInteractionNode::InteractionModeChangedEvent)
      {
      // always update lock if the mode changed, even if this isn't the displayable manager
      // for the markups that is getting placed, but don't update locking on persistence changed event
      this->Helper->UpdateLockedAllWidgetsFromInteractionNode(interactionNode);
      }
    }
  else if (sliceCompositeNode)
    {
    // update requested will trigger an update from mrml when the
    // requested render is done
    this->SetUpdateFromMRMLRequested(1);
    this->RequestRender();
    }
  else if (layoutNode)
    {
    if (event == vtkMRMLLayoutNode::LayoutChangeStartEvent)
      {
      this->LayoutChanging = true;
      }
    else if (event == vtkMRMLLayoutNode::LayoutChangeEndEvent)
      {
      this->LayoutChanging = false;
      this->Helper->RenderAllWidgets();
      }
    }
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnMRMLSceneEndClose()
{
  vtkDebugMacro("OnMRMLSceneEndClose: remove observers?");
  // run through all nodes and remove node and widget
  this->Helper->RemoveAllWidgetsAndNodes();

  this->SetUpdateFromMRMLRequested(1);
  this->RequestRender();

}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::UpdateFromMRMLScene()
{
  this->UpdateFromMRML();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
    {
    return;
    }

  vtkDebugMacro("OnMRMLSceneNodeAddedEvent");

  // if the scene is still updating, jump out
  if (this->GetMRMLScene()->IsBatchProcessing())
    {
    return;
    }
  // if the layout node is in the midst of changing...
  if (this->LayoutChanging)
    {
    std::cout << "OnMRMLSceneNodeAddedEvent: layout is changing." << std::endl;
    }

  if (node->IsA("vtkMRMLInteractionNode"))
    {
    this->AddObserversToInteractionNode();
    return;
    }

  if (node->IsA("vtkMRMLLayoutNode"))
    {
    this->AddObserversToLayoutNode();
    return;
    }

  if (node->IsA("vtkMRMLMarkupsDisplayNode"))
    {
    // have a display node, need to observe it
    vtkObserveMRMLNodeMacro(node);
    return;
    }

  if (!node->IsA(this->Focus))
    {
    // jump out
    vtkDebugMacro("OnMRMLSceneNodeAddedEvent: Not the correct displayableManager for node " << node->GetID() << ", jumping out!")
    // also delete potential seeds
    this->ClickCounter->Reset();
    this->Helper->RemoveSeeds();
    return;
    }

  vtkMRMLMarkupsNode * markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return;
    }

  vtkDebugMacro("OnMRMLSceneNodeAddedEvent:  node " << node->GetID());

  // Node added should not be already managed
  vtkMRMLMarkupsDisplayableManagerHelper::MarkupsNodeListIt it = std::find(
      this->Helper->MarkupsNodeList.begin(),
      this->Helper->MarkupsNodeList.end(),
      markupsNode);
  if (it != this->Helper->MarkupsNodeList.end())
    {
      vtkErrorMacro("OnMRMLSceneNodeAddedEvent: This node is already associated to the displayable manager!")
      return;
    }

  // There should not be a widget for the new node
  if (this->Helper->GetWidget(markupsNode) != 0)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: A widget is already associated to this node!");
    return;
    }

  //std::cout << "OnMRMLSceneNodeAddedEvent ThreeD -> CreateWidget" << std::endl;

  // Create the Widget and add it to the list.
  vtkAbstractWidget* newWidget = this->AddWidget(markupsNode);
  if (!newWidget)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: Widget was not created!")
    return;
    }
  else
    {
    vtkDebugMacro("OnMRMLSceneNodeAddedEvent: widget was created, saved to helper Widgets map");
    }

  // tear down widget creation
  //this->OnWidgetCreated(newWidget, markupsNode);

  // Remove all placed seeds
  this->Helper->RemoveSeeds();

  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  this->Helper->UpdateLockedAllWidgetsFromInteractionNode(interactionNode);

  // and render again after seeds were removed
  this->RequestRender();

}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLSceneNodeRemovedEvent");
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return;
    }

  // Remove the widget and the MRMLnode from the internal lists.
  this->Helper->RemoveWidgetAndNode(markupsNode);

  // Refresh observers
  vtkUnObserveMRMLNodeMacro(markupsNode);

  // and render again after seeds were removed
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnMRMLMarkupsNodeModifiedEvent(vtkMRMLNode* node)
{
  //this->DebugOn();

  vtkDebugMacro("OnMRMLMarkupsNodeModifiedEvent");

  if (this->Updating)
    {
    vtkDebugMacro("OnMRMLMarkupsNodeModifiedEvent: Updating in progress.. Exit now.")
    return;
    }

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeModifiedEvent: Can not access node.")
    return;
    }

  //std::cout << "OnMRMLMarkupsNodeModifiedEvent ThreeD->PropagateMRMLToWidget" << std::endl;

  vtkAbstractWidget * widget = this->Helper->GetWidget(markupsNode);

  if (widget)
    {
    // Propagate MRML changes to widget
    this->PropagateMRMLToWidget(markupsNode, widget);
    }

  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnMRMLMarkupsDisplayNodeModifiedEvent(vtkMRMLNode* node)
{
  //this->DebugOn();

  if (this->Updating)
    {
    vtkDebugMacro("OnMRMLMarkupsDisplayNodeModifiedEvent: Updating in progress.. Exit now.")
    return;
    }

  vtkMRMLMarkupsDisplayNode *markupsDisplayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(node);
  if (!markupsDisplayNode)
    {
    vtkErrorMacro("OnMRMLMarkupsDisplayNodeModifiedEvent: Can not access node.")
    return;
    }

  // find the markups node that has this display node
  vtkMRMLMarkupsNode *markupsNode = this->Helper->GetMarkupsNodeFromDisplayNode(markupsDisplayNode);

  if (!markupsNode)
    {
    return;
    }

  vtkDebugMacro("OnMRMLMarkupsDisplayNodeModifiedEvent: found the markups node "
                << markupsNode->GetID()
                << " associated with the modified display node "
                << markupsDisplayNode->GetID());
  vtkAbstractWidget * widget = this->Helper->GetWidget(markupsNode);

  // Propagate MRML changes to widget
  this->PropagateMRMLToWidget(markupsNode, widget);

  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnMRMLMarkupsPointModifiedEvent(vtkMRMLNode *node, int n)
{
  vtkDebugMacro("OnMRMLMarkupsPointModifiedEvent");
  if (!node)
    {
    return;
    }
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return;
    }
  vtkAbstractWidget *widget = this->Helper->GetWidget(markupsNode);
  if (widget)
    {
    // Update the standard settings of all widgets.
    this->UpdateNthSeedPositionFromMRML(n, widget, markupsNode);

    // Propagate MRML changes to widget
    this->PropagateMRMLToWidget(markupsNode, widget);
    this->RequestRender();
    }
}



//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnMRMLMarkupsNodeTransformModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLMarkupsNodeTransformModifiedEvent");
  if (!node)
    {
    return;
    }
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeTransformModifiedEvent - Can not access node.")
    return;
    }

  vtkAbstractWidget *widget = this->Helper->GetWidget(markupsNode);
  if (widget)
    {
    // Propagate MRML changes to widget
    this->PropagateMRMLToWidget(markupsNode, widget);
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnMRMLMarkupsNodeLockModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLMarkupsNodeLockModifiedEvent");
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeLockModifiedEvent - Can not access node.")
    return;
    }
  // Update the standard settings of all widgets.
  this->Helper->UpdateLocked(markupsNode, this->GetInteractionNode());
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller)
{

  vtkDebugMacro("OnMRMLDisplayableNodeModifiedEvent");

  if (!caller)
    {
    vtkErrorMacro("OnMRMLDisplayableNodeModifiedEvent: Could not get caller.")
    return;
    }

  vtkMRMLSliceNode * sliceNode = vtkMRMLSliceNode::SafeDownCast(caller);

  if (sliceNode)
    {
    // now we call the handle for specific sliceNode actions
    this->OnMRMLSliceNodeModifiedEvent();

    // and exit
    return;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::UpdateWidgetVisibility(vtkMRMLMarkupsNode* markupsNode)
{
//  std::cout << "UpdateWidgetVisibility" << std::endl;
  if (!markupsNode)
    {
    vtkErrorMacro("UpdateWidgetVisibility: no markups node from which to work!");
    return;
    }

   vtkAbstractWidget* widget = this->Helper->GetWidget(markupsNode);

   if (!widget)
     {
     vtkErrorMacro("UpdateWidgetVisibility: We could not get the widget to the node: " << markupsNode->GetID());
     return;
     }

   // check if the markups node is visible according to the current mrml state
   vtkMRMLDisplayNode *displayNode = markupsNode->GetDisplayNode();
   bool visibleOnNode = true;
   if (displayNode)
     {
     visibleOnNode = (displayNode->GetVisibility() == 1 ? true : false);
     }
   // check if the widget is visible according to the widget state
   bool visibleOnWidget = (widget->GetEnabled() == 1 ? true : false);

   // only update the visibility of the widget if it is different than on the node
   // first case: the node says it is not visible, but the widget is
   if (!visibleOnNode && visibleOnWidget)
     {
     // hide the widget immediately
     widget->SetEnabled(0);
     vtkSeedWidget *seedWidget = vtkSeedWidget::SafeDownCast(widget);
     if (seedWidget)
       {
       seedWidget->CompleteInteraction();
       vtkDebugMacro("UpdateWidgetVisibility: complete interaction");
       }
     }
   // second case: the node says it is visible, but the widget is not
   else if (visibleOnNode && !visibleOnWidget)
     {
     widget->SetEnabled(1);
     //this->PropagateMRMLToWidget(markupsNode, widget);
     vtkSeedWidget *seedWidget = vtkSeedWidget::SafeDownCast(widget);
     if (seedWidget)
       {
       seedWidget->CompleteInteraction();
       vtkDebugMacro("UpdateWidgetVisibility: complete interaction");
       }
     }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnMRMLSliceNodeModifiedEvent()
{
  this->Internal->UpdateSliceNode();

  // run through all markup nodes in the helper
  vtkMRMLMarkupsDisplayableManagerHelper::MarkupsNodeListIt it;
  it = this->Helper->MarkupsNodeList.begin();
  while(it != this->Helper->MarkupsNodeList.end())
    {
    // we loop through all nodes
    vtkMRMLMarkupsNode * markupsNode = *it;

    vtkAbstractWidget* widget = this->Helper->GetWidget(markupsNode);
    this->PropagateMRMLToWidget(markupsNode, widget);

    ++it;
    }

  if (this->Helper->MarkupsNodeList.size() > 0)
    {
    this->RequestRender();
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayableManager2D::IsWidgetDisplayableOnSlice(vtkMRMLMarkupsNode* node, int markupIndex)
{

  vtkMRMLSliceNode* sliceNode = this->GetMRMLSliceNode();
  // if no slice node, it doesn't constrain the visibility, so return that
  // it's visible
  if (!sliceNode)
    {
    vtkErrorMacro("IsWidgetDisplayableOnSlice: Could not get the sliceNode.")
    return 1;
    }

  // if there's no node, it's not visible
  if (!node)
    {
    vtkErrorMacro("IsWidgetDisplayableOnSlice: Could not get the markups node.")
    return 0;
    }

  // allow annotations to appear only in designated viewers
  vtkMRMLDisplayNode *displayNode = node->GetDisplayNode();
  if (displayNode && !displayNode->IsDisplayableInView(sliceNode->GetID()))
    {
    return 0;
    }

  bool showWidget = true;
  bool inViewport = true;

  int numberOfPoints =  node->GetNumberOfPointsInNthMarkup(markupIndex);
  for (int i=0; i < numberOfPoints; i++)
    {
    // we loop through all points of each node
    double transformedWorldCoordinates[4];
    node->GetMarkupPointWorld(markupIndex, i, transformedWorldCoordinates);

    // now get the displayCoordinates for the transformed worldCoordinates
    double displayCoordinates[4];
    this->GetWorldToDisplayCoordinates(transformedWorldCoordinates,displayCoordinates);

    if (this->IsInLightboxMode())
      {
      //
      // Lightbox specific code
      //
      // get the corresponding lightbox index for this display coordinate and
      // check if it's in the range of the current number of light boxes being
      // displayed in the grid rows/columns.
      int lightboxIndex = this->GetLightboxIndex(node, markupIndex, i);
      int numberOfLightboxes = sliceNode->GetLayoutGridColumns() * sliceNode->GetLayoutGridRows();
      if (lightboxIndex < 0 ||
          lightboxIndex >= numberOfLightboxes)
        {
        showWidget = false;
        }
      else
        {
        vtkDebugMacro("In light box, show the widget " << markupIndex);
        }
      //
      // End of Lightbox specific code
      //
      }
    // check if the markup is close enough to the slice to be shown
    if (showWidget)
      {
      if (this->IsInLightboxMode())
        {
        // get the volume's spacing to determine the distance between the slice
        // location and the markup
        // default to spacing 1.0 in case can't get volume slice spacing from
        // the logic as that will be a multiplicative no-op
        double spacing = 1.0;
        vtkMRMLSliceLogic *sliceLogic = NULL;
        vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
        if (mrmlAppLogic)
          {
          sliceLogic = mrmlAppLogic->GetSliceLogic(this->GetMRMLSliceNode());
          }
        if (sliceLogic)
          {
          double *volumeSliceSpacing = sliceLogic->GetLowestVolumeSliceSpacing();
          if (volumeSliceSpacing != NULL)
            {
            vtkDebugMacro("Slice node " << this->GetMRMLSliceNode()->GetName()
                          << ": volumeSliceSpacing = "
                          << volumeSliceSpacing[0] << ", "
                          << volumeSliceSpacing[1] << ", "
                          << volumeSliceSpacing[2]);
            spacing = volumeSliceSpacing[2];
            }
          }
        vtkDebugMacro("displayCoordinates: "
                      << displayCoordinates[0] << ","
                      << displayCoordinates[1] << ","
                      << displayCoordinates[2] << "\n\tworld coords: "
                      << transformedWorldCoordinates[0] << ","
                      << transformedWorldCoordinates[1] << ","
                      << transformedWorldCoordinates[2]);
        // calculate the distance from the point in world space to the
        // plane defined by the slice node normal and origin (using same
        // convention as the vtkMRMLThreeDReformatDisplayableManager)
        vtkMatrix4x4 *sliceToRAS = this->GetMRMLSliceNode()->GetSliceToRAS();
        double slicePlaneNormal[3], slicePlaneOrigin[3];
        slicePlaneNormal[0] = sliceToRAS->GetElement(0,2);
        slicePlaneNormal[1] = sliceToRAS->GetElement(1,2);
        slicePlaneNormal[2] = sliceToRAS->GetElement(2,2);
        slicePlaneOrigin[0] = sliceToRAS->GetElement(0,3);
        slicePlaneOrigin[1] = sliceToRAS->GetElement(1,3);
        slicePlaneOrigin[2] = sliceToRAS->GetElement(2,3);
        double distanceToPlane = slicePlaneNormal[0]*(transformedWorldCoordinates[0]-slicePlaneOrigin[0]) +
          slicePlaneNormal[1]*(transformedWorldCoordinates[1]-slicePlaneOrigin[1]) +
          slicePlaneNormal[2]*(transformedWorldCoordinates[2]-slicePlaneOrigin[2]);
        // this gives the distance to light box plane 0, but have to offset by
        // number of light box planes (as determined by the light box index) times the volume
        // slice spacing
        int lightboxIndex = this->GetLightboxIndex(node, markupIndex, i);
        double lightboxOffset = lightboxIndex * spacing;
        double distanceToSlice = distanceToPlane - lightboxOffset;
        double maxDistance = 0.5;
        vtkDebugMacro("\n\tdistance to plane = " << distanceToPlane
                      << "\n\tlightboxIndex = " << lightboxIndex
                      << "\n\tlightboxOffset = " << lightboxOffset
                      << "\n\tdistance to slice = " << distanceToSlice);
        // check that it's within 0.5mm
        if (distanceToSlice < -0.5 || distanceToSlice >= maxDistance)
          {
          vtkDebugMacro("Distance to slice is greater than max distance, not showing the widget");
          showWidget = false;
          break;
          }
        }
      else
        {
        // the third coordinate of the displayCoordinates is the distance to the slice
        float distanceToSlice = displayCoordinates[2];
        float maxDistance = 0.5 + (sliceNode->GetDimensions()[2] - 1);
        vtkDebugMacro("Slice node " << this->GetMRMLSliceNode()->GetName()
                      << ": distance to slice = " << distanceToSlice
                      << ", maxDistance = " << maxDistance
                      << "\n\tslice node dimenions[2] = "
                      << sliceNode->GetDimensions()[2]);
        if (distanceToSlice < -0.5 || distanceToSlice >= maxDistance)
          {
          // if the distance to the slice is more than 0.5mm, we know that at least one coordinate of the widget is outside the current activeSlice
          // hence, we do not want to show this widget
          showWidget = false;
          // we don't even need to continue parsing the points, because we know the widget will not be shown
          break;
          }
        }
      }

    // -----------------------------------------
    // special cases when the slices get panned:

    // if all of the points are outside the viewport coordinates, the widget should not be shown
    // if one point is inside the viewport coordinates, the widget should be shown

    // we need to check if we are inside the viewport
    double coords[2] = {displayCoordinates[0], displayCoordinates[1]};

    vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(coords[0],coords[1]);
    if (!pokedRenderer)
      {
      vtkErrorMacro("IsWidgetDisplayableOnSlice: Could not find the poked renderer!")
      return false;
      }

    pokedRenderer->DisplayToNormalizedDisplay(coords[0],coords[1]);
    pokedRenderer->NormalizedDisplayToViewport(coords[0],coords[1]);
    pokedRenderer->ViewportToNormalizedViewport(coords[0],coords[1]);

    if ((coords[0] < 0.0) || (coords[0] > 1.0) ||
        (coords[1] < 0.0) || (coords[1] > 1.0))
      {
      // current point is outside of view
      inViewport = false;
      break;
      }

    } // end of for loop through points

  return showWidget && inViewport;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::Create()
{
  // Setup the SliceCompositeNode
  this->Internal->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnInteractorStyleEvent(int eventid)
{
  if (this->GetDisableInteractorStyleEventsProcessing())
    {
    vtkWarningMacro("OnInteractorStyleEvent: Processing of events was disabled.")
    return;
    }

  if (!this->IsCorrectDisplayableManager())
    {
    //std::cout << "Markups DisplayableManger: OnInteractorStyleEvent : "
    // << this->Focus << ", not correct displayable manager, returning"
    // << std::endl;
    return;
    }
  vtkDebugMacro("OnInteractorStyleEvent " << this->Focus << " " << eventid);

  if (eventid == vtkCommand::LeftButtonReleaseEvent)
    {
    if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
      {
      this->OnClickInRenderWindowGetCoordinates();
      }
    }
  else if (eventid == vtkCommand::LeftButtonPressEvent)
    {
//    vtkWarningMacro("OnInteractorStyleEvent: unhandled left button press event " << eventid);
    }
  else
    {
    //vtkWarningMacro("OnInteractorStyleEvent: unhandled event " << eventid);
    //std::cout << "Markups DisplayableManager: OnInteractorStyleEvent: unhandled event " << eventid << std::endl;
    }
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLMarkupsDisplayableManager2D::GetWidget(vtkMRMLMarkupsNode * node)
{
  return this->Helper->GetWidget(node);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnClickInRenderWindowGetCoordinates()
{

  double x = this->GetInteractor()->GetEventPosition()[0];
  double y = this->GetInteractor()->GetEventPosition()[1];

  double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
  double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

  if (x < windowWidth && y < windowHeight)
    {
    const char *associatedNodeID = NULL;
    // is there a volume in the background?
    if (this->GetMRMLSliceNode())
      {
      // find the slice composite node in the scene with the matching layout
      // name
      vtkMRMLSliceLogic *sliceLogic = NULL;
      vtkMRMLSliceCompositeNode* sliceCompositeNode = NULL;
      vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
      if (mrmlAppLogic)
        {
        sliceLogic = mrmlAppLogic->GetSliceLogic(this->GetMRMLSliceNode());
        }
      if (sliceLogic)
        {
        sliceCompositeNode = sliceLogic->GetSliceCompositeNode(this->GetMRMLSliceNode());
        }
      if (sliceCompositeNode)
        {
        if (sliceCompositeNode->GetBackgroundVolumeID())
          {
          associatedNodeID = sliceCompositeNode->GetBackgroundVolumeID();
          }
        else if (sliceCompositeNode->GetForegroundVolumeID())
          {
          associatedNodeID = sliceCompositeNode->GetForegroundVolumeID();
          }
        else if (sliceCompositeNode->GetLabelVolumeID())
          {
          associatedNodeID = sliceCompositeNode->GetLabelVolumeID();
          }
        }
      }
    vtkDebugMacro("associatedNodeID set to " << (associatedNodeID ? associatedNodeID : "NULL"));
    this->OnClickInRenderWindow(x, y, associatedNodeID);
    //this->Helper->UpdateLockedAllWidgetsFromNodes();
    }
}


//---------------------------------------------------------------------------
// Placement of widgets through seeds
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/// Place a seed for widgets
void vtkMRMLMarkupsDisplayableManager2D::PlaceSeed(double x, double y)
{

  // place the seed
  this->Helper->PlaceSeed(x,y,this->GetInteractor(),this->GetRenderer());

  this->RequestRender();

}

//---------------------------------------------------------------------------
/// Get the handle of a placed seed
vtkHandleWidget * vtkMRMLMarkupsDisplayableManager2D::GetSeed(int index)
{
  return this->Helper->GetSeed(index);
}

//---------------------------------------------------------------------------
// Coordinate conversions
//---------------------------------------------------------------------------
/// Convert display to world coordinates
void vtkMRMLMarkupsDisplayableManager2D::GetDisplayToWorldCoordinates(double x, double y, double * worldCoordinates)
{

  // we will get the transformation matrix to convert display coordinates to RAS

//    double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
//    double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

//    int numberOfColumns = this->GetMRMLSliceNode()->GetLayoutGridColumns();
//    int numberOfRows = this->GetMRMLSliceNode()->GetLayoutGridRows();

//    float tempX = x / windowWidth;
//    float tempY = (windowHeight - y) / windowHeight;

//    float z = floor(tempY*numberOfRows)*numberOfColumns + floor(tempX*numberOfColumns);

  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(x,y);

  vtkMatrix4x4 * xyToRasMatrix = this->GetMRMLSliceNode()->GetXYToRAS();

  double displayCoordinates[4];
  displayCoordinates[0] = x - pokedRenderer->GetOrigin()[0];
  displayCoordinates[1] = y - pokedRenderer->GetOrigin()[1];
  displayCoordinates[2] = 0;
  displayCoordinates[3] = 1;

  xyToRasMatrix->MultiplyPoint(displayCoordinates, worldCoordinates);

}

//---------------------------------------------------------------------------
/// Convert display to world coordinates
void vtkMRMLMarkupsDisplayableManager2D::GetDisplayToWorldCoordinates(double * displayCoordinates, double * worldCoordinates)
{

  this->GetDisplayToWorldCoordinates(displayCoordinates[0], displayCoordinates[1], worldCoordinates);

}

//---------------------------------------------------------------------------
/// Convert world to display coordinates
void vtkMRMLMarkupsDisplayableManager2D::GetWorldToDisplayCoordinates(double r, double a, double s, double * displayCoordinates)
{
  if (!this->GetMRMLSliceNode())
    {
    vtkErrorMacro("GetWorldToDisplayCoordinates: no slice node!");
    return;
    }

  // we will get the transformation matrix to convert world coordinates to the display coordinates of the specific sliceNode

  vtkMatrix4x4 * xyToRasMatrix = this->GetMRMLSliceNode()->GetXYToRAS();
  vtkNew<vtkMatrix4x4> rasToXyMatrix;

  // we need to invert this matrix
  xyToRasMatrix->Invert(xyToRasMatrix, rasToXyMatrix.GetPointer());

  double worldCoordinates[4];
  worldCoordinates[0] = r;
  worldCoordinates[1] = a;
  worldCoordinates[2] = s;
  worldCoordinates[3] = 1;

  rasToXyMatrix->MultiplyPoint(worldCoordinates,displayCoordinates);
  xyToRasMatrix = NULL;
}

//---------------------------------------------------------------------------
/// Convert world to display coordinates
void vtkMRMLMarkupsDisplayableManager2D::GetWorldToDisplayCoordinates(double * worldCoordinates, double * displayCoordinates)
{
  if (worldCoordinates == NULL)
    {
    return;
    }
  this->GetWorldToDisplayCoordinates(worldCoordinates[0], worldCoordinates[1], worldCoordinates[2], displayCoordinates);

}

//---------------------------------------------------------------------------
/// Convert display to viewport coordinates
void vtkMRMLMarkupsDisplayableManager2D::GetDisplayToViewportCoordinates(double x, double y, double * viewportCoordinates)
{

  if (viewportCoordinates == NULL)
    {
    return;
    }
  if (!this->GetInteractor())
    {
    vtkErrorMacro("GetDisplayToViewportCoordinates: No interactor!");
    return;
    }
  double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
  double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

  int numberOfColumns = this->GetMRMLSliceNode()->GetLayoutGridColumns();
  int numberOfRows = this->GetMRMLSliceNode()->GetLayoutGridRows();

  float tempX = x / windowWidth;
  float tempY = (windowHeight - y) / windowHeight;

  float z = floor(tempY*numberOfRows)*numberOfColumns + floor(tempX*numberOfColumns);

  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(x,y);

  double displayCoordinates[4];
  displayCoordinates[0] = x - pokedRenderer->GetOrigin()[0];
  displayCoordinates[1] = y - pokedRenderer->GetOrigin()[1];
  displayCoordinates[2] = z;
  displayCoordinates[3] = 1;

  if (windowWidth != 0.0)
    {
    viewportCoordinates[0] = displayCoordinates[0]/windowWidth;
    }
  if (windowHeight != 0.0)
    {
    viewportCoordinates[1] = displayCoordinates[1]/windowHeight;
    }
  vtkDebugMacro("GetDisplayToViewportCoordinates: x = " << x << ", y = " << y
                << ", display coords calc as "
                << displayCoordinates[0] << ", " << displayCoordinates[1]
                << ", returning viewport = "
                << viewportCoordinates[0] << ", " << viewportCoordinates[1]);
}

//---------------------------------------------------------------------------
/// Convert display to viewport coordinates
void vtkMRMLMarkupsDisplayableManager2D::GetDisplayToViewportCoordinates(double * displayCoordinates, double * viewportCoordinates)
{
  if (displayCoordinates && viewportCoordinates)
    {
    this->GetDisplayToViewportCoordinates(displayCoordinates[0], displayCoordinates[1], viewportCoordinates);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayableManager2D::RestrictDisplayCoordinatesToViewport(double* displayCoordinates)
{
  double coords[2] = {displayCoordinates[0], displayCoordinates[1]};
  bool restricted = false;

  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(coords[0],coords[1]);
  if (!pokedRenderer)
    {
    vtkErrorMacro("RestrictDisplayCoordinatesToViewport: Could not find the poked renderer!")
    return restricted;
    }

  pokedRenderer->DisplayToNormalizedDisplay(coords[0],coords[1]);
  pokedRenderer->NormalizedDisplayToViewport(coords[0],coords[1]);
  pokedRenderer->ViewportToNormalizedViewport(coords[0],coords[1]);

  if (coords[0]<0.001)
    {
    coords[0] = 0.001;
    restricted = true;
    }
  else if (coords[0]>0.999)
    {
    coords[0] = 0.999;
    restricted = true;
    }

  if (coords[1]<0.001)
    {
    coords[1] = 0.001;
    restricted = true;
    }
  else if (coords[1]>0.999)
    {
    coords[1] = 0.999;
    restricted = true;
    }

  pokedRenderer->NormalizedViewportToViewport(coords[0],coords[1]);
  pokedRenderer->ViewportToNormalizedDisplay(coords[0],coords[1]);
  pokedRenderer->NormalizedDisplayToDisplay(coords[0],coords[1]);

  displayCoordinates[0] = coords[0];
  displayCoordinates[1] = coords[1];

  return restricted;
}

//---------------------------------------------------------------------------
/// Check if there are real changes between two sets of displayCoordinates
bool vtkMRMLMarkupsDisplayableManager2D::GetDisplayCoordinatesChanged(double * displayCoordinates1, double * displayCoordinates2)
{
  bool changed = false;

  if (sqrt( ( displayCoordinates1[0] - displayCoordinates2[0] ) * ( displayCoordinates1[0] - displayCoordinates2[0] )
           + ( displayCoordinates1[1] - displayCoordinates2[1] ) * ( displayCoordinates1[1] - displayCoordinates2[1] ))>1.0)
    {
    changed = true;
    }
  else
    {
    // if in lightbox mode, the third element in the vector may have changed
    if (this->IsInLightboxMode())
      {
      // one of the arguments may be coming from a widget, the other should be
      // the index into the light box array
      double dist = sqrt( (displayCoordinates1[2] - displayCoordinates2[2]) * (displayCoordinates1[2] - displayCoordinates2[2]));
      if (dist > 1.0)
        {
        changed = true;
        }
      }
    }
  return changed;
}

//---------------------------------------------------------------------------
/// Check if there are real changes between two sets of displayCoordinates
bool vtkMRMLMarkupsDisplayableManager2D::GetWorldCoordinatesChanged(double * worldCoordinates1, double * worldCoordinates2)
{
  bool changed = false;

  double distance = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));

  // TODO find a better value?
  // - use a smaller number to make fiducial seeding more smooth
  if (distance > VTK_DBL_EPSILON)
    {
    changed = true;
    }

  return changed;
}

//---------------------------------------------------------------------------
/// Check if it is the correct displayableManager
//---------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayableManager2D::IsCorrectDisplayableManager()
{

  vtkMRMLSelectionNode *selectionNode = this->GetMRMLApplicationLogic()->GetSelectionNode();
  if ( selectionNode == 0 )
    {
    vtkErrorMacro ( "IsCorrectDisplayableManager: No selection node in the scene." );
    return false;
    }
  if ( selectionNode->GetActivePlaceNodeClassName() == 0)
    {
    //vtkErrorMacro ( "IsCorrectDisplayableManager: no active markups");
    return false;
    }
  // the purpose of the displayableManager is hardcoded
  return this->IsManageable(selectionNode->GetActivePlaceNodeClassName());

}
//---------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayableManager2D::IsManageable(vtkMRMLNode* node)
{
  return node->IsA(this->Focus);
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayableManager2D::IsManageable(const char* nodeClassName)
{
  return nodeClassName && !strcmp(nodeClassName, this->Focus);
}

//---------------------------------------------------------------------------
// Functions to overload!
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnClickInRenderWindow(double vtkNotUsed(x), double vtkNotUsed(y), const char * vtkNotUsed(associatedNodeID))
{

  // The user clicked in the renderWindow
  vtkErrorMacro("OnClickInRenderWindow should be overloaded!");
}

//---------------------------------------------------------------------------
vtkAbstractWidget* vtkMRMLMarkupsDisplayableManager2D::CreateWidget(vtkMRMLMarkupsNode* vtkNotUsed(node))
{

  // A widget should be created here.
  vtkErrorMacro("CreateWidget should be overloaded!");
  return 0;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::OnWidgetCreated(vtkAbstractWidget* vtkNotUsed(widget), vtkMRMLMarkupsNode* vtkNotUsed(node))
{

  // Actions after a widget was created should be executed here.
  vtkErrorMacro("OnWidgetCreated should be overloaded!");
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::PropagateMRMLToWidget(vtkMRMLMarkupsNode* vtkNotUsed(node), vtkAbstractWidget* vtkNotUsed(widget))
{
  // update the widget visibility according to the mrml settings of visibility
  // status for 2d (taking into account the current slice)
  //this->UpdateWidgetVisibility(markupsNode);
  // The properties of a widget should be set here, subclasses should call this.
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager2D::PropagateWidgetToMRML(vtkAbstractWidget* vtkNotUsed(widget), vtkMRMLMarkupsNode* vtkNotUsed(node))
{
  // The properties of a widget should be set here.
  vtkErrorMacro("PropagateWidgetToMRML should be overloaded!");
}

//---------------------------------------------------------------------------
/// Convert world coordinates to local using mrml parent transform
void vtkMRMLMarkupsDisplayableManager2D::GetWorldToLocalCoordinates(vtkMRMLMarkupsNode *node,
                                                                     double *worldCoordinates,
                                                                     double *localCoordinates)
{
  if (node == NULL)
    {
    vtkErrorMacro("GetWorldToLocalCoordinates: node is null");
    return;
    }

  vtkMRMLTransformNode* tnode = node->GetParentTransformNode();
  if (tnode != NULL && tnode->IsLinear())
    {
    vtkNew<vtkMatrix4x4> transformToWorld;
    transformToWorld->Identity();
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    lnode->GetMatrixTransformToWorld(transformToWorld.GetPointer());
    transformToWorld->Invert();

    double p[4];
    p[3] = 1;
    int i;
    for (i=0; i<3; i++)
      {
      p[i] = worldCoordinates[i];
      }
    double *xyz = transformToWorld->MultiplyDoublePoint(p);
    for (i=0; i<3; i++)
      {
      localCoordinates[i] = xyz[i];
      }
    }
  else
    {
    for (int i=0; i<3; i++)
      {
      localCoordinates[i] = worldCoordinates[i];
      }
    }
}

//---------------------------------------------------------------------------
/// Create a new widget for this markups node and save it to the helper.
vtkAbstractWidget * vtkMRMLMarkupsDisplayableManager2D::AddWidget(vtkMRMLMarkupsNode *markupsNode)
{
  if (!this->GetRenderer())
    {
    vtkWarningMacro("AddWidget: no renderer!");
    return NULL ;
    }
  if (!this->GetRenderer()->GetActiveCamera())
    {
    vtkWarningMacro("AddWidget: no active camera on renderer!");
    return NULL ;
    }

  vtkDebugMacro("AddWidget: calling create widget");
  vtkAbstractWidget* newWidget = this->CreateWidget(markupsNode);
  if (!newWidget)
    {
    vtkErrorMacro("AddWidget: unable to create a new widget for markups node " << markupsNode->GetID());
    return NULL;
    }

  // record the mapping between node and widget in the helper
  this->Helper->RecordWidgetForNode(newWidget,markupsNode);

  // Refresh observers
  this->SetAndObserveNode(markupsNode);

  // Add the call backs so DM subclasses can respond to vtkWidget interactions
  this->OnWidgetCreated(newWidget, markupsNode);

  // now set up the new widget
  //this->PropagateMRMLToWidget(markupsNode, newWidget);
  this->SetUpdateFromMRMLRequested(1);
  this->RequestRender();

  return newWidget;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayableManager2D::IsInLightboxMode()
{
  bool flag = false;
  vtkMRMLSliceNode *sliceNode = this->GetMRMLSliceNode();
  if (sliceNode)
    {
    int numberOfColumns = sliceNode->GetLayoutGridColumns();
    int numberOfRows = sliceNode->GetLayoutGridRows();
    if (numberOfColumns > 1 ||
        numberOfRows > 1)
      {
      flag = true;
      }
    }
  return flag;
}

//---------------------------------------------------------------------------
int  vtkMRMLMarkupsDisplayableManager2D::GetLightboxIndex(vtkMRMLMarkupsNode *node, int markupIndex, int pointIndex)
{
  int index = -1;

  if (!node)
    {
    return index;
    }
  if (!this->IsInLightboxMode())
    {
    return index;
    }

  if (markupIndex < 0 ||
      markupIndex > node->GetNumberOfMarkups())
    {
    return index;
    }
  if (pointIndex < 0 ||
      pointIndex >= node->GetNumberOfPointsInNthMarkup(markupIndex))
    {
    return index;
    }

  double transformedWorldCoordinates[4];
  node->GetMarkupPointWorld(markupIndex, pointIndex, transformedWorldCoordinates);
  double displayCoordinates[4];
  this->GetWorldToDisplayCoordinates(transformedWorldCoordinates,displayCoordinates);

  index = (int)(floor(displayCoordinates[2]+0.5));


  return index;
}
