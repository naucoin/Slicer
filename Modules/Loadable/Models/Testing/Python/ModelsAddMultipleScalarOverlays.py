import os
import unittest
import vtk, qt, ctk, slicer

#
# ModelsAddMultipleScalarOverlays
#

class ModelsAddMultipleScalarOverlays:
  def __init__(self, parent):
    parent.title = "ModelsAddMultipleScalarOverlays"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Nicole Aucoin (BWH)"]
    parent.helpText = """
    This is a scripted self test to check that adding multiple scalar
    overlays to a model works well: the right number of storage nodes
    with unique storage node ids stored on the model node.
    """
    parent.acknowledgementText = """
    This file was contributed by Nicole Aucoin, BWH, and was partially funded by NIH grant 3P41RR013218-12S1.
"""
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['ModelsAddMultipleScalarOverlays'] = self.runTest

  def runTest(self):
    tester = ModelsAddMultipleScalarOverlaysTest()
    tester.runTest()

#
# qModelsAddMultipleScalarOverlaysWidget
#

class ModelsAddMultipleScalarOverlaysWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

  def setup(self):
    # Instantiate and connect widgets ...

    #
    # Reload and Test area
    #
    reloadCollapsibleButton = ctk.ctkCollapsibleButton()
    reloadCollapsibleButton.text = "Reload && Test"
    self.layout.addWidget(reloadCollapsibleButton)
    reloadFormLayout = qt.QFormLayout(reloadCollapsibleButton)

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "ModelsAddMultipleScalarOverlays Reload"
    reloadFormLayout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    reloadFormLayout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    #
    # Parameters Area
    #
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the dummy collapsible button
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    #
    # Apply Button
    #
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Run the algorithm."
    self.applyButton.enabled = True
    parametersFormLayout.addRow(self.applyButton)

    # connections
    self.applyButton.connect('clicked(bool)', self.onApplyButton)

    # Add vertical spacer
    self.layout.addStretch(1)

  def cleanup(self):
    pass

  def onApplyButton(self):
    logic = ModelsAddMultipleScalarOverlaysLogic()
    print("Run the algorithm")
    logic.run()

  def onReload(self,moduleName="ModelsAddMultipleScalarOverlays"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="ModelsAddMultipleScalarOverlays"):
    try:
      self.onReload()
      evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
      tester = eval(evalString)
      tester.runTest()
    except Exception, e:
      import traceback
      traceback.print_exc()
      qt.QMessageBox.warning(slicer.util.mainWindow(),
          "Reload and Test", 'Exception!\n\n' + str(e) + "\n\nSee Python Console for Stack Trace")


#
# ModelsAddMultipleScalarOverlaysLogic
#

class ModelsAddMultipleScalarOverlaysLogic:
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass

  def delayDisplay(self,message,msec=1000):
    #
    # logic version of delay display
    #
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def run(self):
    """
    Run the actual algorithm
    """

    layoutManager = slicer.app.layoutManager()
    layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)

    self.delayDisplay('Running the aglorithm')

    # go to the models module
    slicer.util.mainWindow().moduleSelector().selectModule('Models')
    self.delayDisplay('Models module')

    # download some data from Midas
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=225950', 'lh.pial'),
        ('http://slicer.kitware.com/midas3/download?items=225951', 'lh.thickness'),
        ('http://slicer.kitware.com/midas3/download?items=225952', 'lh.curv'),
        )
    for url,name in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)

    # load the model and overlays on it using the models logic
    modelsLogic = slicer.modules.models.logic()

    # model
    modelNode = modelsLogic.AddModel( slicer.app.temporaryPath + '/lh.pial')
    if not modelNode:
      self.delayDisplay('Failed to load model lh.pial from %s' % slicer.app.temporaryPath)
      return False

    numStorageNodes = modelNode.GetNumberOfStorageNodes()
    if numStorageNodes != 1:
      self.delayDisplay('After loading the model, expected 1 storage node but have %d' % numStorageNodes)
      return False

    # scalar overlay 1
    scalarStorageNode1 = modelsLogic.AddScalar( slicer.app.temporaryPath + '/lh.thickness', modelNode)
    if not scalarStorageNode1:
      self.delayDisplay('Failed to load overlay lh.thickness from %s' % slicer.app.temporaryPath)
      return False

    numStorageNodes = modelNode.GetNumberOfStorageNodes()
    if numStorageNodes != 2:
      self.delayDisplay('After loading the first scalar overlay, expected 2 storage nodes but have %d' % numStorageNodes)
      return False

    # scalar overlay 2
    scalarStorageNode2 = modelsLogic.AddScalar( slicer.app.temporaryPath + '/lh.curv', modelNode)
    if not scalarStorageNode2:
      self.delayDisplay('Failed to load overlay lh.curv from %s' % slicer.app.temporaryPath)
      return False

    numStorageNodes = modelNode.GetNumberOfStorageNodes()
    if numStorageNodes != 3:
      self.delayDisplay('After loading the second scalar overlay, expected 3 storage nodes but have %d' % numStorageNodes)
      return False

    # make sure that the ids of the storage nodes are unique
    storageNodeIDs = []
    for n in range(numStorageNodes):
      print 'Storage node %d: id = %s' % (n, modelNode.GetNthStorageNodeID(n))
      storageNodeIDs.append(modelNode.GetNthStorageNodeID(n))

    uniqueIDCount = len(set(storageNodeIDs))
    if uniqueIDCount != numStorageNodes:
      self.delayDisplay('Have repeated storage node ids! Expected %d unique storage node ids, got %d' % (numStorageNodes, uniqueIDCount))
      return False

    # make sure that the model node has been marked as modified since
    # read, since it will need to be written as a new .vtk file with
    # the scalar overlays included
    if not modelNode.GetModifiedSinceRead():
      self.delayDisplay('Model node is not marked as modified since read!')
      return False

    return True


class ModelsAddMultipleScalarOverlaysTest(unittest.TestCase):
  """
  This is the test case for your scripted module.
  """

  def delayDisplay(self,message,msec=1000):
    """This utility method displays a small dialog and waits.
    This does two things: 1) it lets the event loop catch up
    to the state of the test so that rendering and widget updates
    have all taken place before the test continues and 2) it
    shows the user/developer/tester the state of the test
    so that we'll know when it breaks.
    """
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    return self.test_ModelsAddMultipleScalarOverlays1()

  def test_ModelsAddMultipleScalarOverlays1(self):

    self.delayDisplay("Starting the test")

    logic = ModelsAddMultipleScalarOverlaysLogic()
    result = logic.run()

    if result:
      self.delayDisplay('Test passed!')
      return True
    else:
      self.delayDisplay('Test failed!')
      slicer.testing.exitFailure('Models add multiple scalar overlays test failed.')
      return False
