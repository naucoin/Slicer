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


  // Get/Set macros for the second point glyph color
  vtkSetVector3Macro(Color2, double);
  vtkGetVector3Macro(Color2, double);

  // Get/Set macros for the second point glyph selected color
  vtkSetVector3Macro(SelectedColor2, double);
  vtkGetVector3Macro(SelectedColor2, double);

  // Get/Set macros for the line color
  vtkSetVector3Macro(LineColor, double);
  vtkGetVector3Macro(LineColor, double);

  // Get/Set macros for the line selected color
  vtkSetVector3Macro(SelectedLineColor, double);
  vtkGetVector3Macro(SelectedLineColor, double);

protected:
  vtkMRMLMarkupsRulerDisplayNode();
  ~vtkMRMLMarkupsRulerDisplayNode();
  vtkMRMLMarkupsRulerDisplayNode( const vtkMRMLMarkupsRulerDisplayNode& );
  void operator= ( const vtkMRMLMarkupsRulerDisplayNode& );
  /// holds the color information for the second glyph
  double Color2[3];
  double SelectedColor2[3];

  /// color of the ruler line
  double LineColor[3];
  /// color of the ruler line when it's selected
  double SelectedLineColor[3];


};

#endif
