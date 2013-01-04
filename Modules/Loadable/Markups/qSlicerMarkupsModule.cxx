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
#include <QtPlugin>

// Markups Logic includes
#include <vtkSlicerMarkupsLogic.h>

// MRMLDisplayableManager includes
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include <vtkMRMLSliceViewDisplayableManagerFactory.h>

// QTGUI includes
//#include <qSlicerApplication.h>
//#include <qSlicerCoreApplication.h>
//#include <qSlicerIOManager.h>

// Markups includes
#include "qSlicerMarkupsModule.h"
#include "qSlicerMarkupsModuleWidget.h"
//#include "vtkSlicerAnnotationModuleLogic.h"
//#include "qSlicerAnnotationsIO.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerMarkupsModule, qSlicerMarkupsModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Markups
class qSlicerMarkupsModulePrivate
{
public:
  qSlicerMarkupsModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerMarkupsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerMarkupsModulePrivate::qSlicerMarkupsModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerMarkupsModule methods

//-----------------------------------------------------------------------------
qSlicerMarkupsModule::qSlicerMarkupsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMarkupsModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsModule::categories()const
{
  return QStringList() << "Developer Tools";
}

//-----------------------------------------------------------------------------
qSlicerMarkupsModule::~qSlicerMarkupsModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerMarkupsModule::helpText()const
{
  QString help = 
    "A module to create and manage markups in 2D and 3D."
    " Extends the Annotations module";
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerMarkupsModule::acknowledgementText()const
{
  return "This work was supported by NAMIC, NAC, and the Slicer Community.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Nicole Aucoin (BWH)");
  // moduleContributors << QString("Richard Roe (Organization2)");
  // ...
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerMarkupsModule::icon()const
{
  return QIcon(":/Icons/Markups.png");
}



//-----------------------------------------------------------------------------
void qSlicerMarkupsModule::setup()
{
  this->Superclass::setup();

  // Register displayable managers
  // 3D
  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager("vtkMRMLMarkupsFiducialDisplayableManager3D");
  // 2D
  vtkMRMLSliceViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager("vtkMRMLMarkupsFiducialDisplayableManager2D");

  // Register IO
/*
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  ioManager->registerIO(
    new qSlicerMarkupsIO(vtkSlicerMarkupsModuleLogic::SafeDownCast(this->logic()), this));
  ioManager->registerIO(new qSlicerNodeWriter(
                          "Markups", qSlicerIO::MarkupsFile,
                          QStringList() << "vtkMRMLMarkupsNode", this));
*/
}


//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerMarkupsModule::createWidgetRepresentation()
{
  return new qSlicerMarkupsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerMarkupsModule::createLogic()
{
  return vtkSlicerMarkupsLogic::New();
}
