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

// SuperLoadableModuleTemplate Logic includes
#include "vtkSlicerSuperLoadableModuleTemplateLogic.h"

// MRML includes

// VTK includes
#include <vtkNew.h>

// STD includes

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerSuperLoadableModuleTemplateLogic);

//----------------------------------------------------------------------------
vtkSlicerSuperLoadableModuleTemplateLogic::vtkSlicerSuperLoadableModuleTemplateLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerSuperLoadableModuleTemplateLogic::~vtkSlicerSuperLoadableModuleTemplateLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerSuperLoadableModuleTemplateLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerSuperLoadableModuleTemplateLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerSuperLoadableModuleTemplateLogic::RegisterNodes()
{
  if (this->GetMRMLScene() == 0)
    {
    vtkErrorMacro("RegisterNodes: mrml scene is null!");
    return;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSuperLoadableModuleTemplateLogic::UpdateFromMRMLScene()
{
  if (this->GetMRMLScene() == 0)
    {
    vtkErrorMacro("UpdateFromMRMLScene: mrml scene is null!");
    return;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSuperLoadableModuleTemplateLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerSuperLoadableModuleTemplateLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

