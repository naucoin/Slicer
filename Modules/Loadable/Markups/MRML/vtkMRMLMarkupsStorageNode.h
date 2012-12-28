/// Markups Module MRML storage nodes
///
/// vtkMRMLMarkupsStorageNode - MRML node for handling markups storage
/// 
/// vtkMRMLMarkupsStorageNode nodes describe the markups storage
/// node that allows to read/write point data from/to file.

#ifndef __vtkMRMLMarkupsStorageNode_h
#define __vtkMRMLMarkupsStorageNode_h

// MRML includes
#include "vtkMRMLStorageNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"

/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLMarkupsStorageNode *New();
  vtkTypeMacro(vtkMRMLMarkupsStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "MarkupsStorage";};

  /// 
  /// Return a default file extension for writing
  virtual const char* GetDefaultWriteFileExtension();

  virtual bool CanReadInReferenceNode(vtkMRMLNode *refNode);

protected:
  vtkMRMLMarkupsStorageNode();
  ~vtkMRMLMarkupsStorageNode();
  vtkMRMLMarkupsStorageNode(const vtkMRMLMarkupsStorageNode&);
  void operator=(const vtkMRMLMarkupsStorageNode&);

  /// Initialize all the supported write file types
  virtual void InitializeSupportedReadFileTypes();

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// Write data from a  referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode);
};

#endif



