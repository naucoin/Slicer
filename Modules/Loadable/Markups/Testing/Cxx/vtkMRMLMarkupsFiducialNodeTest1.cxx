// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkURIHandler.h"

// VTK includes
#include <vtkNew.h>
#include <vtkPolyData.h>

int vtkMRMLMarkupsFiducialNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLMarkupsFiducialNode> node1;

  EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS( vtkMRMLMarkupsFiducialNode, node1 );

  vtkMRMLMarkupsDisplayNode *dispNode = node1->GetMarkupsDisplayNode();
  std::cout << "Get MarkupsDisplayNode returned " << (dispNode ? "valid" : "null") << " pointer" << std::endl;

  // set position
  double inPos[3] = {0.33, 1.55, -99.0};
  int fidIndex = node1->AddFiducial(inPos[0], inPos[1], inPos[2]);
  double outPos[3];
  node1->GetNthFiducialPosition(0, outPos);
  double diff = sqrt(vtkMath::Distance2BetweenPoints(inPos, outPos));
  std::cout << "Diff between AddFiducial and GetNthFiducialPosition = " << diff << std::endl;
  if (diff > 0.1)
    {
    return EXIT_FAILURE;
    }
  inPos[1] = 15.55;
  int fidIndex2 = node1->AddFiducialFromArray(inPos);

  // selected
  node1->SetNthFiducialSelected(fidIndex2, false);
  bool retval = node1->GetNthFiducialSelected(fidIndex2);
  if (retval != false)
    {
    std::cerr << "Error setting/getting selected to false on fid " << fidIndex << std::endl;
    return EXIT_FAILURE;
    }

  // visibility
  node1->SetNthFiducialVisibility(fidIndex2, false);
  retval = node1->GetNthFiducialVisibility(fidIndex2);
  if (retval != false)
    {
    std::cerr << "Error setting/getting visibility to false on fid " << fidIndex << std::endl;
    return EXIT_FAILURE;
    }

  // label
  node1->SetNthFiducialLabel(fidIndex2, std::string("TestingLabelHere"));
  std::string returnLabel = node1->GetNthFiducialLabel(fidIndex2);
  if (returnLabel.compare("TestingLabelHere") != 0)
    {
    std::cerr << "Failure to set/get label for fid " << fidIndex2 << ", got '" << returnLabel.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }

  // associated node id
  std::string inID = "vtkMRMLScalarVolumeNode21";
  node1->SetNthFiducialAssociatedNodeID(fidIndex2, inID.c_str());
  std::string outID = node1->GetNthFiducialAssociatedNodeID(fidIndex2);
  if (outID.compare(inID) != 0)
    {
    std::cerr << "Failed to set fid " << fidIndex2 << " assoc node id to " << inID.c_str() << ", got '" << outID << "'" << std::endl;
    return EXIT_FAILURE;
    }

  // world coords
  double inCoords[4] = {0.4, 10.5, -8, 1.0};
  node1->SetNthFiducialWorldCoordinates(fidIndex2, inCoords);
  double outCoords[4];
  node1->GetNthFiducialWorldCoordinates(fidIndex2, outCoords);
  diff = sqrt(vtkMath::Distance2BetweenPoints(inCoords, outCoords));
  std::cout << "Diff between set world and get world coords = " << diff << std::endl;
  if (diff > 0.1)
    {
    return EXIT_FAILURE;
    }

  node1->Print(std::cout);

  return EXIT_SUCCESS;
}