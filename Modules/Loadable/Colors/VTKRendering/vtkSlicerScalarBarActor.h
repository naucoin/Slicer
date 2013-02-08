/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

///  vtkSlicerScalarBarActor - slicer vtk class for adding color names in scalarbar
///
/// This class enhances the vtkScalarBarActor class by adding color names 
/// in the label display.

#ifndef __vtkSlicerScalarBarActor_h
#define __vtkSlicerScalarBarActor_h

// Std includes
#include <string>
#include <vector>

// vtk includes
#include "vtkScalarBarActor.h"

// MRMLLogic includes
//#include <vtkMRMLColorLogic.h>
#include "vtkSlicerColorsModuleVTKRenderingExport.h"

class VTK_SLICER_COLORS_MODULE_VTKRENDERING_EXPORT vtkSlicerScalarBarActor 
  : public vtkScalarBarActor
{
public:
  /// The Usual vtk class functions
  vtkTypeRevisionMacro(vtkSlicerScalarBarActor,vtkScalarBarActor);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkSlicerScalarBarActor *New();

  /// Get/Set for the flag on using color names as label
  vtkGetMacro(UseColorNameAsLabel, int);
  vtkSetMacro(UseColorNameAsLabel, int);
  vtkBooleanMacro(UseColorNameAsLabel, int);

  /// Set the ith color name.
  int SetColorName(int ind, const char *name);

protected:
  vtkSlicerScalarBarActor();
  ~vtkSlicerScalarBarActor();

  /// overloaded virtual function that adds the color name as label
  virtual void AllocateAndSizeLabels(int *labelSize, int *size,
                                     vtkViewport *viewport, double *range);

  /// A vector of names for the color table elements
  std::vector<std::string> Names;

  /// flag for setting color name as label
  int UseColorNameAsLabel;

private:
  vtkSlicerScalarBarActor(const vtkSlicerScalarBarActor&);  // Not implemented.
  void operator=(const vtkSlicerScalarBarActor&);  // Not implemented.
};


#endif