
# Test adding many fiducials
slicer_add_python_unittest(SCRIPT AddManyMarkupsFiducialTest.py
                           SLICER_ARGS --disable-cli-modules)

# Test scene views
slicer_add_python_test(SCRIPT MarkupsSceneViewRestoreTestSimple.py
                       SLICER_ARGS --disable-cli-modules)
slicer_add_python_test(SCRIPT MarkupsSceneViewRestoreTestManyLists.py
                       SLICER_ARGS --disable-cli-modules)
