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
  vtkMRMLMarkupsStorageNodeTest1.cxx
  vtkSlicerMarkupsLogicTest1.cxx
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

add_executable(${KIT}CxxTests ${Tests})

target_link_libraries(${KIT}CxxTests ${KIT})

MESSAGE(STATUS "kit names =  ${KIT_TEST_NAMES}, Tests= ${Tests}, kit cxx tests = ${${KIT}CxxTests} ")

foreach(testname ${KIT_TEST_NAMES})
  SIMPLE_TEST( ${testname} )
endforeach()

SET (TEST_DIR "${CMAKE_BINARY_DIR}/${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}/Markups/Testing")
# make sure the directory exists
MESSAGE(STATUS "Creating directory ${TEST_DIR}")
FILE(MAKE_DIRECTORY ${TEST_DIR} )

SIMPLE_TEST( vtkMRMLMarkupsDisplayNodeTest1 )
SIMPLE_TEST( vtkSlicerMarkupsLogicTest1 )
SIMPLE_TEST( vtkMRMLMarkupsNodeTest1 )

SET (TEST_MCSV ${TEST_DIR}/markupsStorageNode.mcsv)
FILE(TO_NATIVE_PATH ${TEST_MCSV} TEST_MCSV_NATIVE)
MESSAGE(STATUS "cmake bin dir = ${CMAKE_BINARY_DIR}, share dir = ${Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR}, passing in path ${TEST_MCSV}, native path = ${TEST_MCSV_NATIVE}")


SIMPLE_TEST( vtkMRMLMarkupsStorageNodeTest1 ${TEST_MCSV_NATIVE} )
