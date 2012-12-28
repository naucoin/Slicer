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
  this->UpdateWidgetFromMRML();
  
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
  
  std::cout << "UpdateRow" << std::endl;
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
    qDebug() << "Update Row: unable to get markups node with id " + activeMarkupsNodeID;
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
                   this, SLOT(onActiveMarkupsNodePointModifiedEvent()));
    this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::NthMarkupModifiedEvent,
                      this, SLOT(onActiveMarkupsNodeNthMarkupModifiedEvent()));
    this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::MarkupAddedEvent,
                   this, SLOT(onActiveMarkupsNodeMarkupAddedEvent()));
    qDebug() << "onActiveMarkupMRMLNodeChanged: set up observations on markus node " << activeID;
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
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeNthMarkupModifiedEvent()
{
  Q_D(qSlicerMarkupsModuleWidget);

  qDebug() << "onActiveMarkupsNodeNthMarkupModifiedEvent\n";
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodePointModifiedEvent()//vtkMRMLNode *markupsNode)
{
  Q_D(qSlicerMarkupsModuleWidget);

  qDebug() << "onActiveMarkupsNodePointModifiedEvent\n";
  std::cout << "onActiveMarkupsNodePointModifiedEvent" << std::endl;
  
  QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeId();
  QString nodeID;
  //if (markupsNode)
    {
    //nodeID = QString(markupsNode->GetID());
    }
  qDebug() << QString("onActiveMarkupsNodePointModifiedEvent, passed node id = ") +  nodeID + QString(", active markups node id = ") + activeMarkupsNodeID;
  if (activeMarkupsNodeID.compare(nodeID) != 0)
    {
    qDebug() << "Got point modified event from non active node " + nodeID + ", active id = " + activeMarkupsNodeID;
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeMarkupAddedEvent()//vtkMRMLNode *markupsNode)
{
  Q_D(qSlicerMarkupsModuleWidget);

  qDebug() << "onActiveMarkupsNodeMarkupAddedEvent\n";
  
  QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeId();
  QString nodeID;
  //if (markupsNode)
    {
    //nodeID = QString(markupsNode->GetID());
    }
  qDebug() << QString("onActiveMarkupsNodeMarkupAddedEvent, passed node id = ") +  nodeID + QString(", active markups node id = ") + activeMarkupsNodeID;
  if (activeMarkupsNodeID.compare(nodeID) != 0)
    {
    qDebug() << "Got markup added event from non active node " + nodeID + ", active id = " + activeMarkupsNodeID;
    }

  int newRow = d->activeMarkupTableWidget->rowCount();
  d->activeMarkupTableWidget->insertRow(newRow+1);
  this->UpdateRow(newRow);
}
