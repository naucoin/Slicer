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
#include "vtkMRMLMarkupsRulerNode.h"
#include "vtkMRMLMarkupsRulerStorageNode.h"
#include "vtkMRMLMarkupsRulerNode.h"
#include "vtkURIHandler.h"
#include "vtkMRMLScene.h"
#include "vtkPolyData.h"

// VTK includes
#include <vtkNew.h>


// Test reading in a Slicer4 Annotation ruler .acsv file
int vtkMRMLMarkupsRulerStorageNodeTest2(int argc, char * argv[] )
{
  vtkNew<vtkMRMLMarkupsRulerStorageNode> node1;
  vtkNew<vtkMRMLMarkupsRulerNode> markupsRulerNode;
  vtkNew<vtkMRMLMarkupsRulerNode> displayNode;

  // get the file name
  std::string fileName;
  if (argc > 1)
    {
    fileName = std::string(argv[1]);
    }
  else
    {
    std::cerr << "No file name given!" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Using file name " << fileName.c_str() << std::endl;
  node1->SetFileName(fileName.c_str());

  // set up a scene
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  scene->AddNode(markupsRulerNode.GetPointer());
  scene->AddNode(displayNode.GetPointer());
  markupsRulerNode->SetAndObserveStorageNodeID(node1->GetID());
  markupsRulerNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  //
  // test read
  //
  std::cout << "Reading from " << node1->GetFileName() << std::endl;

  int retval = node1->ReadData(markupsRulerNode.GetPointer());
  if (!retval)
    {
    std::cerr << "Failed to read into Markups ruler node from Slicer4 Annotations Rulers file " << node1->GetFileName() << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "\nMarkup read from file = " << std::endl;
  vtkIndent indent;
  markupsRulerNode->PrintSelf(std::cout, indent);
  std::cout << std::endl;

  // test values on the first ruler
  double inputPoint1[3] = {7.05129, 0.8, 93.4296};
  double inputPoint2[3] = {-101.362, 0.7, 22.0353};
  double outputPoint1[3], outputPoint2[3];
  markupsRulerNode->GetNthRulerPosition1(0, outputPoint1);
  markupsRulerNode->GetNthRulerPosition2(0, outputPoint2);
  double diff1 = fabs(outputPoint1[0] - inputPoint1[0]) + fabs(outputPoint1[1] - inputPoint1[1]) + fabs(outputPoint1[2] - inputPoint1[2]);
  if (diff1 > 0.1)
    {
    std::cerr << "After reading in, expected markup point 1"
              << inputPoint1[0] << "," << inputPoint1[1] << "," << inputPoint1[2]
              << " but got a diff of " << diff1 << " from read in point "
              << outputPoint1[0] << "," << outputPoint1[1] << "," << outputPoint1[2]
              << std::endl;
    return EXIT_FAILURE;
    }
  double diff2 = fabs(outputPoint2[0] - inputPoint2[0]) + fabs(outputPoint2[1] - inputPoint2[1]) + fabs(outputPoint2[2] - inputPoint2[2]);
  if (diff2 > 0.1)
    {
    std::cerr << "After reading in, expected markup point 2 "
              << inputPoint2[0] << "," << inputPoint2[1] << "," << inputPoint2[2]
              << " but got a diff of " << diff2 << " from read in point "
              << outputPoint2[0] << "," << outputPoint2[1] << "," << outputPoint2[2]
              << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
