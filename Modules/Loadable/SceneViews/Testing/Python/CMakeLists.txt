include(SlicerMacroBuildScriptedModule)

# Test adding new storable data after creating a scene view
slicerMacroBuildScriptedModule(
  NAME AddStorableDataAfterSceneViewTest
  SCRIPTS AddStorableDataAfterSceneViewTest.py
)
slicer_add_python_unittest(SCRIPT AddStorableDataAfterSceneViewTest.py
                           SLICER_ARGS --disable-cli-modules)
