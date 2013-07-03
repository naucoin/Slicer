// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsStorageNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkURIHandler.h"
#include "vtkMRMLScene.h"
#include "vtkPolyData.h"

// VTK includes
#include <vtkNew.h>


int vtkMRMLMarkupsStorageNodeTest1(int argc, char * argv[] )
{
  vtkNew<vtkMRMLMarkupsStorageNode> node1;

  EXERCISE_BASIC_STORAGE_MRML_METHODS( vtkMRMLMarkupsStorageNode, node1 );

  vtkNew<vtkMRMLMarkupsNode> markupsNode;
  vtkNew<vtkMRMLMarkupsDisplayNode> displayNode;

  // get the file name
  std::string fileName = std::string("testMarkupsStorageNode.mcsv");
  if (argc > 1)
    {
    fileName = std::string(argv[1]);
    }
  std::cout << "Using file name " << fileName.c_str() << std::endl;
  
  // set up a scene
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  scene->AddNode(markupsNode.GetPointer());
  scene->AddNode(displayNode.GetPointer());
  markupsNode->SetAndObserveStorageNodeID(node1->GetID());
  markupsNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  // set some non default display node settings
  displayNode->SetPower(0.5);
  displayNode->SetOpacity(0.5);
  displayNode->SetAmbient(0.5);
  displayNode->SetTextScale(1.0);
  displayNode->SetGlyphType(4);
  displayNode->SetGlyphScale(3.5);
  std::cout << "Display node: " << std::endl;
  vtkIndent indent;
  displayNode->PrintSelf(std::cout, indent);

  // add a markup with one point with non default values
  int index =  markupsNode->AddMarkupWithNPoints(1);
  double orientation[4] = {0.2, 1.0, 0.0, 0.0};
  markupsNode->SetNthMarkupOrientationFromArray(index, orientation);
  markupsNode->ResetNthMarkupID(index);
  std::string associatedNodeID = std::string("testingAssociatedID");
  markupsNode->SetNthMarkupAssociatedNodeID(index,associatedNodeID);
  markupsNode->SetNthMarkupSelected(index, 0);
  markupsNode->SetNthMarkupVisibility(index, 0);
  markupsNode->SetNthMarkupLocked(index, 1);

  std::string label = std::string("Testing label");
  markupsNode->SetNthMarkupLabel(index, label);
  std::string desc = std::string("description with spaces");
  markupsNode->SetNthMarkupDescription(index, desc);
  double inputPoint[3] = {-9.9, 1.1, 0.87};
  markupsNode->SetMarkupPointFromArray(index, 0, inputPoint);
  // and add a markup with two points, default values
  markupsNode->AddMarkupWithNPoints(2);
  // and another one unsetting the label
  index = markupsNode->AddMarkupWithNPoints(1);
  markupsNode->SetNthMarkupLabel(index,"");

  // test write

  std::cout << "\nWriting this markup to file:" << std::endl;
  markupsNode->PrintSelf(std::cout, indent);
  std::cout << std::endl;

  node1->SetFileName(fileName.c_str());
  std::cout << "Writing " << node1->GetFileName() << std::endl;
  int retval = node1->WriteData(markupsNode.GetPointer());
  if (!retval)
    {
    std::cerr << "Failed to write to file " << node1->GetFileName() << std::endl;
    return EXIT_FAILURE;
    }

  // test read
  vtkNew<vtkMRMLMarkupsStorageNode> snode2;
  vtkNew<vtkMRMLMarkupsNode> markupsNode2;
  vtkNew<vtkMRMLScene> scene2;
  scene2->AddNode(snode2.GetPointer());
  scene2->AddNode(markupsNode2.GetPointer());
  markupsNode2->SetAndObserveStorageNodeID(snode2->GetID());
  snode2->SetFileName(node1->GetFileName());

  std::cout << "Reading from " << snode2->GetFileName() << std::endl;
  retval = snode2->ReadData(markupsNode2.GetPointer());
  if (!retval)
    {
    std::cerr << "Failed to read from file " << snode2->GetFileName() << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "\nMarkup read from file = " << std::endl;
  markupsNode2->PrintSelf(std::cout, indent);
  std::cout << std::endl;

  // test values on the first markup
  index = 0;
  double newOrientation[4];
  markupsNode2->GetNthMarkupOrientation(index, newOrientation);
  for (int r = 0; r < 4; r++)
    {
    if (newOrientation[r] != orientation[r])
      {
      std::cerr << "Failed to set orientation! "
                << "Expected: "
                << orientation[0] << ", "
                << orientation[1] << ", "
                << orientation[2] << ", "
                << orientation[3]
                << " but got: "
                << newOrientation[0] << ", "
                << newOrientation[1] << ", "
                << newOrientation[2] << ", "
                << newOrientation[3] << std::endl;
      return EXIT_FAILURE;
      }
    }
  if (markupsNode2->GetNthMarkupAssociatedNodeID(index).compare(associatedNodeID) != 0)
    {
    std::cerr << "After reading in, expected markup " << index << " associatedNodeID of " << associatedNodeID.c_str() << " got " << markupsNode2->GetNthMarkupAssociatedNodeID(index).c_str() << std::endl;
    return EXIT_FAILURE;
    }
  if (markupsNode2->GetNthMarkupSelected(index) != 0)
    {
    std::cerr << "After reading in, expected markup " << index << " sel of 0, got " << markupsNode2->GetNthMarkupSelected(index) << std::endl;
    return EXIT_FAILURE;
    }
  if (markupsNode2->GetNthMarkupVisibility(index) != 0)
    {
    std::cerr << "After reading in, expected markup " << index << " vis of 0, got " << markupsNode2->GetNthMarkupVisibility(index) << std::endl;
    return EXIT_FAILURE;
    }
  if (markupsNode2->GetNthMarkupLocked(index) != 1)
    {
    std::cerr << "After reading in, expected markup " << index << " locked of 0, got " << markupsNode2->GetNthMarkupLocked(index) << std::endl;
    return EXIT_FAILURE;
    }
  if (markupsNode2->GetNthMarkupLabel(index).compare(label) != 0)
    {
    std::cerr << "After reading in, expected markup " << index << " label of " << label.c_str() << " got '" << markupsNode2->GetNthMarkupLabel(index).c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }
  if (markupsNode2->GetNthMarkupDescription(index).compare(desc) != 0)
    {
    std::cerr << "After reading in, expected markup " << index << " description of " << desc.c_str() << " got '" << markupsNode2->GetNthMarkupDescription(index).c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }
  double outputPoint[3];
  markupsNode2->GetMarkupPoint(index, 0, outputPoint);
  double diff = fabs(outputPoint[0] - inputPoint[0]) + fabs(outputPoint[1] - inputPoint[1]) + fabs(outputPoint[2] - inputPoint[2]);
  if (diff > 0.1)
    {
    std::cerr << "After reading in, expected markup " << index << " point " << inputPoint[0] << "," << inputPoint[1] << "," << inputPoint[2] << " but got a diff of " << diff << " from read in point " << outputPoint[0] << "," << outputPoint[1] << "," << outputPoint[2] << std::endl;
    return EXIT_FAILURE;
    }

  // test the unset label on the third markup
  index = 2;
  if (markupsNode2->GetNthMarkupLabel(index).size() != 0)
    {
    std::cerr << "After reading in, expected markup " << index << " empty label, but got " << markupsNode2->GetNthMarkupLabel(index).c_str() << std::endl;
    return EXIT_FAILURE;
    }

  // now read it again with a display node defined
  vtkNew<vtkMRMLMarkupsDisplayNode> displayNode2;
  scene2->AddNode(displayNode2.GetPointer());
  markupsNode2->SetAndObserveDisplayNodeID(displayNode2->GetID());
  std::cout << "Added display node, re-reading from " << snode2->GetFileName() << std::endl;
  retval = snode2->ReadData(markupsNode2.GetPointer());
  if (!retval)
    {
    std::cerr << "Failed second read from file " << snode2->GetFileName() << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "\nDisplay node read from file = " << std::endl;
  displayNode2->PrintSelf(std::cout, indent);
  std::cout << std::endl;

  // test display stuff
  if (displayNode2->GetPower() != displayNode->GetPower() ||
      displayNode2->GetOpacity() != displayNode->GetOpacity() ||
      displayNode2->GetAmbient() != displayNode->GetAmbient() ||
      displayNode2->GetTextScale() != displayNode->GetTextScale() ||
      displayNode2->GetGlyphScale() != displayNode->GetGlyphScale() ||
      displayNode2->GetGlyphType() != displayNode->GetGlyphType())
    {
    std::cerr << "Display node properties not read back in correctly!" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Read display node properties correctly from file." << std::endl;
    }
  return EXIT_SUCCESS;
}
