// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkSlicerMarkupsLogic.h"
#include "vtkMRMLScene.h"

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
      std::cout << "Failed to set a name on the new node, got node name of '" << (name ? name : "null") << "'" << std::endl;
      return EXIT_FAILURE;
      }
    std::cout << "Added a new markup node to the scene, id = '" << id.c_str() << "', name = '" << name << "'" <<  std::endl;
    
    }

  // cleanup
  
  return EXIT_SUCCESS;
}
