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

int vtkMRMLMarkupsRulerDisplayNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLMarkupsRulerDisplayNode> node1;

  EXERCISE_BASIC_DISPLAY_MRML_METHODS( vtkMRMLMarkupsRulerDisplayNode, node1 );

  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node1, PointColor1, 0.0, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node1, SelectedPointColor1, 0.0, 1.0);

  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node1, PointColor2, 0.0, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node1, SelectedPointColor2, 0.0, 1.0);

  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node1, LineColor, 0.0, 1.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node1, SelectedLineColor, 0.0, 1.0);

  TEST_SET_GET_DOUBLE_RANGE(node1, LineThickness, -1.0, 10.0);
  TEST_SET_GET_DOUBLE_RANGE(node1, UnderLineThickness, -1.0, 5.0);
  TEST_SET_GET_DOUBLE_RANGE(node1, OverLineThickness, -1.0, 8.0);

  return EXIT_SUCCESS;
}
