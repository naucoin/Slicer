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
#include <vtkMRMLMarkupsRulerNode.h>
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMarkupsRulerDisplayNode.h>

// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsRulerDisplayableManager3D.h"

// MarkupsModule/VTKWidgets includes
#include <vtkMarkupsGlyphSource2D.h>

// MRMLDisplayableManager includes
#include <vtkSliceViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLUnitNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkDistanceWidget.h>
#include <vtkDistanceRepresentation3D.h>
#include <vtkFollower.h>
#include <vtkHandleRepresentation.h>
#include <vtkInteractorStyle.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#if (VTK_MAJOR_VERSION >= 6)
#include <vtkPickingManager.h>
#endif
#include <vtkPointHandleRepresentation3D.h>
#include <vtkProperty2D.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>

// STD includes
#include <sstream>
#include <string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLMarkupsRulerDisplayableManager3D);

//---------------------------------------------------------------------------
// vtkMRMLMarkupsRulerDisplayableManager3D Callback
/// \ingroup Slicer_QtModules_Markups
class vtkMarkupsRulerWidgetCallback3D : public vtkCommand
{
public:
  static vtkMarkupsRulerWidgetCallback3D *New()
  { return new vtkMarkupsRulerWidgetCallback3D; }

  vtkMarkupsRulerWidgetCallback3D(){}

  virtual void Execute (vtkObject *vtkNotUsed(caller), unsigned long event, void *vtkNotUsed(callData))
  {
    if ((event == vtkCommand::EndInteractionEvent) || (event == vtkCommand::InteractionEvent))
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
    this->DisplayableManager->PropagateWidgetToMRML(this->Widget, this->Node, this->MarkupNumber);
  }

  void SetWidget(vtkAbstractWidget *w)
    {
    this->Widget = w;
    }
  void SetNode(vtkMRMLMarkupsNode *n)
    {
    this->Node = n;
    }
  void SetDisplayableManager(vtkMRMLMarkupsDisplayableManager3D * dm)
    {
    this->DisplayableManager = dm;
    }

  void SetMarkupNumber(int n)
    {
    this->MarkupNumber = n;
    }
  vtkAbstractWidget * Widget;
  vtkMRMLMarkupsNode * Node;
  vtkMRMLMarkupsDisplayableManager3D * DisplayableManager;
  int MarkupNumber;
};

//---------------------------------------------------------------------------
// vtkMRMLMarkupsRulerDisplayableManager3D methods

//---------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayableManager3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  this->Helper->PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
/// Create a new widget.
vtkAbstractWidget * vtkMRMLMarkupsRulerDisplayableManager3D::CreateWidget(vtkMRMLMarkupsNode* node)
{
  if (!node)
    {
    vtkErrorMacro("CreateWidget: Node not set!")
    return 0;
    }

  vtkMRMLMarkupsRulerNode *rulerNode = vtkMRMLMarkupsRulerNode::SafeDownCast(node);
  vtkMRMLMarkupsRulerDisplayNode *displayNode = (rulerNode ?
                                                 rulerNode->GetMarkupsRulerDisplayNode() :
                                                 NULL);

  vtkDistanceWidget * rulerWidget = vtkDistanceWidget::New();
  vtkNew<vtkPointHandleRepresentation3D> handle;

  vtkNew<vtkDistanceRepresentation3D> distanceRepresentation;
  rulerWidget->SetRepresentation(distanceRepresentation.GetPointer());

  distanceRepresentation->SetHandleRepresentation(handle.GetPointer());
  distanceRepresentation->InstantiateHandleRepresentation();
  distanceRepresentation->RulerModeOn();
  distanceRepresentation->SetRulerDistance(
    this->ApplyUnit(displayNode ? displayNode->GetTickSpacing() : 10.));

#if (VTK_MAJOR_VERSION >= 6)
  if (this->GetInteractor()->GetPickingManager())
    {
    if (!(this->GetInteractor()->GetPickingManager()->GetEnabled()))
      {
      // Managed picking is on by default on the ruler widget, but the interactor
      // will need to have it's picking manager turned on once ruler widgets are
      // going to be used to avoid dragging rulers that are behind others.
      // Enabling it before setting the interactor on the ruler widget seems to
      // work better with tests of two ruler lists.
      this->GetInteractor()->GetPickingManager()->EnabledOn();
      }
    }
#endif
  rulerWidget->SetInteractor(this->GetInteractor());
  rulerWidget->SetCurrentRenderer(this->GetRenderer());

  rulerWidget->SetWidgetStateToManipulate();
  rulerWidget->On();

  vtkDebugMacro("Ruler CreateWidget: Created widget for node " << rulerNode->GetID() << " with a representation");

  return rulerWidget;
  }

