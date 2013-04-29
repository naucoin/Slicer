
// MarkupsModule/Logic includes
//#include <vtkSlicerMarkupsLogic.h>

// MarkupsModule/MRML includes
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMarkupsDisplayNode.h>
//#include <vtkMRMLMarkupsFiducialDisplayNode.h>

// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsFiducialDisplayableManager2D.h"

// MarkupsModule/VTKWidgets includes
#include <vtkMarkupsGlyphSource2D.h>

// MRMLDisplayableManager includes
#include <vtkSliceViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkFollower.h>
#include <vtkHandleRepresentation.h>
#include <vtkInteractorStyle.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOrientedPolygonalHandleRepresentation3D.h>
#include <vtkPointHandleRepresentation2D.h>
#include <vtkProperty2D.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>
#include <vtkSeedRepresentation.h>
#include <vtkSphereSource.h>

// STD includes
#include <sstream>
#include <string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLMarkupsFiducialDisplayableManager2D);
vtkCxxRevisionMacro (vtkMRMLMarkupsFiducialDisplayableManager2D, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// vtkMRMLMarkupsFiducialDisplayableManager2D Callback
/// \ingroup Slicer_QtModules_Markups
class vtkMarkupsFiducialWidgetCallback : public vtkCommand
{
public:
  static vtkMarkupsFiducialWidgetCallback *New()
  { return new vtkMarkupsFiducialWidgetCallback; }

  vtkMarkupsFiducialWidgetCallback(){}

  virtual void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void *callData)
  {
    if (event ==  vtkCommand::PlacePointEvent)
      {
      // std::cout << "Warning: PlacePointEvent not supported" << std::endl;
      }
    else if ((event == vtkCommand::EndInteractionEvent) || (event == vtkCommand::InteractionEvent))
      {
      // sanity checks
      if (!this->DisplayableManager)
        {
        return;
        }
      if (!this->Node)
        {
        return;
        }
      if (!this->Widget)
        {
        return;
        }
      // sanity checks end

      // restrict the widget to the renderer
        
      // we need the widgetRepresentation
      vtkSeedRepresentation * representation = vtkSeedRepresentation::SafeDownCast(this->Widget->GetRepresentation());
      if (!representation)
        {
        std::cerr << "Representation is null.\n";
        return;
        }
        
      double displayCoordinates1[4];
      if (callData != NULL)
        {
        // have a single seed that moved
        unsigned int *n =  reinterpret_cast<unsigned int *>(callData);
        if (n ||
            (n == 0 && representation->GetNumberOfSeeds() == 1))
          {
          // first, we get the current displayCoordinates of the points
          representation->GetSeedDisplayPosition(*n,displayCoordinates1);
          
          // second, we copy these to restrictedDisplayCoordinates
          double restrictedDisplayCoordinates1[4] = {displayCoordinates1[0], displayCoordinates1[1], displayCoordinates1[2], displayCoordinates1[3]};
          
          // modify restrictedDisplayCoordinates 1 and 2, if these are outside the viewport of the current renderer
          this->DisplayableManager->RestrictDisplayCoordinatesToViewport(restrictedDisplayCoordinates1);
          
          // only if we had to restrict the coordinates aka. if the coordinates changed, we update the positions
          if (this->DisplayableManager->GetDisplayCoordinatesChanged(displayCoordinates1,restrictedDisplayCoordinates1))
            {
            representation->SetSeedDisplayPosition(*n,restrictedDisplayCoordinates1);
            }
          }
        }
      
      }

    if (event == vtkCommand::EndInteractionEvent)
      {
      // save the state of the node when done moving, then call
      // PropagateWidgetToMRML to update the node one last time
      if (this->Node->GetScene())
        {
        this->Node->GetScene()->SaveStateForUndo(this->Node);
        }
      }
    // the interaction with the widget ended, now propagate the changes to MRML
    this->DisplayableManager->PropagateWidgetToMRML(this->Widget, this->Node);
  }

  void SetWidget(vtkAbstractWidget *w)
    {
    this->Widget = w;
    }
  void SetNode(vtkMRMLMarkupsNode *n)
    {
    this->Node = n;
    }
  void SetDisplayableManager(vtkMRMLMarkupsDisplayableManager2D * dm)
    {
    this->DisplayableManager = dm;
    }

  vtkAbstractWidget * Widget;
  vtkMRMLMarkupsNode * Node;
  vtkMRMLMarkupsDisplayableManager2D * DisplayableManager;
};

