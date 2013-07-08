// .NAME vtkMRMLMarkupsFiducialNode - MRML node to represent a fiducial markup
// .SECTION Description
// Markups nodes contains control points.
// Visualization parameters for these nodes are controlled by the vtkMRMLMarkupsDisplayNode class.
//

#ifndef __vtkMRMLMarkupsFiducialNode_h
#define __vtkMRMLMarkupsFiducialNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"

// VTK includes

#include <vtkSmartPointer.h>
//#include <vtkVector.h>

/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsFiducialNode : public vtkMRMLMarkupsNode
{
public:
  static vtkMRMLMarkupsFiducialNode *New();
  vtkTypeMacro(vtkMRMLMarkupsFiducialNode,vtkMRMLMarkupsNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char* GetIcon() {return ":/Icons/MarkupsMouseModePlace.png";};

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "MarkupsFiducial";};

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);
  
  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);


  void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );


  // Description:
  // Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  /// Return a cast display node, returns null if none
  vtkMRMLMarkupsDisplayNode *GetMarkupsDisplayNode();

  // wrap some of the generic markup methods
  /// add a new fiducial and return the fiducial index
  int AddFiducial(double x, double y, double z);
  int AddFiducialFromArray(double pos[3]);
  /// Get the position of the nth fiducial
  void GetNthFiducialPosition(int n, double pos[3]);
  /// Get/Set selected property on Nth fiducial
  bool GetNthFiducialSelected(int n = 0);
  void SetNthFiducialSelected(int n, bool flag);
  /// Get/Set visibility property on Nth fiducial
  bool GetNthFiducialVisibility(int n = 0);
  void SetNthFiducialVisibility(int n, bool flag);
  /// Get/Set label on nth fiducial
  std::string GetNthFiducialLabel(int n = 0);
  void SetNthFiducialLabel(int n, std::string label);
  /// Get/Set associated node id on nth fiducial
  std::string GetNthFiducialAssociatedNodeID(int n = 0);
  void SetNthFiducialAssociatedNodeID(int n, const char* id);
  /// Get/Set world coordinates on nth fiducial
  void SetNthFiducialWorldCoordinates(int n, double coords[4]);
  void GetNthFiducialWorldCoordinates(int n, double coords[4]);

protected:
  vtkMRMLMarkupsFiducialNode();
  ~vtkMRMLMarkupsFiducialNode();
  vtkMRMLMarkupsFiducialNode(const vtkMRMLMarkupsFiducialNode&);
  void operator=(const vtkMRMLMarkupsFiducialNode&);

};

#endif
