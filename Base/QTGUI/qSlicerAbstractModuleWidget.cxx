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

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerAbstractModuleWidgetPrivate
{
public:
  qSlicerAbstractModuleWidgetPrivate();
  bool IsEntered;
};

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidgetPrivate::qSlicerAbstractModuleWidgetPrivate()
{
  this->IsEntered = false;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget::qSlicerAbstractModuleWidget(QWidget* parentWidget)
  : qSlicerWidget(parentWidget)
  , d_ptr(new qSlicerAbstractModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget::~qSlicerAbstractModuleWidget()
{
  if (this->isEntered() == true)
    {
    qCritical() << "Abstract module widget destructor: isEntered is true!";
    return;
    }
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::enter()
{
  Q_D(qSlicerAbstractModuleWidget);
  if (d->IsEntered == true)
    {
    qCritical() << "enter: isEntered is already true!";
    return;
    }
  d->IsEntered = true;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::exit()
{
  Q_D(qSlicerAbstractModuleWidget);
  if (d->IsEntered == false)
    {
    qCritical() << "exit: isEntered is already false!";
    return;
    }
  d->IsEntered = false;
}

//-----------------------------------------------------------------------------
bool qSlicerAbstractModuleWidget::isEntered()const
{
  Q_D(const qSlicerAbstractModuleWidget);
  return d->IsEntered;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::setup()
{
  const qSlicerAbstractModule* m =
    qobject_cast<const qSlicerAbstractModule*>(this->module());
  if (m)
    {
    this->setObjectName(QString("%1ModuleWidget").arg(m->name()));
    this->setWindowTitle(m->title());
    this->setWindowIcon(m->icon());
    }
}
