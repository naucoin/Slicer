/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QToolButton>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"
#include "qMRMLSliceWidget.h"
#include "qSlicerMarkupsToolBar_p.h"

// SlicerLogic includes
#include <vtkSlicerApplicationLogic.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceNode.h>

//---------------------------------------------------------------------------
// qSlicerMarkupsToolBarPrivate methods

//---------------------------------------------------------------------------
qSlicerMarkupsToolBarPrivate::qSlicerMarkupsToolBarPrivate(qSlicerMarkupsToolBar& object)
  : q_ptr(&object)
{
  this->CreateAndPlaceToolButton = 0;
  this->CreateAndPlaceMenu = 0;

  this->PersistenceAction = 0;

  this->ActionGroup = 0;
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBarPrivate::init()
{
  Q_Q(qSlicerMarkupsToolBar);

  this->ActionGroup = new QActionGroup(q);
  this->ActionGroup->setExclusive(true);

  // new actions will be added when interaction modes are registered with the
  // scene

  // persistence
  this->PersistenceAction = new QAction(q);
  this->PersistenceAction->setText(QObject::tr("Persistent"));
  this->PersistenceAction->setToolTip(QObject::tr("Switch between single place and persistent place modes."));
  this->PersistenceAction->setCheckable(true);
  this->PersistenceAction->setChecked(false);
  vtkMRMLInteractionNode * interactionNode = NULL;
  if (this->MRMLAppLogic)
    {
    interactionNode = this->MRMLAppLogic->GetInteractionNode();
    }
  if (interactionNode)
    {
    if (interactionNode->GetPlaceModePersistence())
      {
      this->PersistenceAction->setChecked(true);
      }
    }
  connect(this->PersistenceAction, SIGNAL(triggered()), q,
                SLOT(onPersistenceToggled()));
  
  // popuplate the create and place menu, with persistence first
  this->CreateAndPlaceMenu = new QMenu(QObject::tr("Create and Place"), q);
  this->CreateAndPlaceMenu->setObjectName("CreateAndPlaceMenu");
  this->CreateAndPlaceMenu->addAction(this->PersistenceAction);
  this->CreateAndPlaceMenu->addSeparator();
  this->CreateAndPlaceMenu->addActions(this->ActionGroup->actions());
  this->CreateAndPlaceMenu->addSeparator();
  

  this->CreateAndPlaceToolButton = new QToolButton();
  this->CreateAndPlaceToolButton->setObjectName("CreateAndPlaceToolButton");
  this->CreateAndPlaceToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  this->CreateAndPlaceToolButton->setToolTip(QObject::tr("Create and Place"));
  this->CreateAndPlaceToolButton->setText(QObject::tr("Place"));
  this->CreateAndPlaceToolButton->setMenu(this->CreateAndPlaceMenu);
  this->CreateAndPlaceToolButton->setPopupMode(QToolButton::MenuButtonPopup);
  
  // set default action?


  q->addWidget(this->CreateAndPlaceToolButton);  
  
  QObject::connect(q, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   this->CreateAndPlaceToolButton,
                   SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));

}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBarPrivate::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_Q(qSlicerMarkupsToolBar);

  if (newScene == this->MRMLScene)
    {
    return;
    }

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::StartCloseEvent,
                      this, SLOT(OnMRMLSceneStartClose()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::EndImportEvent,
                      this, SLOT(OnMRMLSceneEndImport()));

  this->qvtkReconnect(this->MRMLScene, newScene, vtkMRMLScene::EndCloseEvent,
                      this, SLOT(OnMRMLSceneEndClose()));

  this->MRMLScene = newScene;

  // watch for changes to the interaction, selection nodes so can update the widget
  if (this->MRMLScene && this->MRMLAppLogic)
    {
    vtkMRMLInteractionNode * interactionNode = this->MRMLAppLogic->GetInteractionNode();
    if (interactionNode)
      {
      this->qvtkReconnect(interactionNode, vtkMRMLInteractionNode::InteractionModeChangedEvent,
                          this, SLOT(onInteractionNodeModeChangedEvent()));
      this->qvtkReconnect(interactionNode, vtkMRMLInteractionNode::InteractionModePersistenceChangedEvent,
                          this, SLOT(onInteractionNodeModePersistenceChanged()));
      }
    vtkMRMLSelectionNode *selectionNode = this->MRMLAppLogic->GetSelectionNode();
    if (selectionNode)
      {
      this->qvtkReconnect(selectionNode, vtkMRMLSelectionNode::ActiveMarkupsNodeTypeChangedEvent,
                          this, SLOT(onActiveMarkupsNodeTypeChangedEvent()));
      this->qvtkReconnect(selectionNode, vtkMRMLSelectionNode::MarkupsIDListModifiedEvent,
                          this, SLOT(onMarkupsIDListModifiedEvent()));
      }
    }
  // Update UI
  q->setEnabled(this->MRMLScene != 0);
  if (this->MRMLScene)
    {
    this->updateWidgetFromMRML();
    this->updateWidgetFromSelectionNode();
    }
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBarPrivate::updateWidgetFromMRML()
{
  if (!this->MRMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar isn't set up with application logic";
    return;
    }
  this->onInteractionNodeModePersistenceChanged();

  vtkMRMLInteractionNode * interactionNode = this->MRMLAppLogic->GetInteractionNode();

  int currentMouseMode = interactionNode->GetCurrentInteractionMode();
  switch (currentMouseMode)
    {
    case vtkMRMLInteractionNode::Place:
      {
      // find the active markups id and set it's corresponding action to be checked
      vtkMRMLSelectionNode *selectionNode = this->MRMLAppLogic->GetSelectionNode();
      if ( selectionNode )
        {
        const char *activeMarkupsNodeType = selectionNode->GetActiveMarkupsNodeType();
        this->updateWidgetToMarkups(activeMarkupsNodeType);
        }
      }
      break;
    case vtkMRMLInteractionNode::ViewTransform:
      // reset the widget to view transform, not supporting pick manipulate
      this->updateWidgetToMarkups(0);
      break;
    default:
      qWarning() << "qSlicerMarkupsToolBarPrivate::updateWidgetFromMRML - "
                    "unhandled MouseMode:" << currentMouseMode;
      break;
    }
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBarPrivate::updateWidgetFromSelectionNode()
{
  Q_Q(qSlicerMarkupsToolBar);
  if (!this->MRMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }
  // get the currently active markups
  vtkMRMLSelectionNode *selectionNode = this->MRMLAppLogic->GetSelectionNode();
  if (!selectionNode)
    {
    return;
    }
  
  // make sure that all the elements in the selection node have actions in the
  // create and place menu
  int numIDs = selectionNode->GetNumberOfMarkupsIDsInList();

  // if some were removed, clear out those actions first
  QList<QAction*> actionList = this->CreateAndPlaceMenu->actions();
  int numActions = actionList.size();
  if (numIDs < numActions)
    {
    // iterate over the action list and remove ones that aren't in the
    // selection node
    for (int i = 0; i < actionList.size(); ++i)
      {
      QAction *action = actionList.at(i);
      QString actionText = action->text();
      // don't remove transform view or persistent or spacers
      if (actionText.compare("&Rotate") != 0 &&
          actionText.compare(QObject::tr("Persistent")) != 0 &&
          !actionText.isEmpty())
        {
        actionText = actionText.prepend(QString("vtkMRMLMarkups"));
        actionText = actionText.append(QString("Node"));
        const char *thisAnnotID = actionText.toLatin1();
        if (selectionNode->MarkupsIDInList(thisAnnotID) == -1)
          {
          this->ActionGroup->removeAction(action);
          this->CreateAndPlaceMenu->removeAction(action);
          }
        }
      }
    // update the tool button from the updated action list
    actionList = this->CreateAndPlaceMenu->actions();
    }
//  qDebug() << "\tnumIDs = " << numIDs;
  for (int i = 0; i < numIDs; i++)
    {
    QString markupsID = QString(selectionNode->GetMarkupsIDByIndex(i).c_str());
    QString markupsResource = QString(selectionNode->GetMarkupsResourceByIndex(i).c_str());
    QString markupsName = markupsID;
    markupsName = markupsName.remove(QString("vtkMRMLMarkups"));
    markupsName = markupsName.remove(QString("Node"));
//    qDebug() << "\t" << i << ", id = " << markupsID << ", resource = " << markupsResource;

    bool inMenu = q->isActionTextInMenu(markupsName, this->CreateAndPlaceMenu);
    if (!inMenu)
      {
      // add it
      QAction * newAction = new QAction(this->CreateAndPlaceMenu);
      newAction->setObjectName(markupsID);
      newAction->setIcon(QIcon(markupsResource));
      if (newAction->icon().isNull())
        {
        qCritical() << "qSlicerMarkupsToolBarPrivate::updateWidgetFromSelectionNode - "
                    << "New action icon for id " << markupsID << "is null. "
                    << "Resource:" << markupsResource;
        }
      newAction->setText(markupsName);
      newAction->setIconText(markupsName);
      QString tooltip = QString("Use mouse to Create-and-Place ") + markupsName;
      newAction->setToolTip(tooltip);
      // save the markups id as data on the action
      newAction->setData(markupsID);
      newAction->setCheckable(true);
      connect(newAction, SIGNAL(triggered()),
              q, SLOT(switchPlaceMode()));
      this->CreateAndPlaceToolButton->menu()->addAction(newAction);
      this->ActionGroup->addAction(newAction);
      // if this new one is the fiducial
      if (markupsName.compare("Fiducial") == 0)
        {
        // make it the default
        this->CreateAndPlaceToolButton->setDefaultAction(newAction);
        }
      }
    }
  // select the active one
  const char *activeMarkupsNodeType = selectionNode->GetActiveMarkupsNodeType();
  this->updateWidgetToMarkups(activeMarkupsNodeType);
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBarPrivate::updateWidgetToMarkups(const char *markupsID)
{
  Q_Q(qSlicerMarkupsToolBar);

  if (!this->MRMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }
    
  QAction *checkedAction = 0;
  if (!markupsID)
    {
    //qDebug() << "qSlicerMarkupsToolBarPrivate::updateWidgetToMarkups: "
    //            "null active markups id, resetting to view transform";
    q->changeCursorTo(QCursor());
    q->switchToViewTransformMode();
    }
  else
    {
    // get the actions and check their data for the markups id
    QList<QAction *> actions = this->ActionGroup->actions();
    for (int i = 0; i < actions.size(); ++i)
      {
      QString thisID = actions.at(i)->data().toString();
      if (thisID.compare(markupsID) == 0)
        {
        // set this action checked
        actions.at(i)->setChecked(true);
        checkedAction = actions.at(i);
        //qDebug() << "qSlicerMarkupsToolBarPrivate::updateWidgetToMarkups - "
        //            "Found active markups: " << thisID;
        // update the cursor from the markups id
        vtkMRMLSelectionNode *selectionNode = this->MRMLAppLogic->GetSelectionNode();
        if ( selectionNode )
          {
          std::string resource = selectionNode->GetMarkupsResourceByID(std::string(markupsID));
          //qDebug() << "qSlicerMarkupsToolBarPrivate::updateWidgetToMarkups - updating cursor from selection node";
          q->changeCursorTo(QCursor(QPixmap(resource.c_str()),-1,0));
          }
        else
          {
          // update from the icon, preserving size
          QList<QSize> availableSizes = actions.at(i)->icon().availableSizes();
          if (availableSizes.size() > 0)
            {
            q->changeCursorTo(QCursor(actions.at(i)->icon().pixmap(availableSizes[0])));
            }
          else
            {
            // use a default
            q->changeCursorTo(QCursor(actions.at(i)->icon().pixmap(20)));
            }
          }
        break;
        }
      }
    }
  if (checkedAction)
    {
    //  make the checked one the default action
    //qDebug() << "qSlicerMarkupsToolBarPrivate::updateWidgetToMarkups - setting default action to " << qPrintable(checkedAction->text());
    this->CreateAndPlaceToolButton->setDefaultAction(checkedAction);
    }
  else
    {
    // switching to view transform mode, actions are unchecked by the time we
    // get here
    }
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBarPrivate::OnMRMLSceneStartClose()
{
  Q_Q(qSlicerMarkupsToolBar);
  q->setEnabled(false);
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBarPrivate::OnMRMLSceneEndImport()
{
  Q_Q(qSlicerMarkupsToolBar);

  // re-enable in case it didn't get re-enabled for scene load
  q->setEnabled(true);

  // update the state from mrml
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBarPrivate::OnMRMLSceneEndClose()
{
  Q_Q(qSlicerMarkupsToolBar);
  if (!this->MRMLScene || this->MRMLScene->IsBatchProcessing())
    {
    return;
    }
  // reenable it and update
  q->setEnabled(true);
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBarPrivate::onInteractionNodeModeChangedEvent()
{
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBarPrivate::onInteractionNodeModePersistenceChanged()
{
  // get the place persistence mode from the interaction node
  if (!this->MRMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }
  
  vtkMRMLInteractionNode * interactionNode = this->MRMLAppLogic->GetInteractionNode();
  Q_ASSERT(interactionNode);
  int persistence = interactionNode->GetPlaceModePersistence();
  if (persistence)
    {
    if (!this->PersistenceAction->isChecked())
      {
      this->PersistenceAction->setChecked(true);
      }
    }
  else
    {
    if (this->PersistenceAction->isChecked())
      {
      this->PersistenceAction->setChecked(false);
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBarPrivate::onActiveMarkupsNodeTypeChangedEvent()
{
  //qDebug() << "qSlicerMarkupsToolBarPrivate::onActiveMarkupsNodeTypeChangedEvent";
  this->updateWidgetFromSelectionNode();
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBarPrivate::onMarkupsIDListModifiedEvent()
{
  //qDebug() << "qSlicerMarkupsToolBarPrivate::onMarkupsIDListModifiedEvent";
  this->updateWidgetFromSelectionNode();
}

//---------------------------------------------------------------------------
// qSlicerModuleSelectorToolBar methods

//---------------------------------------------------------------------------
qSlicerMarkupsToolBar::qSlicerMarkupsToolBar(const QString& title, QWidget* parentWidget)
  :Superclass(title, parentWidget)
  , d_ptr(new qSlicerMarkupsToolBarPrivate(*this))
{
  Q_D(qSlicerMarkupsToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qSlicerMarkupsToolBar::qSlicerMarkupsToolBar(QWidget* parentWidget):Superclass(parentWidget)
  , d_ptr(new qSlicerMarkupsToolBarPrivate(*this))
{
  Q_D(qSlicerMarkupsToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qSlicerMarkupsToolBar::~qSlicerMarkupsToolBar()
{
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBar::setApplicationLogic(vtkSlicerApplicationLogic* appLogic)
{
  Q_D(qSlicerMarkupsToolBar);
  d->MRMLAppLogic = appLogic;
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBar::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qSlicerMarkupsToolBar);
  d->setMRMLScene(newScene);
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBar::switchToViewTransformMode()
{
  Q_D(qSlicerMarkupsToolBar);

  if (!d->MRMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }

  vtkMRMLInteractionNode * interactionNode = d->MRMLAppLogic->GetInteractionNode();
  if (interactionNode)
    {
    //qDebug() << "qSlicerMarkupsToolBar::switchToViewTransformMode";

    // update the interaction node, should trigger a cursor update
    interactionNode->SwitchToViewTransformMode();

    // uncheck all 
    d->CreateAndPlaceToolButton->setChecked(false);
    QList<QAction*> actionList =  d->CreateAndPlaceMenu->actions();
    int numActions = actionList.size();
    for (int i = 0; i < numActions; i++)
      {
      QAction *action = actionList.at(i);
      QString actionText = action->text();
      if ( actionText.compare(QObject::tr("Persistent")) != 0  &&
          !actionText.isEmpty())
        {
        action->setChecked(false);
        }
      }
    // cancel all Markups placements
    interactionNode->InvokeEvent(vtkMRMLInteractionNode::EndPlacementEvent);
    }
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBar::changeCursorTo(QCursor cursor)
{
  if (!qSlicerApplication::application())
    {
    qWarning() << "changeCursorTo: can't get a qSlicerApplication";
    return;
    }
  qMRMLLayoutManager *layoutManager = qSlicerApplication::application()->layoutManager();

  if (!layoutManager)
    {
    return;
    }
 
  // loop through all existing threeDViews
  for (int i=0; i < layoutManager->threeDViewCount(); ++i)
    {
    layoutManager->threeDWidget(i)->threeDView()->setCursor(cursor);
    }

  // the slice viewers
  vtkMRMLLayoutLogic *layoutLogic = layoutManager->layoutLogic();
  if (!layoutLogic)
    {
    return;
    }
  // the view nodes list is kept up to date with the currently mapped viewers
  vtkCollection *visibleViews = layoutLogic->GetViewNodes();
  // iterate through the view nodes, getting the layout name to get the slice
  // widget
  int numViews = visibleViews->GetNumberOfItems();
  for (int v = 0; v < numViews; v++)
    {
    // item 0 is usually a vtkMRMLViewNode for the 3d window
    vtkMRMLSliceNode *sliceNode = vtkMRMLSliceNode::SafeDownCast(visibleViews->GetItemAsObject(v));
    if (sliceNode)
      {    
      qMRMLSliceWidget *sliceView = layoutManager->sliceWidget(sliceNode->GetName());
      if (sliceView)
        {
        sliceView->setCursor(cursor);
        }
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBar::switchPlaceMode()
{
  Q_D(qSlicerMarkupsToolBar);

  if (!d->MRMLAppLogic)
    {
    qWarning() << "Mouse Mode Tool Bar not set up with application logic";
    return;
    }
    
  // get the currently checked action
  QString markupsID;
  QAction *thisAction = d->CreateAndPlaceToolButton->menu()->activeAction();
  if (thisAction)
    {
    markupsID = thisAction->data().toString();
//    qDebug() << "qSlicerMarkupsToolBar::switchPlaceMode: got active action data " << markupsID;
    }
  else
    {
    thisAction = d->ActionGroup->checkedAction();
    if (thisAction)
      {
      markupsID = thisAction->data().toString();
      //qDebug() << "qSlicerMarkupsToolBar::switchPlaceMode: got action group checked action text id = " << markupsID;
      }
    }
  if (markupsID.isEmpty())
    {
    qCritical() << "qSlicerMarkupsToolBar::switchPlaceMode: could not get active markups menu item!";
    return;
    }
  // get selection node
  vtkMRMLSelectionNode *selectionNode = d->MRMLAppLogic->GetSelectionNode();
  if ( selectionNode )
    {
    QString previousMarkupsNodeType = QString(selectionNode->GetActiveMarkupsNodeType());
    //qDebug() << "switchPlaceMode: previous markups id is " << qPrintable(previousMarkupsID) << ", changing to " <<  qPrintable(markupsID);
    selectionNode->SetReferenceActiveMarkupsNodeType(markupsID.toLatin1());
    // update the interaction mode, which will trigger an update of the cursor
    vtkMRMLInteractionNode * interactionNode = d->MRMLAppLogic->GetInteractionNode();
    if (interactionNode)
      {
      // is this a click on top of a single or persistent place mode?
      ///qDebug() << "switchPlaceMode: interaction node current interaction mode = " << interactionNode->GetCurrentInteractionMode() << ", previous markups id is " << qPrintable(previousMarkupsID) << ", new markups id = " << qPrintable(markupsID);
      if (interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place &&
          markupsID.compare(previousMarkupsNodeType) == 0)
        {
        //qDebug() << "switchPlaceMode: current interaction mode is place, bail out, switching to view transform mode now";
        this->switchToViewTransformMode();
        return;
        }
      if (d->PersistenceAction->isChecked())
        {
        //qDebug() << "qSlicerMarkupsToolBar::switchPlaceMode: switching to persistent place mode";
        interactionNode->SwitchToPersistentPlaceMode();
        }
      else
        {
        //qDebug() << "qSlicerMarkupsToolBar::switchPlaceMode: switching to single place mode";
        interactionNode->SwitchToSinglePlaceMode();
        }
      }
    else { qCritical() << "qSlicerMarkupsToolBar::switchPlaceMode: unable to get interaction node"; }
    }
  else
    {
    qCritical() << "qSlicerMarkupsToolBar::switchPlaceMode: unable to get selection node";
    }
}

//---------------------------------------------------------------------------
bool qSlicerMarkupsToolBar::isActionTextInMenu(QString actionText, QMenu *menu)
{
  // get a list of all the actions currently in the menu
  QList<QAction*> actionList = menu->actions();
  int numActions = actionList.size();
  for (int i = 0; i < numActions; i++)
    {
//    qDebug() << "qSlicerMarkupsToolBar::isActionTextInMenu, testing action " << i
//             << " with text " << actionList[i] << " against text " << actionText;
    if (actionList[i]->text() == actionText)
      {
      return true;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
void qSlicerMarkupsToolBar::onPersistenceToggled()
{
  Q_D(qSlicerMarkupsToolBar);

  vtkMRMLInteractionNode *interactionNode = NULL;
  if (d->MRMLAppLogic)
    {
    interactionNode = d->MRMLAppLogic->GetInteractionNode();
    }

  if (interactionNode)
    {
    int newPersistence = !interactionNode->GetPlaceModePersistence();
    interactionNode->SetPlaceModePersistence(newPersistence);
    }
  else
    {
    qWarning() << "qSlicerMarkupsToolBar::onPersistenceToggled: "
                  "no interaction node found to toggle.";
    }
}

//---------------------------------------------------------------------------
QString qSlicerMarkupsToolBar::activeActionText()
{
  Q_D(qSlicerMarkupsToolBar);

  QString activeActionText;

  QAction *defaultAction = d->CreateAndPlaceToolButton->defaultAction();
  if (defaultAction)
    {
    activeActionText = defaultAction->text();
    }

  return activeActionText;
}
