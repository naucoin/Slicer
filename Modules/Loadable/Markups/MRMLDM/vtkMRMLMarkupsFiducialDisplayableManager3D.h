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

#ifndef __vtkMRMLMarkupsFiducialDisplayableManager3D_h
#define __vtkMRMLMarkupsFiducialDisplayableManager3D_h

// MarkupsModule includes
#include "vtkSlicerMarkupsModuleMRMLDisplayableManagerExport.h"

// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsDisplayableManager3D.h"

class vtkMRMLMarkupsFiducialNode;
class vtkSlicerViewerWidget;
class vtkMRMLMarkupsDisplayNode;
class vtkTextWidget;

/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLMarkupsFiducialDisplayableManager3D :
    public vtkMRMLMarkupsDisplayableManager3D
{
public:

  static vtkMRMLMarkupsFiducialDisplayableManager3D *New();
  vtkTypeMacro(vtkMRMLMarkupsFiducialDisplayableManager3D, vtkMRMLMarkupsDisplayableManager3D);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLMarkupsFiducialDisplayableManager3D(){this->Focus="vtkMRMLMarkupsFiducialNode";}
  virtual ~vtkMRMLMarkupsFiducialDisplayableManager3D(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID);

  /// Add a new widget to represent this markups node. Record it with
  /// the helper. markupIndex is ignored as this should only be called when a new fiducial
  /// node is added, not when adding markups to the list.
  virtual vtkAbstractWidget *AddWidget(vtkMRMLMarkupsNode *markupsNode, int markupIndex);

  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLMarkupsNode* node);
  /// Create new handle on widget when a new markup is added to a markups node
  virtual void OnMRMLMarkupsNodeMarkupAddedEvent(vtkMRMLMarkupsNode * markupsNode);
  /// Respond to the nth markup modified event
  virtual void OnMRMLMarkupsNodeNthMarkupModifiedEvent(vtkMRMLMarkupsNode * markupsNode, int n);
  /// Respond to a markup being removed from the markups node
  virtual void OnMRMLMarkupsNodeMarkupRemovedEvent(vtkMRMLMarkupsNode * markupsNode, int n);

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget,
                               vtkMRMLMarkupsNode * node, int markupNumber = 0);

  /// Update a single seed from MRML
  void SetNthSeed(int n, vtkMRMLMarkupsFiducialNode* fiducialNode, vtkSeedWidget *seedWidget);
  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLMarkupsNode* node, vtkAbstractWidget * widget);

  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget,
                                     vtkMRMLMarkupsNode* node,
                                     int markupNumber = 0);

  /// Set up an observer on the interactor style to watch for key press events
  virtual void AdditionnalInitializeStep();
  /// Respond to the interactor style event
  virtual void OnInteractorStyleEvent(int eventid);

  /// Update a single seed position from the node, return true if the position changed
  virtual bool UpdateNthWidgetPositionFromMRML(int n, vtkAbstractWidget *widget, vtkMRMLMarkupsNode *pointsNode);
  /// Respond to control point modified events
  virtual void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node);

  // Clean up when scene closes
  virtual void OnMRMLSceneEndClose();

private:

  vtkMRMLMarkupsFiducialDisplayableManager3D(const vtkMRMLMarkupsFiducialDisplayableManager3D&); /// Not implemented
  void operator=(const vtkMRMLMarkupsFiducialDisplayableManager3D&); /// Not Implemented
};

#endif
