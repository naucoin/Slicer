/// Markups Module MRML storage nodes
///
/// vtkMRMLMarkupsFiducialStorageNode - MRML node for handling markups storage
///
/// vtkMRMLMarkupsFiducialStorageNode nodes describe the markups storage
/// node that allows to read/write fiducuial point data from/to file.

#ifndef __vtkMRMLMarkupsFiducialStorageNode_h
#define __vtkMRMLMarkupsFiducialStorageNode_h

// MRML includes
#include "vtkMRMLStorageNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"

/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsFiducialStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLMarkupsFiducialStorageNode *New();
  vtkTypeMacro(vtkMRMLMarkupsFiducialStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "MarkupsFiducialStorage";};

  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Return a default file extension for writing
  virtual const char* GetDefaultWriteFileExtension();

  virtual bool CanReadInReferenceNode(vtkMRMLNode *refNode);

  /// Coordinate system options
  enum
  {
    RAS = 0,
    LPS,
    IJK
  };

  /// Get/Set flag that controls if points are to be written in various coordinate systems
  void SetCoordinateSystem(int system);
  int GetCoordinateSystem();
  std::string GetCoordinateSystemAsString();
  /// Convenience methods to get/set various coordinate system values
  /// \sa SetCoordinateSystem, GetCoordinateSystem
  void UseRASOn();
  bool GetUseRAS();
  void UseLPSOn();
  bool GetUseLPS();
  void UseIJKOn();
  bool GetUseIJK();

protected:
  vtkMRMLMarkupsFiducialStorageNode();
  ~vtkMRMLMarkupsFiducialStorageNode();
  vtkMRMLMarkupsFiducialStorageNode(const vtkMRMLMarkupsFiducialStorageNode&);
  void operator=(const vtkMRMLMarkupsFiducialStorageNode&);

  /// Initialize all the supported write file types
  virtual void InitializeSupportedReadFileTypes();

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// Write data from a  referenced node.
  /// Assumes 1 point per markup for a fiducial referenced node:
  /// x,y,z,ow,ox,oy,oz,vis,sel,lock,label,id,desc,associatedNodeID
  /// orientation is a quaternion, angle and axis
  /// associatedNodeID and description can be empty strings
  /// x,y,z,ow,ox,oy,oz,vis,sel,lock,label,id,,
  /// label can have spaces, everything up to next comma is used, no quotes
  /// necessary, same with the description
  virtual int WriteDataInternal(vtkMRMLNode *refNode);

private:

  /// flag set to enum RAS if the points are to be written out/read in using
  /// the RAS coordinate system, enum LPS if the points are to be written
  /// out/read in using LPS coordinate system, enum IJK if the points are
  /// to be written out in the IJK coordinates for the associated volume node.
  int CoordinateSystem;
};

#endif