//---------------------------------------------------------------------------
// vtkMRMLMarkupsFiducialDisplayableManager2D methods

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  this->Helper->PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new seed widget.
vtkAbstractWidget * vtkMRMLMarkupsFiducialDisplayableManager2D::CreateWidget(vtkMRMLMarkupsNode* node)
{

  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!")
    return 0;
    }

  // 2d glyphs and text need to be scaled by 1/300 to show up properly in the 2d slice windows
  this->SetScaleFactor2D(0.0033);

  vtkMRMLMarkupsFiducialNode* fiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
    {
    vtkErrorMacro("CreateWidget: Could not get fiducial node!")
    return 0;
    }

  vtkMRMLMarkupsDisplayNode *displayNode = fiducialNode->GetMarkupsDisplayNode();

  if (!displayNode)
    {
    // std::cout<<"No DisplayNode!"<<std::endl;
    }

  // unset the glyph type which can be necessary when recreating a widget due to 2d/3d swap
  std::map<vtkMRMLNode*, int>::iterator iter  = this->NodeGlyphTypes.find(displayNode);
  if (iter != this->NodeGlyphTypes.end())
    {
    vtkDebugMacro("CreateWidget: found a glyph type already defined for this node: " << iter->second);
    this->NodeGlyphTypes[displayNode] = vtkMRMLMarkupsDisplayNode::GlyphMin - 1;
    }

  vtkNew<vtkSeedRepresentation> rep;

  if (!this->IsInLightboxMode())
    {
    vtkDebugMacro("CreateWidget: not in light box mode, making a 3d handle");
    vtkNew<vtkOrientedPolygonalHandleRepresentation3D> handle;


    // default to a sphere glyph, update in propagate mrml to widget
    vtkNew<vtkMarkupsGlyphSource2D> glyphSource;
    glyphSource->SetGlyphType(vtkMRMLMarkupsDisplayNode::Sphere3D);
    glyphSource->Update();
    glyphSource->SetScale(1.0);
    handle->SetHandle(glyphSource->GetOutput());
    rep->SetHandleRepresentation(handle.GetPointer());
    }
  else
    {
    vtkDebugMacro("CreateWidget: in light box mode, making a 2d handle");
    vtkNew<vtkPointHandleRepresentation2D> handle;
    rep->SetHandleRepresentation(handle.GetPointer());
    }

  //seed widget
  vtkSeedWidget * seedWidget = vtkSeedWidget::New();
  seedWidget->CreateDefaultRepresentation();

  seedWidget->SetRepresentation(rep.GetPointer());

  seedWidget->SetInteractor(this->GetInteractor());
  // set the renderer on the widget and representation
  if (!this->IsInLightboxMode())
    {
    seedWidget->SetCurrentRenderer(this->GetRenderer());
    seedWidget->GetRepresentation()->SetRenderer(this->GetRenderer());
    }
  else
    {
    int lightboxIndex = this->GetLightboxIndex(fiducialNode, 0,0);
    seedWidget->SetCurrentRenderer(this->GetRenderer(lightboxIndex));
    seedWidget->GetRepresentation()->SetRenderer(this->GetRenderer(lightboxIndex));
    }


  //seedWidget->ProcessEventsOff();

  // create a new handle
  vtkHandleWidget* newhandle = seedWidget->CreateNewHandle();
  if (!newhandle)
    {
    vtkErrorMacro("CreateWidget: error creaing a new handle!");
    }
  vtkDebugMacro("Fids CreateWidget: Created widget for node " << fiducialNode->GetID() << " with a representation");
  
  // init the widget from the mrml node
  //this->PropagateMRMLToWidget(fiducialNode, seedWidget);

  seedWidget->CompleteInteraction();

  return seedWidget;

  }

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLMarkupsFiducialDisplayableManager2D::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLMarkupsNode * node)
{

  if (!widget)
    {
    vtkErrorMacro("OnWidgetCreated: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("OnWidgetCreated: MRML node was null!")
    return;
    }

  // add the callback
  vtkMarkupsFiducialWidgetCallback *myCallback = vtkMarkupsFiducialWidgetCallback::New();
  myCallback->SetNode(node);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  widget->AddObserver(vtkCommand::InteractionEvent,myCallback);
  myCallback->Delete();

}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsFiducialDisplayableManager2D::UpdateNthMarkupPosition(int n, vtkAbstractWidget *widget, vtkMRMLMarkupsNode *pointsNode)
{
  if (!pointsNode || !widget)
    {
    return false;
    }
  if (n > pointsNode->GetNumberOfMarkups())
    {
    return false;
    }
  vtkSeedWidget *seedWidget = vtkSeedWidget::SafeDownCast(widget);
  if (!seedWidget)
    {
    return false;
    }
  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
  if (!seedRepresentation)
    {
    return false;
    }
  bool positionChanged = false;
  
  // for 2d managers, compare the display positions
  double displayCoordinates1[4];
  double displayCoordinatesBuffer1[4];
    
  // get point in world coordinates using parent transforms
  double pointTransformed[4];
  // always only one point in a fiducial
  pointsNode->GetMarkupPointWorld(n, 0, pointTransformed);
  
  this->GetWorldToDisplayCoordinates(pointTransformed,displayCoordinates1);
  
  seedRepresentation->GetSeedDisplayPosition(n,displayCoordinatesBuffer1);
  
  if (this->GetDisplayCoordinatesChanged(displayCoordinates1,displayCoordinatesBuffer1))
    {
    // only update when really changed
    vtkDebugMacro("UpdateNthMarkupPosition: " << this->GetSliceNode()->GetName() << ": display coordinates changed:\n\tseed display = " << displayCoordinatesBuffer1[0] << ", " << displayCoordinatesBuffer1[1] << "\n\tfid display =  " << displayCoordinates1[0] << ", " << displayCoordinates1[1] );
    seedRepresentation->SetSeedDisplayPosition(n,displayCoordinates1);
    positionChanged = true;
    }
  else
    {
    vtkDebugMacro("UpdateNthMarkupPosition: " <<  this->GetSliceNode()->GetName() << ": display coordinates unchanged!");
    }
  return positionChanged;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager2D::SetNthSeed(int n, vtkMRMLMarkupsFiducialNode* fiducialNode, vtkSeedWidget *seedWidget)
{

  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());

  if (!seedRepresentation)
    {
    vtkErrorMacro("SetNthSeed: no seed representation in widget!");
    return;
    }

  vtkMRMLMarkupsDisplayNode *displayNode = fiducialNode->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    vtkDebugMacro("SetNthSeed: Could not get display node for node " << (fiducialNode->GetID() ? fiducialNode->GetID() : "null id"));
    return;
    }

  int numberOfHandles = seedRepresentation->GetNumberOfSeeds();
  vtkDebugMacro("SetNthSeed, n = " << n << ", number of handles = " << numberOfHandles);
  
  // does this handle need to be created?
  bool createdNewHandle = false;
  if (n >= numberOfHandles)
    {
    // create a new handle
    vtkHandleWidget* newhandle = seedWidget->CreateNewHandle();
    if (!newhandle)
      {
      vtkErrorMacro("SetNthSeed: error creaing a new handle!");
      }
    else
      {
      // std::cout << "SetNthSeed: created a new handle,number of seeds = " << seedRepresentation->GetNumberOfSeeds() << std::endl;
      createdNewHandle = true;
      newhandle->ManagesCursorOff();
      if (newhandle->GetEnabled() == 0)
        {
        vtkDebugMacro("turning on the new handle");
        newhandle->EnabledOn();
        }
      }
    }

  // can have a 3d or 2d handle depending on if in light box mode or not
  vtkOrientedPolygonalHandleRepresentation3D *handleRep = vtkOrientedPolygonalHandleRepresentation3D::SafeDownCast(seedRepresentation->GetHandleRepresentation(n));
  vtkPointHandleRepresentation2D *pointHandleRep = vtkPointHandleRepresentation2D::SafeDownCast(seedRepresentation->GetHandleRepresentation(n));

  // update the postion
  bool positionChanged = this->UpdateNthMarkupPosition(n, seedWidget, fiducialNode);
  vtkDebugMacro("Position changed? " << (positionChanged ? "yes" : "no"));
  
  if (!handleRep && !pointHandleRep)
    {
    vtkErrorMacro("Failed to get a handle rep for n = " << n << ", number of seeds = " <<  seedRepresentation->GetNumberOfSeeds() << ", handle rep = " << (seedRepresentation->GetHandleRepresentation(n) ? seedRepresentation->GetHandleRepresentation(n)->GetClassName() : "null"));
    return;
    }

  // visibility for this handle, hide it if the whole list is invisible,
  // this fid is invisible, or the fid isn't visible on this slice
  bool fidVisible = true;
  if (displayNode->GetVisibility() == 0 ||
      fiducialNode->GetNthFiducialVisibility(n) == 0 ||
      this->IsWidgetDisplayableOnSlice(fiducialNode, n) == 0)
    {
    // std::cout << "fidVisible set to FALSE (disp node visib = " << displayNode->GetVisibility() << ", nth fid visib = " << fiducialNode->GetNthFiducialVisibility(n) << ", visb on slice = " << this->IsWidgetDisplayableOnSlice(fiducialNode, n) << ")" << std::endl;
    fidVisible = false;
    }

  if (handleRep)
    {
    // set the glyph type if a new handle was created, or the glyph type changed
    std::map<vtkMRMLNode*, int>::iterator iter  = this->NodeGlyphTypes.find(displayNode);
    if (createdNewHandle ||
        iter == this->NodeGlyphTypes.end() || iter->second != displayNode->GetGlyphType())
      {
      vtkDebugMacro("DisplayNode glyph type = " << displayNode->GetGlyphType() << " = " << displayNode->GetGlyphTypeAsString() << ", is 3d glyph = " << (displayNode->GlyphTypeIs3D() ? "true" : "false") << ", is 2d disp manager.");
      // std::cout << "SetNthSeed " << n << ": Display node glyph type = " << displayNode->GetGlyphType() << std::endl;
      if (displayNode->GlyphTypeIs3D())
        {
        // map the 3d sphere to a filled circle, the 3d diamond to a filled
        // diamond
        vtkNew<vtkMarkupsGlyphSource2D> glyphSource;
        if (displayNode->GetGlyphType() == vtkMRMLMarkupsDisplayNode::Sphere3D)
          {
          // std::cout << "using circle 2d for sphere 3d" << std::endl;
          glyphSource->SetGlyphType(vtkMRMLMarkupsDisplayNode::Circle2D);
          }
        else if (displayNode->GetGlyphType() == vtkMRMLMarkupsDisplayNode::Diamond3D)
          {
          glyphSource->SetGlyphType(vtkMRMLMarkupsDisplayNode::Diamond2D);
          }
        else
          {
          glyphSource->SetGlyphType(vtkMRMLMarkupsDisplayNode::StarBurst2D);
          // std::cout << "2d starburst" << std::endl;
          }
        glyphSource->Update();
        glyphSource->SetScale(1.0);
        handleRep->SetHandle(glyphSource->GetOutput());
        }//if (displayNode->GlyphTypeIs3D())
      else
        {
        // 2D
        vtkNew<vtkMarkupsGlyphSource2D> glyphSource;
        glyphSource->SetGlyphType(displayNode->GetGlyphType());
        glyphSource->Update();
        glyphSource->SetScale(1.0);
        handleRep->SetHandle(glyphSource->GetOutput());
        }
//    if (!createdNewHandle)
      {
      // TBD: keep with the assumption of one glyph type per markups node,
      // that each seed has to have the same type, but update if necessary
      this->NodeGlyphTypes[displayNode] = displayNode->GetGlyphType();
      }
    }  // end of glyph type

    // set the color
    vtkProperty *prop = NULL;
    prop = handleRep->GetProperty();
    if (prop)
      {
      if (fiducialNode->GetNthFiducialSelected(n))
        {
        // use the selected color
        prop->SetColor( displayNode->GetSelectedColor());
        }
      else
        {
        // use the unselected color
        prop->SetColor(displayNode->GetColor());
        }
      
      // set the material properties
      prop->SetOpacity(displayNode->GetOpacity());
      prop->SetAmbient(displayNode->GetAmbient());
      prop->SetDiffuse(displayNode->GetDiffuse());
      prop->SetSpecular(displayNode->GetSpecular());
      }

    // set the scaling
    // the following is only needed since we require a different uniform scale depending on 2D and 3D
    handleRep->SetUniformScale(displayNode->GetGlyphScale()*this->GetScaleFactor2D());

    /// if the text is visible
    std::string textString = fiducialNode->GetNthFiducialLabel(n);
    // update the text string

    if (textString.compare(handleRep->GetLabelText()) != 0)
      {
      handleRep->SetLabelText(textString.c_str());
      }
    // scale the text
    if (textString.compare("") != 0)
      {
      // scale the text
      double textscale[3] = {displayNode->GetTextScale(), displayNode->GetTextScale(), displayNode->GetTextScale()};

      // scale it down for the 2d windows
      textscale[0] *= this->GetScaleFactor2D();
      textscale[1] *= this->GetScaleFactor2D();
      textscale[2] *= this->GetScaleFactor2D();
      
      handleRep->SetLabelTextScale(textscale);
      }
    // set the text colors
    if (handleRep->GetLabelTextActor())
      {
      // set the colours
      if (fiducialNode->GetNthFiducialSelected(n))
        {
        handleRep->GetLabelTextActor()->GetProperty()->SetColor(displayNode->GetSelectedColor());
        }
      else
        {
        handleRep->GetLabelTextActor()->GetProperty()->SetColor(displayNode->GetColor());
        }
      handleRep->GetLabelTextActor()->GetProperty()->SetOpacity(displayNode->GetOpacity());
      }
  
    // set the text visibility
    if (fidVisible)
      {
      handleRep->VisibilityOn();
      handleRep->HandleVisibilityOn();
      if (textString.compare("") != 0)
        {
        handleRep->LabelVisibilityOn();
        }
      seedWidget->GetSeed(n)->EnabledOn();
      }
    else
      {
      handleRep->VisibilityOff();
      handleRep->HandleVisibilityOff();
      handleRep->LabelVisibilityOff();
      seedWidget->GetSeed(n)->EnabledOff();
      }
    // update locked
    int locked = fiducialNode->GetLocked();
    handleRep->SetPickable(!locked);
    handleRep->SetDragable(!locked);
    
    }
  else if (pointHandleRep)
    {
    // set the glyph type - TBD, swapping isn't working
    // set the color
    if (fiducialNode->GetNthFiducialSelected(n))
      { 
      // use the selected color
      pointHandleRep->GetProperty()->SetColor(displayNode->GetSelectedColor());
      }
    else
      {
      // use the unselected color
      pointHandleRep->GetProperty()->SetColor(displayNode->GetColor());
      }
    }
}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLMarkupsFiducialDisplayableManager2D::PropagateMRMLToWidget(vtkMRMLMarkupsNode* node, vtkAbstractWidget * widget)
{
  if (!widget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateMRMLToWidget: MRML node was null!")
    return;
    }

  // cast to the specific widget
  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);

  if (!seedWidget)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get seed widget!")
    return;
    }

  if (seedWidget->GetWidgetState() != vtkSeedWidget::MovingSeed)
    {
    // ignore events not caused by seed movement
    // return;
    std::cout << "2D: Seed widget state is not moving: state = " << seedWidget->GetWidgetState() << " != " << vtkSeedWidget::MovingSeed << std::endl;
    }

  // cast to the specific mrml node
  vtkMRMLMarkupsFiducialNode* fiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get fiducial node!")
    return;
    }

  // disable processing of modified events
  this->Updating = 1;

  
  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  

  vtkMRMLMarkupsDisplayNode *displayNode = fiducialNode->GetMarkupsDisplayNode();

  if (!displayNode)
    {
    vtkDebugMacro("PropagateMRMLToWidget: Could not get display node for node " << (fiducialNode->GetID() ? fiducialNode->GetID() : "null id"));
    }

  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
  if (!seedRepresentation)
    {
    vtkErrorMacro("Unable to get the seed representation!");
    return;
    }

  // can have a 3d or 2d handle depending on if in light box mode or not
  vtkOrientedPolygonalHandleRepresentation3D *handleRep = vtkOrientedPolygonalHandleRepresentation3D::SafeDownCast(seedRepresentation->GetHandleRepresentation());
  // might be in lightbox mode where using a 2d point handle
  vtkPointHandleRepresentation2D *pointHandleRep = vtkPointHandleRepresentation2D::SafeDownCast(seedRepresentation->GetHandleRepresentation());
  // double check that if switch in and out of light box mode, the handle rep
  // is updated
  bool updateHandleType = false;
  if (this->IsInLightboxMode())
    {
    if (handleRep)
      {
      vtkDebugMacro("SetNthSeed: have a 3d handle representation in 2d light box, resetting it.");
      updateHandleType = true;
      }
    }
  else
    {
    if (pointHandleRep)
      {
      vtkDebugMacro("SetNthSeed: Not in light box, but have a point handle.");
      updateHandleType = true;     
      }
    }
  if (updateHandleType)
    {
    // in annotations module, it deletes the whole thing and re-calls PropagateMRMLToWidget
    vtkWarningMacro("WARNING: need to update handle type!!!!");
    }

  // iterate over the fiducials in this markup
  int numberOfFiducials = fiducialNode->GetNumberOfMarkups();
  
  vtkDebugMacro("Fids PropagateMRMLToWidget, node num markups = " << numberOfFiducials);
  
  for (int n = 0; n < numberOfFiducials; n++)
    {
    // std::cout << "Fids PropagateMRMLToWidget: n = " << n << std::endl;
    this->SetNthSeed(n, fiducialNode, seedWidget);
    }

  // now update the position of all the seeds - done in SetNthSeed now
  //this->UpdatePosition(widget, node);

  // update lock status
  this->Helper->UpdateLocked(node);

  // update visibility of widget as a whole
  // std::cout << "PropagateMRMLToWidget: calling UpdateWidgetVisibility" << std::endl;
  this->UpdateWidgetVisibility(node);

  if (seedRepresentation)
    {
    seedRepresentation->NeedToRenderOn();
    }
  seedWidget->Modified();