//---------------------------------------------------------------------------
/// Tear down the widget creation
void vtkMRMLMarkupsRulerDisplayableManager3D::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLMarkupsNode * node, int markupNumber)
{
  if (!widget)
    {
    vtkErrorMacro("OnWidgetCreated: Widget was null!")
    return;
    }
  vtkDistanceWidget *rulerWidget = vtkDistanceWidget::SafeDownCast(widget);
  if (!rulerWidget)
    {
    vtkErrorMacro("OnWidgetCreated: Widget was not a ruler widget!")
    return;
    }

  if (!node)
    {
    vtkErrorMacro("OnWidgetCreated: MRML node was null!")
    return;
    }

  vtkMRMLMarkupsRulerNode *rulerNode = vtkMRMLMarkupsRulerNode::SafeDownCast(node);
  if (!rulerNode)
    {
    vtkErrorMacro("OnWidgetCreated: couldn not get ruler node!");
    return;
    }

  // the widget thinks that the interaction has ended, now set the points from mrml
  double wc1[4] = {0,0,0,1};
  double wc2[4] = {0,0,0,1};
  rulerNode->GetNthRulerWorldCoordinatesInArrays(markupNumber, wc1, wc2);

  std::string format = this->GetLabelFormat(rulerNode, markupNumber);
  double distance = this->ApplyUnit(this->GetDistance(wc1, wc2));

  vtkDistanceRepresentation3D *rep =
    vtkDistanceRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation());
  if (rep)
    {
    rep->SetPoint1WorldPosition(wc1);
    rep->SetPoint2WorldPosition(wc2);

    rep->SetLabelFormat(format.c_str());
    rep->SetRulerDistance(distance);
    }

  // add the callback
  vtkMarkupsRulerWidgetCallback3D *myCallback = vtkMarkupsRulerWidgetCallback3D::New();
  myCallback->SetNode(node);
  myCallback->SetMarkupNumber(markupNumber);
  myCallback->SetWidget(widget);
  myCallback->SetDisplayableManager(this);
  widget->AddObserver(vtkCommand::EndInteractionEvent,myCallback);
  widget->AddObserver(vtkCommand::InteractionEvent,myCallback);
  myCallback->Delete();
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsRulerDisplayableManager3D::UpdateNthWidgetPositionFromMRML(int n, vtkAbstractWidget *widget, vtkMRMLMarkupsNode *markupsNode)
{
  if (!markupsNode || !widget)
    {
    return false;
    }
  if (n > markupsNode->GetNumberOfMarkups())
    {
    return false;
    }
  vtkMRMLMarkupsRulerNode *rulerNode = vtkMRMLMarkupsRulerNode::SafeDownCast(markupsNode);
  if (!rulerNode)
    {
    return false;
    }
  vtkDistanceWidget *rulerWidget = vtkDistanceWidget::SafeDownCast(widget);
  if (!rulerWidget)
    {
    return false;
    }
  vtkDistanceRepresentation3D * rulerRepresentation = vtkDistanceRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation());
  if (!rulerRepresentation)
    {
    return false;
    }
  bool positionChanged = false;

  // transform ruler points using parent transforms
  double nodeWorldCoord1[4];
  double nodeWorldCoord2[4];
  rulerNode->GetNthRulerWorldCoordinatesInArrays(n, nodeWorldCoord1, nodeWorldCoord2);

  // for 3d managers, compare world positions
  double rulerWorldCoord1[3];
  double rulerWorldCoord2[3];
  rulerRepresentation->GetPoint1WorldPosition(rulerWorldCoord1);
  rulerRepresentation->GetPoint2WorldPosition(rulerWorldCoord2);

  double coord[3];
  coord[0] = nodeWorldCoord1[0];
  coord[1] = nodeWorldCoord1[1];
  coord[2] = nodeWorldCoord1[2];
  if (this->GetWorldCoordinatesChanged(coord, rulerWorldCoord1))
    {
    vtkDebugMacro("UpdateNthWidgetPositionFromMRML: 3D:"
                  << " point 1 world coordinates changed:\n\truler node = "
                  << coord[0] << ", "
                  << coord[1] << ", "
                  << coord[2] << "\n\tdistance widget =  "
                  << rulerWorldCoord1[0] << ", "
                  << rulerWorldCoord1[1] << ", "
                  << rulerWorldCoord1[2]);
    rulerRepresentation->SetPoint1WorldPosition(coord);
    positionChanged = true;
    }
  coord[0] = nodeWorldCoord2[0];
  coord[1] = nodeWorldCoord2[1];
  coord[2] = nodeWorldCoord2[2];
  if (this->GetWorldCoordinatesChanged(coord, rulerWorldCoord2))
    {
    vtkDebugMacro("UpdateNthWidgetPositionFromMRML: 3D:"
                  << " point 2 world coordinates changed:\n\truler node = "
                  << coord[0] << ", "
                  << coord[1] << ", "
                  << coord[2] << "\n\tdistance widget =  "
                  << rulerWorldCoord2[0] << ", "
                  << rulerWorldCoord2[1] << ", "
                  << rulerWorldCoord2[2]);
    rulerRepresentation->SetPoint2WorldPosition(coord);
    positionChanged = true;
    }

  return positionChanged;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayableManager3D::SetNthRuler(int n,
                                                          vtkMRMLMarkupsRulerNode* rulerNode,
                                                          vtkDistanceWidget *rulerWidget)
{
  if (!rulerWidget)
    {
    vtkErrorMacro("SetNthRuler: no widget given!");
    return;
    }
  vtkDistanceRepresentation3D * rulerRepresentation = vtkDistanceRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation());

  if (!rulerRepresentation)
    {
    vtkErrorMacro("SetNthRuler: no ruler representation in widget!");
    return;
    }

  vtkMRMLMarkupsRulerDisplayNode *displayNode = rulerNode->GetMarkupsRulerDisplayNode();
  if (!displayNode)
    {
    vtkDebugMacro("SetNthRuler: Could not get ruler display node for node " << (rulerNode->GetID() ? rulerNode->GetID() : "null id"));
    return;
    }

  // update the postion
  // disable processing of modified events
  this->Updating = 1;
  bool positionChanged = this->UpdateNthWidgetPositionFromMRML(n, rulerWidget, rulerNode);
  if (!positionChanged)
    {
    vtkDebugMacro("Position did not change");
    }

  // get the points, to calculate the distance between them
  double worldCoordinates1[4]={0,0,0,1};
  double worldCoordinates2[4]={0,0,0,1};
  rulerNode->GetNthRulerWorldCoordinatesInArrays(n, worldCoordinates1,worldCoordinates2);
  double distance = this->ApplyUnit(this->GetDistance(worldCoordinates1,worldCoordinates2));
  rulerRepresentation->SetRulerDistance(distance);

  // label position
  rulerRepresentation->SetLabelPosition(displayNode->GetLabelPosition());

  // label scale
  double textScale = displayNode->GetTextScale();
  rulerRepresentation->SetLabelScale(textScale,textScale,textScale);

  // handle properties
  vtkHandleRepresentation *pointRep1 = rulerRepresentation->GetPoint1Representation();
  vtkHandleRepresentation *pointRep2 = rulerRepresentation->GetPoint2Representation();
  vtkPointHandleRepresentation3D *handle1 = NULL;
  vtkPointHandleRepresentation3D *handle2 = NULL;
  if (pointRep1 && pointRep2)
    {
    handle1 = vtkPointHandleRepresentation3D::SafeDownCast(pointRep1);
    handle2 = vtkPointHandleRepresentation3D::SafeDownCast(pointRep2);
    }
  if (handle1 && handle2)
    {
    if (rulerNode->GetSelected())
      {
      handle1->GetProperty()->SetColor(displayNode->GetSelectedPointColor1());
      handle2->GetProperty()->SetColor(displayNode->GetSelectedPointColor2());
      }
    else
      {
      handle1->GetProperty()->SetColor(displayNode->GetPointColor1());
      handle2->GetProperty()->SetColor(displayNode->GetPointColor2());
      }
    }

  // use this scale for the ticks
  rulerRepresentation->SetGlyphScale(displayNode->GetGlyphScale());

  // colors
  if (rulerNode->GetSelected())
    {
    // glyph color matches the line
    rulerRepresentation->GetGlyphActor()->GetProperty()->SetColor(displayNode->GetSelectedLineColor());
    // text color is from the top level display node
    rulerRepresentation->GetLabelProperty()->SetColor(displayNode->GetSelectedColor());
    if (rulerRepresentation->GetLineProperty())
      {
      rulerRepresentation->GetLineProperty()->SetColor(displayNode->GetSelectedLineColor());
      }
    }
  else
    {
    rulerRepresentation->GetGlyphActor()->GetProperty()->SetColor(displayNode->GetLineColor());
    rulerRepresentation->GetLabelProperty()->SetColor(displayNode->GetColor());
    if (rulerRepresentation->GetLineProperty())
      {
      rulerRepresentation->GetLineProperty()->SetColor(displayNode->GetLineColor());
      }
    }

  rulerRepresentation->GetLineProperty()->SetLineWidth(displayNode->GetLineThickness());
  rulerRepresentation->GetLineProperty()->SetOpacity(displayNode->GetOpacity());
  // vtkProperty2D only defines opacity, color and line width

  // if the display node says not to show the text, use the label property
  // to set it invisible, otherwise, use the opacity setting
  if (!displayNode->GetTextVisibility())
    {
    rulerRepresentation->GetLabelProperty()->SetOpacity(0);
    }
  else
    {
    rulerRepresentation->GetLabelProperty()->SetOpacity(displayNode->GetOpacity());
    }
  rulerRepresentation->SetRulerDistance(this->ApplyUnit(displayNode->GetTickSpacing()));
  rulerRepresentation->SetNumberOfRulerTicks(displayNode->GetMaxTicks());

  // update the label format
  // cast to distance representation?
  rulerRepresentation->SetLabelFormat(this->GetLabelFormat(rulerNode, n).c_str());


  // visibility for this widget, hide it if the whole list is invisible,
  // this ruler is invisible, or the list isn't visible in this view
  bool  rulerVisible = true;
  vtkMRMLViewNode *viewNode = this->GetMRMLViewNode();
  if ((viewNode && displayNode->GetVisibility(viewNode->GetID()) == 0) ||
      displayNode->GetVisibility() == 0 ||
      rulerNode->GetNthRulerVisibility(n) == 0)
    {
    rulerVisible = false;
    }
  if (rulerVisible)
    {
    rulerRepresentation->VisibilityOn();
    //handleRep->HandleVisibilityOn();
    rulerWidget->EnabledOn();
    }
  else
    {
    rulerRepresentation->VisibilityOff();
//    handleRep->VisibilityOff();
//    handleRep->HandleVisibilityOff();
//    handleRep->LabelVisibilityOff();
    rulerWidget->EnabledOff();
    }

  // update locked
  int listLocked = rulerNode->GetLocked();
  int rulerLocked = rulerNode->GetNthMarkupLocked(n);
  // if the user is placing lots of rulers at once, add this one as locked
  // so that it can't be moved when placing the next rulers. They will be
  // unlocked when the interaction node goes back into ViewTransform
  int persistentPlaceMode = 0;
  vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
  if (interactionNode)
    {
    persistentPlaceMode =
      (interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
      && (interactionNode->GetPlaceModePersistence() == 1);
    }
  if (listLocked || rulerLocked || persistentPlaceMode)
    {
    rulerWidget->ProcessEventsOff();
    }
  else
    {
    rulerWidget->ProcessEventsOn();
    }
/* TODO: update handle types
  // set the glyph type if a new handle was created, or the glyph type changed
  int oldGlyphType = this->Helper->GetNodeGlyphType(displayNode, n);
  if (createdNewHandle ||
      oldGlyphType != displayNode->GetGlyphType())
    {
    vtkDebugMacro("3D: DisplayNode glyph type = "
          << displayNode->GetGlyphType()
          << " = " << displayNode->GetGlyphTypeAsString()
          << ", is 3d glyph = "
          << (displayNode->GlyphTypeIs3D() ? "true" : "false"));
    if (displayNode->GlyphTypeIs3D())
      {
      if (displayNode->GetGlyphType() == vtkMRMLMarkupsDisplayNode::Sphere3D)
        {
        // std::cout << "3d sphere" << std::endl;
        vtkNew<vtkSphereSource> sphereSource;
        sphereSource->SetRadius(0.5);
        sphereSource->SetPhiResolution(10);
        sphereSource->SetThetaResolution(10);
        sphereSource->Update();
        handleRep->SetHandle(sphereSource->GetOutput());
        }
      else
        {
        // the 3d diamond isn't supported yet, use a 2d diamond for now
        vtkNew<vtkMarkupsGlyphSource2D> glyphSource;
        glyphSource->SetGlyphType(vtkMRMLMarkupsDisplayNode::Diamond2D);
        glyphSource->Update();
        glyphSource->SetScale(1.0);
        handleRep->SetHandle(glyphSource->GetOutput());
        }
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
    // TBD: keep with the assumption of one glyph type per markups node,
    // but they may have different glyphs during update
    this->Helper->SetNodeGlyphType(displayNode, displayNode->GetGlyphType(), n);
    }  // end of glyph type
*/
  // enable processing of modified events
  this->Updating = 0;

  rulerRepresentation->NeedToRenderOn();
  rulerWidget->Modified();
}

