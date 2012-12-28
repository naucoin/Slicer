#ifndef VTKMRMLMARKUPSCLICKCOUNTER_H_
#define VTKMRMLMARKUPSCLICKCOUNTER_H_

// MarkupsModule includes
#include "vtkSlicerMarkupsModuleMRMLDisplayableManagerExport.h"

// VTK include
#include <vtkObject.h>

/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLMarkupsClickCounter :
    public vtkObject
{
public:

  static vtkMRMLMarkupsClickCounter *New();
  vtkTypeRevisionMacro(vtkMRMLMarkupsClickCounter, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Check if enough clicks are counted
  bool HasEnoughClicks(int clicks);

  /// Reset the click counter
  void Reset();

protected:

  vtkMRMLMarkupsClickCounter();
  virtual ~vtkMRMLMarkupsClickCounter();

private:

  vtkMRMLMarkupsClickCounter(const vtkMRMLMarkupsClickCounter&); /// Not implemented
  void operator=(const vtkMRMLMarkupsClickCounter&); /// Not Implemented

  int m_Clicks;

};

#endif /* VTKMRMLMARKUPSCLICKCOUNTER_H_ */
