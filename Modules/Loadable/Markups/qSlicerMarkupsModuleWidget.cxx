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
#include <QStringList>
#include <QTableWidgetItem>

// SlicerQt includes
#include "qSlicerMarkupsModuleWidget.h"
#include "ui_qSlicerMarkupsModule.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"

// module includes
#include "vtkMRMLMarkupsNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkSlicerMarkupsLogic.h"

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
};

//-----------------------------------------------------------------------------
// qSlicerMarkupsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidgetPrivate::qSlicerMarkupsModuleWidgetPrivate(qSlicerMarkupsModuleWidget& object)
  : q_ptr(&object)
{
  this->columnLabels << "Selected" << "Visible" << "Name" << "X" << "Y" << "Z";// << "Description";
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
  
  std::cout << "setupUI\n";

  // set up the active markups node selector
  QObject::connect(this->activeMarkupMRMLNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onActiveMarkupMRMLNodeChanged(vtkMRMLNode*)));

  vtkMRMLNode *selectionNode = NULL;
  if (q->mrmlScene())
    {
    selectionNode = q->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton");
    }
  else
    {
    qDebug() << "No mrml scene set! q = " << q;
    }
  if (selectionNode)
    {
    q->qvtkConnect(selectionNode, vtkMRMLSelectionNode::ActiveMarkupsIDChangedEvent,
                   q, SLOT(onSelectionNodeActiveMarkupsIDChanged()));
    }
  else
    {
    qDebug() << "No selection node found in scene!";
    }

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

  // listen for changes so can update mrml node
  QObject::connect(this->activeMarkupTableWidget, SIGNAL(cellChanged(int, int)),
                   q, SLOT(onActiveMarkupTableCellChanged(int, int)));

  // listen for click on a markup
  QObject::connect(this->activeMarkupTableWidget, SIGNAL(itemClicked(QTableWidgetItem*)),
                   q, SLOT(onActiveMarkupTableCellClicked(QTableWidgetItem*)));
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
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::enter()
{
  this->Superclass::enter();

  qDebug() << "enter widget";
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::NodeAddedEvent,
                 this, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
  
  this->UpdateWidgetFromMRML();
  
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::exit()
{
  this->Superclass::exit();

  qDebug() << "exit widget";
  this->qvtkDisconnect(this->mrmlScene(), vtkMRMLScene::NodeAddedEvent,
                 this, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::UpdateWidgetFromMRML()
{
  Q_D(qSlicerMarkupsModuleWidget);
  
  std::cout << "UpdateWidgetFromMRML" << std::endl;

  // update the combo box
  this->onSelectionNodeActiveMarkupsIDChanged();

  // get the active markup
  vtkMRMLNode *node = this->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton");
  vtkMRMLSelectionNode *selectionNode = NULL;
  if (node)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(node);
    }
  vtkMRMLNode *markupsNodeMRML = NULL;
  if (selectionNode)
    {
    markupsNodeMRML = this->mrmlScene()->GetNodeByID(selectionNode->GetActiveMarkupsID());
    }
  vtkMRMLMarkupsNode *markupsNode = NULL;
  if (markupsNodeMRML)
    {
    markupsNode = vtkMRMLMarkupsNode::SafeDownCast(markupsNodeMRML);
    }

  if (!markupsNode)
    {
    qDebug() << "UpdateWidgetFromMRML: Unable to get active markups node, clearing out the table";
    d->activeMarkupTableWidget->clearContents();
    d->activeMarkupTableWidget->setRowCount(0);
    return;
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
  
  qDebug() << QString("UpdateRow: row = ") + QString::number(m) + QString(", number of rows = ") + QString::number(d->activeMarkupTableWidget->rowCount());
  // get active markups node
  QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeId();
  vtkMRMLNode *mrmlNode = this->mrmlScene()->GetNodeByID(activeMarkupsNodeID.toLatin1());
  vtkMRMLMarkupsNode *markupsNode = NULL;
  if (mrmlNode)
    {
    markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (!markupsNode)
    {
    qDebug() << QString("Update Row: unable to get markups node with id ") + activeMarkupsNodeID;
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
  if (d->activeMarkupTableWidget->item(m,0) == NULL ||
      (d->activeMarkupTableWidget->item(m,0)->checkState() != selectedItem->checkState()))
    {
    d->activeMarkupTableWidget->setItem(m,0,selectedItem);
    }
  
  // visible
  QTableWidgetItem* visibleItem = new QTableWidgetItem();
  if (markupsNode->GetNthMarkupVisibility(m))
    {
    visibleItem->setCheckState(Qt::Checked);
    }
  else
    {
    visibleItem->setCheckState(Qt::Unchecked);
    }
   if (d->activeMarkupTableWidget->item(m,1) == NULL ||
      (d->activeMarkupTableWidget->item(m,1)->checkState() != visibleItem->checkState()))
     {
     d->activeMarkupTableWidget->setItem(m,1,visibleItem);
     }
   
  // name
  QString markupLabel = QString(markupsNode->GetNthMarkupLabel(m).c_str());
  if (d->activeMarkupTableWidget->item(m,2) == NULL ||
      d->activeMarkupTableWidget->item(m,2)->text() != markupLabel)
    {
    d->activeMarkupTableWidget->setItem(m,2,new QTableWidgetItem(markupLabel));
    }
  
  // point
  double point[3];
  markupsNode->GetMarkupPoint(m, 0, point);
  //    coordinate = QString::number(point[0]) + QString(", ") + QString::number(point[1]) + QString(", ") + QString::number(point[2]); 
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
    std::cout << "onNodeAddedEvent" << markupsNode->GetID() << std::endl;
    d->activeMarkupMRMLNodeComboBox->setCurrentNode(markupsNode->GetID());
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onNodeRemovedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerMarkupsModuleWidget);

  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (markupsNode)
    {
    // is it the current one? clear out the table
    QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeId();
    std::cout << "onNodeRemovedEvent" << markupsNode->GetID() << ", node combo box = " << qPrintable(activeMarkupsNodeID) << std::endl;
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupMRMLNodeChanged(vtkMRMLNode *markupsNode)
{
  Q_D(qSlicerMarkupsModuleWidget);

  qDebug() << "onActiveMarkupMRMLNodeChanged, markupsNode is " << (markupsNode ? markupsNode->GetID() : "null");
  
  // get the current node from the combo box
  QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeId();
  const char *activeID = NULL;
  if (markupsNode)
    {
    activeID = markupsNode->GetID();
    // make sure get updates from the active markups node
    this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::LockModifiedEvent,
                   this, SLOT(onActiveMarkupsNodeLockModifiedEvent()));
    this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::PointModifiedEvent,
                   this, SLOT(onActiveMarkupsNodePointModifiedEvent(vtkObject*,vtkObject*)));
    this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::NthMarkupModifiedEvent,
                      this, SLOT(onActiveMarkupsNodeNthMarkupModifiedEvent(vtkObject*,vtkObject*)));
    this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::MarkupAddedEvent,
                   this, SLOT(onActiveMarkupsNodeMarkupAddedEvent()));
    qDebug() << "onActiveMarkupMRMLNodeChanged: set up observations on markups node " << activeID;
    }
  
  qDebug() << "setActiveMarkupsNode: combo box says: " << qPrintable(activeMarkupsNodeID) << ", input node says " << (activeID ? activeID : "null");
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
    const char *selectionNodeActiveMarkupsID = selectionNode->GetActiveMarkupsID();
    if (!selectionNodeActiveMarkupsID || !activeID ||
        strcmp(selectionNodeActiveMarkupsID, activeID) != 0)
      {
      selectionNode->SetReferenceActiveMarkupsID(activeID);
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
void qSlicerMarkupsModuleWidget::onSelectionNodeActiveMarkupsIDChanged()
{
  Q_D(qSlicerMarkupsModuleWidget);

  qDebug() << "\n\n******\nonSelectionNodeActiveMarkupsIDChanged\n";
  
  // get the selection node
  vtkMRMLNode *node = this->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton");
  vtkMRMLSelectionNode *selectionNode = NULL;
  if (node)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(node);
    }
  if (selectionNode)
    {
    std::cout << "\tsetting current node to " << selectionNode->GetActiveMarkupsID() << std::endl;
    d->activeMarkupMRMLNodeComboBox->setCurrentNode(selectionNode->GetActiveMarkupsID());
    }
  else
    {
    qDebug() << "Unable to update combo box from selection node";
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
  else if (column == d->columnIndex("Visible"))
    {
    bool flag = (item->checkState() == Qt::Unchecked ? false : true);
    listNode->SetNthMarkupVisibility(n, flag);
    }
  else if (column ==  d->columnIndex("Name"))
    {
    std::string name = std::string(item->text().toLatin1());
    listNode->SetNthMarkupLabel(n, name);
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

  if (item->column() == d->columnIndex(QString("Name")))
    {
    std::cout << "onActiveMarkupTableCellClicked: Name column" << std::endl;
    if (0)
      {
      // get the coordinates from the table
      double x, y, z = 0.0;
      int row = item->row();
      x = d->activeMarkupTableWidget->item(row, d->columnIndex("X"))->text().toDouble();
      y = d->activeMarkupTableWidget->item(row, d->columnIndex("Y"))->text().toDouble();
      z = d->activeMarkupTableWidget->item(row, d->columnIndex("Z"))->text().toDouble();
      // jump to it
      if (this->logic() != NULL &&
          vtkSlicerMarkupsLogic::SafeDownCast(this->logic()) != NULL)
        {
        vtkSlicerMarkupsLogic::SafeDownCast(this->logic())->JumpSlicesToLocation(x, y, z);
        }
      }
    else
      {
      // use the node id + row index
       // get the active list
      vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
      if (!mrmlNode)
        {
        return;
        }
      int row = item->row();
      // jump to it
      if (this->logic() != NULL &&
          vtkSlicerMarkupsLogic::SafeDownCast(this->logic()) != NULL)
        {
        vtkSlicerMarkupsLogic::SafeDownCast(this->logic())->JumpSlicesToNthPointInMarkup(mrmlNode->GetID(), row);
        }
      }
    } 

}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeLockModifiedEvent()//vtkMRMLNode *markupsNode)
{
  Q_D(qSlicerMarkupsModuleWidget);

  QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeId();
  QString nodeID;
  //if (markupsNode)
    {
    //nodeID = QString(markupsNode->GetID());
    }
  qDebug() << QString("onActiveMarkupsNodeLockModifiedEvent, passed node id = ") +  nodeID + QString(", active markups node id = ") + activeMarkupsNodeID;
  if (activeMarkupsNodeID.compare(nodeID) != 0)
    {
    qDebug() << "Got event from non active node " + nodeID + ", active id = " + activeMarkupsNodeID;
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeNthMarkupModifiedEvent(vtkObject *caller, vtkObject *callData)
{
  qDebug() << "onActiveMarkupsNodeNthMarkupModifiedEvent\n";

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
  qDebug() << "onActiveMarkupsNodePointModifiedEvent";

  // the call data should be the index n
  if (caller == NULL || callData == NULL)
    {
    return;
    }
  qDebug() << "\tcaller class = " << caller->GetClassName();
  int *nPtr = NULL;
  int n = -1;
  nPtr = reinterpret_cast<int *>(callData);
  if (nPtr)
    {
    n = *nPtr;
    }
  qDebug() << "\tn = " << QString::number(n);
  this->UpdateRow(n);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeMarkupAddedEvent()//vtkMRMLNode *markupsNode)
{
  Q_D(qSlicerMarkupsModuleWidget);

  qDebug() << "onActiveMarkupsNodeMarkupAddedEvent";
  
  QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeId();

  qDebug() << QString("active markups node id from combo box = ") + activeMarkupsNodeID;

  int newRow = d->activeMarkupTableWidget->rowCount();
  qDebug() << QString("\tnew row / row count = ") + QString::number(newRow);
  d->activeMarkupTableWidget->insertRow(newRow);
  qDebug() << QString("\t after insreting a row, row count = ") + QString::number(d->activeMarkupTableWidget->rowCount());
  this->UpdateRow(newRow);

  // scroll to the new row
  d->activeMarkupTableWidget->setCurrentCell(newRow, 0);
}