//---------------------------------------------------------------------------
/// Propagate properties of MRML node to widget.
void vtkMRMLMarkupsRulerDisplayableManager3D::PropagateMRMLToWidget(vtkMRMLMarkupsNode* node, vtkAbstractWidget * vtkNotUsed(widget))
{
  if (!node)
    {
    vtkErrorMacro("PropagateMRMLToWidget: MRML node was null!")
    return;
    }

  // cast to the specific mrml node
  vtkMRMLMarkupsRulerNode* rulerNode = vtkMRMLMarkupsRulerNode::SafeDownCast(node);

  if (!rulerNode)
    {
    vtkErrorMacro("PropagateMRMLToWidget: Could not get ruler node!")
    return;
    }

  // disable processing of modified events
  this->Updating = 1;

  // iterate over the rulers in this markup
  int numberOfRulers = rulerNode->GetNumberOfMarkups();

  vtkDebugMacro("Fids PropagateMRMLToWidget, node num markups = " << numberOfRulers);

  for (int n = 0; n < numberOfRulers; n++)
    {
    // std::cout << "Fids PropagateMRMLToWidget: n = " << n << std::endl;
    vtkAbstractWidget *rulerWidget = this->Helper->GetWidgetForMarkup(rulerNode->GetNthMarkupID(n));
    this->SetNthRuler(n, rulerNode, vtkDistanceWidget::SafeDownCast(rulerWidget));
    }

  // enable processing of modified events
  this->Updating = 0;
}

