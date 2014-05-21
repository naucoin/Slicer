import os
import time
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# AddManyMarkupsRulerTest
#

class AddManyMarkupsRulerTest:
  def __init__(self, parent):
    parent.title = "AddManyMarkupsRulerTest"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Nicole Aucoin (BWH)"]
    parent.helpText = """
    This is a test case that adds many Markup Rulers to the scene and times the operation.
    """
    parent.acknowledgementText = """
    This file was originally developed by Nicole Aucoin, BWH and was partially funded by NIH grant 3P41RR013218-12S1.
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['AddManyMarkupsRulerTest'] = self.runTest

  def runTest(self):
    tester = AddManyMarkupsRulerTestTest()
    tester.runTest()

#
# qAddManyMarkupsRulerTestWidget
#

class AddManyMarkupsRulerTestWidget:
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
    self.reloadButton.name = "AddManyMarkupsRulerTest Reload"
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
    # number of rulers to add
    #
    self.numToAddSliderWidget = ctk.ctkSliderWidget()
    self.numToAddSliderWidget.singleStep = 1.0
    self.numToAddSliderWidget.minimum = 0.0
    self.numToAddSliderWidget.maximum = 1000.0
    self.numToAddSliderWidget.value = 100.0
    self.numToAddSliderWidget.toolTip = "Set the number of rulers to add."
    parametersFormLayout.addRow("Number of Rulers to Add", self.numToAddSliderWidget)

    #
    # check box to trigger fewer modify events, adding all the new points
    # is wrapped inside of a StartModify/EndModify block
    #
    self.fewerModifyFlagCheckBox = qt.QCheckBox()
    self.fewerModifyFlagCheckBox.checked = 0
    self.fewerModifyFlagCheckBox.toolTip = 'If checked, wrap adding points inside of a StartModify - EndModify block'
    parametersFormLayout.addRow("Fewer Modify Events", self.fewerModifyFlagCheckBox)

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
    logic = AddManyMarkupsRulerTestLogic()
    sliderValue = int(self.numToAddSliderWidget.value)
    fewerModifyFlag = self.fewerModifyFlagCheckBox.checked
    print("Run the logic method to add %s rulers" % sliderValue)
    logic.run(sliderValue,0,fewerModifyFlag)

  def onReload(self,moduleName="AddManyMarkupsRulerTest"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="AddManyMarkupsRulerTest"):
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
# AddManyMarkupsRulerTestLogic
#

class AddManyMarkupsRulerTestLogic:

  def __init__(self):
    pass

  def run(self,numToAdd=100,rOffset=0,usefewerModifyCalls=0):
    """
    Run the actual algorithm
    """
    print('Running test to add %s rulers' % (numToAdd,))
    print('Index\tTime to add ruler\tDelta between adds')
    print "%(index)04s\t" % {'index': "i"}, "t\tdt'"
    r = rOffset
    a = 0
    s = 0
    r2 = 10 + rOffset
    a2 = 10
    s2 = 10
    t1 = 0
    t2 = 0
    t3 = 0
    t4 = 0
    timeToAddThisRuler = 0
    timeToAddLastRuler = 0

    testStartTime = time.clock()
    displayNode = slicer.vtkMRMLMarkupsRulerDisplayNode()
    slicer.mrmlScene.AddNode(displayNode)
    rulerNode = slicer.vtkMRMLMarkupsRulerNode()
    slicer.mrmlScene.AddNode(rulerNode)
    rulerNode.SetAndObserveDisplayNodeID(displayNode.GetID())

    if usefewerModifyCalls == 1:
      print "Start modify"
      mod = rulerNode.StartModify()

    # iterate over the number of rulers to add
    for i in range(numToAdd):
      #    print "i = ", i, "/", numToAdd, ", r = ", r, ", a = ", a, ", s = ", s, ", r2 = ", r2, ", a2 = ", a2, ", s2 = ", s2
      t1 = time.clock()
      rulerNode.AddRuler(r,a,s,r2,a2,s2)
      t2 = time.clock()
      timeToAddThisRuler = t2 - t1
      dt = timeToAddThisRuler - timeToAddLastRuler
      print '%(index)04d\t' % {'index': i}, timeToAddThisRuler, "\t", dt
      r = r + 1.0
      a = a + 1.0
      s = s + 1.0
      r2 = r2 + 2.0
      a2 = r2 + 2.0
      s2 = s2 + 2.0
      timeToAddLastRuler = timeToAddThisRuler

    if usefewerModifyCalls == 1:
      rulerNode.EndModify(mod)

    testEndTime = time.clock()
    testTime = testEndTime - testStartTime
    print "Total time to add ",numToAdd," = ", testTime

    return True


class AddManyMarkupsRulerTestTest(unittest.TestCase):
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
    self.test_AddManyMarkupsRulerTest1()

  def test_AddManyMarkupsRulerTest1(self):

    self.delayDisplay("Starting the add many Markups ruler test")

    # start in the welcome module
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('Welcome')

    logic = AddManyMarkupsRulerTestLogic()
    logic.run(100)

    self.delayDisplay("Now running it while the Markups Module is open")
    m.moduleSelector().selectModule('Markups')
    logic.run(100,100)

    self.delayDisplay('Test passed!')
