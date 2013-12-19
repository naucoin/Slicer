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

// LoadableModuleTemplate Logic includes
#include "vtkSlicerLoadableModuleTemplateLogic.h"

// MRML includes

// VTK includes
#include <vtkNew.h>

// STD includes

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerLoadableModuleTemplateLogic);

//----------------------------------------------------------------------------
vtkSlicerLoadableModuleTemplateLogic::vtkSlicerLoadableModuleTemplateLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerLoadableModuleTemplateLogic::~vtkSlicerLoadableModuleTemplateLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerLoadableModuleTemplateLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerLoadableModuleTemplateLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerLoadableModuleTemplateLogic::RegisterNodes()
{
  if (this->GetMRMLScene() == 0)
    {
    vtkErrorMacro("ReigsterNodes: mrml scene is NULL!");
    return;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerLoadableModuleTemplateLogic::UpdateFromMRMLScene()
{
  if (this->GetMRMLScene() == 0)
    {
    vtkErrorMacro("UpdateFromMRMLScene: mrml scene is NULL!");
    return;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerLoadableModuleTemplateLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerLoadableModuleTemplateLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

