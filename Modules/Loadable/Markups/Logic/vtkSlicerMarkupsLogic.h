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

// .NAME vtkSlicerMarkupsLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerMarkupsLogic_h
#define __vtkSlicerMarkupsLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerMarkupsModuleLogicExport.h"

class vtkMRMLMarkupsNode;
class vtkMRMLMarkupsDisplayNode;

/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_LOGIC_EXPORT vtkSlicerMarkupsLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerMarkupsLogic *New();
  vtkTypeMacro(vtkSlicerMarkupsLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ProcessMRMLNodesEvents(vtkObject *caller,
                                      unsigned long event,
                                      void *callData );

  /// Create a new display node and observe it on the markups node
  /// On success, return the id, on failure return an empty string.
  std::string AddNewDisplayNodeForMarkupsNode(vtkMRMLNode *mrmlNode);
  
  /// Create a new markups fiducial node and associated display node, adding both to
  /// the scene. Also make it the active on on the selection node.
  /// On success, return the id, on failure return an empty string.
  std::string AddNewFiducialNode(const char *name = NULL);
  
  /// jump the slice windows to the given coordinate
  void JumpSlicesToLocation(double x, double y, double z);
  /// jump the slice windows to the nth markup with the mrml id id
  void JumpSlicesToNthPointInMarkup(const char *id, int n);

  /// Load a markups fiducial list from fileName, return NULL on error, node ID string
  /// otherwise. Adds the appropriate storage and display nodes to the scene
  /// as well.
  char *LoadMarkupsFiducials(const char *fileName, const char *fidsName);

  /// Utility methods to operate on all markups in a markups node
  void SetAllMarkupsVisibility(vtkMRMLMarkupsNode *node, bool flag);
  void SetAllMarkupsLocked(vtkMRMLMarkupsNode *node, bool flag);
  void SetAllMarkupsSelected(vtkMRMLMarkupsNode *node, bool flag);

  /// set/get the default markups display node settings
  int GetDefaultMarkupsDisplayNodeGlyphType();
  void SetDefaultMarkupsDisplayNodeGlyphType(int glyphType);
  void SetDefaultMarkupsDisplayNodeGlyphTypeFromString(const char *glyphType);
  std::string GetDefaultMarkupsDisplayNodeGlyphTypeAsString();

  double GetDefaultMarkupsDisplayNodeGlyphScale();
  void SetDefaultMarkupsDisplayNodeGlyphScale(double scale);

  double GetDefaultMarkupsDisplayNodeTextScale();
  void SetDefaultMarkupsDisplayNodeTextScale(double scale);

  double GetDefaultMarkupsDisplayNodeOpacity();
  void SetDefaultMarkupsDisplayNodeOpacity(double opacity);

  double *GetDefaultMarkupsDisplayNodeColor();
  void SetDefaultMarkupsDisplayNodeColor(double *color);
  void SetDefaultMarkupsDisplayNodeColor(double r, double g, double b);

  double *GetDefaultMarkupsDisplayNodeSelectedColor();
  void SetDefaultMarkupsDisplayNodeSelectedColor(double *color);
  void SetDefaultMarkupsDisplayNodeSelectedColor(double r, double g, double b);

  /// utility method to set up a display node from the defaults
  void SetDisplayNodeToDefaults(vtkMRMLMarkupsDisplayNode *displayNode);
  
protected:
  vtkSlicerMarkupsLogic();
  virtual ~vtkSlicerMarkupsLogic();

  /// Initialize listening to MRML events
  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);
  virtual void ObserveMRMLScene();

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

private:

  vtkSlicerMarkupsLogic(const vtkSlicerMarkupsLogic&); // Not implemented
  void operator=(const vtkSlicerMarkupsLogic&);               // Not implemented

  /// keep a markups display node with default values that can be updated from
  /// the application settings
  vtkMRMLMarkupsDisplayNode *defaultMarkupsDisplayNode;
};

#endif

