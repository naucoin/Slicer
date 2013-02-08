
// VTK includes
#include <vtkActor.h>
#include <vtkCommand.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkLookupTable.h>

// STD includes
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "vtkSlicerScalarBarActor.h"

#include "vtkMRMLCoreTestingMacros.h"

// The actual test function
int vtkSlicerScalarBarActorTest1( int argc, char *argv[] )
{
  // Create the RenderWindow, Renderer
  //
  vtkRenderer *renderer = vtkRenderer::New();
  vtkRenderWindow *renderWindow = vtkRenderWindow::New();
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(600,600);

  vtkRenderWindowInteractor *renderInteractor = vtkRenderWindowInteractor::New();
  renderInteractor->SetRenderWindow(renderWindow);
  vtkInteractorStyleSwitch* iStyle = vtkInteractorStyleSwitch::New();
  iStyle->SetCurrentStyleToTrackballCamera();

  renderInteractor->SetInteractorStyle(iStyle);

  renderInteractor->Initialize();
  
  vtkLookupTable *lookupTable = vtkLookupTable::New();
  lookupTable->SetRange(1,10);
  lookupTable->Allocate(10);
  lookupTable->SetRampToLinear();
  lookupTable->Build();
  
  vtkSlicerScalarBarActor *scalarBarActor = vtkSlicerScalarBarActor::New();
  scalarBarActor->SetLookupTable(lookupTable);
  scalarBarActor->SetMaximumNumberOfColors(10);
  scalarBarActor->SetNumberOfLabels(10);
  
  for (int i=0; i<10; i++)
  {
    std::stringstream ss;
    ss << i;
    scalarBarActor->SetColorName(i, ss.str().c_str());
  }
  scalarBarActor->UseColorNameAsLabelOn();

  renderer->AddActor(scalarBarActor);
  renderer->SetBackground(0.1, 0.2, 0.4);

  //renderInteractor->Start();

  lookupTable->Delete();
  scalarBarActor->Delete();
  iStyle->Delete();
  renderInteractor->Delete();
  renderWindow->Delete();
  renderer->Delete();
  
  return EXIT_SUCCESS;
}

