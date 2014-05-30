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

// .NAME vtkMRMLMarkupsRulerDisplayNode - MRML node to represent display properties for rulers
// .SECTION Description
// vtkMRMLMarkupsRulerDisplayNode nodes store display properties of ruler markups,
// expanding the superclass to record ruler end point display properties, and line
// display properties
//

#ifndef __vtkMRMLMarkupsRulerDisplayNode_h
#define __vtkMRMLMarkupsRulerDisplayNode_h

#include "vtkSlicerMarkupsModuleMRMLExport.h"

#include "vtkMRMLMarkupsDisplayNode.h"

/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsRulerDisplayNode : public vtkMRMLMarkupsDisplayNode
{
public:
  static vtkMRMLMarkupsRulerDisplayNode *New();
  vtkTypeMacro ( vtkMRMLMarkupsRulerDisplayNode,vtkMRMLMarkupsDisplayNode );
  void PrintSelf ( ostream& os, vtkIndent indent );

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance (  );

  // Read node attributes from XML (MRML) file
  virtual void ReadXMLAttributes ( const char** atts );

  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML ( ostream& of, int indent );


  // Copy the node's attributes to this object
  virtual void Copy ( vtkMRMLNode *node );

  // Get node XML tag name (like Volume, Markups)
  virtual const char* GetNodeTagName() {return "MarkupsRulerDisplay";};

  // Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );

  // Get/Set macros for the first point glyph color
  vtkSetVector3Macro(PointColor1, double);
  vtkGetVector3Macro(PointColor1, double);

  // Get/Set macros for the first point glyph selected color
  vtkSetVector3Macro(SelectedPointColor1, double);
  vtkGetVector3Macro(SelectedPointColor1, double);

  // Get/Set macros for the second point glyph color
  vtkSetVector3Macro(PointColor2, double);
  vtkGetVector3Macro(PointColor2, double);

  // Get/Set macros for the second point glyph selected color
  vtkSetVector3Macro(SelectedPointColor2, double);
  vtkGetVector3Macro(SelectedPointColor2, double);

  // Get/Set macros for the line color
  vtkSetVector3Macro(LineColor, double);
  vtkGetVector3Macro(LineColor, double);

  // Get/Set macros for the line selected color
  vtkSetVector3Macro(SelectedLineColor, double);
  vtkGetVector3Macro(SelectedLineColor, double);

  /// Get/Set for ruler line thickness
  vtkSetMacro(LineThickness, double);
  vtkGetMacro(LineThickness, double);

  /// Set SliceProjection to Dashed
  inline void SliceProjectionDashedOn();

  /// Set SliceProjection to Plain
  inline void SliceProjectionDashedOff();

  /// Set line colored when parallel to slice plane
  inline void SliceProjectionColoredWhenParallelOn();

  /// Set line color unchanged when parallel to slice plane
  inline void SliceProjectionColoredWhenParallelOff();

  /// Set line thicker when on top of the plane, thiner when under
  inline void SliceProjectionThickerOnTopOn();

  /// Set line thickness uniform
  inline void SliceProjectionThickerOnTopOff();

  /// Set projection color as ruler color
  ///\sa SetSliceProjectionColor
  inline void SliceProjectionUseRulerColorOn();

  /// Manually set projection color
  ///\sa SetSliceProjectionColor
  inline void SliceProjectionUseRulerColorOff();

  /// ProjectionDashed : Set projected line dashed
  /// ProjectionColoredWhenParallel : Set projected line
  /// colored when parallel to slice plane
  /// ProjectionThickerOnTop : Set projected line thicker
  /// on top of the plane, thiner when under
  /// Projection Off, Dashed, ColoredWhenParallel,
  /// ThickerOnTop and UseRulerColor by default
  /// \enum ProjectionFlag
  enum ProjectionFlag
  {
  ProjectionDashed = 0x02,
  ProjectionColoredWhenParallel = 0x04,
  ProjectionThickerOnTop = 0x08,
  ProjectionUseRulerColor = 0x10
  };

  /// Get/Set the thickness of the line under the plane
  /// Default: 1.0
  vtkSetMacro(UnderLineThickness, double);
  vtkGetMacro(UnderLineThickness, double);

  /// Get/Set the thickness of the line over the plane
  /// Default: 3.0
  vtkSetMacro(OverLineThickness, double);
  vtkGetMacro(OverLineThickness, double);

protected:
  vtkMRMLMarkupsRulerDisplayNode();
  ~vtkMRMLMarkupsRulerDisplayNode();
  vtkMRMLMarkupsRulerDisplayNode( const vtkMRMLMarkupsRulerDisplayNode& );
  void operator= ( const vtkMRMLMarkupsRulerDisplayNode& );

  /// holds the color information for the glyphs
  double PointColor1[3];
  double SelectedPointColor1[3];
  double PointColor2[3];
  double SelectedPointColor2[3];

  /// color of the ruler line
  double LineColor[3];
  /// color of the ruler line when it's selected
  double SelectedLineColor[3];


  /// thickness of the ruler line
  double LineThickness;

  /// projection settings
  double UnderLineThickness;
  double OverLineThickness;
};

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode
::SliceProjectionDashedOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkMRMLMarkupsRulerDisplayNode::ProjectionDashed);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode
::SliceProjectionDashedOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkMRMLMarkupsRulerDisplayNode::ProjectionDashed);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode
::SliceProjectionColoredWhenParallelOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkMRMLMarkupsRulerDisplayNode::ProjectionColoredWhenParallel);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode
::SliceProjectionColoredWhenParallelOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkMRMLMarkupsRulerDisplayNode::ProjectionColoredWhenParallel);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode
::SliceProjectionThickerOnTopOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkMRMLMarkupsRulerDisplayNode::ProjectionThickerOnTop);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode
::SliceProjectionThickerOnTopOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkMRMLMarkupsRulerDisplayNode::ProjectionThickerOnTop);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode
::SliceProjectionUseRulerColorOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkMRMLMarkupsRulerDisplayNode::ProjectionUseRulerColor);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsRulerDisplayNode
::SliceProjectionUseRulerColorOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkMRMLMarkupsRulerDisplayNode::ProjectionUseRulerColor);
}

#endif
