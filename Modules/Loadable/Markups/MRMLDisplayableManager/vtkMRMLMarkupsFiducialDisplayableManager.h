#ifndef __vtkMRMLMarkupsFiducialDisplayableManager_h
#define __vtkMRMLMarkupsFiducialDisplayableManager_h

// MarkupsModule includes
#include "vtkSlicerMarkupsModuleMRMLDisplayableManagerExport.h"

// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsDisplayableManager.h"

class vtkMRMLMarkupsFiducialNode;
class vtkSlicerViewerWidget;
class vtkMRMLMarkupsDisplayNode;
class vtkTextWidget;

/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLMarkupsFiducialDisplayableManager :
    public vtkMRMLMarkupsDisplayableManager
{
public:

  static vtkMRMLMarkupsFiducialDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLMarkupsFiducialDisplayableManager, vtkMRMLMarkupsDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLMarkupsFiducialDisplayableManager(){this->Focus="vtkMRMLMarkupsFiducialNode";}
  virtual ~vtkMRMLMarkupsFiducialDisplayableManager(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLMarkupsNode* node);
  /// create new handle on widget when a new markup is added to a markups node
  virtual void OnMRMLMarkupsNodeMarkupAddedEvent(vtkMRMLMarkupsNode * markupsNode);
  /// respond to the nth markup modified event
  virtual void OnMRMLMarkupsNodeNthMarkupModifiedEvent(vtkMRMLMarkupsNode * markupsNode, int n);
  
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
  virtual bool UpdateNthMarkupPosition(int n, vtkAbstractWidget *widget, vtkMRMLMarkupsNode *pointsNode);
  /// respond to control point modified events
  virtual void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node);


  std::map<vtkMRMLNode*, int> NodeGlyphTypes;

  // clean up when scene closes
  virtual void OnMRMLSceneEndClose();

private:

  vtkMRMLMarkupsFiducialDisplayableManager(const vtkMRMLMarkupsFiducialDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLMarkupsFiducialDisplayableManager&); /// Not Implemented

};

#endif