//  seedWidget->CompleteInteraction();

  // enable processing of modified events
  this->Updating = 0;


}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLMarkupsFiducialDisplayableManager2D::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLMarkupsNode* node)
{

  if (!widget)
    {
    vtkErrorMacro("PropagateWidgetToMRML: Widget was null!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("PropagateWidgetToMRML: MRML node was null!")
    return;
    }

  // cast to the specific widget
  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);

  if (!seedWidget)
   {
   vtkErrorMacro("PropagateWidgetToMRML: Could not get seed widget!")
   return;
   }

  // cast to the specific mrml node
  vtkMRMLMarkupsFiducialNode* fiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(node);

  if (!fiducialNode)
   {
   vtkErrorMacro("PropagateWidgetToMRML: Could not get fiducial node!")
   return;
   }

  // disable processing of modified events
  this->Updating = 1;
  // this was stopping PointModifiedEvent from being invoked, need that to
  // update the GUI
//  int disabledModify = fiducialNode->StartModify();


  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
  int numberOfSeeds = seedRepresentation->GetNumberOfSeeds();

  bool positionChanged = false;
  for (int n = 0; n < numberOfSeeds; n++)
    {
    double worldCoordinates1[4];

    // 2D widget was changed
    
    double displayCoordinates1[4];
    seedRepresentation->GetSeedDisplayPosition(n,displayCoordinates1);
    vtkDebugMacro("PropagateWidgetToMRML: 2d displayable manager: widget display coords = " << displayCoordinates1[0] << ", " << displayCoordinates1[1] << ", " << displayCoordinates1[2]);
    this->GetDisplayToWorldCoordinates(displayCoordinates1,worldCoordinates1);
    vtkDebugMacro("PropagateWidgetToMRML: 2d: widget seed " << n << " world coords = " << worldCoordinates1[0] << ", " << worldCoordinates1[1] << ", "<< worldCoordinates1[2]);
      
    // was there a change?
    double currentCoordinates[4];
    fiducialNode->GetNthFiducialWorldCoordinates(n,currentCoordinates);
    vtkDebugMacro("PropagateWidgetToMRML: fiducial " << n << " current world coordinates = " << currentCoordinates[0] << ", " << currentCoordinates[1] << ", " << currentCoordinates[2]);

    double currentCoords[3];
    currentCoords[0] = currentCoordinates[0];
    currentCoords[1] = currentCoordinates[1];
    currentCoords[2] = currentCoordinates[2];
    double newCoords[3];
    newCoords[0] = worldCoordinates1[0];
    newCoords[1] = worldCoordinates1[1];
    newCoords[2] = worldCoordinates1[2];
    if (this->GetWorldCoordinatesChanged(currentCoords, newCoords))
      {
      vtkDebugMacro("PropagateWidgetToMRML: position changed.");
      positionChanged = true;
      }

    if (positionChanged)
      {
      vtkDebugMacro("PropagateWidgetToMRML: position changed, setting fiducial coordinates");
      fiducialNode->SetNthFiducialWorldCoordinates(n,worldCoordinates1);
      }
    }
  
  // enable processing of modified events
//  fiducialNode->EndModify(disabledModify);

  // did any of the positions change?
  if (positionChanged)
    {
    vtkDebugMacro("PropagateWidgetToMRML: position changed, calling point modified on the fiducial node");
    fiducialNode->Modified();
    //fiducialNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent,fiducialNode);
    fiducialNode->GetScene()->InvokeEvent(vtkMRMLMarkupsNode::PointModifiedEvent,fiducialNode);
    }
  // This displayableManager should now consider ModifiedEvent again
  this->Updating = 0;
}

