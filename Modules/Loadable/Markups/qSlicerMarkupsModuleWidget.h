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

#ifndef __qSlicerMarkupsModuleWidget_h
#define __qSlicerMarkupsModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerMarkupsModuleExport.h"

class qSlicerMarkupsModuleWidgetPrivate;
class vtkMRMLNode;
class QTableWidgetItem;
class vtkSlicerMarkupsLogic;
class QModelIndex;

/// \ingroup Slicer_QtModules_Markups
class Q_SLICER_QTMODULES_MARKUPS_EXPORT qSlicerMarkupsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerMarkupsModuleWidget(QWidget *parent=0);
  virtual ~qSlicerMarkupsModuleWidget();

  /// Set up the GUI from mrml when entering
  virtual void enter();
  /// disconnect from scene when exiting
  virtual void exit();

  /// get the logic in the proper class
  vtkSlicerMarkupsLogic *markupsLogic();

  /// refresh the gui from the currently active markup node as determined by
  /// the selection node
  void UpdateWidgetFromMRML();
  /// refresh a row of the gui from the mth markup in the currently active
  /// markup node as defined by the selection node combo box
  void UpdateRow(int m);

  /// add observations to the markups node, and remove them from other markups
  /// nodes (from all nodes if markupsNode is null)
  void observeMarkupsNode(vtkMRMLNode *markupsNode);

  /// reset the GUI elements: clear out the table
  void clearGUI();

  /// utility methods to convert between Qt colors and c++ colors
  static void toQColor(const double* color, QColor &qcolor);
  static void toColor(const QColor &qcolor, double* color);

  /// Set up the logic default display settings from the appplication settings
  void updateLogicFromSettings();

public slots:

  /// respond to the scene events
  /// when a markups node is added, make it the active one in the combo box
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  /// when a node is removed and it is the last one in the scene, clear out
  /// the gui - the node combo box will signal that a remaining node has been
  /// selected and the GUI will update separately in that case
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);
  /// update the table after a scene is imported
  void onMRMLSceneEndImportEvent();
  /// update the table after batch processing is done, needed to trigger
  /// an update after a markups file is read in
  void onMRMLSceneEndBatchProcessEvent();
  /// clear out the gui when the scene is closed
  void onMRMLSceneEndCloseEvent();

  /// display property slots
  void onSelectedColorPickerButtonChanged(QColor qcolor);
  void onUnselectedColorPickerButtonChanged(QColor qcolor);
  void onGlyphTypeComboBoxChanged(QString value);
  void onGlyphScaleSliderWidgetChanged(double value);
  void onTextScaleSliderWidgetChanged(double value);
  void onOpacitySliderWidgetChanged(double value);

  /// display property button slots
  void onResetToDefaultDisplayPropertiesPushButtonClicked();
  void onSaveToDefaultDisplayPropertiesPushButtonClicked();

  /// list button slots
  void onVisibilityOnAllMarkupsInListPushButtonClicked();
  void onVisibilityOffAllMarkupsInListPushButtonClicked();
  void onLockAllMarkupsInListPushButtonClicked();
  void onUnlockAllMarkupsInListPushButtonClicked();
  void onSelectAllMarkupsInListPushButtonClicked();
  void onDeselectAllMarkupsInListPushButtonClicked();
  void onAddMarkupPushButtonClicked();
  void onMoveMarkupUpPushButtonClicked();
  void onMoveMarkupDownPushButtonClicked();
  void onDeleteMarkupPushButtonClicked();
  void onDeleteAllMarkupsInListPushButtonClicked();

  /// update the selection node from the combo box
  void onActiveMarkupMRMLNodeChanged(vtkMRMLNode *markupsNode);
  /// update the combo box from the selection node
  void onSelectionNodeActivePlaceNodeIDChanged();

  /// when the user clicks in the comob box to create a new markups node,
  /// make sure that a display node is added
  void onActiveMarkupMRMLNodeAdded(vtkMRMLNode *markupsNode);

  /// toggle the markups node visibility flag
  void onListVisibileInvisiblePushButtonClicked();
  /// update the icon and tool tip on the list visibility button
  void updateListVisibileInvisiblePushButton(int visibleFlag);

  /// toggle the markups node locked flag
  void onListLockedUnlockedPushButtonClicked();

  /// update the markup from the check box
  void onUseListNameForMarkupsCheckBoxToggled(bool flag);

  /// update the mrml node from the table
  void onActiveMarkupTableCellChanged(int row, int column);
  /// react to clicks in the table
  void onActiveMarkupTableCellClicked(QTableWidgetItem* item);

  /// enable/disable editing the table if the markups node is un/locked
  void onActiveMarkupsNodeLockModifiedEvent();//vtkMRMLNode *markupsNode);
  /// update the table with the modified point information if the node is
  /// active
  void onActiveMarkupsNodePointModifiedEvent(vtkObject *caller, vtkObject *callData);
  /// update the table with the new markup if the node is active
  void onActiveMarkupsNodeMarkupAddedEvent();//vtkMRMLNode *markupsNode);
  /// update the table for the removed markup if the node is active
  void onActiveMarkupsNodeMarkupRemovedEvent();//vtkMRMLNode *markupsNode);
  /// update a table row from a modified markup
  void onActiveMarkupsNodeNthMarkupModifiedEvent(vtkObject *caller, vtkObject *callData);

  /// create a new markups node and copy the display node settings from the
  /// current markups node if set, otherwise just uses the defaults.
  void onNewMarkupWithCurrentDisplayPropertiesTriggered();

protected:
  QScopedPointer<qSlicerMarkupsModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsModuleWidget);
  Q_DISABLE_COPY(qSlicerMarkupsModuleWidget);
};

#endif
