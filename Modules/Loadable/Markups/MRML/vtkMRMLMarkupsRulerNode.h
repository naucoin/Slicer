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

#ifndef __vtkMRMLMarkupsRulerNode_h
#define __vtkMRMLMarkupsRulerNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsRulerDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"

// VTK includes
#include <vtkSmartPointer.h>

/// \brief MRML node to represent a ruler markup
/// Ruler Markups nodes contain a list of rulers, each containing 2 points.
/// Visualization parameters are set in the vtkMRMLMarkupsDisplayNode class.
/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsRulerNode : public vtkMRMLMarkupsNode
{
public:
  static vtkMRMLMarkupsRulerNode *New();
  vtkTypeMacro(vtkMRMLMarkupsRulerNode,vtkMRMLMarkupsNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char* GetIcon() {return ":/Icons/MarkupsMouseModePlace.png";};

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "MarkupsRuler";};

  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Calls the superclass UpdateScene
  void UpdateScene(vtkMRMLScene *scene);

  /// Alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );


  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  /// Returns a ruler display node, null if none
  vtkMRMLMarkupsRulerDisplayNode *GetMarkupsRulerDisplayNode();

  // Wrapping some of the generic markup methods

  /// Get the number of rulers in this node
  int GetNumberOfRulers() { return this->GetNumberOfMarkups(); } ;

  /// Add a new ruler from x,y,z coordinates and return the ruler index
  int AddRuler(double x1, double y1, double z1, double x2, double y2, double z2);
  /// Add a new ruler from an array (two sets of triples) and return the ruler index
  int AddRulerFromArray(double pos[6]);
  /// Add a new ruler from a pair of arrays, and return the ruler index
  int AddRulerFromArrays(double pos1[3], double pos2[3]);

  /// Get the position of the two points of the nth ruler, returning it in the pos array
  void GetNthRulerPosition(int n, double pos[6]);
  /// Get the position of the two points of the nth ruler, returning it in the pos1 and pos2 arrays
  void GetNthRulerPositionInArrays(int n, double pos1[3], double pos2[3]);
  /// Get the position of the first point of the nth ruler, returning it in the pos array
  void GetNthRulerPosition1(int n, double pos[3]);
  /// Get the position of the second point of the nth ruler, returning it in the pos array
  void GetNthRulerPosition2(int n, double pos[3]);

  /// Set the position of the nth ruler from x1, y1, z1, x2, y2, z2 coordinates
  void SetNthRulerPosition(int n, double x1, double y1, double z1,
                           double x2, double y2, double z2);
  /// Set the position of the first point in the nth ruler from x, y, z coordinates
  void SetNthRulerPosition1(int n, double x, double y, double z);
  /// Set the position of the second point in the nth ruler from x, y, z coordinates
  void SetNthRulerPosition2(int n, double x, double y, double z);

  /// Set the position of the nth ruler from double array
  void SetNthRulerPositionFromArray(int n, double pos[6]);
  /// Set the position of the nth ruler from double arrays
  void SetNthRulerPositionFromArrays(int n, double pos1[3], double pos2[3]);
  /// Set the position of the first point in the nth ruler from a double array
  void SetNthRulerPosition1FromArray(int n, double pos[3]);
  /// Set the position of the second point in the nth ruler from a double array
  void SetNthRulerPosition2FromArray(int n, double pos[3]);

  /// Get selected property on Nth ruler
  bool GetNthRulerSelected(int n = 0);
  /// Set selected property on Nth ruler
  void SetNthRulerSelected(int n, bool flag);
  /// Get visibility property on Nth ruler
  bool GetNthRulerVisibility(int n = 0);
  /// Set visibility property on Nth ruler. If the visibility is set to
  /// true on the node/list as a whole, the nth ruler visibility is used to
  /// determine if it is visible. If the visibility is set to false on the node
  /// as a whole, all rulers are hidden but keep this value for when the
  /// list as a whole is turned visible.
  /// \sa vtkMRMLDisplayableNode::SetDisplayVisibility
  /// \sa vtkMRMLDisplayNode::SetVisibility
  void SetNthRulerVisibility(int n, bool flag);
  /// Get label on nth ruler
  std::string GetNthRulerLabel(int n = 0);
  /// Set label on nth ruler
  void SetNthRulerLabel(int n, std::string label);
  /// Get associated node id on nth ruler
  std::string GetNthRulerAssociatedNodeID(int n = 0);
  /// Set associated node id on nth ruler
  void SetNthRulerAssociatedNodeID(int n, const char* id);

  /// Set world coordinates on nth ruler from one array, two sets of four doubles
  void SetNthRulerWorldCoordinates(int n, double coords[8]);
  /// Set world coordinates on nth ruler
  void SetNthRulerWorldCoordinatesFromArrays(int n, double coords1[4], double coords2[4]);
  /// Set world coordinates on first point in nth ruler
  void SetNthRulerWorldCoordinates1(int n, double coords[4]);
  /// Set world coordinates on second point in nth ruler
  void SetNthRulerWorldCoordinates2(int n, double coords[4]);

  /// Get world coordinates on nth ruler in one array
  void GetNthRulerWorldCoordinates(int n, double coords[8]);
  /// Get world coordinates on nth ruler in two arrays
  void GetNthRulerWorldCoordinatesInArrays(int n, double coords1[4], double coords2[4]);

protected:
  vtkMRMLMarkupsRulerNode();
  ~vtkMRMLMarkupsRulerNode();
  vtkMRMLMarkupsRulerNode(const vtkMRMLMarkupsRulerNode&);
  void operator=(const vtkMRMLMarkupsRulerNode&);

};

#endif
