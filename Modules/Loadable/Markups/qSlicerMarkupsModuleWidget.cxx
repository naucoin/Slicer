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

// Qt includes
#include <QDebug>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QMouseEvent>
#include <QSettings>
#include <QSignalMapper>
#include <QShortcut>
#include <QStringList>
#include <QTableWidgetItem>

// SlicerQt includes
#include "qSlicerMarkupsModuleWidget.h"
#include "ui_qSlicerMarkupsModule.h"
#include "qMRMLSceneModel.h"
#include "qSlicerApplication.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"

// module includes
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkSlicerMarkupsLogic.h"
#include "qSlicerMarkupsModule.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Markups
class qSlicerMarkupsModuleWidgetPrivate: public Ui_qSlicerMarkupsModule
{
  Q_DECLARE_PUBLIC(qSlicerMarkupsModuleWidget);
protected:
  qSlicerMarkupsModuleWidget* const q_ptr;

public:
  qSlicerMarkupsModuleWidgetPrivate(qSlicerMarkupsModuleWidget& object);
  ~qSlicerMarkupsModuleWidgetPrivate();

  void setupUi(qSlicerWidget* widget);

  /// the number of columns matches the column labels by using the size of the QStringList
  int numberOfColumns();
  /// return the column index for a given QString, -1 if not a valid header
  int columnIndex(QString label);

private:
  QStringList columnLabels;

  QAction*    newMarkupWithCurrentDisplayPropertiesAction;
};

//-----------------------------------------------------------------------------
// qSlicerMarkupsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidgetPrivate::qSlicerMarkupsModuleWidgetPrivate(qSlicerMarkupsModuleWidget& object)
  : q_ptr(&object)
{
  this->columnLabels << "Selected" << "Locked" << "Visible" << "Name" << "Description" << "X" << "Y" << "Z";

  this->newMarkupWithCurrentDisplayPropertiesAction = 0;
}

//-----------------------------------------------------------------------------
int qSlicerMarkupsModuleWidgetPrivate::columnIndex(QString label)
{
  return this->columnLabels.indexOf(label);
}

//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidgetPrivate::~qSlicerMarkupsModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  Q_Q(qSlicerMarkupsModuleWidget);
  this->Ui_qSlicerMarkupsModule::setupUi(widget);

  //std::cout << "setupUI\n";

  // use the ctk color dialog on the color picker buttons
  this->selectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);
  this->unselectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

  // set up the display properties
  QObject::connect(this->selectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                   q, SLOT(onSelectedColorPickerButtonChanged(QColor)));
  QObject::connect(this->unselectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                q, SLOT(onUnselectedColorPickerButtonChanged(QColor)));
  QObject::connect(this->glyphTypeComboBox, SIGNAL(currentIndexChanged(QString)),
                q, SLOT(onGlyphTypeComboBoxChanged(QString)));
  QObject::connect(this->glyphScaleSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onGlyphScaleSliderWidgetChanged(double)));
  QObject::connect(this->textScaleSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onTextScaleSliderWidgetChanged(double)));
  QObject::connect(this->opacitySliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onOpacitySliderWidgetChanged(double)));

  // populate the glyph type combo box
  if (this->glyphTypeComboBox->count() == 0)
    {
    vtkMRMLMarkupsDisplayNode *displayNode = vtkMRMLMarkupsDisplayNode::New();
    int min = displayNode->GetMinimumGlyphType();
    int max = displayNode->GetMaximumGlyphType();
    this->glyphTypeComboBox->setEnabled(false);
    for (int i = min; i <= max; i++)
      {
      displayNode->SetGlyphType(i);
      this->glyphTypeComboBox->addItem(displayNode->GetGlyphTypeAsString());
      }
    this->glyphTypeComboBox->setEnabled(true);
    displayNode->Delete();
    }
  // set the default value if not set
  if (this->glyphTypeComboBox->currentIndex() == 0)
    {
    vtkSmartPointer<vtkMRMLMarkupsDisplayNode> displayNode = vtkSmartPointer<vtkMRMLMarkupsDisplayNode>::New();
    QString glyphType = QString(displayNode->GetGlyphTypeAsString());
    this->glyphTypeComboBox->setEnabled(false);
    int index =  this->glyphTypeComboBox->findData(glyphType);
    if (index != -1)
      {
      this->glyphTypeComboBox->setCurrentIndex(index);
      }
    else
      {
      // glyph types start at 1, combo box is 0 indexed
      this->glyphTypeComboBox->setCurrentIndex(displayNode->GetGlyphType() - 1);
      }
    this->glyphTypeComboBox->setEnabled(true);
    }
  // set up the display properties buttons
  QObject::connect(this->resetToDefaultDisplayPropertiesPushButton,  SIGNAL(clicked()),
                   q, SLOT(onResetToDefaultDisplayPropertiesPushButtonClicked()));
  QObject::connect(this->saveToDefaultDisplayPropertiesPushButton,  SIGNAL(clicked()),
                   q, SLOT(onSaveToDefaultDisplayPropertiesPushButtonClicked()));

  // set up the list buttons
  // visibility
  QObject::connect(this->visibilityOnAllMarkupsInListPushButton, SIGNAL(clicked()),
                   q, SLOT(onVisibilityOnAllMarkupsInListPushButtonClicked()));
  QObject::connect(this->visibilityOffAllMarkupsInListPushButton, SIGNAL(clicked()),
                   q, SLOT(onVisibilityOffAllMarkupsInListPushButtonClicked()));
  // lock
  QObject::connect(this->lockAllMarkupsInListPushButton, SIGNAL(clicked()),
                   q, SLOT(onLockAllMarkupsInListPushButtonClicked()));
  QObject::connect(this->unlockAllMarkupsInListPushButton, SIGNAL(clicked()),
                   q, SLOT(onUnlockAllMarkupsInListPushButtonClicked()));
  // selected
  QObject::connect(this->selectAllMarkupsInListPushButton, SIGNAL(clicked()),
                   q, SLOT(onSelectAllMarkupsInListPushButtonClicked()));
  QObject::connect(this->deselectAllMarkupsInListPushButton, SIGNAL(clicked()),
                   q, SLOT(onDeselectAllMarkupsInListPushButtonClicked()));
  // add
  QObject::connect(this->addMarkupPushButton, SIGNAL(clicked()),
                   q, SLOT(onAddMarkupPushButtonClicked()));
  // move
  QObject::connect(this->moveMarkupUpPushButton, SIGNAL(clicked()),
                   q, SLOT(onMoveMarkupUpPushButtonClicked()));
  QObject::connect(this->moveMarkupDownPushButton, SIGNAL(clicked()),
                   q, SLOT(onMoveMarkupDownPushButtonClicked()));
  // delete
  QObject::connect(this->deleteMarkupPushButton, SIGNAL(clicked()),
                   q, SLOT(onDeleteMarkupPushButtonClicked()));
  QObject::connect(this->deleteAllMarkupsInListPushButton, SIGNAL(clicked()),
                   q, SLOT(onDeleteAllMarkupsInListPushButtonClicked()));

  // set up the active markups node selector
  QObject::connect(this->activeMarkupMRMLNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onActiveMarkupMRMLNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->activeMarkupMRMLNodeComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
                   q, SLOT(onActiveMarkupMRMLNodeAdded(vtkMRMLNode*)));

  vtkMRMLNode *selectionNode = NULL;
  if (q->mrmlScene())
    {
    selectionNode = q->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton");
    }
  else
    {
    //qDebug() << "No mrml scene set! q = " << q;
    }
  if (selectionNode)
    {
    q->qvtkConnect(selectionNode, vtkMRMLSelectionNode::ActivePlaceNodeIDChangedEvent,
                   q, SLOT(onSelectionNodeActivePlaceNodeIDChanged()));
    }
  else
    {
    //qDebug() << "No selection node found in scene!";
    }

  //
  // add an action to create a new markups list using the display node
  // settings from the currently active list
  //

  this->newMarkupWithCurrentDisplayPropertiesAction =
    new QAction("New markups with current display properties",
                this->activeMarkupMRMLNodeComboBox);
  this->activeMarkupMRMLNodeComboBox->addMenuAction(this->newMarkupWithCurrentDisplayPropertiesAction);
  this->activeMarkupMRMLNodeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  QObject::connect(this->newMarkupWithCurrentDisplayPropertiesAction, SIGNAL(triggered()),
                    q, SLOT(onNewMarkupWithCurrentDisplayPropertiesTriggered()));

  //
  // set up the list visibility/locked buttons
  //
  QObject::connect(this->listVisibileInvisiblePushButton, SIGNAL(clicked()),
                   q, SLOT(onListVisibileInvisiblePushButtonClicked()));
  QObject::connect(this->listLockedUnlockedPushButton, SIGNAL(clicked()),
                   q, SLOT(onListLockedUnlockedPushButtonClicked()));
  //
  // set up the use list name for markups check box
  //
  QObject::connect(this->useListNameForMarkupsCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onUseListNameForMarkupsCheckBoxToggled(bool)));
  //
  // set up the use list name for markups check box
  //
  QObject::connect(this->useListNameForMarkupsCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onUseListNameForMarkupsCheckBoxToggled(bool)));
  //
  // set up the table
  //

  // only select rows rather than cells
  this->activeMarkupTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  // allow multi select
  this->activeMarkupTableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

  // number of columns with headers
  this->activeMarkupTableWidget->setColumnCount(this->numberOfColumns());
  this->activeMarkupTableWidget->setHorizontalHeaderLabels(this->columnLabels);
  // use an icon for some column headers
  // selected is a check box
  QTableWidgetItem *selectedHeader = this->activeMarkupTableWidget->horizontalHeaderItem(this->columnIndex("Selected"));
  selectedHeader->setText("");
  selectedHeader->setIcon(QIcon(":/Icons/MarkupsSelected.png"));
  selectedHeader->setToolTip(QString("Click in this column to select/deselect markups for passing to CLI modules"));
  this->activeMarkupTableWidget->setColumnWidth(this->columnIndex("Selected"), 30);
  // locked is an open and closed lock
  QTableWidgetItem *lockedHeader = this->activeMarkupTableWidget->horizontalHeaderItem(this->columnIndex("Locked"));
  lockedHeader->setText("");
  lockedHeader->setIcon(QIcon(":/Icons/Small/SlicerLockUnlock.png"));
  lockedHeader->setToolTip(QString("Click in this column to lock/unlock markups to prevent them from being moved by mistake"));
  this->activeMarkupTableWidget->setColumnWidth(this->columnIndex("Locked"), 30);
  // visible is an open and closed eye
  QTableWidgetItem *visibleHeader = this->activeMarkupTableWidget->horizontalHeaderItem(this->columnIndex("Visible"));
  visibleHeader->setText("");
  visibleHeader->setIcon(QIcon(":/Icons/Small/SlicerVisibleInvisible.png"));
  visibleHeader->setToolTip(QString("Click in this column to show/hide markups in 2D and 3D"));
  this->activeMarkupTableWidget->setColumnWidth(this->columnIndex("Visible"), 30);

  // listen for changes so can update mrml node
  QObject::connect(this->activeMarkupTableWidget, SIGNAL(cellChanged(int, int)),
                   q, SLOT(onActiveMarkupTableCellChanged(int, int)));

  // listen for click on a markup
  QObject::connect(this->activeMarkupTableWidget, SIGNAL(itemClicked(QTableWidgetItem*)),
                   q, SLOT(onActiveMarkupTableCellClicked(QTableWidgetItem*)));
  // listen for a right click
  this->activeMarkupTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(this->activeMarkupTableWidget, SIGNAL(customContextMenuRequested(QPoint)),
                   q, SLOT(onRightClickActiveMarkupTableWidget(QPoint)));
}

