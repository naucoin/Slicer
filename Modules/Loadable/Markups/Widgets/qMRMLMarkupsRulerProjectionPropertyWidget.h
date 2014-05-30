/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Laurent Chauvin, Brigham and Women's
  Hospital. The project was supported by grants 5P01CA067165,
  5R01CA124377, 5R01CA138586, 2R44DE019322, 7R01CA124377,
  5R42CA137886, 5R42CA137886

==============================================================================*/

#ifndef __qMRMLMarkupsRulerProjectionPropertyWidget_h
#define __qMRMLMarkupsRulerProjectionPropertyWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qMRMLWidget.h"

#include "qSlicerMarkupsModuleWidgetsExport.h"

class qMRMLMarkupsRulerProjectionPropertyWidgetPrivate;
class vtkMRMLMarkupsRulerNode;

/// \ingroup Slicer_QtModules_Markups
class Q_SLICER_MODULE_MARKUPS_WIDGETS_EXPORT
qMRMLMarkupsRulerProjectionPropertyWidget
  : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLWidget Superclass;
  qMRMLMarkupsRulerProjectionPropertyWidget(QWidget *newParent = 0);
  virtual ~qMRMLMarkupsRulerProjectionPropertyWidget();

public slots:
  void setMRMLRulerNode(vtkMRMLMarkupsRulerNode* rulerNode);
  void setProjectionVisibility(bool showProjection);
  void setUseRulerColor(bool useRulerColor);
  void setProjectionColor(QColor newColor);
  void setOverlineThickness(int thickness);
  void setUnderlineThickness(int thickness);
  void setColoredWhenParallel(bool coloredWhenParallel);
  void setThickerOnTop(bool thickerOnTop);
  void setDashed(bool dashed);

protected slots:
  void updateWidgetFromDisplayNode();

protected:
  QScopedPointer<qMRMLMarkupsRulerProjectionPropertyWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLMarkupsRulerProjectionPropertyWidget);
  Q_DISABLE_COPY(qMRMLMarkupsRulerProjectionPropertyWidget);

};

#endif
