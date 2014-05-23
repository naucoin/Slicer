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
#include <QFileInfo>

// SlicerQt includes
#include "qSlicerMarkupsReader.h"

// Logic includes
#include <vtkSlicerApplicationLogic.h>
#include "vtkSlicerMarkupsLogic.h"

// MRML includes

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerMarkupsReaderPrivate
{
  public:
  vtkSmartPointer<vtkSlicerMarkupsLogic> MarkupsLogic;
};

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotations
//-----------------------------------------------------------------------------
qSlicerMarkupsReader::qSlicerMarkupsReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMarkupsReaderPrivate)
{
}

qSlicerMarkupsReader::qSlicerMarkupsReader(vtkSlicerMarkupsLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMarkupsReaderPrivate)
{
  this->setMarkupsLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerMarkupsReader::~qSlicerMarkupsReader()
{
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsReader::setMarkupsLogic(vtkSlicerMarkupsLogic* logic)
{
  Q_D(qSlicerMarkupsReader);
  d->MarkupsLogic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerMarkupsLogic* qSlicerMarkupsReader::markupsLogic()const
{
  Q_D(const qSlicerMarkupsReader);
  return d->MarkupsLogic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerMarkupsReader::description()const
{
  return "Markups";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerMarkupsReader::fileType()const
{
  return QString("MarkupsFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsReader::extensions()const
{
  return QStringList()
    << "Markups Fiducials (*.fcsv)"
    << " Markups Rulers (*.rcsv)"
    << " Annotation Fiducial (*.acsv)"
    << " Annotation Ruler (*.acsv)";
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsReader::load(const IOProperties& properties)
{
  Q_D(qSlicerMarkupsReader);

  // get the properties
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QString name = QFileInfo(fileName).baseName();
  if (properties.contains("name"))
    {
    name = properties["name"].toString();
    }

  if (d->MarkupsLogic.GetPointer() == 0)
    {
    return false;
    }

  QString ext = QFileInfo(fileName).suffix();

  // pass to logic to do the loading
  char * nodeIDs = NULL;
  if (ext.compare("fcsv") == 0)
    {
    // Markups fiducial list
    nodeIDs = d->MarkupsLogic->LoadMarkupsFiducials(fileName.toLatin1(),
                                                    name.toLatin1());
    }
  else if (ext.compare("rcsv") == 0)
    {
    // Markups ruler list
    nodeIDs = d->MarkupsLogic->LoadMarkupsRulers(fileName.toLatin1(),
                                                 name.toLatin1());
    }
  else if (ext.compare("acsv") == 0)
    {
    // Annotation fiducial or ruler
    // try loading as rulers first since that will fail if it doesn't
    // find two points
    nodeIDs = d->MarkupsLogic->LoadMarkupsRulers(fileName.toLatin1(),
                                                 name.toLatin1());
    if (nodeIDs == NULL)
      {
      // try reading it as a ruler
      nodeIDs = d->MarkupsLogic->LoadMarkupsFiducials(fileName.toLatin1(),
                                                      name.toLatin1());
      }
    }

  if (nodeIDs)
    {
    // returned a comma separated list of ids of the nodes that were loaded
    QStringList nodeIDList;
    char *ptr = strtok(nodeIDs, ",");

    while (ptr)
      {
      nodeIDList.append(ptr);
      ptr = strtok(NULL, ",");
      }
    this->setLoadedNodes(nodeIDList);
    }
  else
    {
    this->setLoadedNodes(QStringList());
    return false;
    }

  return nodeIDs != 0;
}