//---------------------------------------------------------------------------
/// Create a markupsMRMLnode
void vtkMRMLMarkupsFiducialDisplayableManager2D::OnClickInRenderWindow(double x, double y, const char *associatedNodeID)
{
  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    vtkDebugMacro("OnClickInRenderWindow: x = " << x << ", y = " << y << ", incorrect displayable manager, focus = " << this->Focus << ", jumping out");
    return;
    }

  // place the seed where the user clicked
  vtkDebugMacro("OnClickInRenderWindow: placing seed at " << x << ", " << y);
  // switch to updating state to avoid events mess
  this->Updating = 1;

  double displayCoordinates1[2];
  displayCoordinates1[0] = x;
  displayCoordinates1[1] = y;


  double worldCoordinates1[4];

  this->GetDisplayToWorldCoordinates(displayCoordinates1,worldCoordinates1);

  // Is there an active markups node that's a fiducial node?
  vtkMRMLMarkupsFiducialNode *activeFiducialNode = NULL;
  
  vtkMRMLSelectionNode *selectionNode = this->GetSelectionNode();
  if (selectionNode)
    {
    const char *activeMarkupsID = selectionNode->GetActivePlaceNodeID();
    vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(activeMarkupsID);
    if (mrmlNode &&
        mrmlNode->IsA("vtkMRMLMarkupsFiducialNode"))
      {
      activeFiducialNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(mrmlNode);
      }
    else
      {
      vtkErrorMacro("OnClickInRenderWindow: active markup id = " << (activeMarkupsID ? activeMarkupsID : "null") << ", mrml node is " << (mrmlNode ? mrmlNode->GetID() : "null") << ", not a vtkMRMLMarkupsFiducialNode");
      }
    }

  bool newNode = false;
  if (!activeFiducialNode)
    {
    newNode = true;
    // create the MRML node
    activeFiducialNode = vtkMRMLMarkupsFiducialNode::New();
    activeFiducialNode->SetName("Fiducial List");
    }
  
  // add a fiducial: this will trigger an update of the widgets
  int fiducialIndex = activeFiducialNode->AddMarkupWithNPoints(1);
  if (fiducialIndex == -1)
    {
    vtkErrorMacro("OnClickInRenderWindow: unable to add a fiducial to active fiducial list!");
    if (newNode)
      {
      activeFiducialNode->Delete();
      }
    return;
    }
  // set values on it
  activeFiducialNode->SetNthFiducialWorldCoordinates(fiducialIndex,worldCoordinates1);
  // std::cout << "OnClickInRenderWindow: Setting " << fiducialIndex << "th fiducial label from " << activeFiducialNode->GetNthFiducialLabel(fiducialIndex);
  if (!activeFiducialNode->GetUseListNameForMarkups())
    {
    int numberOfMarkups = activeFiducialNode->GetNumberOfMarkups();
    std::stringstream ss;
    ss << numberOfMarkups;
    std::string numberString;
    ss >> numberString;
    std::string labelString = std::string("F") + numberString;
    activeFiducialNode->SetNthFiducialLabel(fiducialIndex, labelString);
    }
  // std::cout << " to "  << activeFiducialNode->GetNthFiducialLabel(fiducialIndex) << std::endl;

  // reset updating state
  this->Updating = 0;

  // if this was a one time place, go back to view transform mode
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (interactionNode && interactionNode->GetPlaceModePersistence() != 1)
    {
    vtkDebugMacro("End of one time place, place mode persistence = " << interactionNode->GetPlaceModePersistence());
    interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
    }

  // save for undo and add the node to the scene after any reset of the
  // interaction node so that don't end up back in place mode
  this->GetMRMLScene()->SaveStateForUndo();

  // is there a node associated with this?
  if (associatedNodeID)
    {
    activeFiducialNode->SetNthFiducialAssociatedNodeID(fiducialIndex, associatedNodeID);
    }

  if (newNode)
    {
    // create a display node and add node and display node to scene
    vtkMRMLMarkupsDisplayNode *displayNode = vtkMRMLMarkupsDisplayNode::New();
    this->GetMRMLScene()->AddNode(displayNode);
    activeFiducialNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    this->GetMRMLScene()->AddNode(activeFiducialNode);

    // save it as the active markups list
    if (selectionNode)
      {
      selectionNode->SetActivePlaceNodeID(activeFiducialNode->GetID());
      }
    // clean up
    displayNode->Delete();
    activeFiducialNode->Delete();
    }
}

