#include "GUI/qSlicerSceneViewsModuleWidget.h"
#include "ui_qSlicerSceneViewsModuleWidget.h"

// CTK includes
#include "ctkCollapsibleButton.h"

// QT includes
#include <QBuffer>
#include <QButtonGroup>
#include <QFile>
#include <QFileDialog>
#include <QFontMetrics>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QImageWriter>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QStatusBar>
#include <QTextBrowser>
#include <QWebFrame>
#include <QUrl>

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSceneViewNode.h"

// VTK includes
#include "vtkCollection.h"
#include "vtkImageData.h"
#include "vtkNew.h"
#include "vtkPNGWriter.h"
#include "vtkSmartPointer.h"

// GUI includes
#include "GUI/qSlicerSceneViewsModuleDialog.h"

#include "qSlicerApplication.h"

#include "qMRMLSceneModel.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SceneViews
class qSlicerSceneViewsModuleWidgetPrivate: public Ui_qSlicerSceneViewsModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerSceneViewsModuleWidget);
protected:
  qSlicerSceneViewsModuleWidget* const q_ptr;
public:

  qSlicerSceneViewsModuleWidgetPrivate(qSlicerSceneViewsModuleWidget& object);
  ~qSlicerSceneViewsModuleWidgetPrivate();
  void setupUi(qSlicerWidget* widget);

  vtkSlicerSceneViewsModuleLogic* logic() const;

  qSlicerSceneViewsModuleDialog* sceneViewDialog();

  QPointer<qSlicerSceneViewsModuleDialog> SceneViewDialog;

  QString htmlFromSceneView(vtkMRMLSceneViewNode *sceneView);
};

//-----------------------------------------------------------------------------
// qSlicerSceneViewsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
vtkSlicerSceneViewsModuleLogic*
qSlicerSceneViewsModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerSceneViewsModuleWidget);
  return vtkSlicerSceneViewsModuleLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleDialog* qSlicerSceneViewsModuleWidgetPrivate::sceneViewDialog()
{
  if (!this->SceneViewDialog)
    {
    this->SceneViewDialog = new qSlicerSceneViewsModuleDialog();

    // pass a pointer to the logic class
    this->SceneViewDialog->setLogic(this->logic());

    // create slots which listen to events fired by the OK and CANCEL button on the dialog
    //this->connect(this->SceneViewDialog, SIGNAL(rejected()),
    //              this->SceneViewDialog, SLOT(hide()));
    //this->connect(this->SceneViewDialog, SIGNAL(accepted()),
    //              this->SceneViewDialog, SLOT(hide()));
    }
  return this->SceneViewDialog;
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidgetPrivate::qSlicerSceneViewsModuleWidgetPrivate(qSlicerSceneViewsModuleWidget& object)
  : q_ptr(&object)
{
  this->SceneViewDialog = 0;
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidgetPrivate::~qSlicerSceneViewsModuleWidgetPrivate()
{
  if (this->SceneViewDialog)
    {
    this->SceneViewDialog->close();
    delete this->SceneViewDialog.data();
    }
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  Q_Q(qSlicerSceneViewsModuleWidget);
  this->Ui_qSlicerSceneViewsModuleWidget::setupUi(widget);

  QObject::connect(this->hierarchyTreeView,
                   SIGNAL(restoreSceneViewRequested(QString)),
                   q, SLOT(restoreSceneView(QString)));
  QObject::connect(this->hierarchyTreeView,
                   SIGNAL(editSceneViewRequested(QString)),
                   q, SLOT(editSceneView(QString)));

  // setup the hierarchy treeWidget
  this->hierarchyTreeView->setLogic(this->logic());
  this->hierarchyTreeView->setMRMLScene(this->logic()->GetMRMLScene());
  // setMRMLScene calls setRoot

  q->connect(this->moveDownSelectedButton, SIGNAL(clicked()),
             q, SLOT(moveDownSelected()));
  q->connect(this->moveUpSelectedButton, SIGNAL(clicked()),
             q, SLOT(moveUpSelected()));

  QObject::connect(this->deleteSelectedButton, SIGNAL(clicked()),
                   this->hierarchyTreeView, SLOT(deleteSelected()));

  QObject::connect(this->sceneView, SIGNAL(clicked()),
                   q, SLOT(showSceneViewDialog()));

  q->connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
             this->hierarchyTreeView, SLOT(setMRMLScene(vtkMRMLScene*)));

  // listen to some mrml events
  q->qvtkConnect(this->logic()->GetMRMLScene(), vtkMRMLScene::EndImportEvent,
                 this->hierarchyTreeView, SLOT(onSceneEndImportEvent()));
  q->qvtkConnect(this->logic()->GetMRMLScene(), vtkMRMLScene::EndRestoreEvent,
                 this->hierarchyTreeView, SLOT(onSceneEndRestoreEvent()));
  q->qvtkConnect(this->logic()->GetMRMLScene(), vtkMRMLScene::EndBatchProcessEvent,
                 this->hierarchyTreeView, SLOT(onSceneEndBatchProcessEvent()));


  this->sceneViewsWebView->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOn);
  // capture link clicked
  this->sceneViewsWebView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
  QObject::connect(this->sceneViewsWebView, SIGNAL(linkClicked(const QUrl &)),
                    q, SLOT(captureLinkClicked(QUrl)));
}

