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

#include "qSlicerMarkupsToolBarPlugin.h"
#include "qSlicerMarkupsToolBar.h"

qSlicerMarkupsToolBarPlugin::qSlicerMarkupsToolBarPlugin(QObject* parent)
  : qSlicerQTGUIAbstractPlugin(parent)
{
}

QWidget *qSlicerMarkupsToolBarPlugin::createWidget(QWidget* parentWidget)
{
  qSlicerMarkupsToolBar* widget = new qSlicerMarkupsToolBar(parentWidget);
  return widget;
}

QString qSlicerMarkupsToolBarPlugin::domXml() const
{
  return "<widget class=\"qSlicerMarkupsToolBar\" \
          name=\"SlicerMarkupsToolBar\">\n"
          "</widget>\n";
}

QString qSlicerMarkupsToolBarPlugin::includeFile() const
{
  return "qSlicerMarkupsToolBar.h";
}

bool qSlicerMarkupsToolBarPlugin::isContainer() const
{
  return false;
}

QString qSlicerMarkupsToolBarPlugin::name() const
{
  return "qSlicerMarkupsToolBar";
}
