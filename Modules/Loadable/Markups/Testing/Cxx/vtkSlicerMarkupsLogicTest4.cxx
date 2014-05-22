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

// Annotations includes
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkSlicerAnnotationModuleLogic.h"

// Markups includes
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsRulerNode.h"
#include "vtkSlicerMarkupsLogic.h"

// MRML includes
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"


// VTK includes
#include <vtkNew.h>

// test converting Annotation rulers to Markups rulers
int vtkSlicerMarkupsLogicTest4(int , char * [] )
{
  vtkNew<vtkSlicerMarkupsLogic> logic1;
  vtkNew<vtkMRMLScene> scene;
  // add a selection node
  vtkMRMLApplicationLogic* applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene.GetPointer());

  // Test converting annotations in the scene to markups

  // no scene
  logic1->ConvertAnnotationRulersToMarkups();

  // empty scene
  logic1->SetMRMLScene(scene.GetPointer());
  logic1->ConvertAnnotationRulersToMarkups();

  // set up the annotation module logic
  vtkNew<vtkSlicerAnnotationModuleLogic> annotLogic;
  annotLogic->SetMRMLScene(scene.GetPointer());

  // add some annotations
  for (int n = 0; n < 10; n++)
    {
    vtkNew<vtkMRMLAnnotationRulerNode> annotRuler;
    double p1[3] = {1.1, -2.2, 3.3};
    p1[0] = (double)n;
    annotRuler->SetPosition1(p1);
    double p2[3] = {-4.4, 5.5, -6.6};
    p2[1] = (double)n;
    annotRuler->SetPosition2(p2);
    annotRuler->Initialize(scene.GetPointer());
    }
  std::cout << "After one list, active hierarchy = " << annotLogic->GetActiveHierarchyNode()->GetID() << std::endl;
  // and another hierarchy and make it active
  annotLogic->AddHierarchy();
  std::cout << "After adding a new hierarchy, active hierarchy = " << annotLogic->GetActiveHierarchyNode()->GetID() << std::endl;
  // add some more annotations
  for (int n = 0; n < 5; n++)
    {
    vtkNew<vtkMRMLAnnotationRulerNode> annotRuler;
    double p1[3] = {7.7, -8.8, 0.0};
    p1[2] = (double)n;
    annotRuler->SetPosition1(p1);
    double p2[3] = {9.9, -10.1, 0.0};
    p2[2] = (double)n;
    annotRuler->SetPosition2(p2);
    annotRuler->Initialize(scene.GetPointer());
    if (n == 3)
      {
      annotRuler->SetDescription("testing description");
      }
    if (n == 4)
      {
      annotRuler->SetAttribute("AssociatedNodeID", "vtkMRMLScalarVolumeNode4");
      }
    }

  // convert and test
  logic1->ConvertAnnotationRulersToMarkups();

  int numAnnotationRulers = scene->GetNumberOfNodesByClass("vtkMRMLAnnotationRulerNode");
  int numMarkupsRulers = scene->GetNumberOfNodesByClass("vtkMRMLMarkupsRulerNode");
  if (numAnnotationRulers != 0 ||
      numMarkupsRulers != 2)
    {
    std::cerr << "Failed to convert 15 annotation rulers in two hierarchies "
    << " to 2 markup lists, have " << numAnnotationRulers
    << " annotation rulers and " << numMarkupsRulers
    << " markups ruler lists" << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Converted annotation rulers to " << numMarkupsRulers
              << " markups ruler lists" << std::endl;
    }
/*
    vtkIndent indent;
    for (int n = 0; n < numMarkupsRulers; ++n)
      {
      vtkMRMLNode *mrmlNode = scene->GetNthNodeByClass(n, "vtkMRMLMarkupsRulerNode");
      std::cout << "\nConverted Markups node " << n << ":" << std::endl;
      mrmlNode->PrintSelf(std::cout, indent);
      }
*/
  // clean up before testing
  applicationLogic->SetMRMLScene(NULL);
  logic1->SetMRMLScene(NULL);
  annotLogic->SetMRMLScene(NULL);
  applicationLogic->Delete();

  // check the second list
  vtkMRMLNode *mrmlNode = scene->GetNthNodeByClass(1, "vtkMRMLMarkupsRulerNode");
  if (mrmlNode)
    {
    vtkMRMLMarkupsRulerNode *markupsRuler = vtkMRMLMarkupsRulerNode::SafeDownCast(mrmlNode);
    if (markupsRuler)
      {
      std::string desc = markupsRuler->GetNthMarkupDescription(3);
      if (desc.compare("testing description") != 0)
        {
        std::cerr << "Failed to get the expected description on markup 3, got: "
                  << desc.c_str() << std::endl;
        return EXIT_FAILURE;
        }
      std::string assocNodeID = markupsRuler->GetNthMarkupAssociatedNodeID(4);
      if (assocNodeID.compare("vtkMRMLScalarVolumeNode4") != 0)
        {
        std::cerr << "Failed to get the expected associated node id on markup 4, got: "
                  << assocNodeID.c_str() << std::endl;
        return EXIT_FAILURE;
        }
      double pos1[3], pos2[3];
      markupsRuler->GetNthRulerPosition1(1, pos1);
      markupsRuler->GetNthRulerPosition2(1, pos2);
      double expectedPos1[3] = {7.7, -8.8, 1.0};
      double expectedPos2[3] = {9.9, -10.1, 1.0};
      if (vtkMath::Distance2BetweenPoints(pos1, expectedPos1) > 0.01)
        {
        std::cerr << "\n\n\nExpected second markup position 1 of "
                  << expectedPos1[0] << ", " << expectedPos1[1] << ", " << expectedPos1[2]
                  << " but got: "
                  << pos1[0] << "," << pos1[1] << "," << pos1[2] << std::endl;
        return EXIT_FAILURE;
        }
      if (vtkMath::Distance2BetweenPoints(pos2, expectedPos2) > 0.01)
        {
        std::cerr << "\n\n\nExpected second markup position 2 of "
                  << expectedPos2[0] << ", " << expectedPos2[1] << ", " << expectedPos2[2]
                  << " but got: "
                  << pos2[0] << "," << pos2[1] << "," << pos2[2] << std::endl;
        return EXIT_FAILURE;
        }
      vtkMRMLMarkupsRulerDisplayNode *dispNode = markupsRuler->GetMarkupsRulerDisplayNode();
      if (dispNode)
        {
        double col[3];
        dispNode->GetColor(col);
        double annotCol[3];
        vtkNew<vtkMRMLAnnotationPointDisplayNode> pointDispNode;
        pointDispNode->GetColor(annotCol);
        if (vtkMath::Distance2BetweenPoints(col, annotCol) > 0.01)
          {
          std::cerr << "Failed to set color, expected "
                    << annotCol[0] << ","
                    << annotCol[1] << ","
                    << annotCol[2] << ", but got: "
                    << col[0] << ","
                    << col[1] << ","
                    << col[2] << std::endl;
          return EXIT_FAILURE;
          }
        }
      else
        {
        std::cerr << "Second markups node doesn't have a display node!"
                  << std::endl;
        return EXIT_FAILURE;
        }
      }
    else
      {
      std::cerr << "Unable to get second markups fiducial node for testing!" << std::endl;
      return EXIT_FAILURE;
      }
    }
  // cleanup
  scene->Clear(1);

  return EXIT_SUCCESS;
}
