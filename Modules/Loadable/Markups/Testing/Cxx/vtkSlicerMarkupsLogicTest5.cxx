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
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkSlicerMarkupsLogic.h"
// VTK includes
#include <vtkNew.h>

// test adding rulers
int vtkSlicerMarkupsLogicTest5(int , char * [] )
{
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkNew<vtkSlicerMarkupsLogic> logic1;

  // test without a scene
  std::string id = logic1->AddNewRulerNode();
  if (id.compare("") != 0)
    {
    std::cerr << "Failure to add a new markup node to empty scene, got id of '" << id.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Passed adding a node to no scene." << std::endl;
    }

  int rulerIndex = logic1->AddRuler();
  if (rulerIndex != -1)
    {
    std::cerr << "Failure to add a new ruler to empty scene, got index of '"
              << rulerIndex << "'" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Passed adding a ruler to no scene." << std::endl;
    }

  // test with a scene
  logic1->SetMRMLScene(scene);

  const char *testName = "Test node 2";
  id = logic1->AddNewRulerNode(testName);
  if (id.compare("") == 0)
    {
    std::cerr << "Failure to add a new node to a valid scene, got id of '" << id.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }

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

  // test without a selection node
  rulerIndex = logic1->AddRuler(5.0, 6.0, -7.0, 8.8, -9.0, 10.5);
  if (rulerIndex != -1)
    {
    std::cerr << "Failed on adding a ruler to the scene with no selection node, expected index of -1, but got: "
              << rulerIndex << std::endl;
      return EXIT_FAILURE;
    }

  if (logic1->StartPlaceMode(0))
    {
    std::cerr << "Failed to fail starting place mode!" << std::endl;
    return EXIT_FAILURE;
    }

  // add a selection node
  vtkMRMLApplicationLogic* applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene);

  if (!logic1->StartPlaceMode(1))
    {
    std::cerr << "Failed to start place mode after adding a selection node!"
              << std::endl;
    return EXIT_FAILURE;
    }

  // test adding a ruler to an active list - no app logic
  rulerIndex = logic1->AddRuler(-1.1, 100.0, 500.0, 1.1, -100.0, -1.1);
  if (rulerIndex == -1)
    {
    std::cerr << "Failed to add a ruler to the active ruler list in the scene, got index of "
              << rulerIndex << std::endl;
      return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Added a ruler to the active ruler list, index = " << rulerIndex << std::endl;
    }


  // adding with app logic
  logic1->SetMRMLApplicationLogic(applicationLogic);
  rulerIndex = logic1->AddRuler(-11, 10.0, 50.0, 11, -10.0, -50.0);
  if (rulerIndex == -1)
    {
    std::cerr << "Failed to add a ruler to the active ruler list from the app logic, got index of "
              << rulerIndex << std::endl;
      return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Added a ruler to the active ruler list, index = " << rulerIndex << std::endl;
    }

  std::string activeListID = logic1->GetActiveListID();
  mrmlNode = scene->GetNodeByID(activeListID.c_str());
  vtkMRMLMarkupsNode *activeMarkupsNode = NULL;
  if (mrmlNode)
    {
    activeMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (!activeMarkupsNode)
    {
    std::cerr << "Failed to get active markups list from id " << activeListID.c_str() << std::endl;
    applicationLogic->Delete();
    return EXIT_FAILURE;
    }

  // test setting active list id
  std::string newID = logic1->AddNewRulerNode("New list", scene);
  activeListID = logic1->GetActiveListID();
  if (activeListID.compare(newID) != 0)
    {
    std::cerr << "Failed to set new ruler node active. newID = "
              << newID.c_str() << ", active id = "
              << activeListID.c_str() << std::endl;
    return EXIT_FAILURE;
    }
  // set the old one active
  logic1->SetActiveListID(activeMarkupsNode);
  activeListID = logic1->GetActiveListID();
  if (activeListID.compare(activeMarkupsNode->GetID()) != 0)
    {
    std::cerr << "Failed to set old ruler node active. old id = "
              << activeMarkupsNode->GetID() << ", current active id = "
              << activeListID.c_str() << std::endl;
    return EXIT_FAILURE;
    }

  // cleanup
  applicationLogic->Delete();

  return EXIT_SUCCESS;
}