//-----------------------------------------------------------------------------
QString qSlicerSceneViewsModuleWidgetPrivate::htmlFromSceneView(vtkMRMLSceneViewNode *sceneView)
{
  QString html;

  if (!sceneView)
    {
    return html;
    }
  QString name = sceneView->GetName();
  QString id = sceneView->GetID();
  QString description = sceneView->GetSceneViewDescription().c_str();
  // replace any carriage returns with html line breaks
  description.replace(QString("\n"),
                      QString("<br>\n"));
  QString tempDir = qSlicerApplication::application()->defaultTemporaryPath();
  QString thumbnailPath = tempDir + "/" + id + ".png";
  /// tbd: always write out the image?
  // if (!QFile::exists(thumbnailPath))
    {
    //qDebug() << "writing out file " << thumbnailPath;
    vtkNew<vtkPNGWriter> writer;
    writer->SetFileName(thumbnailPath.toLatin1());
    writer->SetInput( sceneView->GetScreenShot() );
    try
      {
      writer->Write();
      }
    catch (...)
      {
      qWarning() << "Unable to write file " << thumbnailPath;
      }
    }
  QString restoreImagePath = QString("qrc:///Icons/Restore.png");

  html = "<li>";
  html += " <div style=\"width:100%;overflow-x:hidden;overflow-y:hidden;background-image:none;\">\n";
  html += "  <div style=\"float:left; width:200px; margin:5px;\">\n";
  html += "   <a href=\"Edit " + id + "\">\n";
  html += "    <img src=\"file://" + thumbnailPath + "\" ";
  html += "style=\"visibility:visible; max-width:200; max-height:none; ";
  html += "display:block; image-rendering:auto; width:auto; height:auto; ";
  html += "margin-left:14px; margin-top:0px; opacity:1;\">\n";
  html += "   </a>\n";
  html += "  </div>\n";
  html += "  <div style=\"margin-left: 220px;\">";
  html += "   <h3><a href=\"Restore " + id  + "\"><img src=\"" + restoreImagePath + "\"></a> ";
  html += "   " + name + "</h3>\n";
  html += "   " + description + "\n";
  html += "  </div>\n";
  html += " </div>\n";
  html += "</li>\n";

  return html;
}

