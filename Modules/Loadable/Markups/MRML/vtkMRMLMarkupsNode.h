// .NAME vtkMRMLMarkupsNode - MRML node to represent a markup
// .SECTION Description
// Markups nodes contains control points.
// Visualization parameters for these nodes are controlled by the vtkMRMLMarkupsDisplayNode class.
//

#ifndef __vtkMRMLMarkupsNode_h
#define __vtkMRMLMarkupsNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"

// VTK includes

#include <vtkSmartPointer.h>
#include <vtkVector.h>

class vtkStringArray;
class vtkMatrix4x4;

/// Each markup is defined by a certain number of RAS points,
/// 1 for fiducials, 2 for rulers, 3 for angles, etc. 
/// Each markup also has an associated node id, set when the markup 
/// is placed on a data set to link the markup to the volume or model.
/// Each markup can also be individually un/selected, un/locked, in/visibile,
/// and have a label (short, shown in the viewers) and description (longer,
/// shown in the GUI).
typedef struct
{
  std::string Label;
  std::string Description;
  std::string AssociatedNodeID;
  std::vector < vtkVector3d> points;
  bool Selected;
  bool Locked;
  bool Visibility;
} Markup;

/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsNode : public vtkMRMLDisplayableNode
{
public:
  static vtkMRMLMarkupsNode *New();
  vtkTypeMacro(vtkMRMLMarkupsNode,vtkMRMLDisplayableNode);

  void PrintMarkup(ostream&  os, vtkIndent indent, Markup *markup);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  virtual const char* GetIcon() {return "";};

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Markups";};

  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);
  
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Write this node's information to a string for passing to a CLI, write
  /// out the prefix before each markup
  virtual void WriteCLI(std::ostringstream& ss, std::string prefix);
  
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// currently only calls superclass UpdateScene
  void UpdateScene(vtkMRMLScene *scene);

  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );


  /// Description:
  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();  


  int AddText(const char *newText); 
  void SetText(int id, const char *newText);
  vtkStdString GetText(int id); 
  int DeleteText(int id); 

  int GetNumberOfTexts(); 

  /// Invoke events when markups change, passing the markup index if applicable
  /// invoke the lock modified event when a markup's lock status is changed.
  /// invoke the point modified event when a markup's location changes
  /// invoke the NthMarkupModifiedEvent event when a markup's non location value
  /// invoke the markup added event when adding a new markup to a markups node
  /// invoke the markup removed event when removing one or all markups from a node
  /// (caught by the displayable manager to make sure the widgets match the node)
  enum 
  {
    LockModifiedEvent = 19000,
    PointModifiedEvent,
    NthMarkupModifiedEvent,
    MarkupAddedEvent,
    MarkupRemovedEvent,
  };
  
  /// Description:
  /// Clear out the node of all markups
  virtual void RemoveAllMarkups();

  /// Description:
  /// Get/Set the Locked property on the markup.
  /// If set to 1 then parameters should not be changed 
  vtkGetMacro(Locked, int);
  void SetLocked(int locked);
  vtkBooleanMacro(Locked, int);

  /// Return true if n is a valid markup, false otherwise
  bool MarkupExists(int n);
  /// Return the number of markups that are stored in this node
  int GetNumberOfMarkups();
  /// Return true if p is a valid point in a valid markup n, false otherwise
  bool PointExistsInMarkup(int p, int n);
  /// Return the number of points in a markup, 0 if n is invalid
  int GetNumberOfPointsInNthMarkup(int n);
  /// Return a pointer to the nth markup stored in this node, null if n is out of bounds
  Markup * GetNthMarkup(int n);
  /// Initialise a markup to default values
  void InitMarkup(Markup *markup);
  /// Add a markup to the end of the list
  void AddMarkup(Markup markup);
  /// Create a new markup with n points, init points to (0,0,0). Return index
  /// of new markup, -1 on failure.
  int AddMarkupWithNPoints(int n);
  /// Create a new markup and add a point to it, returning the markup index
  int AddPointToNewMarkup(vtkVector3d point);
  /// Add a point to the nth markup, returning the point index
  int AddPointToNthMarkup(vtkVector3d point, int n);

  /// Get points
  vtkVector3d GetMarkupPointVector(int markupIndex, int pointIndex);
  void GetMarkupPoint(int markupIndex, int pointIndex, double point[3]);
  /// Return a three element double giving the world position (any parent
  /// transform on the markup applied to the return of GetMarkupPoint
  /// returns 0 on failure, 1 on success.
  int GetMarkupPointWorld(int markupIndex, int pointIndex, double worldxyz[4]);

  /// Remove a markup
  void RemoveMarkup(int m);

  /// Insert a markup in this list at targetIndex.
  /// If targetIndex is < 0, insert at the start of the list.
  /// If targetIndex is > list size - 1, append to end of list.
  /// Returns true on success, false on failure.
  bool InsertMarkup(Markup m, int targetIndex);

  /// Copy settings from source markup to target markup
  void CopyMarkup(Markup *source, Markup *target);
  
  /// Swap the position of two markups
  void SwapMarkups(int m1, int m2);

  /// Set a point in a markup
  void SetMarkupPointFromPointer(const int markupIndex, const int pointIndex, const double * pos);
  void SetMarkupPointFromArray(const int markupIndex, const int pointIndex, const double pos[3]);
  void SetMarkupPoint(const int markupIndex, const int pointIndex, const double x, const double y, const double z);
  /// Set the mth markup's point p to xyz transformed by the inverse of the transform to world for the node. Calls SetMarkupPoint after transforming the passed in coordinate
  void SetMarkupPointWorld(const int markupIndex, const int pointIndex, const double x, const double y, const double z);

  /// Get/Set the associated node id for the nth markup
  std::string GetNthMarkupAssociatedNodeID(int n = 0);
  void SetNthMarkupAssociatedNodeID(int n, std::string id);

  /// Get/Set the Selected, Locked and Visibility flags on the nth markup.
  /// Get returns false if markup doesn't exist
  bool GetNthMarkupSelected(int n = 0);
  void SetNthMarkupSelected(int n, bool flag);
  bool GetNthMarkupLocked(int n = 0);
  void SetNthMarkupLocked(int n, bool flag);
  bool GetNthMarkupVisibility(int n = 0);
  void SetNthMarkupVisibility(int n, bool flag);
  /// Get/Set the Label on the nth markup
  std::string GetNthMarkupLabel(int n = 0);
  void SetNthMarkupLabel(int n, std::string label);
  /// Get/Set the Description on the nth markup
  std::string GetNthMarkupDescription(int n = 0);
  void SetNthMarkupDescription(int n, std::string description);

  /// transform utility functions
  virtual bool CanApplyNonLinearTransforms()const;
  virtual void ApplyTransformMatrix(vtkMatrix4x4* transformMatrix);
  virtual void ApplyTransform(vtkAbstractTransform* transform);  

  /// toggle using the name of the markups list node to name the new points added to it.
  /// if true, sets the label automatically as a numbered version of the list name.
  /// if false, defauls to using M as the default prefix
  vtkSetMacro(UseListNameForMarkups, int);
  vtkGetMacro(UseListNameForMarkups, int);
  vtkBooleanMacro (UseListNameForMarkups, int);

  /// Reimplemented to take into account the modified time of the markups
  /// Returns true if the node (default behavior) or the markups are modified
  /// since read/written.
  /// Note: The MTime of the markups node is used to know if it has been modified.
  /// So if you invoke class specific modified events without calling Modified() on the
  /// markups, GetModifiedSinceRead() won't return true.
  /// \sa vtkMRMLStorableNode::GetModifiedSinceRead()
  virtual bool GetModifiedSinceRead();
  
protected:
  vtkMRMLMarkupsNode();
  ~vtkMRMLMarkupsNode();
  vtkMRMLMarkupsNode(const vtkMRMLMarkupsNode&);
  void operator=(const vtkMRMLMarkupsNode&);

  vtkStringArray *TextList;

  /// vector of point sets, each markup can have N markups of the same type
  /// saved in the vector. 
  std::vector < Markup > Markups;
  
  int Locked;

  int UseListNameForMarkups;
  
};

#endif