//---------------------------------------------------------------------------
/// Propagate properties of widget to MRML node.
void vtkMRMLMarkupsRulerDisplayableManager3D::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLMarkupsNode* node, int markupNumber)
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
  vtkDistanceWidget* rulerWidget = vtkDistanceWidget::SafeDownCast(widget);

  if (!rulerWidget)
   {
   vtkErrorMacro("PropagateWidgetToMRML: Could not get ruler widget!")
   return;
   }

  // cast to the specific mrml node
  vtkMRMLMarkupsRulerNode* rulerNode = vtkMRMLMarkupsRulerNode::SafeDownCast(node);

  if (!rulerNode)
   {
   vtkErrorMacro("PropagateWidgetToMRML: Could not get ruler node!")
   return;
   }

  // disable processing of modified events
  this->Updating = 1;

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  vtkDistanceRepresentation3D * rulerRepresentation = vtkDistanceRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation());

  bool positionChanged = false;

  double worldCoordinates1[3];
  double worldCoordinates2[3];
  rulerRepresentation->GetPoint1WorldPosition(worldCoordinates1);
  rulerRepresentation->GetPoint2WorldPosition(worldCoordinates2);
  vtkDebugMacro("PropagateWidgetToMRML: 3d: widget ruler " << markupNumber
                << " world coords1 = "
                << worldCoordinates1[0] << ", "
                << worldCoordinates1[1] << ", "
                << worldCoordinates1[2]);

  // was there a change?
  double currentCoordinates1[4];
  double currentCoordinates2[4];
  rulerNode->GetNthRulerWorldCoordinatesInArrays(markupNumber,
                                                 currentCoordinates1,
                                                 currentCoordinates2);
  vtkDebugMacro("PropagateWidgetToMRML: ruler " << markupNumber
                << " current world coordinates 1 = "
                << currentCoordinates1[0] << ", "
                << currentCoordinates1[1] << ", "
                << currentCoordinates1[2]);

  double currentCoords1[3];
  currentCoords1[0] = currentCoordinates1[0];
  currentCoords1[1] = currentCoordinates1[1];
  currentCoords1[2] = currentCoordinates1[2];
  if (this->GetWorldCoordinatesChanged(currentCoords1, worldCoordinates1))
    {
    positionChanged = true;
    vtkDebugMacro("PropagateWidgetToMRML: position 1 changed, setting ruler coordinates");
    rulerNode->SetNthRulerWorldCoordinates1(markupNumber,worldCoordinates1);
    }

  double currentCoords2[3];
  currentCoords2[0] = currentCoordinates2[0];
  currentCoords2[1] = currentCoordinates2[1];
  currentCoords2[2] = currentCoordinates2[2];
  if (this->GetWorldCoordinatesChanged(currentCoords2, worldCoordinates2))
    {
    positionChanged = true;
    vtkDebugMacro("PropagateWidgetToMRML: position 2 changed, setting ruler coordinates");
    rulerNode->SetNthRulerWorldCoordinates2(markupNumber,worldCoordinates2);
    }


  // did any of the positions change?
  if (positionChanged)
    {
    vtkDebugMacro("PropagateWidgetToMRML: position changed, calling point modified on the ruler node");
    rulerNode->Modified();
    rulerNode->GetScene()->InvokeEvent(vtkMRMLMarkupsNode::PointModifiedEvent,rulerNode);
    }
  // This displayableManager should now consider ModifiedEvent again
  this->Updating = 0;
}

