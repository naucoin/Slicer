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
  std::string id = logic1->AddNewMarkupsNode();
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

  id = logic1->AddNewMarkupsNode();
  if (id.compare("") == 0)
    {
    std::cerr << "Failure to add a new node to a valid scene, got id of '" << id.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Added a new markup node to the scene, id = '" << id.c_str() << "'" << std::endl;
    }

  // cleanup
  
  return EXIT_SUCCESS;
}
