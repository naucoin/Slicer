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

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkSlicerMarkupsLogic.h"
// VTK includes
#include <vtkNew.h>


int vtkSlicerMarkupsLogicTest1(int , char * [] )
{
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkNew<vtkSlicerMarkupsLogic> logic1;

  // test without a scene
  std::string id = logic1->AddNewFiducialNode();
  if (id.compare("") != 0)
    {
    std::cerr << "Failure to add a new markup node to empty scene, got id of '" << id.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Passed adding a node to no scene." << std::endl;
    }
  
  // test with a scene
  logic1->SetMRMLScene(scene);

  const char *testName = "Test node 2";
  id = logic1->AddNewFiducialNode(testName);
  if (id.compare("") == 0)
    {
    std::cerr << "Failure to add a new node to a valid scene, got id of '" << id.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    vtkMRMLNode *mrmlNode = scene->GetNodeByID(id.c_str());
    if (!mrmlNode)
      {
      std::cerr << "Failure to add a new node to a valid scene, couldn't find node with id'" << id.c_str() << "'" << std::endl;
      return EXIT_FAILURE;
      }
    char *name = mrmlNode->GetName();
    if (!name || strcmp(testName, name) != 0)
      {
      std::cerr << "Failed to set a name on the new node, got node name of '" << (name ? name : "null") << "'" << std::endl;
      return EXIT_FAILURE;
      }
    std::cout << "Added a new markup node to the scene, id = '" << id.c_str() << "', name = '" << name << "'" <<  std::endl;
    vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    if (!markupsNode)
      {
      std::cerr << "Failed to get the new node as a markups node" << std::endl;
      return EXIT_FAILURE;
      }
    // test the list stuff
    logic1->SetAllMarkupsVisibility(NULL, true);
    logic1->SetAllMarkupsLocked(NULL, false);
    logic1->SetAllMarkupsSelected(NULL, true);

    // no points
    logic1->SetAllMarkupsVisibility(markupsNode, false);
    logic1->SetAllMarkupsVisibility(markupsNode, true);
    logic1->SetAllMarkupsLocked(markupsNode, true);
    logic1->SetAllMarkupsLocked(markupsNode, false);
    logic1->SetAllMarkupsSelected(markupsNode, false);
    logic1->SetAllMarkupsSelected(markupsNode, true);

    // add some points
    markupsNode->AddMarkupWithNPoints(5);
    logic1->SetAllMarkupsVisibility(markupsNode, false);
    logic1->SetAllMarkupsVisibility(markupsNode, true);
    logic1->SetAllMarkupsLocked(markupsNode, true);
    logic1->SetAllMarkupsLocked(markupsNode, false);
    logic1->SetAllMarkupsSelected(markupsNode, false);
    logic1->SetAllMarkupsSelected(markupsNode, true);
    }

  // test the default display node settings
  vtkSmartPointer<vtkMRMLMarkupsDisplayNode> displayNode = vtkSmartPointer<vtkMRMLMarkupsDisplayNode>::New();
  TEST_SET_GET_INT_RANGE(logic1, DefaultMarkupsDisplayNodeGlyphType, displayNode->GetMinimumGlyphType(), displayNode->GetMaximumGlyphType());
  TEST_SET_GET_DOUBLE_RANGE(logic1, DefaultMarkupsDisplayNodeGlyphScale, 0.0, 10.0);
  TEST_SET_GET_DOUBLE_RANGE(logic1, DefaultMarkupsDisplayNodeTextScale, 0.0, 15.0);
  TEST_SET_GET_DOUBLE_RANGE(logic1, DefaultMarkupsDisplayNodeOpacity, 0.0, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(logic1, DefaultMarkupsDisplayNodeColor, 0.0, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(logic1, DefaultMarkupsDisplayNodeSelectedColor, 0.0, 1.0);

  // make a test display node and reset it to defaults
  int originalGlyphType = logic1->GetDefaultMarkupsDisplayNodeGlyphType();
  double originalGlyphScale = logic1->GetDefaultMarkupsDisplayNodeGlyphScale();
  double originalTextScale = logic1->GetDefaultMarkupsDisplayNodeTextScale();
  int glyphType = 3;
  displayNode->SetGlyphType(glyphType);
  double textScale = 3.33;
  displayNode->SetTextScale(textScale);
  double glyphScale = 0.33;
  displayNode->SetGlyphScale(glyphScale);

  // reset the display node to defaults
  logic1->SetDisplayNodeToDefaults(displayNode);
  // check that the logic didn't change
  if (logic1->GetDefaultMarkupsDisplayNodeGlyphType() != originalGlyphType)
    {
    std::cerr << "Error resetting display node glyph type to " << originalGlyphType << ", logic was changed and now have glyph type: " << logic1->GetDefaultMarkupsDisplayNodeGlyphType() << std::endl;
    return EXIT_FAILURE;
    }
  // check that the display node is changed
  if (displayNode->GetGlyphType() != originalGlyphType)
    {
    std::cerr << "Error resetting display node glyph type to defaults, was expecting  " << originalGlyphType << ", but got " << displayNode->GetGlyphType() << std::endl;
    return EXIT_FAILURE;
    }
  if (displayNode->GetGlyphScale() != originalGlyphScale)
    {
    std::cerr << "Error resetting display node glyph scale to defaults, was expecting  " << originalGlyphScale << ", but got " << displayNode->GetGlyphScale() << std::endl;
    return EXIT_FAILURE;
    }
  if (displayNode->GetTextScale() != originalTextScale)
    {
    std::cerr << "Error resetting display node text scale to defaults, was expecting  " << originalTextScale << ", but got " << displayNode->GetTextScale() << std::endl;
    return EXIT_FAILURE;
    }
  // cleanup
  
  return EXIT_SUCCESS;
}