//---------------------------------------------------------------------------
/// Create a markupsMRMLnode
void vtkMRMLMarkupsRulerDisplayableManager3D::OnClickInRenderWindow(double x, double y, const char *associatedNodeID)
{
  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    vtkDebugMacro("OnClickInRenderWindow: x = " << x << ", y = " << y
                  << ", incorrect displayable manager, focus = "
                  << this->Focus << ", jumping out");
    return;
    }

  // place the seed where the user clicked
  this->PlaceSeed(x,y);

  if (!this->ClickCounter->HasEnoughClicks(2))
    {
    this->GetDisplayToWorldCoordinates(x, y, this->LastClickWorldCoordinates);
    }
  else
    {
    // switch to updating state to avoid events mess
    this->Updating = 1;

    vtkHandleWidget *h2 = this->GetSeed(1);

    // convert the coordinates
    double* displayCoordinates2 = vtkHandleRepresentation::SafeDownCast(h2->GetRepresentation())->GetDisplayPosition();

    double worldCoordinates1[4] = {this->LastClickWorldCoordinates[0],
                                  this->LastClickWorldCoordinates[1],
                                  this->LastClickWorldCoordinates[2],
                                  1};
    double worldCoordinates2[4] = {0,0,0,1};

    this->GetDisplayToWorldCoordinates(displayCoordinates2[0],displayCoordinates2[1],worldCoordinates2);

    // Is there an active markups node that's a ruler node?
    vtkMRMLMarkupsRulerNode *activeRulerNode = NULL;

    vtkMRMLSelectionNode *selectionNode = this->GetSelectionNode();
    if (selectionNode)
      {
      const char *activeMarkupsID = selectionNode->GetActivePlaceNodeID();
      vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(activeMarkupsID);
      if (mrmlNode &&
          mrmlNode->IsA("vtkMRMLMarkupsRulerNode"))
        {
        activeRulerNode = vtkMRMLMarkupsRulerNode::SafeDownCast(mrmlNode);
        }
      else
        {
        vtkDebugMacro("OnClickInRenderWindow: active markup id = "
                      << (activeMarkupsID ? activeMarkupsID : "null")
                      << ", mrml node is "
                      << (mrmlNode ? mrmlNode->GetID() : "null")
                      << ", not a vtkMRMLMarkupsRulerNode");
        }
      }

    bool newNode = false;
    if (!activeRulerNode)
      {
      newNode = true;
      // create the MRML node
      activeRulerNode = vtkMRMLMarkupsRulerNode::New();
      std::string nodeName = this->GetMRMLScene()->GenerateUniqueName("M");
      activeRulerNode->SetName(nodeName.c_str());
      }
    // add a ruler: this will trigger adding a widget at origin
    int rulerIndex = activeRulerNode->AddMarkupWithNPoints(2);
    if (rulerIndex == -1)
      {
      vtkErrorMacro("OnClickInRenderWindow: unable to add a ruler to active ruler list!");
      if (newNode)
        {
        activeRulerNode->Delete();
        }
      return;
      }

    // set values on it
    activeRulerNode->SetNthRulerWorldCoordinates1(rulerIndex, worldCoordinates1);
    activeRulerNode->SetNthRulerWorldCoordinates2(rulerIndex, worldCoordinates2);

    // reset updating state
    this->Updating = 0;

    // if this was a one time place, go back to view transform mode
    vtkMRMLInteractionNode *interactionNode = this->GetInteractionNode();
    if (interactionNode && interactionNode->GetPlaceModePersistence() != 1)
      {
      interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
      }

    this->GetMRMLScene()->SaveStateForUndo();

    // is there a node associated with this?
    if (associatedNodeID)
      {
      activeRulerNode->SetNthRulerAssociatedNodeID(rulerIndex, associatedNodeID);
      }

    if (newNode)
      {
      // create a display node and add node and display node to scene
      vtkMRMLMarkupsRulerDisplayNode *displayNode = vtkMRMLMarkupsRulerDisplayNode::New();
      this->GetMRMLScene()->AddNode(displayNode);
      // let the logic know that it needs to set it to defaults
      displayNode->InvokeEvent(vtkMRMLMarkupsDisplayNode::ResetToDefaultsEvent);

      activeRulerNode->AddAndObserveDisplayNodeID(displayNode->GetID());
      this->GetMRMLScene()->AddNode(activeRulerNode);

      // have to reset the ruler id since the ruler node generates a scene
      // unique id only if the node was in the scene when the point was added.
      // But the widget was created and recorded when the node was added to
      // the scene, so have to reset the widget to be linked to the new markup id
      std::string oldID = activeRulerNode->GetNthMarkupID(0);
      if (!activeRulerNode->ResetNthMarkupID(0))
        {
        vtkWarningMacro("Failed to reset the unique ID on the first ruler in a new list: "
                        << activeRulerNode->GetNthMarkupID(0));
        }
      std::string newID = activeRulerNode->GetNthMarkupID(0);
      this->Helper->UpdateWidgetForMarkupID(oldID, newID);

      // save it as the active markups list
      if (selectionNode)
        {
        selectionNode->SetActivePlaceNodeID(activeRulerNode->GetID());
        }
      // clean up
      displayNode->Delete();
      activeRulerNode->Delete();
      }
    }
}

