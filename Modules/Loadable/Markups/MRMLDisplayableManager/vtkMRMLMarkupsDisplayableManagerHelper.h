#ifndef VTKMRMLMARKUPSDISPLAYABLEMANAGERHELPER_H_
#define VTKMRMLMARKUPSDISPLAYABLEMANAGERHELPER_H_

// MarkupsModule includes
#include "vtkSlicerMarkupsModuleMRMLDisplayableManagerExport.h"

// MarkupsModule/MRML includes
#include <vtkMRMLMarkupsNode.h>

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkHandleWidget.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLInteractionNode.h>
class vtkMRMLMarkupsDisplayNode;

/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLMarkupsDisplayableManagerHelper :
    public vtkObject
{
public:

  static vtkMRMLMarkupsDisplayableManagerHelper *New();
  vtkTypeRevisionMacro(vtkMRMLMarkupsDisplayableManagerHelper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Lock/Unlock all widgets based on the state of the nodes
  void UpdateLockedAllWidgetsFromNodes();
  /// Lock/Unlock all widgets from interaction node
  void UpdateLockedAllWidgetsFromInteractionNode(vtkMRMLInteractionNode* interactionNode);
  /// Lock/Unlock all widgets
  void UpdateLockedAllWidgets(bool locked);
  /// Lock/Unlock a widget
  void UpdateLocked(vtkMRMLMarkupsNode* node);

  /// Keep track of the mapping between widgets and nodes
  void RecordWidgetForNode(vtkAbstractWidget* widget, vtkMRMLMarkupsNode *node);
  
  /// Get a vtkAbstractWidget* given a node
  vtkAbstractWidget * GetWidget(vtkMRMLMarkupsNode * node);
  /// ...an its associated vtkAbstractWidget* for Slice intersection representation
  vtkAbstractWidget * GetIntersectionWidget(vtkMRMLMarkupsNode * node);
  /// Remove all widgets, intersection widgets, nodes
  void RemoveAllWidgetsAndNodes();
  /// Remove a node, its widget and its intersection widget
  void RemoveWidgetAndNode(vtkMRMLMarkupsNode *node);


  /// Search the markups node list and return the markups node that has this display node
  vtkMRMLMarkupsNode * GetMarkupsNodeFromDisplayNode(vtkMRMLMarkupsDisplayNode *displayNode);
  
  //----------------------------------------------------------------------------------
  // The Lists!!
  //
  // A markup which is managed by a displayableManager consists of
  //   a) the Markups MRML Node (MarkupsNodeList)
  //   b) the vtkWidget to show this markup (Widgets)
  //   c) a vtkWidget to represent sliceIntersections in the slice viewers (WidgetIntersections)
  //

  /// List of Nodes managed by the DisplayableManager
  std::vector<vtkMRMLMarkupsNode*> MarkupsNodeList;

  /// .. and its associated convenient typedef
  typedef std::vector<vtkMRMLMarkupsNode*>::iterator MarkupsNodeListIt;

  /// Map of vtkWidget indexed using associated node ID
  std::map<vtkMRMLMarkupsNode*, vtkAbstractWidget*> Widgets;

  /// .. and its associated convenient typedef
  typedef std::map<vtkMRMLMarkupsNode*, vtkAbstractWidget*>::iterator WidgetsIt;

  /// Map of vtkWidgets to reflect the Slice intersections indexed using associated node ID
  std::map<vtkMRMLMarkupsNode*, vtkAbstractWidget*> WidgetIntersections;

  /// .. and its associated convenient typedef
  typedef std::map<vtkMRMLMarkupsNode*, vtkAbstractWidget*>::iterator WidgetIntersectionsIt;

  //
  // End of The Lists!!
  //
  //----------------------------------------------------------------------------------


  /// Placement of seeds for widget placement
  void PlaceSeed(double x, double y, vtkRenderWindowInteractor * interactor, vtkRenderer * renderer);

  /// Get a placed seed
  vtkHandleWidget * GetSeed(int index);

  /// Remove all placed seeds
  void RemoveSeeds();


protected:

  vtkMRMLMarkupsDisplayableManagerHelper();
  virtual ~vtkMRMLMarkupsDisplayableManagerHelper();

private:

  vtkMRMLMarkupsDisplayableManagerHelper(const vtkMRMLMarkupsDisplayableManagerHelper&); /// Not implemented
  void operator=(const vtkMRMLMarkupsDisplayableManagerHelper&); /// Not Implemented

  /// SeedWidget for point placement
  vtkSmartPointer<vtkSeedWidget> SeedWidget;
  /// List of Handles for the SeedWidget
  std::vector<vtkSmartPointer<vtkHandleWidget> > HandleWidgetList;
  /// .. and its associated convenient typedef
  typedef std::vector<vtkSmartPointer<vtkHandleWidget> >::iterator HandleWidgetListIt;

};

#endif /* VTKMRMLMARKUPSDISPLAYABLEMANAGERHELPER_H_ */
