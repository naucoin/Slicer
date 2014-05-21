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
#include "vtkMRMLMarkupsRulerDisplayNode.h"
#include "vtkMRMLMarkupsRulerStorageNode.h"
#include "vtkMRMLMarkupsRulerNode.h"
#include "vtkURIHandler.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <algorithm>

int vtkMRMLMarkupsRulerStorageNodeTest1(int argc, char * argv[] )
{
  vtkNew<vtkMRMLMarkupsRulerStorageNode> node1;

  EXERCISE_BASIC_STORAGE_MRML_METHODS( vtkMRMLMarkupsRulerStorageNode, node1 );

  vtkNew<vtkMRMLMarkupsRulerNode> markupsNode;
  vtkNew<vtkMRMLMarkupsRulerDisplayNode> displayNode;

  // get the file name
  std::string fileName = std::string("testMarkupsRulerStorageNode.fcsv");
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

  // add a ruler with two points with non default values
  int index =  markupsNode->AddMarkupWithNPoints(2);
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
  double inputPoint1[3] = {-9.9, 1.1, 0.87};
  double inputPoint2[3] = {19.9, -10.1, -10.87};
  markupsNode->SetNthRulerPositionFromArrays(index, inputPoint1, inputPoint2);

  // and add a ruler with 2 point, default values
  markupsNode->AddMarkupWithNPoints(2);
  // and another one unsetting the label
  index = markupsNode->AddMarkupWithNPoints(2);
  markupsNode->SetNthMarkupLabel(index,"");

  // add another one with a label and description that have commas in them
  int commaIndex = markupsNode->AddMarkupWithNPoints(2);
  markupsNode->SetNthMarkupLabel(commaIndex, "Label, commas, two");
  markupsNode->SetNthMarkupDescription(commaIndex, "Description one comma \"and two quotes\", for more testing");
  // add another one with a label and description with complex combos of commas and quotes
  int quotesIndex = markupsNode->AddMarkupWithNPoints(2);
  markupsNode->SetNthMarkupLabel(quotesIndex, "Label with end quotes \"around the last phrase\"");
  markupsNode->SetNthMarkupDescription(quotesIndex, "\"Description fully quoted\"");

  //
  // test write
  //

  std::cout << "\n\nWriting this ruler to file:" << std::endl;
  vtkIndent indent;
  markupsNode->PrintSelf(std::cout, indent);
  std::cout << std::endl;

  node1->SetFileName(fileName.c_str());
  std::cout << "Writing " << node1->GetFileName() << std::endl;
  int retval = node1->WriteData(markupsNode.GetPointer());
  if (!retval)
    {
    std::cerr << "Failed with return value " << retval
              << "\n\twhen writing to file:\n\t"
              << node1->GetFileName() << std::endl;
    return EXIT_FAILURE;
    }

  //
  // test read
  //
  vtkNew<vtkMRMLMarkupsRulerStorageNode> snode2;
  vtkNew<vtkMRMLMarkupsRulerNode> markupsNode2;
  vtkNew<vtkMRMLScene> scene2;
  scene2->AddNode(snode2.GetPointer());
  scene2->AddNode(markupsNode2.GetPointer());
  markupsNode2->SetAndObserveStorageNodeID(snode2->GetID());
  snode2->SetFileName(node1->GetFileName());

  std::cout << "Reading from " << snode2->GetFileName() << std::endl;

  retval = snode2->ReadData(markupsNode2.GetPointer());
  if (!retval)
    {
    std::cerr << "Failed to read from file " << snode2->GetFileName()
              << std::endl;
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
    std::cerr << "After reading in, expected markup " << index
              << " associatedNodeID of " << associatedNodeID.c_str()
              << " got "
              << markupsNode2->GetNthMarkupAssociatedNodeID(index).c_str()
              << std::endl;
    return EXIT_FAILURE;
    }
  if (markupsNode2->GetNthMarkupSelected(index) != 0)
    {
    std::cerr << "After reading in, expected markup " << index
              << " sel of 0, got "
              << markupsNode2->GetNthMarkupSelected(index) << std::endl;
    return EXIT_FAILURE;
    }
  if (markupsNode2->GetNthMarkupVisibility(index) != 0)
    {
    std::cerr << "After reading in, expected markup " << index
              << " vis of 0, got "
              << markupsNode2->GetNthMarkupVisibility(index) << std::endl;
    return EXIT_FAILURE;
    }
  if (markupsNode2->GetNthMarkupLocked(index) != 1)
    {
    std::cerr << "After reading in, expected markup " << index
              << " locked of 0, got "
              << markupsNode2->GetNthMarkupLocked(index) << std::endl;
    return EXIT_FAILURE;
    }
  if (markupsNode2->GetNthMarkupLabel(index).compare(label) != 0)
    {
    std::cerr << "After reading in, expected markup " << index
              << " label of " << label.c_str() << " got '"
              << markupsNode2->GetNthMarkupLabel(index).c_str()
              << "'" << std::endl;
    return EXIT_FAILURE;
    }
  if (markupsNode2->GetNthMarkupDescription(index).compare(desc) != 0)
    {
    std::cerr << "After reading in, expected markup " << index
              << " description of " << desc.c_str() << " got '"
              << markupsNode2->GetNthMarkupDescription(index).c_str()
              << "'" << std::endl;
    return EXIT_FAILURE;
    }

  double outputPoint1[3], outputPoint2[3];
  markupsNode2->GetNthRulerPositionInArrays(index, outputPoint1, outputPoint2);
  double diff1 = fabs(outputPoint1[0] - inputPoint1[0]) + fabs(outputPoint1[1] - inputPoint1[1]) + fabs(outputPoint1[2] - inputPoint1[2]);
  if (diff1 > 0.1)
    {
    std::cerr << "After reading in, expected ruler " << index << " point 1 "
              << inputPoint1[0] << "," << inputPoint1[1] << "," << inputPoint1[2]
              << " but got a diff of " << diff1 << " from read in point "
              << outputPoint1[0] << "," << outputPoint1[1] << "," << outputPoint1[2]
              << std::endl;
    return EXIT_FAILURE;
    }
  double diff2 = fabs(outputPoint2[0] - inputPoint2[0]) + fabs(outputPoint2[1] - inputPoint2[1]) + fabs(outputPoint2[2] - inputPoint2[2]);
  if (diff1 > 0.1)
    {
    std::cerr << "After reading in, expected ruler " << index << " point 2 "
              << inputPoint2[0] << "," << inputPoint2[1] << "," << inputPoint2[2]
              << " but got a diff of " << diff1 << " from read in point "
              << outputPoint2[0] << "," << outputPoint2[1] << "," << outputPoint2[2]
              << std::endl;
    return EXIT_FAILURE;
    }

  // test the unset label on the third markup
  index = 2;
  if (markupsNode2->GetNthMarkupLabel(index).size() != 0)
    {
    std::cerr << "After reading in, expected markup " << index
              << " empty label, but got "
              << markupsNode2->GetNthMarkupLabel(index).c_str() << std::endl;
    return EXIT_FAILURE;
    }

  // now read it again with a display node defined
  vtkNew<vtkMRMLMarkupsRulerDisplayNode> displayNode2;
  scene2->AddNode(displayNode2.GetPointer());
  markupsNode2->SetAndObserveDisplayNodeID(displayNode2->GetID());
  std::cout << "Added display node, re-reading from "
            << snode2->GetFileName() << std::endl;
  retval = snode2->ReadData(markupsNode2.GetPointer());
  if (!retval)
    {
    std::cerr << "Failed second read from file "
              << snode2->GetFileName() << std::endl;
    return EXIT_FAILURE;
    }

  //
  // test with use LPS flag on
  node1->UseLPSOn();
  std::cout << "Writing file with use LPS flag set:\n"
            << node1->GetFileName() << std::endl;
  retval = node1->WriteData(markupsNode.GetPointer());
  if (!retval)
    {
    std::cerr << "Failed with return value " << retval
              << "\n\twhen writing with LPS to file:\n\t"
              << node1->GetFileName() << std::endl;
    return EXIT_FAILURE;
    }
  // read it in after clearing out the test data
  snode2->UseLPSOn();
  markupsNode2->RemoveAllMarkups();
  std::cout << "Reading with LPS from " << snode2->GetFileName() << std::endl;

  retval = snode2->ReadData(markupsNode2.GetPointer());
  if (!retval)
    {
    std::cerr << "Failed to read from file with LPS " << snode2->GetFileName()
              << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "\nMarkups with LPS read from file = " << std::endl;
  markupsNode2->PrintSelf(std::cout, indent);

  // check the point so that it's been converted back to be the same
  double outputPoint1AfterLPS[3], outputPoint2AfterLPS[3];
  index = 0;
  markupsNode2->GetNthRulerPositionInArrays(index,
                                            outputPoint1AfterLPS, outputPoint2AfterLPS);
  diff1 =  fabs(outputPoint1AfterLPS[0] - inputPoint1[0]) + fabs(outputPoint1AfterLPS[1] - inputPoint1[1]) + fabs(outputPoint1AfterLPS[2] - inputPoint1[2]);
  if (diff1 > 0.1)
    {
    std::cerr << "After reading in after using LPS, expected markup " << index
              << " point 1 "
              << inputPoint1[0] << "," << inputPoint1[1] << "," << inputPoint1[2]
              << " but got a diff of " << diff1 << " from read in point "
              << outputPoint1AfterLPS[0] << "," << outputPoint1AfterLPS[1] << ","
              << outputPoint1AfterLPS[2] << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    double lpsPoint1[3];
    markupsNode2->GetMarkupPointLPS(index, 0, lpsPoint1);
    std::cout << "Successfully wrote out point as lps "
              << lpsPoint1[0] << "," << lpsPoint1[1] << "," << lpsPoint1[2]
              << " and read it back in as RAS "
              << outputPoint1AfterLPS[0] << "," << outputPoint1AfterLPS[1] << ","
              << outputPoint1AfterLPS[2] << std::endl;
    }
  diff2 =  fabs(outputPoint2AfterLPS[0] - inputPoint2[0]) + fabs(outputPoint2AfterLPS[1] - inputPoint2[1]) + fabs(outputPoint2AfterLPS[2] - inputPoint2[2]);
  if (diff2 > 0.1)
    {
    std::cerr << "After reading in after using LPS, expected markup " << index
              << " point 2 "
              << inputPoint2[0] << "," << inputPoint2[1] << "," << inputPoint2[2]
              << " but got a diff of " << diff2 << " from read in point "
              << outputPoint2AfterLPS[0] << "," << outputPoint2AfterLPS[1] << ","
              << outputPoint2AfterLPS[2] << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    double lpsPoint2[3];
    markupsNode2->GetMarkupPointLPS(index, 1, lpsPoint2);
    std::cout << "Successfully wrote out point as lps "
              << lpsPoint2[0] << "," << lpsPoint2[1] << "," << lpsPoint2[2]
              << " and read it back in as RAS "
              << outputPoint2AfterLPS[0] << "," << outputPoint2AfterLPS[1] << ","
              << outputPoint2AfterLPS[2] << std::endl;
    }

  // check for commas in the markup label and description
  std::string labelWithCommas = markupsNode2->GetNthMarkupLabel(commaIndex);
  int numCommas = std::count(labelWithCommas.begin(), labelWithCommas.end(), ',');
  if (numCommas != 2)
    {
    std::cerr << "Expected a label with two commas, got " << numCommas
              << " in label " << labelWithCommas.c_str() << std::endl;
    return EXIT_FAILURE;
    }
  std::string descriptionWithCommasAndQuotes = markupsNode2->GetNthMarkupDescription(commaIndex);
  numCommas = std::count(descriptionWithCommasAndQuotes.begin(),
                         descriptionWithCommasAndQuotes.end(),
                         ',');
  if (numCommas != 1)
    {
    std::cerr << "Expected a description with one comma, got " << numCommas
              << " in description '" << descriptionWithCommasAndQuotes.c_str()
              << "'" << std::endl;
    return EXIT_FAILURE;
    }
  int numQuotes = std::count(descriptionWithCommasAndQuotes.begin(),
                            descriptionWithCommasAndQuotes.end(),
                            '"');
  if (numQuotes != 2)
    {
    std::cerr << "Expected a description with two quotes, got " << numQuotes
              << " in description '" << descriptionWithCommasAndQuotes
              << "'" << std::endl;
    return EXIT_FAILURE;
    }

  // check ending quoted label
  std::string labelWithQuotes = markupsNode2->GetNthMarkupLabel(quotesIndex);
  numQuotes = std::count(labelWithQuotes.begin(), labelWithQuotes.end(), '"');
  if (numQuotes != 2)
    {
    std::cerr << "Expected 2 quotes in '" << labelWithQuotes << "' but got " << numQuotes << std::endl;
    return EXIT_FAILURE;
    }
  // check fully quoted description
  std::string descWithQuotes = markupsNode2->GetNthMarkupDescription(quotesIndex);
  numQuotes = std::count(descWithQuotes.begin(), descWithQuotes.end(), '"');
  if (numQuotes != 2)
    {
    std::cerr << "Expected 2 quotes in '" << descWithQuotes << "' but got " << numQuotes << std::endl;
    }
  return EXIT_SUCCESS;
}
