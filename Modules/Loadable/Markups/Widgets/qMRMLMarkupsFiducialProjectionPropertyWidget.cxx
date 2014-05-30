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
  It was then updated for the Markups module by Nicole Aucoin, BWH.

==============================================================================*/

// qMRML includes
#include "qMRMLMarkupsFiducialProjectionPropertyWidget.h"
#include "ui_qMRMLMarkupsFiducialProjectionPropertyWidget.h"

// MRML includes
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMarkupsDisplayNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Markups
class qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate
  : public Ui_qMRMLMarkupsFiducialProjectionPropertyWidget
{
  Q_DECLARE_PUBLIC(qMRMLMarkupsFiducialProjectionPropertyWidget);
protected:
  qMRMLMarkupsFiducialProjectionPropertyWidget* const q_ptr;
public:
  qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate(qMRMLMarkupsFiducialProjectionPropertyWidget& object);
  void init();

  vtkMRMLMarkupsDisplayNode* PointDisplayNode;
};

//-----------------------------------------------------------------------------
// qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate
::qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate(qMRMLMarkupsFiducialProjectionPropertyWidget& object)
  : q_ptr(&object)
{
  this->PointDisplayNode = NULL;
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate
::init()
{
  Q_Q(qMRMLMarkupsFiducialProjectionPropertyWidget);
  this->setupUi(q);
  QObject::connect(this->point2DProjectionCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setProjectionVisibility(bool)));
  QObject::connect(this->pointProjectionColorPickerButton, SIGNAL(colorChanged(QColor)),
                   q, SLOT(setProjectionColor(QColor)));
  QObject::connect(this->pointUseFiducialColorCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setUseFiducialColor(bool)));
  QObject::connect(this->pointOutlinedBehindSlicePlaneCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setOutlinedBehindSlicePlane(bool)));
  QObject::connect(this->projectionOpacitySliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(setProjectionOpacity(double)));
  q->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
// qMRMLMarkupsFiducialProjectionPropertyWidget methods

//-----------------------------------------------------------------------------
qMRMLMarkupsFiducialProjectionPropertyWidget
::qMRMLMarkupsFiducialProjectionPropertyWidget(QWidget *newParent) :
    Superclass(newParent)
  , d_ptr(new qMRMLMarkupsFiducialProjectionPropertyWidgetPrivate(*this))
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qMRMLMarkupsFiducialProjectionPropertyWidget
::~qMRMLMarkupsFiducialProjectionPropertyWidget()
{
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidget
::setMRMLMarkupsNode(vtkMRMLMarkupsNode* pointNode)
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);

  if (!pointNode)
    {
    return;
    }

  vtkMRMLMarkupsDisplayNode* displayNode
    = pointNode->GetMarkupsDisplayNode();

  qvtkReconnect(d->PointDisplayNode, displayNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDisplayNode()));

  d->PointDisplayNode = displayNode;
  this->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidget
::setProjectionVisibility(bool showProjection)
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
  if (!d->PointDisplayNode)
    {
    return;
    }
  if (showProjection)
    {
    d->PointDisplayNode->SliceProjectionOn();
    }
  else
    {
    d->PointDisplayNode->SliceProjectionOff();
    }
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidget
::setProjectionColor(QColor newColor)
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
  if (!d->PointDisplayNode)
    {
    return;
    }
  d->PointDisplayNode
    ->SetSliceProjectionColor(newColor.redF(), newColor.greenF(), newColor.blueF());
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidget
::setUseFiducialColor(bool useFiducialColor)
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
  if (!d->PointDisplayNode)
    {
    return;
    }
  if (useFiducialColor)
    {
    d->PointDisplayNode->SliceProjectionUseFiducialColorOn();
    d->pointProjectionColorLabel->setEnabled(false);
    d->pointProjectionColorPickerButton->setEnabled(false);
    }
  else
    {
    d->PointDisplayNode->SliceProjectionUseFiducialColorOff();
    d->pointProjectionColorLabel->setEnabled(true);
    d->pointProjectionColorPickerButton->setEnabled(true);
    }
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidget
::setOutlinedBehindSlicePlane(bool outlinedBehind)
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
  if (!d->PointDisplayNode)
    {
    return;
    }
  if (outlinedBehind)
    {
    d->PointDisplayNode->SliceProjectionOutlinedBehindSlicePlaneOn();
    }
  else
    {
    d->PointDisplayNode->SliceProjectionOutlinedBehindSlicePlaneOff();
    }
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidget
::setProjectionOpacity(double opacity)
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);
  if (!d->PointDisplayNode)
    {
    return;
    }
  d->PointDisplayNode->SetSliceProjectionOpacity(opacity);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsFiducialProjectionPropertyWidget
::updateWidgetFromDisplayNode()
{
  Q_D(qMRMLMarkupsFiducialProjectionPropertyWidget);

  this->setEnabled(d->PointDisplayNode != 0);

  if (!d->PointDisplayNode)
    {
    return;
    }

  // Update widget if different from MRML node
  // -- 2D Projection Visibility
  d->point2DProjectionCheckBox->setChecked(
    d->PointDisplayNode->GetSliceProjection() &
    vtkMRMLMarkupsDisplayNode::ProjectionOn);

  // -- Projection Color
  double pColor[3];
  d->PointDisplayNode->GetSliceProjectionColor(pColor);
  QColor displayColor = QColor(pColor[0]*255, pColor[1]*255, pColor[2]*255);
  d->pointProjectionColorPickerButton->setColor(displayColor);

  // -- Use Fiducial Color
  bool useFiducialColor = d->PointDisplayNode->GetSliceProjectionUseFiducialColor();
  d->pointUseFiducialColorCheckBox->setChecked(useFiducialColor);
  d->pointProjectionColorLabel->setEnabled(!useFiducialColor);
  d->pointProjectionColorPickerButton->setEnabled(!useFiducialColor);

  // -- Outlined Behind Slice Plane
  d->pointOutlinedBehindSlicePlaneCheckBox->setChecked(
     d->PointDisplayNode->GetSliceProjectionOutlinedBehindSlicePlane());

  // -- Opacity
  d->projectionOpacitySliderWidget->setValue(
     d->PointDisplayNode->GetSliceProjectionOpacity());
}
