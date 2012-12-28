
// MarkupsModule/MRML includes
#include <vtkMRMLMarkupsDisplayNode.h>
#include <vtkMRMLMarkupsNode.h>

// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsDisplayableManager.h"

// MRMLDisplayableManager includes
#include <vtkMRMLModelDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLInteractionNode.h>
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
#include <vtkSmartPointer.h>
#include <vtkWidgetRepresentation.h>

// STD includes
#include <algorithm>
#include <map>
#include <vector>

typedef void (*fp)(void);

#define NUMERIC_ZERO 0.001

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLMarkupsDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLMarkupsDisplayableManager, "$Revision: 1.2 $");

//---------------------------------------------------------------------------
vtkMRMLMarkupsDisplayableManager::vtkMRMLMarkupsDisplayableManager()
{
  this->Helper = vtkMRMLMarkupsDisplayableManagerHelper::New();
  this->ClickCounter = vtkMRMLMarkupsClickCounter::New();
  this->DisableInteractorStyleEventsProcessing = 0;
  this->Updating = 0;

  this->Focus = "vtkMRMLMarkupsNode";

  // by default, this displayableManager handles a ThreeDView
  this->SliceNode = 0;

  // by default, multiply the display node scale by this when setting scale on elements in 2d windows
  this->ScaleFactor2D = 0.00333;

  this->LastClickWorldCoordinates[0]=0;
  this->LastClickWorldCoordinates[1]=0;
  this->LastClickWorldCoordinates[2]=0;
  this->LastClickWorldCoordinates[3]=1;
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsDisplayableManager::~vtkMRMLMarkupsDisplayableManager()
{

  this->DisableInteractorStyleEventsProcessing = 0;
  this->Updating = 0;
  this->Focus = 0;

  this->Helper->Delete();
  this->ClickCounter->Delete();

  this->SliceNode = 0;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "DisableInteractorStyleEventsProcessing = " << this->DisableInteractorStyleEventsProcessing << std::endl;
  if (this->SliceNode &&
      this->SliceNode->GetID())
    {
    os << indent << "Slice node id = " << this->SliceNode->GetID() << std::endl;
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
void vtkMRMLMarkupsDisplayableManager::SetAndObserveNode(vtkMRMLMarkupsNode *markupsNode)
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
  nodeEvents->InsertNextValue(vtkMRMLMarkupsNode::LockModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);

 if (markupsNode)// && !markupsNode->HasObserver(vtkMRMLTransformableNode::TransformModifiedEvent))
   {
   vtkUnObserveMRMLNodeMacro(markupsNode);
   vtkObserveMRMLNodeEventsMacro(markupsNode, nodeEvents.GetPointer());
   }
}
//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::SetAndObserveNodes()
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
void vtkMRMLMarkupsDisplayableManager::AddObserversToInteractionNode()
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
void vtkMRMLMarkupsDisplayableManager::RemoveObserversFromInteractionNode()
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
/*
//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::Create()
{

  // hack to force initialization of the renderview
  //this->GetInteractor()->InvokeEvent(vtkCommand::MouseWheelBackwardEvent);
  //this->GetInteractor()->InvokeEvent(vtkCommand::MouseWheelForwardEvent);
  this->UpdateFromMRML();
  //this->DebugOn();
}
*/
//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::RequestRender()
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
void vtkMRMLMarkupsDisplayableManager::RemoveMRMLObservers()
{
  // run through all associated nodes
  vtkMRMLMarkupsDisplayableManagerHelper::MarkupsNodeListIt it;
  it = this->Helper->MarkupsNodeList.begin();
  while(it != this->Helper->MarkupsNodeList.end())
    {
    vtkUnObserveMRMLNodeMacro(*it);
    ++it;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::UpdateFromMRML()
{
  // this gets called from RequestRender, so make sure to jump out quickly if possible
  if (this->GetMRMLScene() == NULL || this->Focus == NULL)
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
        vtkWarningMacro("UpdateFromMRML: creating a widget for node " << markupsNode->GetID());
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
      }
    node = this->GetMRMLScene()->GetNextNodeByClass(this->Focus);
    }
  // set up observers on all the nodes
//  this->SetAndObserveNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  Superclass::SetMRMLSceneInternal(newScene);

  // after a new scene got associated, we want to make sure everything old is gone
  this->OnMRMLSceneEndClose();

  if (newScene)
    {
    this->AddObserversToInteractionNode();
    }
  else
    {
    // there's no scene to get the interaction node from, so this won't do anything
    this->RemoveObserversFromInteractionNode();
    }
  vtkDebugMacro("SetMRMLSceneInternal: add observer on interaction node now?");

}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager
::ProcessMRMLNodesEvents(vtkObject *caller,unsigned long event,void *callData)
{

  vtkMRMLMarkupsNode * markupsNode = vtkMRMLMarkupsNode::SafeDownCast(caller);
  vtkMRMLMarkupsDisplayNode *displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(caller);
  vtkMRMLInteractionNode * interactionNode = vtkMRMLInteractionNode::SafeDownCast(caller);
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
  else
    {
    this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::OnMRMLSceneEndClose()
{
  vtkDebugMacro("OnMRMLSceneEndClose: remove observers?");
  // run through all nodes and remove node and widget
  this->Helper->RemoveAllWidgetsAndNodes();

  this->SetUpdateFromMRMLRequested(1);
  this->RequestRender();

}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::UpdateFromMRMLScene()
{
  this->UpdateFromMRML();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
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

  if (node->IsA("vtkMRMLInteractionNode"))
    {
    this->AddObserversToInteractionNode();
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

  vtkWarningMacro("OnMRMLSceneNodeAddedEvent:  node " << node->GetID());

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
    vtkWarningMacro("OnMRMLSceneNodeAddedEvent: widget was created, saved to helper Widgets map");
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
void vtkMRMLMarkupsDisplayableManager::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
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
void vtkMRMLMarkupsDisplayableManager::OnMRMLMarkupsNodeModifiedEvent(vtkMRMLNode* node)
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
void vtkMRMLMarkupsDisplayableManager::OnMRMLMarkupsDisplayNodeModifiedEvent(vtkMRMLNode* node)
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

  vtkDebugMacro("OnMRMLMarkupsDisplayNodeModifiedEvent: found the markups node " << markupsNode->GetID() << " associated with the modified display node " << markupsDisplayNode->GetID());
  vtkAbstractWidget * widget = this->Helper->GetWidget(markupsNode);

  // Propagate MRML changes to widget
  this->PropagateMRMLToWidget(markupsNode, widget);

  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::OnMRMLMarkupsPointModifiedEvent(vtkMRMLNode *node, int n)
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
    this->UpdateNthMarkupPosition(n, widget, markupsNode);

    // Propagate MRML changes to widget
    this->PropagateMRMLToWidget(markupsNode, widget);
    this->RequestRender();
    }
}



//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::OnMRMLMarkupsNodeTransformModifiedEvent(vtkMRMLNode* node)
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
void vtkMRMLMarkupsDisplayableManager::OnMRMLMarkupsNodeLockModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLMarkupsNodeLockModifiedEvent");
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeLockModifiedEvent - Can not access node.")
    return;
    }
  // Update the standard settings of all widgets.
  this->Helper->UpdateLocked(markupsNode);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller)
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
    // the associated renderWindow is a 2D SliceView
    // this is the entry point for all events fired by one of the three sliceviews
    // (f.e. change slice number, zoom etc.)

    // we remember that this instance of the displayableManager deals with 2D
    // this is important for widget creation etc. and save the actual SliceNode
    // because during Slicer startup the SliceViews fire events, it will be always set correctly
    this->SliceNode = sliceNode;

    // now we call the handle for specific sliceNode actions
    this->OnMRMLSliceNodeModifiedEvent(sliceNode);

    // and exit
    return;
    }

  vtkMRMLViewNode * viewNode = vtkMRMLViewNode::SafeDownCast(caller);

  if (viewNode)
    {
    // the associated renderWindow is a 3D View
    vtkDebugMacro("OnMRMLDisplayableNodeModifiedEvent: This displayableManager handles a ThreeD view.")
    return;
    }

}