//---------------------------------------------------------------------------
/// observe key press events
void vtkMRMLMarkupsRulerDisplayableManager3D::AdditionnalInitializeStep()
{
  // don't add the key press event, as it triggers a crash on start up
  //vtkDebugMacro("Adding an observer on the key press event");
  this->AddInteractorStyleObservableEvent(vtkCommand::KeyPressEvent);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayableManager3D::OnInteractorStyleEvent(int eventid)
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
    vtkDebugMacro("OnInteractorStyleEvent 3D: key press event position = "
              << this->GetInteractor()->GetEventPosition()[0] << ", "
              << this->GetInteractor()->GetEventPosition()[1]
              << ", key sym = " << (keySym == NULL ? "null" : keySym));
    if (!keySym)
      {
      return;
      }
    if (strcmp(keySym, "r") == 0)
      {
      if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
        {
        this->OnClickInRenderWindowGetCoordinates();
        }
      else
        {
        vtkDebugMacro("Ruler DisplayableManager: key press p, but not in Place mode! Returning.");
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
void vtkMRMLMarkupsRulerDisplayableManager3D::UpdatePosition(vtkAbstractWidget *vtkNotUsed(widget), vtkMRMLNode *node)
{
//  vtkWarningMacro("UpdatePosition, node is " << (node == NULL ? "null" : node->GetID()));
  if (!node)
    {
    return;
    }
  vtkMRMLMarkupsRulerNode *markupsNode = vtkMRMLMarkupsRulerNode::SafeDownCast(node);
  if (!markupsNode)
    {
    vtkErrorMacro("UpdatePosition - Can not access ruler node from node with id " << node->GetID());
    return;
    }

  // now get the widget properties (coordinates, measurement etc.) and if the mrml node has changed, propagate the changes
  bool positionChanged = false;
  int numberOfRulers = markupsNode->GetNumberOfMarkups();
  for (int n = 0; n < numberOfRulers; n++)
    {
    // get the widget
    vtkAbstractWidget *widget = this->Helper->GetWidgetForMarkup(markupsNode->GetNthMarkupID(n));
    vtkDistanceWidget* rulerWidget = vtkDistanceWidget::SafeDownCast(widget);

    if (!rulerWidget)
      {
      vtkErrorMacro("UpdatePosition: Could not get " << n << " th ruler widget!")
        return;
      }

    if (this->UpdateNthWidgetPositionFromMRML(n, rulerWidget, markupsNode))
      {
      positionChanged = true;
      if (this->Updating == 0)
        {
        vtkDistanceRepresentation3D * rulerRepresentation = vtkDistanceRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation());
        rulerRepresentation->NeedToRenderOn();
        rulerWidget->Modified();
        }
      }
    }
  // TODO: remove?
  // did any of the positions change?
  if (positionChanged && this->Updating == 0)
    {
    // not already updating from propagate mrml to widget, so trigger a render

    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayableManager3D::OnMRMLSceneEndClose()
{
  // clear out the map of glyph types
  this->Helper->ClearNodeGlyphTypes();
  // TODO: deleting widgets isn't working well, so remove them all
  this->Helper->RemoveAllWidgetsAndNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayableManager3D::OnMRMLMarkupsNodeNthMarkupModifiedEvent(vtkMRMLMarkupsNode* node, int n)
{
  int numberOfMarkups = node->GetNumberOfMarkups();
  if (n < 0 || n >= numberOfMarkups)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeNthMarkupModifiedEvent: n = " << n << " is out of range 0-" << numberOfMarkups);
    return;
    }

  vtkAbstractWidget *widget = this->Helper->GetWidgetForMarkup(node->GetNthMarkupID(n));
  if (!widget)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeNthMarkupModifiedEvent: a markup was added to a node that doesn't already have a widget! Returning..");
    return;
    }

  vtkDistanceWidget* rulerWidget = vtkDistanceWidget::SafeDownCast(widget);
  if (!rulerWidget)
   {
   vtkErrorMacro("OnMRMLMarkupsNodeNthMarkupModifiedEvent: Could not get ruler widget!")
   return;
   }
  this->SetNthRuler(n, vtkMRMLMarkupsRulerNode::SafeDownCast(node), rulerWidget);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayableManager3D::OnMRMLMarkupsNodeMarkupAddedEvent(vtkMRMLMarkupsNode * markupsNode)
{
  vtkDebugMacro("OnMRMLMarkupsNodeMarkupAddedEvent");

  if (!markupsNode)
    {
    return;
    }

  // need to add a widget
  int markupIndex = markupsNode->GetNumberOfMarkups() - 1;
  if (this->AddWidget(markupsNode, markupIndex) == NULL)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeMarkupAddedEvent: "
                  << "failed to add a new widget at markup index "
                  << markupIndex);
    return;
    }


  vtkAbstractWidget *widget = this->Helper->GetWidgetForMarkup(markupsNode->GetNthMarkupID(markupIndex));
  if (!widget)
    {
    // TBD: create a widget?
    vtkErrorMacro("OnMRMLMarkupsNodeMarkupAddedEvent: a markup was added to a node that doesn't already have a widget! Returning..");
    return;
    }

  vtkDistanceWidget* rulerWidget = vtkDistanceWidget::SafeDownCast(widget);
  if (!rulerWidget)
   {
   vtkErrorMacro("OnMRMLMarkupsNodeMarkupAddedEvent: Could not get ruler widget!")
   return;
   }

  // this call will create a new handle and set it
  this->SetNthRuler(markupIndex, vtkMRMLMarkupsRulerNode::SafeDownCast(markupsNode),
                    rulerWidget);

  vtkDistanceRepresentation3D * rulerRepresentation = vtkDistanceRepresentation3D::SafeDownCast(rulerWidget->GetRepresentation());
  rulerRepresentation->NeedToRenderOn();
  rulerWidget->Modified();

  // remove any seeds that were placed
  this->Helper->RemoveSeeds();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayableManager3D::
  OnMRMLMarkupsNodeMarkupRemovedEvent(vtkMRMLMarkupsNode * markupsNode, int n)
{
  vtkDebugMacro("OnMRMLMarkupsNodeMarkupRemovedEvent, n = " << n);

  if (!markupsNode)
    {
    return;
    }
  std::string removedID = markupsNode->GetNthMarkupID(n);
  vtkDebugMacro("Removed event: removedID = " << removedID.c_str());
  vtkAbstractWidget *widget = this->Helper->GetWidgetForMarkup(removedID);
  if (!widget)
    {
    vtkErrorMacro("OnMRMLMarkupsNodeMarkupRemovedEvent: a markup was removed from a node, but the widget doesn't already exist! Returning..");
    return;
    }

  this->Helper->RemoveWidget(markupsNode, n);
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLMarkupsRulerDisplayableManager3D::
  AddWidget(vtkMRMLMarkupsNode *markupsNode, int markupIndex)
{
  vtkDebugMacro("AddWidget: calling create widget");
  vtkAbstractWidget* newWidget = this->CreateWidget(markupsNode);
  if (!newWidget)
    {
    vtkErrorMacro("AddWidget: unable to create a new widget for markups node " << markupsNode->GetID());
    return NULL;
    }

  // record the mapping between node and widget in the helper
  this->Helper->RecordWidgetForMarkup(newWidget, markupsNode, markupIndex);

  if (markupIndex == 0)
    {
    // first markup in the list, so refresh observers
    this->SetAndObserveNode(markupsNode);
    }

  // TODO do we need this render call?
  this->RequestRender();

  // set up the call back for this ruler
  this->OnWidgetCreated(newWidget, markupsNode, markupIndex);

  // now set up the new widget
  this->SetNthRuler(markupIndex,
                    vtkMRMLMarkupsRulerNode::SafeDownCast(markupsNode),
                    vtkDistanceWidget::SafeDownCast(newWidget));

  return newWidget;
}

//---------------------------------------------------------------------------
std::string vtkMRMLMarkupsRulerDisplayableManager3D::GetLabelFormat
(vtkMRMLMarkupsRulerNode *rulerNode, int markupIndex)
{
  if (!rulerNode)
    {
    return "";
    }

  std::string format = "%-#6.3g mm";
  if (rulerNode->GetMarkupsRulerDisplayNode())
    {
    format = rulerNode->GetMarkupsRulerDisplayNode()->GetDistanceMeasurementFormat();
    }
  if (this->GetMRMLScene())
    {
    vtkMRMLSelectionNode* selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));

    if (selectionNode)
      {
      vtkMRMLUnitNode* lengthUnit = vtkMRMLUnitNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID(
          selectionNode->GetUnitNodeID("length")));
      if (lengthUnit)
        {
        format = lengthUnit->GetDisplayStringFormat();
        }
      }
    }

  format = std::string(rulerNode->GetNthRulerLabel(markupIndex)) + std::string(" :") + format;

  return format;
}

//---------------------------------------------------------------------------
double vtkMRMLMarkupsRulerDisplayableManager3D::GetDistance(const double* wc1, const double* wc2)
{
  double distance = sqrt(vtkMath::Distance2BetweenPoints(wc1,wc2));
  return distance;
}

//---------------------------------------------------------------------------
double vtkMRMLMarkupsRulerDisplayableManager3D::ApplyUnit(double lengthInMM)
{
  double length = lengthInMM;
  if (this->GetMRMLScene())
    {
    vtkMRMLSelectionNode* selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));

    if (selectionNode)
      {
      vtkMRMLUnitNode* lengthUnit = vtkMRMLUnitNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID(
          selectionNode->GetUnitNodeID("length")));
      if (lengthUnit)
        {
        length = lengthUnit->GetDisplayValueFromValue(lengthInMM);
        }
      }
    }
  return length;
}