//-----------------------------------------------------------------------------
// qSlicerSceneViewsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidget::qSlicerSceneViewsModuleWidget(QWidget* parent) :
  qSlicerAbstractModuleWidget(parent)
  , d_ptr(new qSlicerSceneViewsModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerSceneViewsModuleWidget::~qSlicerSceneViewsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::setup()
{
  Q_D(qSlicerSceneViewsModuleWidget);
  this->Superclass::setup();
  d->setupUi(this);

  this->updateTreeViewModel();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::moveDownSelected()
{
  Q_D(qSlicerSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewDown(
    d->hierarchyTreeView->firstSelectedNode().toLatin1());

  d->hierarchyTreeView->clearSelection();
  d->hierarchyTreeView->setSelectedNode(id);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::moveUpSelected()
{
  Q_D(qSlicerSceneViewsModuleWidget);

  const char* id = d->logic()->MoveSceneViewUp(
    d->hierarchyTreeView->firstSelectedNode().toLatin1());

  d->hierarchyTreeView->clearSelection();
  d->hierarchyTreeView->setSelectedNode(id);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::restoreSceneView(const QString& mrmlId)
{
  Q_D(qSlicerSceneViewsModuleWidget);

  d->logic()->RestoreSceneView(mrmlId.toLatin1());

  qSlicerApplication::application()->mainWindow()->statusBar()->showMessage("The SceneView was restored including the attached scene.", 2000);
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::editSceneView(const QString& mrmlId)
{
  Q_D(qSlicerSceneViewsModuleWidget);
  d->sceneViewDialog()->loadNode(mrmlId);
  d->sceneViewDialog()->exec();
  this->updateFromMRMLScene();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::updateFromMRMLScene()
{
  Q_D(qSlicerSceneViewsModuleWidget);

  // logic will be listening for this event as well and filling in missing
  // hierarchy nodes, so just refresh the tree
  this->updateTreeViewModel();

  int numSceneViews = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLSceneViewNode");
  QString headerHtml;
  headerHtml = "<html>";
  headerHtml += "<head></head>";
  headerHtml += "<body>";
  headerHtml += " <div id=\"modalDialog\" style=\"display:none;\"></div>";
  headerHtml += " <div class=\"MainDialog\" style=\"display:none;\"></div>";
  headerHtml += " <div class=\"category\"></div>";
  headerHtml += " <div id=\"container_main\">";
  headerHtml += "  <div id=\"extension_banner\"></div>";
  headerHtml += "  <div class=\"left_column\"></div>";
  headerHtml += "  <div class=\"right_content\">";
  headerHtml += "   <div class=\"content_header\">Scene Views ("
    + QString::number(numSceneViews) + "):</div>";
  headerHtml += "   <div class=\"screenshots\" style=\"width:100%;\">";
  headerHtml += "    <ul>";

  QString htmlPage = headerHtml;

  for (int i = 0; i < numSceneViews; ++i)
    {
    vtkMRMLNode *mrmlNode = this->mrmlScene()->GetNthNodeByClass(i, "vtkMRMLSceneViewNode");
    if (!mrmlNode)
      {
      continue;
      }
    vtkMRMLSceneViewNode *sceneView = vtkMRMLSceneViewNode::SafeDownCast(mrmlNode);
    QString sceneViewHtml = d->htmlFromSceneView(sceneView);
    htmlPage += sceneViewHtml;
    }
  QString footerHtml;
  footerHtml = "    </ul>\n";
  footerHtml += "   </div>\n";
  footerHtml += "  </div>\n";
  footerHtml += " </body>\n";
  footerHtml += "</html>\n";
  htmlPage += footerHtml;

  QString baseURL;
  d->sceneViewsWebView->setHtml(htmlPage, baseURL);
  d->sceneViewsWebView->show();

}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::enter()
{
  this->Superclass::enter();

  // set up mrml scene observations so that the GUI gets updated
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::NodeAddedEvent,
                    this, SLOT(onMRMLSceneEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::NodeRemovedEvent,
                    this, SLOT(onMRMLSceneEvent(vtkObject*, vtkObject*)));

  this->updateFromMRMLScene();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::exit()
{
  this->Superclass::exit();

  // qDebug() << "exit widget";

  // remove mrml scene observations, don't need to update the GUI while the
  // module is not showing
  this->qvtkDisconnectAll();
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::onMRMLSceneEvent(vtkObject*, vtkObject* node)
{
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
  vtkMRMLSceneViewNode* sceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(node);
  if (sceneViewNode)
    {
    this->updateFromMRMLScene();
    }
}

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::captureLinkClicked(const QUrl &url)
{
  QString toParse = url.toString();

  QStringList operationAndID = toParse.split(" ");
  QString operation = operationAndID[0];
  QString id = operationAndID[1];
  if (operation == QString("Edit"))
    {
    this->editSceneView(id);
    }
  else if (operation == QString("Restore"))
    {
    this->restoreSceneView(id);
    }
  else
    {
    qWarning() << "caputreLinkClicked: unsupported operation: " << operation;
    }
}

//-----------------------------------------------------------------------------
// Refresh the hierarchy tree after an sceneView was added or modified.
//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::updateTreeViewModel()
{
  Q_D(qSlicerSceneViewsModuleWidget);

  if (d->logic() && d->logic()->GetMRMLScene() &&
      d->logic()->GetMRMLScene()->IsBatchProcessing())
    {
    // scene is updating, return
    return;
    }
  //qDebug("updateTreeViewModel");

  // use lazy update to trigger a refresh after batch processing
  d->hierarchyTreeView->sceneModel()->setLazyUpdate(true);

  // set mrml scene calls expand all and set root
  d->hierarchyTreeView->setMRMLScene(d->logic()->GetMRMLScene());
}

//-----------------------------------------------------------------------------
// SceneView functionality
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void qSlicerSceneViewsModuleWidget::showSceneViewDialog()
{
  Q_D(qSlicerSceneViewsModuleWidget);
  // show the dialog
  d->sceneViewDialog()->reset();
  d->sceneViewDialog()->exec();
}

