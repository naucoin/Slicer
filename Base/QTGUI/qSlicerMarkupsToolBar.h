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

#ifndef __qSlicerMarkupsToolBar_h
#define __qSlicerMarkupsToolBar_h

// Qt includes
#include <QToolBar>
#include <QMenu>

// CTK includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerMarkupsToolBarPrivate;
class vtkMRMLScene;
class vtkSlicerApplicationLogic;

///
/// qSlicerMarkupsToolBar is a toolbar that can be used to switch between 
/// mouse modes: PickMode, PickModePersistent, PlaceMode, PlaceModePersistent, TransformMode
/// \note The toolbar expects qSlicerCoreApplication::mrmlApplicationLogic() to return a valid object
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerMarkupsToolBar: public QToolBar
{
  Q_OBJECT
public:
  typedef QToolBar Superclass;

  /// Constructor
  /// Title is the name of the toolbar (can appear using right click on the toolbar area)
  qSlicerMarkupsToolBar(const QString& title, QWidget* parent = 0);
  qSlicerMarkupsToolBar(QWidget* parent = 0);
  virtual ~qSlicerMarkupsToolBar();

public slots:

  void setApplicationLogic(vtkSlicerApplicationLogic* logic);

  void setMRMLScene(vtkMRMLScene* newScene);

  void switchToViewTransformMode();

  void changeCursorTo(QCursor cursor);

  /// Switch to placing items of markupID type
  void switchPlaceMode();

  /// Update the interaction node's persistent place mode from the UI
  void onPersistenceToggled();

  /// For testing, return the active action text
  QString activeActionText();
  
protected:
  QScopedPointer<qSlicerMarkupsToolBarPrivate> d_ptr;

  bool isActionTextInMenu(QString actionText, QMenu *menu);
private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsToolBar);
  Q_DISABLE_COPY(qSlicerMarkupsToolBar);
};

#endif