//---------------------------------------------------------------------------
/// observe key press events
void vtkMRMLMarkupsFiducialDisplayableManager2D::AdditionnalInitializeStep()
{
  // don't add the key press event, as it triggers a crash on start up
  //vtkDebugMacro("Adding an observer on the key press event");
  this->AddInteractorStyleObservableEvent(vtkCommand::KeyPressEvent);
}


//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager2D::OnInteractorStyleEvent(int eventid)
{
  this->Superclass::OnInteractorStyleEvent(eventid);

  if (this->GetDisableInteractorStyleEventsProcessing())
    {
    vtkWarningMacro("OnInteractorStyleEvent: Processing of events was disabled.")
    return;
    }

  if (eventid == vtkCommand::KeyPressEvent)
    {
    char *keySym = this->GetInteractor()->GetKeySym();
    vtkDebugMacro("OnInteractorStyleEvent 2D: key press event position = " << this->GetInteractor()->GetEventPosition()[0] << ", " << this->GetInteractor()->GetEventPosition()[1] << ", key sym = " << (keySym == NULL ? "null" : keySym));
    if (!keySym)
      {
      return;
      }
    if (strcmp(keySym, "p") == 0)
      {
      if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
        {
        this->OnClickInRenderWindowGetCoordinates();
        }
      else
        {
        vtkDebugMacro("Fiducial DisplayableManager: key press p, but not in Place mode! Returning.");
        return;
        }
      }
    }
  else if (eventid == vtkCommand::KeyReleaseEvent)
    {
    vtkDebugMacro("Got a key release event");
    }
}


