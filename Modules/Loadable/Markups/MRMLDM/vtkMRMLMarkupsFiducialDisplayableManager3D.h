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
  vtkTypeRevisionMacro(vtkMRMLMarkupsFiducialDisplayableManager3D, vtkMRMLMarkupsDisplayableManager3D);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLMarkupsFiducialDisplayableManager3D(){this->Focus="vtkMRMLMarkupsFiducialNode";}
  virtual ~vtkMRMLMarkupsFiducialDisplayableManager3D(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLMarkupsNode* node);
  /// create new handle on widget when a new markup is added to a markups node
  virtual void OnMRMLMarkupsNodeMarkupAddedEvent(vtkMRMLMarkupsNode * markupsNode);
  /// respond to the nth markup modified event
  virtual void OnMRMLMarkupsNodeNthMarkupModifiedEvent(vtkMRMLMarkupsNode * markupsNode, int n);
  /// respond to a markup being removed from the markups node
  virtual void OnMRMLMarkupsNodeMarkupRemovedEvent(vtkMRMLMarkupsNode * markupsNode);

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLMarkupsNode * node);

  /// Update a single seed from MRML
  void SetNthSeed(int n, vtkMRMLMarkupsFiducialNode* fiducialNode, vtkSeedWidget *seedWidget);
  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLMarkupsNode* node, vtkAbstractWidget * widget);

  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLMarkupsNode* node);

  /// set up an observer on the interactor style to watch for key press events
  virtual void AdditionnalInitializeStep();
  /// respond to the interactor style event
  virtual void OnInteractorStyleEvent(int eventid);

  /// update a single seed position from the node, return true if the position changed
  virtual bool UpdateNthSeedPositionFromMRML(int n, vtkAbstractWidget *widget, vtkMRMLMarkupsNode *pointsNode);
  /// respond to control point modified events
  virtual void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node);

  // clean up when scene closes
  virtual void OnMRMLSceneEndClose();

private:

  vtkMRMLMarkupsFiducialDisplayableManager3D(const vtkMRMLMarkupsFiducialDisplayableManager3D&); /// Not implemented
  void operator=(const vtkMRMLMarkupsFiducialDisplayableManager3D&); /// Not Implemented
};

#endif
