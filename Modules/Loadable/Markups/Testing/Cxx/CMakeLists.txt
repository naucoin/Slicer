set(KIT qSlicer${MODULE_NAME}Module)

set(KIT_TEST_SRCS)
set(KIT_TEST_NAMES)
set(KIT_TEST_NAMES_CXX)
SlicerMacroConfigureGenericCxxModuleTests(${MODULE_NAME} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
  vtkMRMLMarkupsDisplayNodeTest1.cxx
  vtkMRMLMarkupsFiducialNodeTest1.cxx
  vtkMRMLMarkupsNodeTest1.cxx
  vtkMRMLMarkupsNodeTest2.cxx
  vtkMRMLMarkupsFiducialStorageNodeTest1.cxx
  vtkMRMLMarkupsFiducialStorageNodeTest2.cxx
  vtkMRMLMarkupsFiducialStorageNodeTest3.cxx
  vtkMRMLMarkupsStorageNodeTest1.cxx
  vtkSlicerMarkupsLogicTest1.cxx
  vtkSlicerMarkupsLogicTest2.cxx
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

add_executable(${KIT}CxxTests ${Tests})

target_link_libraries(${KIT}CxxTests ${KIT})

foreach(testname ${KIT_TEST_NAMES})
  SIMPLE_TEST( ${testname} )
endforeach()

SET (TEST_DIR "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/Markups/Testing")
# make sure the directory exists
MESSAGE(STATUS "Creating directory ${TEST_DIR}")
FILE(MAKE_DIRECTORY ${TEST_DIR} )

SIMPLE_TEST( vtkMRMLMarkupsDisplayNodeTest1 )
SIMPLE_TEST( vtkSlicerMarkupsLogicTest1 )
SIMPLE_TEST( vtkSlicerMarkupsLogicTest2 )
SIMPLE_TEST( vtkMRMLMarkupsNodeTest1 )
SIMPLE_TEST( vtkMRMLMarkupsNodeTest2 )

SET (TEST_FCSV ${TEST_DIR}/markupsFiducialStorageNode.fcsv)
FILE(TO_NATIVE_PATH ${TEST_FCSV} TEST_FCSV_NATIVE)
SIMPLE_TEST( vtkMRMLMarkupsFiducialStorageNodeTest1 ${TEST_FCSV_NATIVE} )

SIMPLE_TEST( vtkMRMLMarkupsStorageNodeTest1 )

# test legacy Slicer3 fcsv file
SET (TEST_S3_FCSV ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input/slicer3.fcsv)
FILE(TO_NATIVE_PATH ${TEST_S3_FCSV} TEST_S3_FCSV_NATIVE)
MESSAGE(STATUS "Slicer 3 fiducials test: \nCMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}\n file for markups = ${TEST_S3_FCSV_NATIVE}")
SIMPLE_TEST( vtkMRMLMarkupsFiducialStorageNodeTest2 ${TEST_S3_FCSV_NATIVE} )

# test Slicer4 annotation acsv file
SET (TEST_S4_ACSV ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input/slicer4.acsv)
FILE(TO_NATIVE_PATH ${TEST_S4_ACSV} TEST_S4_ACSV_NATIVE)
SIMPLE_TEST( vtkMRMLMarkupsFiducialStorageNodeTest3 ${TEST_S4_ACSV_NATIVE} )
