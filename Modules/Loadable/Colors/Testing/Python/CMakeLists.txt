include(SlicerMacroBuildScriptedModule)
set(COLORS_PYTHON_RESOURCES
  )

# Test vtkSlicerScalarBarActor class
slicerMacroBuildScriptedModule(
  NAME ColorsScalarBarSelfTest
  SCRIPTS ColorsScalarBarSelfTest.py
  RESOURCES ${MARKUPS_PYTHON_RESOURCES}
  )