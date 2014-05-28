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
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsRulerNode.h"
#include "vtkURIHandler.h"

// VTK includes
#include <vtkNew.h>
#include <vtkPolyData.h>

int vtkMRMLMarkupsRulerNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLMarkupsRulerNode> node1;

  EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS( vtkMRMLMarkupsRulerNode, node1 );

  node1->SetName("rulerTest");

  vtkMRMLMarkupsDisplayNode *dispNode = node1->GetMarkupsDisplayNode();
  std::cout << "Get MarkupsDisplayNode returned " << (dispNode ? "valid" : "null") << " pointer" << std::endl;
  vtkMRMLMarkupsRulerDisplayNode *rulerDispNode = node1->GetMarkupsRulerDisplayNode();
  std::cout << "Get Markups Ruler Display node returned " << (rulerDispNode ? "valid" : "null") << " pointer" << std::endl;

  std::cout << "Number of rulers = " << node1->GetNumberOfRulers() << std::endl;

  // add rulers
  double inPos1[3] = {0.33, 1.55, -99.0};
  double inPos2[3] = {0.66, 3.55, 99.0};
  int rulerIndex = node1->AddRuler(inPos1[0], inPos1[1], inPos1[2],
                                   inPos2[0], inPos2[1], inPos2[2]);
  double outPos1[3], outPos2[3];
  node1->GetNthRulerPositionInArrays(0, outPos1, outPos2);
  double diff1 = sqrt(vtkMath::Distance2BetweenPoints(inPos1, outPos1));
  double diff2 = sqrt(vtkMath::Distance2BetweenPoints(inPos2, outPos2));
  std::cout << "Diff between AddRuler and GetNthRulerPosition 1 = " << diff1 << std::endl;
  std::cout << "Diff between AddRuler and GetNthRulerPosition 2 = " << diff2 << std::endl;
  if (diff1 > 0.1 || diff2 > 0.1)
    {
    return EXIT_FAILURE;
    }

  double inPos6[6] = {0.33, 155.5, -99.0, 0.66, 3.55, 99.0};
  int rulerIndex2 = node1->AddRulerFromArray(inPos6);
  node1->GetNthRulerPosition1(rulerIndex2, outPos1);
  if (outPos1[0] != inPos6[0] ||
      outPos1[1] != inPos6[1] ||
      outPos1[2] != inPos6[2])
    {
    std::cerr << "Adding Ruler from a single 6 elment array failed on first point"
              << std::endl;
    return EXIT_FAILURE;
    }
  node1->GetNthRulerPosition2(rulerIndex2, outPos2);
  if (outPos2[0] != inPos6[3] ||
      outPos2[1] != inPos6[4] ||
      outPos2[2] != inPos6[5])
    {
      std::cerr << "Adding Ruler from a single 6 elment array failed on second point"
                << std::endl;
      return EXIT_FAILURE;
    }

  inPos1[1] = 15.55;
  int rulerIndex3 = node1->AddRulerFromArrays(inPos1, inPos2);
  node1->GetNthRulerPosition1(rulerIndex3, outPos1);
  if (outPos1[1] != inPos1[1])
    {
    std::cerr << "Adding ruler from two arrays failed" << std::endl;
    return EXIT_FAILURE;
    }


  // set/get position
  double x1 = 0.99, y1 = 1.33, z1 = -9.0;
  double x2 = -0.99, y2 = -1.33, z2 = 9.0;
  node1->SetNthRulerPosition(rulerIndex2, x1, y1, z1, x2, y2, z2);
  double outPos6[6];
  node1->GetNthRulerPosition(rulerIndex2, outPos6);
  if (outPos6[0] != x1 ||
      outPos6[1] != y1 ||
      outPos6[2] != z1 ||
      outPos6[3] != x2 ||
      outPos6[4] != y2 ||
      outPos6[5] != z2)
    {
    std::cerr << "Failed to set ruler position from 6 doubles, getting as an array of 6" << std::endl;
    return EXIT_FAILURE;
    }

  node1->SetNthRulerPosition1(rulerIndex2, x2, y2, z2);
  node1->GetNthRulerPosition1(rulerIndex2, outPos1);
  if (outPos1[0] != x2 ||
      outPos1[1] != y2 ||
      outPos1[2] != z2)
    {
    std::cerr << "Failed to set ruler " << rulerIndex2 << " position 1 from 3 doubles" << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetNthRulerPosition2(rulerIndex2, x1, y1, z1);
  node1->GetNthRulerPosition2(rulerIndex2, outPos2);
  if (outPos2[0] != x1 ||
      outPos2[1] != y1 ||
      outPos2[2] != z1)
    {
    std::cerr << "Failed to set ruler position 2 from 3 doubles" << std::endl;
    return EXIT_FAILURE;
    }

  node1->SetNthRulerPositionFromArray(rulerIndex2,inPos6);
  node1->GetNthRulerPosition(rulerIndex2, outPos6);
  if (outPos6[0] != inPos6[0] ||
      outPos6[1] != inPos6[1] ||
      outPos6[2] != inPos6[2] ||
      outPos6[3] != inPos6[3] ||
      outPos6[4] != inPos6[4] ||
      outPos6[5] != inPos6[5])
    {
    std::cerr << "Failed to set ruler position from array of 6, getting as an array of 6" << std::endl;
    return EXIT_FAILURE;
    }

  double end1In[3];
  end1In[0] = 0.99;
  end1In[1] = 1.33;
  end1In[2] = -9.0;
  double end2In[3];
  end2In[0] = 45.45;
  end2In[1] = -9848484;
  end2In[2] = -0.000067;
  node1->SetNthRulerPosition1FromArray(rulerIndex2, end1In);
  node1->SetNthRulerPosition2FromArray(rulerIndex2, end2In);
  double end1Out[3], end2Out[3];
  node1->GetNthRulerPositionInArrays(rulerIndex2, end1Out, end2Out);
  diff1 = sqrt(vtkMath::Distance2BetweenPoints(end1In, end1Out));
  diff2 = sqrt(vtkMath::Distance2BetweenPoints(end2In, end2Out));
  std::cout << "Diff between set nth ruler postion array and get for end 1 = " << diff1 << std::endl;
  std::cout << "Diff between set nth ruler postion array and get for end 2 = " << diff2 << std::endl;
  if (diff1 > 0.1 || diff2 > 0.1)
    {
    return EXIT_FAILURE;
    }

  // position as points
  end1In[1] = -4.5;
  node1->SetNthRulerPosition1(rulerIndex2, end1In[0], end1In[1], end1In[2]);
  node1->GetNthRulerPosition1(rulerIndex2, end1Out);
  diff1 = sqrt(vtkMath::Distance2BetweenPoints(end1In, end1Out));
  std::cout << "Diff between set nth ruler postion 1 and get = " << diff1 << std::endl;
  if (diff1 > 0.1)
    {
    return EXIT_FAILURE;
    }

  // selected
  node1->SetNthRulerSelected(rulerIndex2, false);
  bool retval = node1->GetNthRulerSelected(rulerIndex2);
  if (retval != false)
    {
    std::cerr << "Error setting/getting selected to false on fid " << rulerIndex << std::endl;
    return EXIT_FAILURE;
    }

  // visibility
  node1->SetNthRulerVisibility(rulerIndex2, false);
  retval = node1->GetNthRulerVisibility(rulerIndex2);
  if (retval != false)
    {
    std::cerr << "Error setting/getting visibility to false on fid " << rulerIndex << std::endl;
    return EXIT_FAILURE;
    }

  // label
  node1->SetNthRulerLabel(rulerIndex2, std::string("TestingLabelHere"));
  std::string returnLabel = node1->GetNthRulerLabel(rulerIndex2);
  if (returnLabel.compare("TestingLabelHere") != 0)
    {
    std::cerr << "Failure to set/get label for fid " << rulerIndex2 << ", got '" << returnLabel.c_str() << "'" << std::endl;
    return EXIT_FAILURE;
    }

  // associated node id
  std::string inID = "vtkMRMLScalarVolumeNode21";
  node1->SetNthRulerAssociatedNodeID(rulerIndex2, inID.c_str());
  std::string outID = node1->GetNthRulerAssociatedNodeID(rulerIndex2);
  if (outID.compare(inID) != 0)
    {
    std::cerr << "Failed to set ruler " << rulerIndex2 << " assoc node id to " << inID.c_str() << ", got '" << outID << "'" << std::endl;
    return EXIT_FAILURE;
    }

 // world coords
  double inCoords[8] = {0.4, 10.5, -8, 1.0, -0.4, -10.5, 8.0, 1.0};
  double outCoords[8];
  node1->SetNthRulerWorldCoordinates(rulerIndex2, inCoords);
  node1->GetNthRulerWorldCoordinates(rulerIndex2, outCoords);
  for (unsigned int i = 0; i < 8; ++i)
    {
    if (inCoords[i] != outCoords[i])
      {
      std::cout << "Failed to set/get world coordinates, element "
                << i << " doe not match: "
                << inCoords[i] << " != " << outCoords[i]
                << std::endl;
      return EXIT_FAILURE;
      }
    }

  double inCoords1[4] = {0.4, 10.5, -8, 1.0};
  double inCoords2[4] = {-0.4, -10.5, 8.0, 1.0};
  double outCoords1[4], outCoords2[4];
  node1->SetNthRulerWorldCoordinatesFromArrays(rulerIndex2, inCoords1, inCoords2);

  node1->GetNthRulerWorldCoordinatesInArrays(rulerIndex2, outCoords1, outCoords2);
  diff1 = sqrt(vtkMath::Distance2BetweenPoints(inCoords1, outCoords1));
  diff2 = sqrt(vtkMath::Distance2BetweenPoints(inCoords2, outCoords2));
  std::cout << "Diff between set world and get world coords 1 = " << diff1
            << ", 2 = " << diff2 << std::endl;
  if (diff1 > 0.1 || diff2 > 0.1)
    {
    return EXIT_FAILURE;
    }

  inCoords1[1] = 44.44;
  inCoords2[1] = -44.445;
  node1->SetNthRulerWorldCoordinates1(rulerIndex2, inCoords1);
  node1->SetNthRulerWorldCoordinates2(rulerIndex2, inCoords2);
  node1->GetNthRulerWorldCoordinatesInArrays(rulerIndex2, outCoords1, outCoords2);
  diff1 = sqrt(vtkMath::Distance2BetweenPoints(inCoords1, outCoords1));
  diff2 = sqrt(vtkMath::Distance2BetweenPoints(inCoords2, outCoords2));
  std::cout << "Diff between set world and get world coords via arrays, end 1 = " << diff1
            << ", end 2 = " << diff2 << std::endl;
  if (diff1 > 0.1 || diff2 > 0.1)
    {
    return EXIT_FAILURE;
    }

  // distance measurement
  double d1 = node1->GetDistanceMeasurement(rulerIndex2);
  double d2 = node1->GetDistanceMeasurementWorld(rulerIndex2);
  std::cout << "Distance measurement from ruler " << rulerIndex2
            << ", d1 = " << d1 << ", d2 = " << d2 << std::endl;
  // no transform so should be the same
  if (d1 != d2)
    {
    std::cerr << "Error getting distance measurement from ruler " << rulerIndex2
              << ", " << d1 << " != " << d2 << std::endl;
    return EXIT_FAILURE;
    }
  // test with equal points
  int zeroIndex = node1->AddRuler(0.0, 0.0, 0.0,
                                  0.0, 0.0, 0.0);
  d1 = node1->GetDistanceMeasurement(zeroIndex);
  std::cout << "0-0 distance measurement = " << d1 << std::endl;
  if (d1 != 0.0)
    {
    std::cerr << "Error getting 0 distance measurement between identical end points, "
              << d1 << " != 0.0" << std::endl;
    return EXIT_FAILURE;
    }
  node1->Print(std::cout);

  return EXIT_SUCCESS;
}
