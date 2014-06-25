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

#ifndef __vtkMRMLMarkupsRulerDisplayableManager3D_h
#define __vtkMRMLMarkupsRulerDisplayableManager3D_h

// MarkupsModule includes
#include "vtkSlicerMarkupsModuleMRMLDisplayableManagerExport.h"

// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsDisplayableManager3D.h"

class vtkMRMLMarkupsRulerNode;
class vtkSlicerViewerWidget;
class vtkMRMLMarkupsDisplayNode;

class vtkDistanceWidget;

/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLMarkupsRulerDisplayableManager3D :
    public vtkMRMLMarkupsDisplayableManager3D
{
public:

  static vtkMRMLMarkupsRulerDisplayableManager3D *New();
  vtkTypeMacro(vtkMRMLMarkupsRulerDisplayableManager3D, vtkMRMLMarkupsDisplayableManager3D);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLMarkupsRulerDisplayableManager3D(){this->Focus="vtkMRMLMarkupsRulerNode";}
  virtual ~vtkMRMLMarkupsRulerDisplayableManager3D(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLMarkupsNode* node);
  /// Create new widget when a new markup is added to a markups node
  virtual void OnMRMLMarkupsNodeMarkupAddedEvent(vtkMRMLMarkupsNode * markupsNode);
  /// Respond to the nth markup modified event
  virtual void OnMRMLMarkupsNodeNthMarkupModifiedEvent(vtkMRMLMarkupsNode * markupsNode, int n);
  /// Respond to a markup being removed from the markups node
  virtual void OnMRMLMarkupsNodeMarkupRemovedEvent(vtkMRMLMarkupsNode * markupsNode, int m);

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLMarkupsNode * node,
                               int markupNumber);

  /// Update a single ruler from MRML
  void SetNthRuler(int n, vtkMRMLMarkupsRulerNode* rulerNode, vtkDistanceWidget *rulerWidget);
  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLMarkupsNode* node, vtkAbstractWidget * widget);

  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLMarkupsNode* node, int markupNumber);

  /// Set up an observer on the interactor style to watch for key press events
  virtual void AdditionnalInitializeStep();
  /// Respond to the interactor style event
  virtual void OnInteractorStyleEvent(int eventid);

  /// Update a single ruler position from the node, return true if the position changed
  virtual bool UpdateNthWidgetPositionFromMRML(int n, vtkAbstractWidget *widget, vtkMRMLMarkupsNode *pointsNode);
  /// Respond to control point modified events
  virtual void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node);

  /// Clean up when scene closes
  virtual void OnMRMLSceneEndClose();

  /// Create a new widget for this markup and save it to the helper.
  virtual vtkAbstractWidget * AddWidget(vtkMRMLMarkupsNode *markupsNode, int n);

  /// Get the label from the node and unit node
  std::string GetLabelFormat(vtkMRMLMarkupsRulerNode* rulerNode, int markupIndex);

  /// Compute the distance in mm between 2 world coordinates points
  /// \sa ApplyUnit()
  double GetDistance(const double* wc1, const double* wc2);
  /// Apply the current unit to a length in mm.
  /// \sa GetDistance()
  double ApplyUnit(double lengthInMM);

private:

  vtkMRMLMarkupsRulerDisplayableManager3D(const vtkMRMLMarkupsRulerDisplayableManager3D&); /// Not implemented
  void operator=(const vtkMRMLMarkupsRulerDisplayableManager3D&); /// Not Implemented
};

#endif