//---------------------------------------------------------------------------
vtkMRMLSliceNode * vtkMRMLMarkupsDisplayableManager::GetSliceNode()
{

  return this->SliceNode;

}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayableManager::Is2DDisplayableManager()
{
  if (this->SliceNode != NULL)
    {
    return true;
    }
  else
    {
    return false;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::UpdateWidgetVisibility(vtkMRMLMarkupsNode* markupsNode)
{
  std::cout << "UpdateWidgetVisibility" << std::endl;
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
     visibleOnNode = (displayNode->GetVisibility()== 1 ? true : false);
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

   // it's visible, just update the position
   //  this->UpdatePosition(widget, markupsNode);
   
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::OnMRMLSliceNodeModifiedEvent(vtkMRMLSliceNode* sliceNode)
{

  if (!sliceNode)
    {
    vtkErrorMacro("OnMRMLSliceNodeModifiedEvent: Could not get the sliceNode.")
    return;
    }

  // run through all associated nodes
  vtkMRMLMarkupsDisplayableManagerHelper::MarkupsNodeListIt it;
  it = this->Helper->MarkupsNodeList.begin();
  while(it != this->Helper->MarkupsNodeList.end())
    {
    // we loop through all nodes
    vtkMRMLMarkupsNode * markupsNode = *it;

//    this->UpdateWidgetVisibility(markupsNode, sliceNode);
    vtkAbstractWidget* widget = this->Helper->GetWidget(markupsNode);
    this->PropagateMRMLToWidget(markupsNode, widget);
    
    ++it;
    }

}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsDisplayableManager::IsWidgetDisplayableOnSlice(vtkMRMLMarkupsNode* node, int markupIndex)
{
  // always visible if called from a 3d displayable manager
  if (!this->Is2DDisplayableManager())
    {
    return 1;
    }
  
  vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
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

  bool showWidget = true;
  bool inViewport = false;

  // down cast the node as a controlpoints node to get the coordinates
  vtkMRMLMarkupsNode * controlPointsNode = vtkMRMLMarkupsNode::SafeDownCast(node);

  if (!controlPointsNode)
    {
    vtkErrorMacro("IsWidgetDisplayableOnSlice: Could not get the controlpoints node.")
    return 0;
    }

  int numberOfControlPoints =  controlPointsNode->GetNumberOfPointsInNthMarkup(markupIndex);
  for (int i=0; i < numberOfControlPoints; i++)
    {
    // we loop through all controlpoints of each node
    double transformedWorldCoordinates[4];
    controlPointsNode->GetMarkupPointWorld(markupIndex, i, transformedWorldCoordinates);

    // now get the displayCoordinates for the transformed worldCoordinates
    double displayCoordinates[4];
    this->GetWorldToDisplayCoordinates(transformedWorldCoordinates,displayCoordinates);

    //
    // Lightbox specific code
    //

    // get the right renderer index by checking the z coordinate
    int rendererIndex = (int)(displayCoordinates[2]+0.5);

    // get all renderers associated with this renderWindow
    // when lightbox mode is enabled, there will be different renderers associated with the renderWindow of the sliceView
    vtkRendererCollection* rendererCollection = this->GetInteractor()->GetRenderWindow()->GetRenderers();

    // check if lightbox is enabled
    if (sliceNode->GetLayoutGridRows() > 1 || sliceNode->GetLayoutGridColumns() > 1)
      {

      // check if the rendererIndex is valid for the current lightbox view
      if (rendererIndex >= 0 && rendererIndex < rendererCollection->GetNumberOfItems())
        {

        vtkRenderer* currentRenderer = vtkRenderer::SafeDownCast(rendererCollection->GetItemAsObject(rendererIndex));

        // now we get the widget..
        vtkAbstractWidget* widget = this->GetWidget(node);

        // TODO this code blocks the movement of the widget in lightbox mode
        if (widget)
          {
          // ..and turn it off..
          widget->Off();
          // ..place it and its representation to the right renderer..
          widget->SetCurrentRenderer(currentRenderer);
          widget->GetRepresentation()->SetRenderer(currentRenderer);
          // ..and turn it on again!
          widget->On();
          // if it's a seed widget, go to complete interaction state
          vtkSeedWidget *seedWidget = vtkSeedWidget::SafeDownCast(widget);
          if (seedWidget)
            {
            vtkDebugMacro("SeedWidget: Complete interaction");
            seedWidget->CompleteInteraction();
            }

          // we need to render again
          if (currentRenderer)
            {
            currentRenderer->Render();
            }
          }
        }

      }

    //
    // End of Lightbox specific code
    //

    // the third coordinate of the displayCoordinates is the distance to the slice
    float distanceToSlice = displayCoordinates[2];

    if (distanceToSlice < -0.5 || distanceToSlice >= (0.5+this->SliceNode->GetDimensions()[2]-1))
      {
      // if the distance to the slice is more than 0.5mm, we know that at least one coordinate of the widget is outside the current activeSlice
      // hence, we do not want to show this widget
      showWidget = false;
      // we don't even need to continue parsing the controlpoints, because we know the widget will not be shown
      break;
      }

    // -----------------------------------------
    // special cases when the slices get panned:

    // if all of the controlpoints are outside the viewport coordinates, the widget should not be shown
    // if one controlpoint is inside the viewport coordinates, the widget should be shown

    // we need to check if we are inside the viewport
    double coords[2] = {displayCoordinates[0], displayCoordinates[1]};

    vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(coords[0],coords[1]);
    if (!pokedRenderer)
      {
      vtkErrorMacro("RestrictDisplayCoordinatesToViewport: Could not find the poked renderer!")
      return false;
      }

    pokedRenderer->DisplayToNormalizedDisplay(coords[0],coords[1]);
    pokedRenderer->NormalizedDisplayToViewport(coords[0],coords[1]);
    pokedRenderer->ViewportToNormalizedViewport(coords[0],coords[1]);

    if ((coords[0]>0.0) && (coords[1]<1.0) && (coords[1]>0.0) && (coords[1]<1.0))
      {
      // current point is inside of view
      inViewport = true;
      }

    } // end of for loop through control points


  return showWidget && inViewport;

}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  if (this->GetDisableInteractorStyleEventsProcessing())
    {
    vtkWarningMacro("OnInteractorStyleEvent: Processing of events was disabled.")
    return;
    }

  if (!this->IsCorrectDisplayableManager())
    {
    //std::cout << "Markups DisplayableManger: OnInteractorStyleEvent : " << this->Focus << ", not correct displayable manager, returning" << std::endl;
    return;
    }
  vtkDebugMacro("OnInteractorStyleEvent " << this->Focus << " " << eventid);

  if (eventid == vtkCommand::LeftButtonReleaseEvent)
    {
    if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
      {
      //std::cout << "OnInteractorStyleEvent got a left button release " << eventid << ", and are in place mode, calling OnClickInRenderWindowGetCoordinates" << std::endl;
      this->OnClickInRenderWindowGetCoordinates();
      }
//    else { vtkWarningMacro("OnInteractorStyleEvent: not in vtkMRMLInteractionNode::Place interaction mode"); }
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
vtkAbstractWidget * vtkMRMLMarkupsDisplayableManager::GetWidget(vtkMRMLMarkupsNode * node)
{
  return this->Helper->GetWidget(node);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::OnClickInRenderWindowGetCoordinates()
{

  double x = this->GetInteractor()->GetEventPosition()[0];
  double y = this->GetInteractor()->GetEventPosition()[1];

  double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
  double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

  if (x < windowWidth && y < windowHeight)
    {
    const char *associatedNodeID = NULL;
    // is this a 2d manager that has a volume in the background?
    if (this->Is2DDisplayableManager())
      {
      if (this->GetSliceNode())
        {
        // find the slice composite node in the scene with the matching layout
        // name
        vtkMRMLSliceLogic *sliceLogic = NULL;
        vtkMRMLSliceCompositeNode* sliceCompositeNode = NULL;
        vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
        if (mrmlAppLogic)
          {
          sliceLogic = mrmlAppLogic->GetSliceLogic(this->GetSliceNode());
          }
        if (sliceLogic)
          {
          sliceCompositeNode = sliceLogic->GetSliceCompositeNode(this->GetSliceNode());
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
      }
    else
      {
      // it's a 3D displayable manager and the click could have been on a node
      vtkMRMLModelDisplayableManager * modelDisplayableManager =
        vtkMRMLModelDisplayableManager::SafeDownCast(
          this->GetMRMLDisplayableManagerGroup()->GetDisplayableManagerByClassName(
            "vtkMRMLModelDisplayableManager"));
      double yNew = windowHeight - y - 1;
      if (modelDisplayableManager &&
          modelDisplayableManager->Pick(x,yNew) &&
          strcmp(modelDisplayableManager->GetPickedNodeID(),"") != 0)
        {
        // find the node id, the picked node name is probably the display node
        const char *pickedNodeID = modelDisplayableManager->GetPickedNodeID();
        vtkDebugMacro("Click was on model " << pickedNodeID);
        vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(pickedNodeID);
        vtkMRMLDisplayNode *displayNode = NULL;
        if (mrmlNode)
          {
          vtkDebugMacro("Got a mrml node by name, id = " << mrmlNode->GetID());
          displayNode = vtkMRMLDisplayNode::SafeDownCast(mrmlNode);
          }
        else
          {
          vtkDebugMacro("couldn't find a mrml node with ID " << pickedNodeID);
          }
        if (displayNode)
          {
          vtkDebugMacro("Got display node for picked node name " << displayNode->GetID());
          vtkMRMLDisplayableNode *displayableNode = displayNode->GetDisplayableNode();
          if (displayableNode)
            {
            // start with the assumption that it's a generic displayable node,
            // then look for it to be a slice node that has the string
            // CompositeID in it's Description with a valid mrml node id after it
            associatedNodeID = displayableNode->GetID();
            // it might be a slice node, check the Description field for the string CompositeID
            if (displayableNode->GetDescription())
              {
              std::string desc = displayableNode->GetDescription();
              size_t ptr = desc.find("CompositeID");
              // does it have the string CompositeID in the description with
              // something after it?
              vtkDebugMacro("Desc len = " << desc.length() << ", ptr = " << ptr); 
              if (ptr != std::string::npos &&
                  (desc.length() > (ptr + 12)))
                {
                std::string compID = desc.substr(ptr + 12);
                vtkDebugMacro("Found composite node id = " << compID.c_str());
                vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(compID.c_str());
                // was this a valid composite node id?
                if (mrmlNode)
                  {
                  vtkMRMLSliceCompositeNode* sliceCompositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(mrmlNode);
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
                    vtkDebugMacro("Markups was placed on a 3d slice model, found the volume id " << associatedNodeID);
                    }
                  }
                }
              }
            }
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
void vtkMRMLMarkupsDisplayableManager::PlaceSeed(double x, double y)
{

  // place the seed
  this->Helper->PlaceSeed(x,y,this->GetInteractor(),this->GetRenderer());

  this->RequestRender();

}

//---------------------------------------------------------------------------
/// Get the handle of a placed seed
vtkHandleWidget * vtkMRMLMarkupsDisplayableManager::GetSeed(int index)
{
  return this->Helper->GetSeed(index);
}

//---------------------------------------------------------------------------
// Coordinate conversions
//---------------------------------------------------------------------------
/// Convert display to world coordinates
void vtkMRMLMarkupsDisplayableManager::GetDisplayToWorldCoordinates(double x, double y, double * worldCoordinates)
{

  if (this->Is2DDisplayableManager())
    {
    // 2D case

    // we will get the transformation matrix to convert display coordinates to RAS

//    double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
//    double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

//    int numberOfColumns = this->GetSliceNode()->GetLayoutGridColumns();
//    int numberOfRows = this->GetSliceNode()->GetLayoutGridRows();

//    float tempX = x / windowWidth;
//    float tempY = (windowHeight - y) / windowHeight;

//    float z = floor(tempY*numberOfRows)*numberOfColumns + floor(tempX*numberOfColumns);

    vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(x,y);

    vtkMatrix4x4 * xyToRasMatrix = this->GetSliceNode()->GetXYToRAS();

    double displayCoordinates[4];
    displayCoordinates[0] = x - pokedRenderer->GetOrigin()[0];
    displayCoordinates[1] = y - pokedRenderer->GetOrigin()[1];
    displayCoordinates[2] = 0;
    displayCoordinates[3] = 1;

    xyToRasMatrix->MultiplyPoint(displayCoordinates, worldCoordinates);

    }
  else
    {

    // for 3D, we want to convert the coordinates using the pick function

    // ModelDisplayableManager is expected to be instantiated !
    vtkMRMLModelDisplayableManager * modelDisplayableManager =
      vtkMRMLModelDisplayableManager::SafeDownCast(
        this->GetMRMLDisplayableManagerGroup()->GetDisplayableManagerByClassName(
          "vtkMRMLModelDisplayableManager"));
    assert(modelDisplayableManager);

    double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

    double yNew = windowHeight - y - 1;

    if (modelDisplayableManager->Pick(x,yNew))
      {
      double* pickedWorldCoordinates = modelDisplayableManager->GetPickedRAS();
      worldCoordinates[0] = pickedWorldCoordinates[0];
      worldCoordinates[1] = pickedWorldCoordinates[1];
      worldCoordinates[2] = pickedWorldCoordinates[2];
      worldCoordinates[3] = 1;
      }
    else
      {
      // we could not pick so just convert to world coordinates
      vtkInteractorObserver::ComputeDisplayToWorld(this->GetRenderer(),x,y,0,worldCoordinates);
      }
    }
}

//---------------------------------------------------------------------------
/// Convert display to world coordinates
void vtkMRMLMarkupsDisplayableManager::GetDisplayToWorldCoordinates(double * displayCoordinates, double * worldCoordinates)
{

  this->GetDisplayToWorldCoordinates(displayCoordinates[0], displayCoordinates[1], worldCoordinates);

}

//---------------------------------------------------------------------------
/// Convert world to display coordinates
void vtkMRMLMarkupsDisplayableManager::GetWorldToDisplayCoordinates(double r, double a, double s, double * displayCoordinates)
{

  if (this->Is2DDisplayableManager())
    {
    // 2D case

    // we will get the transformation matrix to convert world coordinates to the display coordinates of the specific sliceNode

    vtkMatrix4x4 * xyToRasMatrix = this->GetSliceNode()->GetXYToRAS();
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
  else
    {
    vtkInteractorObserver::ComputeWorldToDisplay(this->GetRenderer(),r,a,s,displayCoordinates);
    }


}

//---------------------------------------------------------------------------
/// Convert world to display coordinates
void vtkMRMLMarkupsDisplayableManager::GetWorldToDisplayCoordinates(double * worldCoordinates, double * displayCoordinates)
{
  if (worldCoordinates == NULL)
    {
    return;
    }
  this->GetWorldToDisplayCoordinates(worldCoordinates[0], worldCoordinates[1], worldCoordinates[2], displayCoordinates);

}

//---------------------------------------------------------------------------
/// Convert display to viewport coordinates
void vtkMRMLMarkupsDisplayableManager::GetDisplayToViewportCoordinates(double x, double y, double * viewportCoordinates)
{

  if (viewportCoordinates == NULL)
    {
    return;
    }
  double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
  double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

  if (this->Is2DDisplayableManager())
    {
    // 2D case

    int numberOfColumns = this->GetSliceNode()->GetLayoutGridColumns();
    int numberOfRows = this->GetSliceNode()->GetLayoutGridRows();

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
    vtkDebugMacro("GetDisplayToViewportCoordinates: x = " << x << ", y = " << y << ", display coords calc as " << displayCoordinates[0] << ", " << displayCoordinates[1] << ", returning viewport = " << viewportCoordinates[0] << ", " << viewportCoordinates[1]);
    }
  else
    {
    if (windowWidth != 0.0)
      {
      viewportCoordinates[0] = x/windowWidth;
      }
    if (windowHeight != 0.0)
      {
      viewportCoordinates[1] = y/windowHeight;
      }
    vtkDebugMacro("GetDisplayToViewportCoordinates: x = " << x << ", y = " << y << ", returning viewport = " << viewportCoordinates[0] << ", " << viewportCoordinates[1]);
    }
}

//---------------------------------------------------------------------------
/// Convert display to viewport coordinates
void vtkMRMLMarkupsDisplayableManager::GetDisplayToViewportCoordinates(double * displayCoordinates, double * viewportCoordinates)
{
  if (displayCoordinates && viewportCoordinates)
    {
    this->GetDisplayToViewportCoordinates(displayCoordinates[0], displayCoordinates[1], viewportCoordinates);
    }
}

//---------------------------------------------------------------------------
/// Check if the displayCoordinates are inside the viewport and if not, correct the displayCoordinates
void vtkMRMLMarkupsDisplayableManager::RestrictDisplayCoordinatesToViewport(double* displayCoordinates)
{
  double coords[2] = {displayCoordinates[0], displayCoordinates[1]};

  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(coords[0],coords[1]);
  if (!pokedRenderer)
    {
    vtkErrorMacro("RestrictDisplayCoordinatesToViewport: Could not find the poked renderer!")
    return;
    }

  pokedRenderer->DisplayToNormalizedDisplay(coords[0],coords[1]);
  pokedRenderer->NormalizedDisplayToViewport(coords[0],coords[1]);
  pokedRenderer->ViewportToNormalizedViewport(coords[0],coords[1]);

  if (coords[0]<0.001)
    {
    coords[0] = 0.001;
    }
  else if (coords[0]>0.999)
    {
    coords[0] = 0.999;
    }

  if (coords[1]<0.001)
    {
    coords[1] = 0.001;
    }
  else if (coords[1]>0.999)
    {
    coords[1] = 0.999;
    }

  pokedRenderer->NormalizedViewportToViewport(coords[0],coords[1]);
  pokedRenderer->ViewportToNormalizedDisplay(coords[0],coords[1]);
  pokedRenderer->NormalizedDisplayToDisplay(coords[0],coords[1]);

//  if (this->GetDisplayCoordinatesChanged(displayCoordinates,coords))
//    {
  displayCoordinates[0] = coords[0];
  displayCoordinates[1] = coords[1];
//    }
}

//---------------------------------------------------------------------------
/// Check if there are real changes between two sets of displayCoordinates
bool vtkMRMLMarkupsDisplayableManager::GetDisplayCoordinatesChanged(double * displayCoordinates1, double * displayCoordinates2)
{
  bool changed = false;

  if (sqrt( ( displayCoordinates1[0] - displayCoordinates2[0] ) * ( displayCoordinates1[0] - displayCoordinates2[0] )
           + ( displayCoordinates1[1] - displayCoordinates2[1] ) * ( displayCoordinates1[1] - displayCoordinates2[1] ))>1.0)
    {
    changed = true;
    }

  return changed;
}

//---------------------------------------------------------------------------
/// Check if there are real changes between two sets of displayCoordinates
bool vtkMRMLMarkupsDisplayableManager::GetWorldCoordinatesChanged(double * worldCoordinates1, double * worldCoordinates2)
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
bool vtkMRMLMarkupsDisplayableManager::IsCorrectDisplayableManager()
{

  vtkMRMLSelectionNode *selectionNode = this->GetMRMLApplicationLogic()->GetSelectionNode();
  if ( selectionNode == 0 )
    {
    vtkErrorMacro ( "IsCorrectDisplayableManager: No selection node in the scene." );
    return false;
    }
  if ( selectionNode->GetActiveMarkupsNodeType() == 0)
    {
    //vtkErrorMacro ( "IsCorrectDisplayableManager: no active markups");
    return false;
    }
  // the purpose of the displayableManager is hardcoded
  return !strcmp(selectionNode->GetActiveMarkupsNodeType(), this->Focus);

}

//---------------------------------------------------------------------------
// Functions to overload!
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::OnClickInRenderWindow(double vtkNotUsed(x), double vtkNotUsed(y), const char * vtkNotUsed(associatedNodeID))
{

  // The user clicked in the renderWindow
  vtkErrorMacro("OnClickInRenderWindow should be overloaded!");
}

//---------------------------------------------------------------------------
vtkAbstractWidget* vtkMRMLMarkupsDisplayableManager::CreateWidget(vtkMRMLMarkupsNode* vtkNotUsed(node))
{

  // A widget should be created here.
  vtkErrorMacro("CreateWidget should be overloaded!");
  return 0;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::OnWidgetCreated(vtkAbstractWidget* vtkNotUsed(widget), vtkMRMLMarkupsNode* vtkNotUsed(node))
{

  // Actions after a widget was created should be executed here.
  vtkErrorMacro("OnWidgetCreated should be overloaded!");
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::PropagateMRMLToWidget(vtkMRMLMarkupsNode* vtkNotUsed(node), vtkAbstractWidget* vtkNotUsed(widget))
{
  // update the widget visibility according to the mrml settings of visibility
  // status for 2d (taking into account the current slice) and 3d
  //this->UpdateWidgetVisibility(markupsNode);
  // The properties of a widget should be set here, subclasses should call this.
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget* vtkNotUsed(widget), vtkMRMLMarkupsNode* vtkNotUsed(node))
{
  // The properties of a widget should be set here.
  vtkErrorMacro("PropagateWidgetToMRML should be overloaded!");
}

//---------------------------------------------------------------------------
/// Convert world coordinates to local using mrml parent transform
void vtkMRMLMarkupsDisplayableManager::GetWorldToLocalCoordinates(vtkMRMLMarkupsNode *node,
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
    vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
    transformToWorld->Identity();
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    lnode->GetMatrixTransformToWorld(transformToWorld);
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
vtkAbstractWidget * vtkMRMLMarkupsDisplayableManager::AddWidget(vtkMRMLMarkupsNode *markupsNode)
{
  vtkWarningMacro("AddWidget: calling create widget");
  vtkAbstractWidget* newWidget = this->CreateWidget(markupsNode);
  if (!newWidget)
    {
    vtkErrorMacro("AddWidget: unable to create a new widget for markups node " << markupsNode->GetID());
    return NULL;
    }

  // record the mapping between node and widget in the helper
  this->Helper->RecordWidgetForNode(newWidget,markupsNode);

  vtkWarningMacro("AddWidget: saved to helper ");
//  vtkIndent indent;
//  this->Helper->PrintSelf(std::cout, indent);

  // Refresh observers
  this->SetAndObserveNode(markupsNode);

  // TODO do we need this render call?
  this->RequestRender();
  
  // ?
  this->OnWidgetCreated(newWidget, markupsNode);

  // now set up the new widget
  this->PropagateMRMLToWidget(markupsNode, newWidget);
  
  return newWidget;
}