//-----------------------------------------------------------------------------
int qSlicerMarkupsModuleWidgetPrivate::numberOfColumns()
{
  return this->columnLabels.size();
}
//-----------------------------------------------------------------------------
// qSlicerMarkupsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidget::qSlicerMarkupsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
    , d_ptr( new qSlicerMarkupsModuleWidgetPrivate(*this) )
{
  this->pToAddShortcut = 0;
}


//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidget::~qSlicerMarkupsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::setup()
{
  Q_D(qSlicerMarkupsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  // for now disable the combo box: when the combo box has it's mrml scene
  // set, it sets the first markups node as the current node, which can end
  // up over riding the selection node's active place node id
  // d->activeMarkupMRMLNodeComboBox->setEnabled(false);
  d->activeMarkupMRMLNodeComboBox->blockSignals(true);

  // this->updateLogicFromSettings();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::enter()
{
  Q_D(qSlicerMarkupsModuleWidget);

  this->Superclass::enter();

  // qDebug() << "enter widget";

  // set up mrml scene observations so that the GUI gets updated
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::NodeAddedEvent,
                    this, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::NodeRemovedEvent,
                    this, SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndImportEvent,
                    this, SLOT(onMRMLSceneEndImportEvent()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndBatchProcessEvent,
                    this, SLOT(onMRMLSceneEndBatchProcessEvent()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndCloseEvent,
                    this, SLOT(onMRMLSceneEndCloseEvent()));

  // now enable the combo box and update
  //d->activeMarkupMRMLNodeComboBox->setEnabled(true);
  d->activeMarkupMRMLNodeComboBox->blockSignals(false);

  // install some shortcuts for use while in this module
  this->installShortcuts();

  this->UpdateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::installShortcuts()
{
  // add some shortcut keys
  if (this->pToAddShortcut == 0)
    {
    this->pToAddShortcut = new QShortcut(QKeySequence(QString("p")), this);
    }
  QObject::connect(this->pToAddShortcut, SIGNAL(activated()),
                   this, SLOT(onPKeyActivated()));
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::removeShortcuts()
{
  if (this->pToAddShortcut != 0)
    {
    //qDebug() << "removeShortcuts";
    this->pToAddShortcut->disconnect(SIGNAL(activated()));
    // TODO: when parent is set to null, using the mouse to place a fid when outside the Markups module is triggering a crash
//    this->pToAddShortcut->setParent(NULL);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::exit()
{
  this->Superclass::exit();

  // qDebug() << "exit widget";

  this->removeShortcuts();

  // remove mrml scene observations, don't need to update the GUI while the
  // module is not showing
  this->qvtkDisconnectAll();
}

//-----------------------------------------------------------------------------
vtkSlicerMarkupsLogic *qSlicerMarkupsModuleWidget::markupsLogic()
{
  if (this->logic() == NULL)
    {
    return NULL;
    }
  return vtkSlicerMarkupsLogic::SafeDownCast(this->logic());
}


//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::UpdateWidgetFromMRML()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // std::cout << "UpdateWidgetFromMRML" << std::endl;

  // get the active markup
  vtkMRMLNode *node = this->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton");
  vtkMRMLSelectionNode *selectionNode = NULL;
  if (node)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(node);
    }
  vtkMRMLNode *markupsNodeMRML = NULL;
  QString activePlaceNodeID;
  if (selectionNode)
    {
    activePlaceNodeID = selectionNode->GetActivePlaceNodeID();
    markupsNodeMRML = this->mrmlScene()->GetNodeByID(activePlaceNodeID.toLatin1());
    }
  vtkMRMLMarkupsNode *markupsNode = NULL;
  if (markupsNodeMRML)
    {
    markupsNode = vtkMRMLMarkupsNode::SafeDownCast(markupsNodeMRML);
    }

  // make sure that the GUI updates on changes to the current mark up node,
  // remove observations if no current node
  this->observeMarkupsNode(markupsNode);

  if (!markupsNode)
    {
    // qDebug() << "UpdateWidgetFromMRML: Unable to get active markups node,
    // clearing out the table";
    this->clearGUI();

    return;
    }

  // update the combo box
//  this->onSelectionNodeActivePlaceNodeIDChanged();
  QString currentNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeID();
  //std::cout << "UpdateWidgetFromMRML: selection node's active place node id: " << qPrintable(activePlaceNodeID) << ", combo box current node id = " << qPrintable(currentNodeID) << std::endl;
  if (currentNodeID == "" ||
      (currentNodeID != activePlaceNodeID &&
       activePlaceNodeID.contains("vtkMRMLMarkups")))
    {
    d->activeMarkupMRMLNodeComboBox->setCurrentNode(activePlaceNodeID);
    }

  // update the display properties from the markups display node
  vtkMRMLDisplayNode *displayNode = markupsNode->GetDisplayNode();
  double *color;
  QColor qColor;

  if (displayNode)
    {
    // selected color
    color = displayNode->GetSelectedColor();
    qSlicerMarkupsModuleWidget::toQColor(color, qColor);
    d->selectedColorPickerButton->setColor(qColor);

    // unselected color
    color = displayNode->GetColor();
    qSlicerMarkupsModuleWidget::toQColor(color, qColor);
    d->unselectedColorPickerButton->setColor(qColor);

    // opacity
    double opacity = displayNode->GetOpacity();
    d->opacitySliderWidget->setValue(opacity);

    // now for some markups specific display properties
    vtkMRMLMarkupsDisplayNode *markupsDisplayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(displayNode);
    if (markupsDisplayNode)
      {
      // does the glyph type combo box have entries?
      if (d->glyphTypeComboBox->count() == 0)
        {
        qDebug() << "No entries in the glyph type combo box";
        }
      else
        {
        // glyph type
        const char *glyphType = markupsDisplayNode->GetGlyphTypeAsString();
        int index =  d->glyphTypeComboBox->findData(QString(glyphType));
        if (index != -1)
          {
          d->glyphTypeComboBox->setCurrentIndex(index);
          }
        else
          {
          // glyph types start at 1, combo box is 0 indexed
          d->glyphTypeComboBox->setCurrentIndex(markupsDisplayNode->GetGlyphType() - 1);
          }
        }

      // glyph scale
      double glyphScale = markupsDisplayNode->GetGlyphScale();
      d->glyphScaleSliderWidget->setValue(glyphScale);

      // text scale
      double textScale = markupsDisplayNode->GetTextScale();
      d->textScaleSliderWidget->setValue(textScale);
      }
    }
  else
    {
    // reset to defaults from logic
    color = this->markupsLogic()->GetDefaultMarkupsDisplayNodeSelectedColor();
    qSlicerMarkupsModuleWidget::toQColor(color, qColor);
    d->selectedColorPickerButton->setColor(qColor);
    color = this->markupsLogic()->GetDefaultMarkupsDisplayNodeColor();
    qSlicerMarkupsModuleWidget::toQColor(color, qColor);
    d->unselectedColorPickerButton->setColor(qColor);
    d->opacitySliderWidget->setValue(this->markupsLogic()->GetDefaultMarkupsDisplayNodeOpacity());
    QString glyphTypeString = QString(this->markupsLogic()->GetDefaultMarkupsDisplayNodeGlyphTypeAsString().c_str());
    int glyphTypeInt = this->markupsLogic()->GetDefaultMarkupsDisplayNodeGlyphType();
    // glyph types start at 1, combo box is 0 indexed
    int glyphTypeIndex = glyphTypeInt - 1;
    if (glyphTypeIndex != -1)
      {
      d->glyphTypeComboBox->setCurrentIndex(glyphTypeIndex);
      }
    d->glyphScaleSliderWidget->setValue(this->markupsLogic()->GetDefaultMarkupsDisplayNodeGlyphScale());
    d->textScaleSliderWidget->setValue(this->markupsLogic()->GetDefaultMarkupsDisplayNodeTextScale());
    }
  // update the visibility and locked buttons
  this->updateListVisibileInvisiblePushButton(markupsNode->GetDisplayVisibility());

  if (markupsNode->GetLocked())
    {
    d->listLockedUnlockedPushButton->setIcon(QIcon(":Icons/Medium/SlicerLock.png"));
    d->listLockedUnlockedPushButton->setToolTip(QString("Click to unlock this markup list so that the markups can be moved by the mouse"));
    }
  else
    {
    d->listLockedUnlockedPushButton->setIcon(QIcon(":Icons/Medium/SlicerUnlock.png"));
    d->listLockedUnlockedPushButton->setToolTip(QString("Click to lock this markup list so that the markups cannot be moved by the mouse"));
    }

  // update the table
  int numberOfMarkups = markupsNode->GetNumberOfMarkups();
  if (d->activeMarkupTableWidget->rowCount() != numberOfMarkups)
    {
    // clear it out
    d->activeMarkupTableWidget->setRowCount(numberOfMarkups);
    }
  // update the table per markup
  for (int m = 0; m < numberOfMarkups; m++)
    {
    this->UpdateRow(m);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::UpdateRow(int m)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // this is updating the qt widget from MRML, and should not trigger any updates on the node, so turn off events
  d->activeMarkupTableWidget->blockSignals(true);

  // qDebug() << QString("UpdateRow: row = ") + QString::number(m) + QString(", number of rows = ") + QString::number(d->activeMarkupTableWidget->rowCount());
  // get active markups node
  QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeID();
  vtkMRMLNode *mrmlNode = this->mrmlScene()->GetNodeByID(activeMarkupsNodeID.toLatin1());
  vtkMRMLMarkupsNode *markupsNode = NULL;
  if (mrmlNode)
    {
    markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (!markupsNode)
    {
    //qDebug() << QString("Update Row: unable to get markups node with id ") + activeMarkupsNodeID;
    return;
    }

  // selected
  QTableWidgetItem* selectedItem = new QTableWidgetItem();
  if (markupsNode->GetNthMarkupSelected(m))
    {
    selectedItem->setCheckState(Qt::Checked);
    }
  else
    {
    selectedItem->setCheckState(Qt::Unchecked);
    }
  int selectedIndex = d->columnIndex("Selected");
  if (d->activeMarkupTableWidget->item(m,selectedIndex) == NULL ||
      (d->activeMarkupTableWidget->item(m,selectedIndex)->checkState() != selectedItem->checkState()))
    {
    d->activeMarkupTableWidget->setItem(m,selectedIndex,selectedItem);
    }

  // locked
  QTableWidgetItem* lockedItem = new QTableWidgetItem();
  // disable checkable
  lockedItem->setData(Qt::CheckStateRole, QVariant());
  lockedItem->setFlags(lockedItem->flags() & ~Qt::ItemIsUserCheckable);
  if (markupsNode->GetNthMarkupLocked(m))
    {
    lockedItem->setData(Qt::UserRole, QVariant(true));
    lockedItem->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerLock.png"));
    }
  else
    {
    lockedItem->setData(Qt::UserRole, QVariant(false));
    lockedItem->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerUnlock.png"));
    }
  int lockedIndex = d->columnIndex("Locked");
  if (d->activeMarkupTableWidget->item(m,lockedIndex) == NULL ||
      d->activeMarkupTableWidget->item(m,lockedIndex)->data(Qt::UserRole) != lockedItem->data(Qt::UserRole))
    {
    d->activeMarkupTableWidget->setItem(m,lockedIndex,lockedItem);
    }

  // visible
  QTableWidgetItem* visibleItem = new QTableWidgetItem();
  // disable checkable
  visibleItem->setData(Qt::CheckStateRole, QVariant());
  visibleItem->setFlags(visibleItem->flags() & ~Qt::ItemIsUserCheckable);
  if (markupsNode->GetNthMarkupVisibility(m))
    {
    visibleItem->setData(Qt::UserRole, QVariant(true));
    visibleItem->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerVisible.png"));
    }
  else
    {
    visibleItem->setData(Qt::UserRole, QVariant(false));
    visibleItem->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerInvisible.png"));
    }
    int visibleIndex = d->columnIndex("Visible");
   if (d->activeMarkupTableWidget->item(m,visibleIndex) == NULL ||
       d->activeMarkupTableWidget->item(m,visibleIndex)->data(Qt::UserRole) != visibleItem->data(Qt::UserRole))
     {
     d->activeMarkupTableWidget->setItem(m,visibleIndex,visibleItem);
     }

   // name
   int nameIndex = d->columnIndex("Name");
   QString markupLabel = QString(markupsNode->GetNthMarkupLabel(m).c_str());
   if (d->activeMarkupTableWidget->item(m,nameIndex) == NULL ||
       d->activeMarkupTableWidget->item(m,nameIndex)->text() != markupLabel)
     {
     d->activeMarkupTableWidget->setItem(m,nameIndex,new QTableWidgetItem(markupLabel));
     }

   // description
   int descriptionIndex = d->columnIndex("Description");
   QString markupDescription = QString(markupsNode->GetNthMarkupDescription(m).c_str());
   if (d->activeMarkupTableWidget->item(m,descriptionIndex) == NULL ||
       d->activeMarkupTableWidget->item(m,descriptionIndex)->text() != markupLabel)
     {
     d->activeMarkupTableWidget->setItem(m,descriptionIndex,new QTableWidgetItem(markupDescription));
     }

   // point
   double point[3];
  markupsNode->GetMarkupPoint(m, 0, point);
  int xColumnIndex = d->columnIndex("X");
  for (int p = 0; p < 3; p++)
    {
    // last argument to number sets the precision
    QString coordinate = QString::number(point[p], 'f', 6);
    if (d->activeMarkupTableWidget->item(m,xColumnIndex + p) == NULL ||
        d->activeMarkupTableWidget->item(m,xColumnIndex + p)->text() != coordinate)
      {
      d->activeMarkupTableWidget->setItem(m,xColumnIndex + p,new QTableWidgetItem(coordinate));
      }
    }

  // unblock so that changes to the table will propagate to MRML
  d->activeMarkupTableWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onNodeAddedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerMarkupsModuleWidget);

  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (markupsNode)
    {
    // make it active
    d->activeMarkupMRMLNodeComboBox->setCurrentNode(markupsNode->GetID());
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);

  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }

  // only respond if it was the last node that was removed
  int numNodes = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLMarkupsNode");
  if (numNodes == 0)
    {
    this->clearGUI();
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMRMLSceneEndImportEvent()
{
  this->UpdateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMRMLSceneEndBatchProcessEvent()
{
  if (!this->mrmlScene())
    {
    return;
    }
  // qDebug() << "qSlicerMarkupsModuleWidget::onMRMLSceneEndBatchProcessEvent";
  vtkMRMLNode *node = this->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton");
  vtkMRMLSelectionNode *selectionNode = NULL;
  if (node)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(node);
    }

  QString activePlaceNodeID;
  if (selectionNode)
    {
    activePlaceNodeID = selectionNode->GetActivePlaceNodeID();
    }
  if (activePlaceNodeID.isEmpty())
    {
    // this might have been triggered after a file load, set the last markups node active
    int numNodes = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLMarkupsNode");
    if (numNodes > 0)
      {
      vtkMRMLNode *lastNode = this->mrmlScene()->GetNthNodeByClass(numNodes - 1, "vtkMRMLMarkupsNode");
      if (lastNode)
        {
        qDebug() << "onMRMLSceneEndBatchProcessEvent: setting active place node id to " << lastNode->GetID();
        selectionNode->SetActivePlaceNodeID(lastNode->GetID());
        }
      }
    }
  this->UpdateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMRMLSceneEndCloseEvent()
{
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
  // qDebug() << "qSlicerMarkupsModuleWidget::onMRMLSceneEndCloseEvent";
  this->clearGUI();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onPKeyActivated()
{
  QPoint pos = QCursor::pos();

  // find out which widget it was over
  QWidget *widget = qSlicerApplication::application()->widgetAt(pos);

  // simulate a mouse press inside the widget
  QPoint widgetPos = widget->mapFromGlobal(pos);
  QMouseEvent click(QEvent::MouseButtonRelease, widgetPos, Qt::LeftButton, 0, 0);
  click.setAccepted(true);

  // and send it to the widget
  //qDebug() << "onPKeyActivated: sending event with pos " << widgetPos;
  QCoreApplication::sendEvent(widget, &click);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSelectedColorPickerButtonChanged(QColor qcolor)
{
  Q_D(qSlicerMarkupsModuleWidget);

  double color[3];
  qSlicerMarkupsModuleWidget::toColor(qcolor, color);

   // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsFiducialNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(mrmlNode);
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = NULL;
  if (listNode)
    {
    displayNode = listNode->GetMarkupsDisplayNode();
    }
  if (displayNode)
    {
    displayNode->SetSelectedColor(color);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onUnselectedColorPickerButtonChanged(QColor qcolor)
{
  Q_D(qSlicerMarkupsModuleWidget);

  double color[3];
  qSlicerMarkupsModuleWidget::toColor(qcolor, color);

   // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsFiducialNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(mrmlNode);
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = NULL;
  if (listNode)
    {
    displayNode = listNode->GetMarkupsDisplayNode();
    }
  if (displayNode)
    {
    displayNode->SetColor(color);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onGlyphTypeComboBoxChanged(QString value)
{
  Q_D(qSlicerMarkupsModuleWidget);

  if (value.isEmpty())
    {
    return;
    }
  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsFiducialNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(mrmlNode);
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = NULL;
  if (listNode)
    {
    displayNode = listNode->GetMarkupsDisplayNode();
    }
  if (displayNode)
    {
    displayNode->SetGlyphTypeFromString(value.toLatin1());
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onGlyphScaleSliderWidgetChanged(double value)
{
  Q_D(qSlicerMarkupsModuleWidget);
  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsFiducialNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(mrmlNode);
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = NULL;
  if (listNode)
    {
    displayNode = listNode->GetMarkupsDisplayNode();
    }
  if (displayNode)
    {
    displayNode->SetGlyphScale(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onTextScaleSliderWidgetChanged(double value)
{
  Q_D(qSlicerMarkupsModuleWidget);
  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsFiducialNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(mrmlNode);
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = NULL;
  if (listNode)
    {
    displayNode = listNode->GetMarkupsDisplayNode();
    }
  if (displayNode)
    {
    displayNode->SetTextScale(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onOpacitySliderWidgetChanged(double value)
{
   Q_D(qSlicerMarkupsModuleWidget);
  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsFiducialNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(mrmlNode);
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = NULL;
  if (listNode)
    {
    displayNode = listNode->GetMarkupsDisplayNode();
    }
  if (displayNode)
    {
    displayNode->SetOpacity(value);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onResetToDefaultDisplayPropertiesPushButtonClicked()
{
   Q_D(qSlicerMarkupsModuleWidget);
  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsFiducialNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(mrmlNode);
    }
  if (!listNode)
    {
    return;
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = NULL;
  displayNode = listNode->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return;
    }

  // set the display node from the logic defaults
  this->markupsLogic()->SetDisplayNodeToDefaults(displayNode);

  // push an update on the GUI
  this->UpdateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSaveToDefaultDisplayPropertiesPushButtonClicked()
{
   Q_D(qSlicerMarkupsModuleWidget);
  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsFiducialNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(mrmlNode);
    }
  if (!listNode)
    {
    return;
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = NULL;
  displayNode = listNode->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return;
    }

  // save the settings
  QSettings *settings = qSlicerApplication::application()->settingsDialog()->settings();
  settings->setValue("Markups/GlyphType", displayNode->GetGlyphTypeAsString());
  QColor qcolor;
  double  *selectedColor = displayNode->GetSelectedColor();
  if (selectedColor)
    {
    qcolor = QColor::fromRgbF(selectedColor[0], selectedColor[1], selectedColor[2]);
    }
  settings->setValue("Markups/SelectedColor", qcolor);
  double *unselectedColor = displayNode->GetColor();
  if (unselectedColor)
    {
    qcolor = QColor::fromRgbF(unselectedColor[0], unselectedColor[1], unselectedColor[2]);
    }
  settings->setValue("Markups/UnselectedColor", qcolor);
  settings->setValue("Markups/GlyphScale", displayNode->GetGlyphScale());
  settings->setValue("Markups/TextScale", displayNode->GetTextScale());
  settings->setValue("Markups/Opacity", displayNode->GetOpacity());

  // set the logic defaults from the settings
  this->updateLogicFromSettings();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onVisibilityOnAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsVisibility(listNode, true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onVisibilityOffAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsVisibility(listNode, false);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onLockAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsLocked(listNode, true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onUnlockAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsLocked(listNode, false);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSelectAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsSelected(listNode, true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onDeselectAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsSelected(listNode, false);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onAddMarkupPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (listNode)
    {
    // for now, assume a fiducial
    listNode->AddMarkupWithNPoints(1);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMoveMarkupUpPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, that only one is selected
  if ((selectedItems.size() / d->numberOfColumns()) != 1)
    {
    qDebug() << "Move up: only select one markup to move, current selected: " << selectedItems.size() << ", number of columns = " << d->numberOfColumns();
    return;
    }
  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (listNode)
    {
    int thisIndex = selectedItems.at(0)->row();
    //qDebug() << "Swapping " << thisIndex << " and " << thisIndex - 1;
    listNode->SwapMarkups(thisIndex, thisIndex - 1);
    // now make sure the new row is selected so a user can keep moving it up
    d->activeMarkupTableWidget->selectRow(thisIndex - 1);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMoveMarkupDownPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, that only one is selected
  if ((selectedItems.size() / d->numberOfColumns()) != 1)
    {
    return;
    }
  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (listNode)
    {
    int thisIndex = selectedItems.at(0)->row();
    //qDebug() << "Swapping " << thisIndex << " and " << thisIndex + 1;
    listNode->SwapMarkups(thisIndex, thisIndex + 1);
    // now make sure the new row is selected so a user can keep moving it down
    d->activeMarkupTableWidget->selectRow(thisIndex + 1);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onDeleteMarkupPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (!listNode)
    {
    qDebug() << "Delete markup: no active list from which to delete";
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  for (int i = 0; i < selectedItems.size(); i += d->numberOfColumns())
    {
    // get the row
    int row = selectedItems.at(i)->row();
    // qDebug() << "Saving: i = " << i << ", row = " << row;
    rows << row;
    }
  // sort the list
  qSort(rows);
  // delete from the end
  for (int i = rows.size() - 1; i >= 0; --i)
    {
    int index = rows.at(i);
    // qDebug() << "Deleting: i = " << i << ", index = " << index;
    // remove the markup at that row
    listNode->RemoveMarkup(index);
    }
  // clear the selection on the table
  d->activeMarkupTableWidget->clearSelection();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onDeleteAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (listNode)
    {
    // qDebug() << "Removing markups from list " << listNode->GetName();
    listNode->RemoveAllMarkups();
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupMRMLNodeChanged(vtkMRMLNode *markupsNode)
{
  Q_D(qSlicerMarkupsModuleWidget);

  //qDebug() << "onActiveMarkupMRMLNodeChanged, markupsNode is " << (markupsNode ? markupsNode->GetID() : "null");

  // get the current node from the combo box
  QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeID();
  const char *activeID = NULL;
  if (markupsNode)
    {
    activeID = markupsNode->GetID();
    }

  //qDebug() << "setActiveMarkupsNode: combo box says: " << qPrintable(activeMarkupsNodeID) << ", input node says " << (activeID ? activeID : "null");
  // update the selection node
  vtkMRMLNode *node = this->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton");
  vtkMRMLSelectionNode *selectionNode = NULL;
  if (node)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(node);
    }
  if (selectionNode)
    {
    // check if changed
    const char *selectionNodeActivePlaceNodeID = selectionNode->GetActivePlaceNodeID();
    // don't update the selection node if the active ID is null (can happen
    // when entering the module)
    if (activeID != NULL)
      {
      if (!selectionNodeActivePlaceNodeID || !activeID ||
          strcmp(selectionNodeActivePlaceNodeID, activeID) != 0)
        {
        selectionNode->SetReferenceActivePlaceNodeID(activeID);
        }
      }
    else
      {
      if (selectionNodeActivePlaceNodeID != NULL)
        {
        //std::cout << "Setting combo box from selection node " << selectionNodeActivePlaceNodeID << std::endl;
        d->activeMarkupMRMLNodeComboBox->setCurrentNode(selectionNodeActivePlaceNodeID);
        }
      }
    }
  else
    {
    qDebug() << "Failed to change active markups node id on selection node";
    }

  // update the GUI
  this->UpdateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupMRMLNodeAdded(vtkMRMLNode *markupsNode)
{
  // qDebug() << "onActiveMarkupMRMLNodeAdded, markupsNode is " << (markupsNode ? markupsNode->GetID() : "null");

  if (this->markupsLogic())
    {
    this->markupsLogic()->AddNewDisplayNodeForMarkupsNode(markupsNode);
    }
  // update the visibility button
  vtkMRMLMarkupsNode *displayableNode = NULL;
  if (markupsNode)
    {
    displayableNode = vtkMRMLMarkupsNode::SafeDownCast(markupsNode);
    }
  if (displayableNode)
    {
    int visibleFlag = displayableNode->GetDisplayVisibility();
    this->updateListVisibileInvisiblePushButton(visibleFlag);
    }

  // make sure it's set up for the mouse mode tool bar to easily add points to
  // it by making it active in the selection node
  vtkMRMLNode *node = this->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton");
  vtkMRMLSelectionNode *selectionNode = NULL;
  if (node)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(node);
    }
  if (selectionNode)
    {
    // check if need to update the current type of node that's being placed
    const char *activePlaceNodeClassName = selectionNode->GetActivePlaceNodeClassName();
    if (!activePlaceNodeClassName ||
        (activePlaceNodeClassName &&
         strcmp(activePlaceNodeClassName, markupsNode->GetClassName()) != 0))
      {
      // call the set reference to make sure the event is invoked
      selectionNode->SetReferenceActivePlaceNodeClassName(markupsNode->GetClassName());
      }
    // set this markup node active if it's not already
    const char *activePlaceNodeID = selectionNode->GetActivePlaceNodeID();
    if (!activePlaceNodeID ||
        (activePlaceNodeID && strcmp(activePlaceNodeID, markupsNode->GetID()) != 0))
      {
      selectionNode->SetActivePlaceNodeID(markupsNode->GetID());
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSelectionNodeActivePlaceNodeIDChanged()
{
  Q_D(qSlicerMarkupsModuleWidget);

  //qDebug() << "onSelectionNodeActivePlaceNodeIDChanged";

  // get the selection node
  vtkMRMLNode *node = this->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton");
  vtkMRMLSelectionNode *selectionNode = NULL;
  if (node)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(node);
    }

  if (selectionNode)
    {
    if (selectionNode->GetActivePlaceNodeID() != NULL)
      {
      QString activePlaceNodeID = selectionNode->GetActivePlaceNodeID();
      //std::cout << "\ttesting selection node's active place node id: " << qPrintable(activePlaceNodeID) << std::endl;
      QString currentNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeID();
      //std::cout << "\t\tcombo box current node id = " << qPrintable(currentNodeID) << std::endl;
      if (currentNodeID == "" ||
          (currentNodeID != activePlaceNodeID &&
           activePlaceNodeID.contains("vtkMRMLMarkups")))
        {
        d->activeMarkupMRMLNodeComboBox->setCurrentNode(activePlaceNodeID);
        }
      }
    else
      {
      qDebug() << "onSelectionNodeActivePlaceNodeIDChanged: No current active place node id";
      d->activeMarkupMRMLNodeComboBox->setCurrentNode("");
      }
    }
  else
    {
    qDebug() << "Unable to update combo box from selection node, node is " << (selectionNode ? selectionNode->GetID() : "null");
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onListVisibileInvisiblePushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (!listNode)
    {
    return;
    }
  // toggle the visibility
  int visibleFlag = listNode->GetDisplayVisibility();
  visibleFlag = !visibleFlag;
  listNode->SetDisplayVisibility(visibleFlag);
  this->updateListVisibileInvisiblePushButton(visibleFlag);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::updateListVisibileInvisiblePushButton(int visibleFlag)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // update the list visibility button icon and tool tip
  if (visibleFlag)
    {
    d->listVisibileInvisiblePushButton->setIcon(QIcon(":Icons/Medium/SlicerVisible.png"));
    d->listVisibileInvisiblePushButton->setToolTip(QString("Click to hide this markup list"));
    }
  else
    {
    d->listVisibileInvisiblePushButton->setIcon(QIcon(":Icons/Medium/SlicerInvisible.png"));
    d->listVisibileInvisiblePushButton->setToolTip(QString("Click to show this markup list"));
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onListLockedUnlockedPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (!listNode)
    {
    return;
    }
  // toggle the lock
  int locked = listNode->GetLocked();
  locked = !locked;
  listNode->SetLocked(locked);

  // update the button
  if (locked)
    {
    d->listLockedUnlockedPushButton->setIcon(QIcon(":Icons/Medium/SlicerLock.png"));
    d->listLockedUnlockedPushButton->setToolTip(QString("Click to unlock this markup list so that the markups can be moved by the mouse"));
    }
  else
    {
    d->listLockedUnlockedPushButton->setIcon(QIcon(":Icons/Medium/SlicerUnlock.png"));
    d->listLockedUnlockedPushButton->setToolTip(QString("Click to lock this markup list so that the markups cannot be moved by the mouse"));
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onUseListNameForMarkupsCheckBoxToggled(bool flag)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active list
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (!listNode)
    {
    qDebug() << QString("List name check box toggled: unable to get current list");
    return;
    }
  if (flag)
    {
    listNode->UseListNameForMarkupsOn();
    }
  else
    {
    listNode->UseListNameForMarkupsOff();
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupTableCellChanged(int row, int column)
{
  Q_D(qSlicerMarkupsModuleWidget);

//  qDebug() << QString("cell changed: row = ") + QString::number(row) + QString(", col = ") + QString::number(column);
  // get the active list
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  vtkMRMLMarkupsNode *listNode = NULL;
  if (mrmlNode)
    {
    listNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (!listNode)
    {
    qDebug() << QString("Cell Changed: unable to get current list");
    return;
    }

  // row corresponds to the index in the list
  int n = row;

  // now switch on the property
  QTableWidgetItem *item = d->activeMarkupTableWidget->item(row, column);
  if (!item)
    {
    qDebug() << QString("Unable to find item in table at ") + QString::number(row) + QString(", ") + QString::number(column);
    return;
    }
  if (column == d->columnIndex("Selected"))
    {
    bool flag = (item->checkState() == Qt::Unchecked ? false : true);
    listNode->SetNthMarkupSelected(n, flag);
    }
  else if (column == d->columnIndex("Locked"))
    {
    bool flag = item->data(Qt::UserRole) == QVariant(true) ? true : false;
    // update the icon
    if (flag)
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerLock.png"));
      }
    else
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerUnlock.png"));
      }
    listNode->SetNthMarkupLocked(n, flag);
    }
  else if (column == d->columnIndex("Visible"))
    {
    bool flag = item->data(Qt::UserRole) == QVariant(true) ? true : false;
    // update the eye icon
    if (flag)
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerVisible.png"));
      }
    else
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerInvisible.png"));
      }
    listNode->SetNthMarkupVisibility(n, flag);
    }
  else if (column ==  d->columnIndex("Name"))
    {
    std::string name = std::string(item->text().toLatin1());
    listNode->SetNthMarkupLabel(n, name);
    }
  else if (column ==  d->columnIndex("Description"))
    {
    std::string description = std::string(item->text().toLatin1());
    listNode->SetNthMarkupDescription(n, description);
    }
  else if (column == d->columnIndex("X") ||
           column == d->columnIndex("Y") ||
           column == d->columnIndex("Z"))
    {
    // get the new value
    double newPoint[3] = {0.0, 0.0, 0.0};
    if (d->activeMarkupTableWidget->item(row, d->columnIndex("X")) == NULL ||
        d->activeMarkupTableWidget->item(row, d->columnIndex("Y")) == NULL ||
        d->activeMarkupTableWidget->item(row, d->columnIndex("Z")) == NULL)
      {
      // init state, return
      return;
      }
    newPoint[0] = d->activeMarkupTableWidget->item(row, d->columnIndex("X"))->text().toDouble();
    newPoint[1] = d->activeMarkupTableWidget->item(row, d->columnIndex("Y"))->text().toDouble();
    newPoint[2] = d->activeMarkupTableWidget->item(row, d->columnIndex("Z"))->text().toDouble();

    // get the old value
    double point[3];
    listNode->GetMarkupPoint(n, 0, point);

    // changed?
    double minChange = 0.001;
    if (fabs(newPoint[0] - point[0]) > minChange ||
        fabs(newPoint[1] - point[1]) > minChange ||
        fabs(newPoint[2] - point[2]) > minChange)
      {
      vtkMRMLMarkupsFiducialNode *fidList = vtkMRMLMarkupsFiducialNode::SafeDownCast(listNode);
      if (fidList)
        {
        fidList->SetNthFiducialWorldCoordinates(n, newPoint);
        }
      }
    else
      {
      //qDebug() << QString("Cell changed: no change in location bigger than ") + QString::number(minChange);
      }
    }
  else
    {
    qDebug() << QString("Cell Changed: unknown column: ") + QString::number(column);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupTableCellClicked(QTableWidgetItem* item)
{
  Q_D(qSlicerMarkupsModuleWidget);

  if (item == 0)
    {
    return;
    }

  int row = item->row();
  int column = item->column();
  //qDebug() << "onActiveMarkupTableCellClicked: row = " << row << ", col = " << column;

  if (column == d->columnIndex(QString("Name")))
    {
    // use the node id + row index
    // get the active list
    vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
    if (!mrmlNode)
      {
      return;
      }
    // jump to it
    if (this->markupsLogic())
      {
      // qDebug() << "\tjumping to " << row << "th point in markup";
      this->markupsLogic()->JumpSlicesToNthPointInMarkup(mrmlNode->GetID(), row);
      }
    }
  else if (column == d->columnIndex(QString("Visible")) ||
           column == d->columnIndex(QString("Locked")))
    {
    // toggle the user role, the icon update is triggered by this change
    if (item->data(Qt::UserRole) == QVariant(false))
      {
      item->setData(Qt::UserRole, QVariant(true));
      }
    else
      {
      item->setData(Qt::UserRole, QVariant(false));
      }
    }

}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onRightClickActiveMarkupTableWidget(QPoint pos)
{
  Q_UNUSED(pos);

  // qDebug() << "onRightClickActiveMarkupTableWidget: pos = " << pos;

  QMenu menu;
  QMenu *moveMenu = menu.addMenu(QString("Move fiducial to another list"));

  QAction *moveToSameIndexInListAction =
    new QAction(QString("Move fiducial to same index in another list"), moveMenu);
  QAction *moveToTopOfListAction =
    new QAction(QString("Move fiducial to top of another list"), moveMenu);
  QAction *moveToBottomOfListAction =
    new QAction(QString("Move fiducial to bottom of another list"), moveMenu);

  moveMenu->addAction(moveToSameIndexInListAction);
  moveMenu->addAction(moveToTopOfListAction);
  moveMenu->addAction(moveToBottomOfListAction);

  QSignalMapper *signalMapper = new QSignalMapper(this);
  QObject::connect(moveToSameIndexInListAction, SIGNAL(triggered()),
          signalMapper, SLOT(map()));
  QObject::connect(moveToTopOfListAction, SIGNAL(triggered()),
          signalMapper, SLOT(map()));
  QObject::connect(moveToBottomOfListAction, SIGNAL(triggered()),
          signalMapper, SLOT(map()));
  signalMapper->setMapping(moveToSameIndexInListAction, QString("Same"));
  signalMapper->setMapping(moveToTopOfListAction, QString("Top"));
  signalMapper->setMapping(moveToBottomOfListAction, QString("Bottom"));
  QObject::connect(signalMapper, SIGNAL(mapped(QString)),
                   this, SLOT(onMoveToOtherListActionTriggered(QString)));

  menu.exec(QCursor::pos());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMoveToOtherListActionTriggered(QString destinationPosition)
{
  Q_D(qSlicerMarkupsModuleWidget);

  if (this->mrmlScene() == 0)
    {
    return;
    }

  // qDebug() << "onMoveToOtherListActionTriggered: " << destinationPosition;

  // sanity check: is there another list to move to?
  int numNodes = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLMarkupsNode");
  if (numNodes < 2)
    {
    qWarning() << "No other list to move it to! Define another list first.";
    return;
    }

  // get the active list
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  if (!mrmlNode)
    {
    return;
    }
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  if (!markupsNode)
    {
    return;
    }
  // get the target list
  QStringList otherLists;
  for (int n = 0; n < numNodes; n++)
    {
    vtkMRMLNode *listNodeN = this->mrmlScene()->GetNthNodeByClass(n, "vtkMRMLMarkupsNode");
    if (strcmp(listNodeN->GetID(), markupsNode->GetID()) != 0)
      {
      otherLists.append(QString(listNodeN->GetName()));
      }
    }

  // make a dialog with the other lists to select
  QInputDialog listDialog;
  listDialog.setWindowTitle("Pick destination list");
  listDialog.setLabelText("Destination list:");
  listDialog.setComboBoxItems(otherLists);
  listDialog.setInputMode(QInputDialog::TextInput);
  // hack: set the object name and get it in the slot
  listDialog.setObjectName(destinationPosition);
  QObject::connect(&listDialog, SIGNAL(textValueSelected(const QString &)),
                   this,SLOT(moveSelectedToNamedList(const QString &)));
  listDialog.exec();
}

//-----------------------------------------------------------------------------
  void qSlicerMarkupsModuleWidget::moveSelectedToNamedList(QString listName)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // qDebug() << "moveSelectedToNamedList: " << listName;

  QString destinationPosition = QString("Same");
  if (sender() != 0)
    {
    destinationPosition = sender()->objectName();
    }
  // qDebug() << "\tdestinationPosition: " << destinationPosition;

  // get the selected point
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();
  int rowNumber = selectedItems.at(0)->row();
  if (selectedItems.size() / d->numberOfColumns() > 1)
    {
    QMessageBox msgBox;
    msgBox.setText(QString("Move is only implemented for one row."));
    msgBox.setInformativeText(QString("Click Ok to move single markup from row ") + QString::number(rowNumber));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    int ret = msgBox.exec();
    if (ret != QMessageBox::Ok)
      {
      // bail out
      return;
      }
    }

  if (!this->markupsLogic())
    {
    qWarning() << "No markups logic class, unable to move markup";
    return;
    }

  // get the active list
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  if (!mrmlNode)
    {
    return;
    }
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  if (!markupsNode)
    {
    return;
    }
  // get the new list
  vtkMRMLNode *newNode = this->mrmlScene()->GetFirstNodeByName(listName.toLatin1());
  if (!newNode)
    {
    qWarning() << "Unable to find list named " << listName << " in scene!";
    return;
    }
  vtkMRMLMarkupsNode *newMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(newNode);

  // calculate the index based on the destination position
  int newIndex = rowNumber;
  if (destinationPosition == QString("Top"))
    {
    newIndex = 0;
    }
  else if (destinationPosition == QString("Bottom"))
    {
    newIndex = newMarkupsNode->GetNumberOfMarkups();
    }

  // and move
  bool retval = this->markupsLogic()->MoveNthMarkupToNewListAtIndex(rowNumber, markupsNode, newMarkupsNode, newIndex);
  if (!retval)
    {
    qWarning() << "Failed to move " << rowNumber << " markup to list named " << listName;
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::observeMarkupsNode(vtkMRMLNode *markupsNode)
{
  if (this->mrmlScene())
    {
    // remove all connections
    int numNodes = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLMarkupsNode");
    // qDebug() << "observeMarkupsNode: have " << numNodes << " markups nodes";
    for (int i = 0; i < numNodes; i++)
      {
      vtkMRMLNode *node = this->mrmlScene()->GetNthNodeByClass(i, "vtkMRMLMarkupsNode");
      if (node)
        {
        if (markupsNode)
          {
          // is this the markups node?
          if (node->GetID() && markupsNode->GetID() && strcmp(node->GetID(), markupsNode->GetID()) == 0)
            {
            // don't disconnect
            // qDebug() << "\tskipping disconnecting " << node->GetID();
            continue;
            }
          }
        // qDebug() << "\tdisconnecting " << node->GetID();
        this->qvtkDisconnect(node, vtkMRMLMarkupsNode::LockModifiedEvent,
                             this, SLOT(onActiveMarkupsNodeLockModifiedEvent()));
        this->qvtkDisconnect(node, vtkMRMLMarkupsNode::PointModifiedEvent,
                             this, SLOT(onActiveMarkupsNodePointModifiedEvent(vtkObject*,vtkObject*)));
        this->qvtkDisconnect(node, vtkMRMLMarkupsNode::NthMarkupModifiedEvent,
                             this, SLOT(onActiveMarkupsNodeNthMarkupModifiedEvent(vtkObject*,vtkObject*)));
        this->qvtkDisconnect(node, vtkMRMLMarkupsNode::MarkupAddedEvent,
                             this, SLOT(onActiveMarkupsNodeMarkupAddedEvent()));
        }
      }
    }
  else
    {
    qWarning() << "observeMarkupsNode: no scene";
    }
  if (markupsNode)
    {
    // is the node already connected?
    if (this->qvtkIsConnected(markupsNode, vtkMRMLMarkupsNode::LockModifiedEvent,
                                     this, SLOT(onActiveMarkupsNodeLockModifiedEvent())))
      {
      // qDebug() << "\tmarkups node is already connected: " << markupsNode->GetID();
      }
    else
      {
      // add connections for this node
      this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::LockModifiedEvent,
                        this, SLOT(onActiveMarkupsNodeLockModifiedEvent()));
      this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::PointModifiedEvent,
                        this, SLOT(onActiveMarkupsNodePointModifiedEvent(vtkObject*,vtkObject*)));
      this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::NthMarkupModifiedEvent,
                        this, SLOT(onActiveMarkupsNodeNthMarkupModifiedEvent(vtkObject*,vtkObject*)));
      this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::MarkupAddedEvent,
                        this, SLOT(onActiveMarkupsNodeMarkupAddedEvent()));
      this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::MarkupRemovedEvent,
                        this, SLOT(onActiveMarkupsNodeMarkupRemovedEvent()));
      // qDebug() << "\tconnected markups node " << markupsNode->GetID();
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::clearGUI()
{
  Q_D(qSlicerMarkupsModuleWidget);

  d->activeMarkupTableWidget->clearContents();
  d->activeMarkupTableWidget->setRowCount(0);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeLockModifiedEvent()//vtkMRMLNode *markupsNode)
{
  Q_D(qSlicerMarkupsModuleWidget);

  QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeID();
  QString nodeID;
  //if (markupsNode)
    {
    //nodeID = QString(markupsNode->GetID());
    }
    //qDebug() << QString("onActiveMarkupsNodeLockModifiedEvent, passed node id = ") +  nodeID + QString(", active markups node id = ") + activeMarkupsNodeID;
  if (activeMarkupsNodeID.compare(nodeID) != 0)
    {
    //qDebug() << "Got event from non active node " + nodeID + ", active id = " + activeMarkupsNodeID;
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeNthMarkupModifiedEvent(vtkObject *caller, vtkObject *callData)
{
  //qDebug() << "onActiveMarkupsNodeNthMarkupModifiedEvent\n";

  // the call data should be the index n
  if (caller == NULL || callData == NULL)
    {
    return;
    }

  int *nPtr = NULL;
  int n = -1;
  nPtr = reinterpret_cast<int *>(callData);
  if (nPtr)
    {
    n = *nPtr;
    }
  this->UpdateRow(n);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodePointModifiedEvent(vtkObject *caller, vtkObject *callData)
{
  //qDebug() << "onActiveMarkupsNodePointModifiedEvent";

  // the call data should be the index n
  if (caller == NULL || callData == NULL)
    {
    return;
    }
  // qDebug() << "\tcaller class = " << caller->GetClassName();
  int *nPtr = NULL;
  int n = -1;
  nPtr = reinterpret_cast<int *>(callData);
  if (nPtr)
    {
    n = *nPtr;
    }
  // qDebug() << "\tn = " << QString::number(n);
  this->UpdateRow(n);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeMarkupAddedEvent()//vtkMRMLNode *markupsNode)
{
  Q_D(qSlicerMarkupsModuleWidget);

  //qDebug() << "onActiveMarkupsNodeMarkupAddedEvent";

  QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeID();

  //qDebug() << QString("active markups node id from combo box = ") + activeMarkupsNodeID;

  int newRow = d->activeMarkupTableWidget->rowCount();
  //qDebug() << QString("\tnew row / row count = ") + QString::number(newRow);
  d->activeMarkupTableWidget->insertRow(newRow);
  //qDebug() << QString("\t after insreting a row, row count = ") + QString::number(d->activeMarkupTableWidget->rowCount());

  this->UpdateRow(newRow);

  // scroll to the new row
  d->activeMarkupTableWidget->setCurrentCell(newRow, 0);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeMarkupRemovedEvent()//vtkMRMLNode *markupsNode)
{
  // do a general update
  this->UpdateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::toQColor(const double* color, QColor &qcolor)
{
  if (color)
    {
    qcolor = QColor::fromRgbF(color[0], color[1], color[2]);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::toColor(const QColor &qcolor, double* color)
{
  color[0] = qcolor.redF();
  color[1] = qcolor.greenF();
  color[2] = qcolor.blueF();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onNewMarkupWithCurrentDisplayPropertiesTriggered()
{
  Q_D(qSlicerMarkupsModuleWidget);

//  qDebug() << "onNewMarkupWithCurrentDisplayPropertiesTriggered";

  // get the active list
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  if (!mrmlNode)
    {
    // if there's no currently active markups list, trigger the default add
    // node
    d->activeMarkupMRMLNodeComboBox->addNode();
    return;
    }

  // otherwise make a new one of the same class
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  if (!markupsNode)
    {
    qDebug() << "Unable to get the currently active markups node";
    return;
    }

  // get the display node
  vtkMRMLDisplayNode *displayNode = markupsNode->GetDisplayNode();
  if (!displayNode)
    {
    qDebug() << "Unable to get the display node on the markups node";
    }

  // create a new one
  vtkMRMLNode *newDisplayNode = this->mrmlScene()->CreateNodeByClass(displayNode->GetClassName());
  // copy the old one
  newDisplayNode->Copy(displayNode);
  /// add to the scene
  this->mrmlScene()->AddNode(newDisplayNode);

  // now create the new markups node
  const char *className = markupsNode->GetClassName();
  vtkMRMLNode *newMRMLNode = this->mrmlScene()->CreateNodeByClass(className);
  // copy the name and let them rename it
  newMRMLNode->SetName(markupsNode->GetName());
  this->mrmlScene()->AddNode(newMRMLNode);
  // and observe the copied display node
  vtkMRMLDisplayableNode *newDisplayableNode = vtkMRMLDisplayableNode::SafeDownCast(newMRMLNode);
  newDisplayableNode->SetAndObserveDisplayNodeID(newDisplayNode->GetID());

  // set it active
  d->activeMarkupMRMLNodeComboBox->setCurrentNode(newMRMLNode->GetID());
  // let the user rename it
  d->activeMarkupMRMLNodeComboBox->renameCurrentNode();

  // update the display properties manually since the display node wasn't
  // observed when it was added
  this->UpdateWidgetFromMRML();

  // clean up
  newDisplayNode->Delete();
  newMRMLNode->Delete();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::updateLogicFromSettings()
{
  // update logic from settings
  if (this->logic() == NULL)
    {
    qWarning() << "updateLogicFromSettings: no logic to set";
    return;
    }

  if (!qSlicerApplication::application() ||
      !qSlicerApplication::application()->settingsDialog())
    {
    qWarning() << "updateLogicFromSettings: null application or settings dialog";
    return;
    }

  QSettings *settings = qSlicerApplication::application()->settingsDialog()->settings();
  if (!settings)
    {
    qWarning() << "updateLogicFromSettings: null settings";
    return;
    }

  // have settings been saved before?
  if (!settings->contains("Markups/GlyphType") ||
      !settings->contains("Markups/SelectedColor") ||
      !settings->contains("Markups/UnselectedColor") ||
      !settings->contains("Markups/GlyphScale") ||
      !settings->contains("Markups/TextScale") ||
      !settings->contains("Markups/Opacity"))
    {
    qDebug() << "Markups: display settings not saved yet, using defaults";
    return;
    }

  QString glyphType = settings->value("Markups/GlyphType").toString();
  QColor qcolor;
  QVariant variant = settings->value("Markups/SelectedColor");
  qcolor = variant.value<QColor>();
  double selectedColor[3];
  qSlicerMarkupsModuleWidget::toColor(qcolor, selectedColor);
  variant = settings->value("Markups/UnselectedColor");
  QColor qcolorUnsel = variant.value<QColor>();
  double unselectedColor[3];
  qSlicerMarkupsModuleWidget::toColor(qcolorUnsel, unselectedColor);
  double glyphScale = settings->value("Markups/GlyphScale").toDouble();
  double textScale = settings->value("Markups/TextScale").toDouble();
  double opacity = settings->value("Markups/Opacity").toDouble();

  /*
  qDebug() << "updateLogicFromSettings:";
  qDebug() << "Settings glyph type = " << glyphType;
  qDebug() << "Settings selected color = " << qcolor;
  qDebug() << "Settings unselected color = " << qcolorUnsel;
  qDebug() << "Settings glyph scale = " << glyphScale;
  qDebug() << "Settings text scale = " << textScale;
  qDebug() << "Settings opacity = " << opacity;
  qDebug() << "Copying default settings to logic";
  */
  if (!this->markupsLogic())
    {
    qWarning() << "Unable to get markups logic";
    return;
    }
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeGlyphTypeFromString(glyphType.toLatin1());
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeGlyphScale(glyphScale);
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeTextScale(textScale);
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeSelectedColor(selectedColor);
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeColor(unselectedColor);
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeOpacity(opacity);
}
