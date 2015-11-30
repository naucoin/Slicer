import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# testSaveLabelMapsAsPNG4091
#

class testSaveLabelMapsAsPNG4091(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "testSaveLabelMapsAsPNG4091" # TODO make this more human readable by adding spaces
    self.parent.categories = ["Testing.TestCases"]
    self.parent.dependencies = []
    self.parent.contributors = ["Nicole Aucoin (BWH)"]
    self.parent.helpText = """
    Test a round trip of loading a PNG file, editing it, saving, reloading.
    """
    self.parent.acknowledgementText = """
    This file was originally developed by Nicole Aucoin, BWH, and was funded by a grant with Stonybrook.
""" # replace with organization, grant and thanks.

#
# qtestSaveLabelMapsAsPNG4091Widget
#

class testSaveLabelMapsAsPNG4091Widget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    # Instantiate and connect widgets ...

    #
    # Parameters Area
    #
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the dummy collapsible button
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    #
    # input volume selector
    #
    self.inputSelector = slicer.qMRMLNodeComboBox()
    self.inputSelector.nodeTypes = ["vtkMRMLScalarVolumeNode"]
    self.inputSelector.selectNodeUponCreation = True
    self.inputSelector.addEnabled = False
    self.inputSelector.removeEnabled = False
    self.inputSelector.noneEnabled = False
    self.inputSelector.showHidden = False
    # allow children types since pngs get loaded as vector volumes
    self.inputSelector.showChildNodeTypes = True
    self.inputSelector.setMRMLScene( slicer.mrmlScene )
    self.inputSelector.setToolTip( "Pick the input volume.")
    parametersFormLayout.addRow("Input Volume: ", self.inputSelector)

    #
    # Apply Button
    #
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Run the test."
    self.applyButton.enabled = False
    parametersFormLayout.addRow(self.applyButton)

    # connections
    self.applyButton.connect('clicked(bool)', self.onApplyButton)
    self.inputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Refresh Apply button state
    self.onSelect()

  def cleanup(self):
    pass

  def onSelect(self):
    self.applyButton.enabled = self.inputSelector.currentNode() is not None

  def onApplyButton(self):
    logic = testSaveLabelMapsAsPNG4091Logic()
    logic.run(self.inputSelector.currentNode())

#
# testSaveLabelMapsAsPNG4091Logic
#

class testSaveLabelMapsAsPNG4091Logic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def hasImageData(self,volumeNode):
    if not volumeNode:
      logging.debug('hasImageData failed: no volume node')
      return False
    if volumeNode.GetImageData() == None:
      logging.debug('hasImageData failed: no image data in volume node')
      return False
    return True

  def run(self, inputVolume):
    """
    Run the actual algorithm
    """

    logging.info('Processing started')

    print 'InputVolume', inputVolume.GetID()

    # double check that have data
    if not self.hasImageData(inputVolume):
      return False

    # is it a vector volume?
    if inputVolume.GetClassName() != 'vtkMRMLVectorVolumeNode':
      logging.error('Assuming that PNG files get loaded as vector volumes due to the order of attempted node readers, but the input image was loaded as %s' % inputVolume.GetClassName())
      return False

    logging.info('Image is a vector volume, creating a grey scale')

    # get the matrix information from the input volume
    directionMatrix = vtk.vtkMatrix4x4()
    inputVolume.GetIJKToRASDirectionMatrix(directionMatrix)

    # calculate the vector magnitude
    magnitude = vtk.vtkImageMagnitude()
    magnitude.SetInputData(inputVolume.GetImageData())
    magnitude.Update()

    # create the greyscale volume
    greyVolume = slicer.vtkMRMLScalarVolumeNode()
    greyVolume.SetImageDataConnection(magnitude.GetOutputPort())
    greyVolume.SetName(inputVolume.GetName() + '_grey')
    greyVolume.SetIJKToRASDirectionMatrix(directionMatrix)

    # add to scene
    slicer.mrmlScene.AddNode(greyVolume)
    greyID = greyVolume.GetID()

    # set it up in the slice viewers
    lm = slicer.app.layoutManager()
    redWidget = lm.sliceWidget("Red")
    redSliceLogic = redWidget.sliceLogic()
    redComposite = redSliceLogic.GetSliceCompositeNode()
    redComposite.SetForegroundVolumeID(inputVolume.GetID())
    redComposite.SetBackgroundVolumeID(greyID)

    # set up a label map and set it for editing
    logging.info('Setting up label map')
    volumesLogic = slicer.modules.volumes.logic()
    greyLabel = volumesLogic.CreateAndAddLabelVolume(slicer.mrmlScene, greyVolume, greyVolume.GetName() + '-label')
    selectionNode = slicer.app.applicationLogic().GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID( greyVolume.GetID() )
    selectionNode.SetReferenceActiveLabelVolumeID( greyLabel.GetID() )
    slicer.app.applicationLogic().PropagateVolumeSelection(0)

    # convert to a scalar format that can be saved
    if greyLabel.GetImageData().GetScalarTypeAsString() != 'unsigned char':
      castScalarVolume = slicer.modules.castscalarvolume
      parameters = {}
      parameters["InputVolume"] = greyLabel.GetID()
      parameters["OutputVolume"] = greyLabel.GetID()
      parameters["Type"] = "UnsignedChar"
      cliNode = None
      cliNode = slicer.cli.run(castScalarVolume, cliNode, parameters, wait_for_completion=True)
      if greyLabel.GetImageData().GetScalarTypeAsString() != 'unsigned char':
        logging.error('Failed to convert scalar label volume to unsigned char, type is %s' % greyLabel.GetImageData().GetScalarTypeAsString())
        return False
      logging.info('Label converted to unsigned char')

    # go to the editor
    logging.info('Setting up the editor and painting')
    import EditorLib
    editUtil = EditorLib.EditUtil.EditUtil()
    parameterNode = editUtil.getParameterNode()
    paintEffectOptions = EditorLib.PaintEffectOptions()
    paintEffectOptions.setMRMLDefaults()
    paintEffectOptions.__del__()

    logging.info('Paint radius is %s' % parameterNode.GetParameter('PaintEffect,radius'))
    parameterNode.SetParameter('PaintEffect,radius', '20')
    paintTool = EditorLib.PaintEffectTool(redWidget)
    self.delayDisplay('Paint radius is %s, tool radius is %d' % (parameterNode.GetParameter('PaintEffect,radius'),paintTool.radius))

    size = min(redWidget.width, redWidget.height)
    step = size / 12
    center = size / 2
    for label in xrange(1,5):
      editUtil.setLabel(label)
      pos = center - 2*step + (step * label)
      logging.info('Painting %d at (%d, %d)' % (label, pos,pos))
      paintTool.paintAddPoint(pos, pos)
      paintTool.paintApply()
    paintTool.cleanup()
    paintTool = None

    logging.info('Painting complete')

    # save as png
    snode = greyLabel.CreateDefaultStorageNode()
    slicer.mrmlScene.AddNode(snode)
    greyLabel.SetAndObserveStorageNodeID(snode.GetID())
    snode.SetFileName(greyLabel.GetName() + '.png')
    logging.info('Created storage node with file name %s, ref count = %d' % (snode.GetFileName(), snode.GetReferenceCount()))
    snode.SetReferenceCount(snode.GetReferenceCount() - 1)
    if snode.WriteData(greyLabel) == 0:
      logging.error('Failed to write file %s' % snode.GetFileName())
      return False

    # clear the scene and reload the file
    fileName = snode.GetFullNameFromFileName()
    logging.info('Getting file name for reloading: %s' % fileName)
    slicer.mrmlScene.Clear(0)

    loadedFlag = slicer.util.loadLabelVolume(fileName)
    if loadedFlag == False:
      logging.error('Failed to reload saved label map file %s' % fileName)
      return False
    loadedVolume = slicer.util.getNode('image01_grey-label')
    if loadedVolume.GetClassName() != 'vtkMRMLLabelMapVolumeNode':
      logging.error('Failed to reload saved label map file %s as a label map volume, type = %s' % (fileName, loadedVolume.GetClassName()))
      return False

    logging.info('Processing completed')

    return True


class testSaveLabelMapsAsPNG4091Test(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_testSaveLabelMapsAsPNG40911()

  def test_testSaveLabelMapsAsPNG40911(self):
    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://pais.bmi.stonybrookmedicine.edu/download/segmentation_training/training_set/image01.png', 'image01.png', slicer.util.loadVolume),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        logging.info('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        logging.info('Loading %s...' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading')

    volumeNode = slicer.util.getNode(pattern="image01")
    logic = testSaveLabelMapsAsPNG4091Logic()
    self.assertTrue( logic.hasImageData(volumeNode) )
    returnFlag = logic.run(volumeNode)
    if returnFlag:
      self.delayDisplay('Test passed!')
    else:
      self.delayDisplay('Test failed!')
