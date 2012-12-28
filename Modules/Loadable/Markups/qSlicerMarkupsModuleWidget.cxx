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
  this->columnLabels << " " << "Visible" << "Name" << "X" << "Y" << "Z";// << "Description";
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
  // set up the table
  //
  
  // number of columns with headers
  this->activeMarkupTableWidget->setColumnCount(this->numberOfColumns());
  this->activeMarkupTableWidget->setHorizontalHeaderLabels(this->columnLabels);
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
  d->activeMarkupTableWidget->setItem(m,0,selectedItem);
  
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
  d->activeMarkupTableWidget->setItem(m,1,visibleItem);
  
  // name
  QString markupLabel = QString(markupsNode->GetNthMarkupLabel(m).c_str());
  d->activeMarkupTableWidget->setItem(m,2,new QTableWidgetItem(markupLabel));
  
  // point
  double point[3];
  markupsNode->GetMarkupPoint(m, 0, point);
  //    coordinate = QString::number(point[0]) + QString(", ") + QString::number(point[1]) + QString(", ") + QString::number(point[2]); 
  int xColumnIndex = d->columnIndex("X");
  for (int p = 0; p < 3; p++)
    {
    // last argument to number sets the precision
    QString coordinate = QString::number(point[p], 'f', 6);
    d->activeMarkupTableWidget->setItem(m,xColumnIndex + p,new QTableWidgetItem(coordinate));
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
}