//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager2D::UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node)
{
//  vtkWarningMacro("UpdatePosition, node is " << (node == NULL ? "null" : node->GetID()));
  if (!node)
    {
    return;
    }
  vtkMRMLMarkupsNode *pointsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!pointsNode)
    {
    vtkErrorMacro("UpdatePosition - Can not access control points node from node with id " << node->GetID());
    return;
    }
  // get the widget
  if (!widget)
    {
    vtkErrorMacro("UpdatePosition: no widget associated with points node " << pointsNode->GetID());
    return;
    }
  // cast to a seed widget
  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);

  if (!seedWidget)
   {
   vtkErrorMacro("UpdatePosition: Could not get seed widget!")
   return;
   }

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
 
  
  // disable processing of modified events
  //this->Updating = 1;
  bool positionChanged = false;
  int numberOfFiducials = pointsNode->GetNumberOfMarkups();
  for (int n = 0; n < numberOfFiducials; n++)
    {
    if (this->UpdateNthMarkupPosition(n, seedWidget, pointsNode))
      {
      positionChanged = true;
      }
    }
  // did any of the positions change?
  if (positionChanged && this->Updating == 0)
    {
    // not already updating from propagate mrml to widget, so trigger a render
    vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
    seedRepresentation->NeedToRenderOn();
    seedWidget->Modified();
//    seedWidget->CompleteInteraction();
    }
  // enable processing of modified events
  //this->Updating = 0;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager2D::OnMRMLSceneEndClose()
{
  // clear out the map of glyph types
  this->NodeGlyphTypes.clear();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager2D::OnMRMLMarkupsNodeNthMarkupModifiedEvent(vtkMRMLMarkupsNode* node, int n)
{
  int numberOfMarkups = node->GetNumberOfMarkups();
  if (n < 0 || n >= numberOfMarkups)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeNthMarkupModifiedEvent: n = " << n << " is out of range 0-" << numberOfMarkups);
    return;
    }
  
  vtkAbstractWidget *widget = this->Helper->GetWidget(node);
  if (!widget)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeNthMarkupModifiedEvent: a markup was added to a node that doesn't already have a widget! Returning..");
    return;
    }
  
  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);
  if (!seedWidget)
   {
   vtkErrorMacro("OnMRMLMarkupsNodeNthMarkupModifiedEvent: Could not get seed widget!")
   return;
   }
  this->SetNthSeed(n, vtkMRMLMarkupsFiducialNode::SafeDownCast(node), seedWidget);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialDisplayableManager2D::OnMRMLMarkupsNodeMarkupAddedEvent(vtkMRMLMarkupsNode * markupsNode)
{
  vtkDebugMacro("OnMRMLMarkupsNodeMarkupAddedEvent");

  if (!markupsNode)
    {
    return;
    }
  vtkAbstractWidget *widget = this->Helper->GetWidget(markupsNode);
  if (!widget)
    {
    // TBD: create a widget?
    vtkErrorMacro("OnMRMLMarkupsNodeMarkupAddedEvent: a markup was added to a node that doesn't already have a widget! Returning..");
    return;
    }
  
  vtkSeedWidget* seedWidget = vtkSeedWidget::SafeDownCast(widget);
  if (!seedWidget)
   {
   vtkErrorMacro("OnMRMLMarkupsNodeMarkupAddedEvent: Could not get seed widget!")
   return;
   }

  // this call will create a new handle and set it
  std::cout << "OnMRMLMarkupsNodeMarkupAddedEvent: adding to markups node that currently has " << markupsNode->GetNumberOfMarkups() << std::endl;
  int n = markupsNode->GetNumberOfMarkups() - 1;
  this->SetNthSeed(n, vtkMRMLMarkupsFiducialNode::SafeDownCast(markupsNode), seedWidget);

  vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(seedWidget->GetRepresentation());
  seedRepresentation->NeedToRenderOn();
  seedWidget->Modified();
}
