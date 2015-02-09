// JT format reading and visualization tools
// Copyright (C) 2015 OPEN CASCADE SAS
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2 of the License, or any later
// version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// Copy of the GNU General Public License is in LICENSE.txt and  
// on <http://www.gnu.org/licenses/>.

#include "JTGui_MainWindow.hxx"
#include "JTGui_Window.hxx"
#include "JTGui_AboutDialog.hxx"
#include "JTGui_SceneGraphTree.hxx"

#pragma warning (push, 0)
#include "gen/ui_MainWindow.h"

#include <QLayout>
#include <QSplitter>
#include <QSettings>
#include <QFileDialog>
#include <QTreeWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QKeyEvent>
#include <QButtonGroup>
#include <QComboBox>
#include <QColorDialog>
#include <QElapsedTimer>
#pragma warning (pop)

#include <JtData_Reader.hxx>
#include <JTData/JTData_SceneGraph.hxx>
#include <JTData/JTData_GeometrySource.hxx>
#include <JTVis/JTVis_Scene.hxx>
#include <JTData/JTData_Node.hxx>

//=======================================================================
// function : JTGui_KeyBindEdit
// purpose  :
//=======================================================================
JTGui_KeyBindEdit::JTGui_KeyBindEdit()
  : QLineEdit(),
    myBindedKey (JTGui_MouseLayout::NoKey)
{
  setText ("None");
}

//=======================================================================
// function : keyPressEvent
// purpose  :
//=======================================================================
void JTGui_KeyBindEdit::keyPressEvent (QKeyEvent* theEvent)
{
  switch (theEvent->key())
  {
    case Qt::Key_Control: setBindedKey (Qt::ControlModifier); break;
    case Qt::Key_Alt:     setBindedKey (Qt::AltModifier);     break;
    case Qt::Key_Shift:   setBindedKey (Qt::ShiftModifier);   break;
    default:
      setBindedKey (JTGui_MouseLayout::NoKey);
      break;
  }
}

//=======================================================================
// function : setBindedKey
// purpose  :
//=======================================================================
void JTGui_KeyBindEdit::setBindedKey (const Qt::KeyboardModifier theKey)
{
  myBindedKey = theKey;

  switch (theKey)
  {
    case Qt::ControlModifier:
      setText ("Ctrl");
      break;

    case Qt::AltModifier:
      setText ("Alt");
      break;

    case Qt::ShiftModifier:
      setText ("Shift");
      break;

    default:
      setText ("None");
      break;
  }
}

//=======================================================================
// function : JTGui_ClickableLabel
// purpose  :
//=======================================================================
JTGui_ClickableLabel::JTGui_ClickableLabel ( QWidget * theParent)
  : QLabel (theParent)
{
  //
}

//=======================================================================
// function : mouseReleaseEvent
// purpose  :
//=======================================================================
void JTGui_ClickableLabel::mouseReleaseEvent (QMouseEvent * theEvent)
{
  Q_UNUSED (theEvent)
  emit clicked();
}

#define LAYOUT_VERSION 1

//=======================================================================
// function : JTGui_MainWindow
// purpose  :
//=======================================================================
JTGui_MainWindow::JTGui_MainWindow (QWidget *parent) :
  QMainWindow (parent),
  ui (new Ui::MainWindow),
  myIgnoreNextSelectionUpdate (false),
  isListeningItemChanges (true),
  isRightMouseButtonUsed (false)
{
  ui->setupUi (this);
  ui->tabWidget->setCurrentIndex(0);

  myRenderWindow = new JTGui_Window();

  QWidget* aContainer = QWidget::createWindowContainer (myRenderWindow);
  aContainer->setMinimumSize (100, 100);
  aContainer->setFocusPolicy (Qt::ClickFocus);

  ui->myContainer->layout()->addWidget (aContainer);

  myKeyBindEdit = new JTGui_KeyBindEdit ();
  ui->myKeyBindLayout->addWidget (myKeyBindEdit);

  ui->myMenuWarningLabel->setVisible (false);

  ui->statusbar->setVisible (false);

  // Read settings before change signals connected
  loadSettings();

  // Set up action signals and slots
  connect (ui->actionOpen,        SIGNAL (triggered()), this, SLOT (openFile()));
  connect (ui->actionExit,        SIGNAL (triggered()), this, SLOT (closeWindow()));
  connect (ui->actionClose,       SIGNAL (triggered()), this, SLOT (closeFile()));
  connect (ui->actionFitAll,      SIGNAL (triggered()), this, SLOT (fitAll()));
  connect (ui->actionCollapseAll, SIGNAL (triggered()), this, SLOT (collapseAll()));
  connect (ui->actionHideItem,    SIGNAL (triggered()), this, SLOT (hideItem()));
  connect (ui->actionViewOnly,    SIGNAL (triggered()), this, SLOT (viewOnly()));
  connect (ui->actionViewAll,     SIGNAL (triggered()), this, SLOT (viewAll()));

  connect (ui->actionShowToolbar, SIGNAL (triggered()), this, SLOT (showGUIItems()));
  connect (ui->actionShowBrowser, SIGNAL (triggered()), this, SLOT (showGUIItems()));

  connect (ui->actionViewDefault, SIGNAL (triggered()), this, SLOT (viewReset()));

  connect (ui->actionViewBack,    SIGNAL (triggered()), this, SLOT (viewBack()));
  connect (ui->actionViewFront,   SIGNAL (triggered()), this, SLOT (viewFront()));
  connect (ui->actionViewLeft,    SIGNAL (triggered()), this, SLOT (viewLeft()));
  connect (ui->actionViewRight,   SIGNAL (triggered()), this, SLOT (viewRight()));
  connect (ui->actionViewTop,     SIGNAL (triggered()), this, SLOT (viewTop()));
  connect (ui->actionViewBottom,  SIGNAL (triggered()), this, SLOT (viewBottom()));

  connect (ui->actionEnablePerspective, SIGNAL (triggered (bool)), this, SLOT (perspectiveMode (bool)));
  connect (ui->actionShowAxes,          SIGNAL (triggered()), this, SLOT (updateSettings()));

  connect (ui->actionAbout,  SIGNAL (triggered()), this, SLOT (showAbout()));

  connect (ui->myTreeWidget, SIGNAL (itemSelectionChanged()), this, SLOT (selectionChanged()));

  connect (ui->lodQualitySlider,    SIGNAL (valueChanged (int)), this, SLOT (updateSettings()));
  connect (ui->viewCullingCheckBox, SIGNAL (clicked()),          this, SLOT (updateSettings()));
  connect (ui->sizeCullingCheckBox, SIGNAL (clicked()),          this, SLOT (updateSettings()));
  connect (ui->myAnimateCheck,      SIGNAL (clicked()),          this, SLOT (updateSettings()));
  connect (ui->myOsdCheck,          SIGNAL (clicked()),          this, SLOT (updateSettings()));

  connect (ui->mySelectionButtons,   SIGNAL (buttonClicked (int)),          this, SLOT (updateMouseLayout()));
  connect (ui->myRotationButtons,    SIGNAL (buttonClicked (int)),          this, SLOT (updateMouseLayout()));
  connect (ui->myPanningButtons,     SIGNAL (buttonClicked (int)),          this, SLOT (updateMouseLayout()));
  connect (ui->myZoomingButtons,     SIGNAL (buttonClicked (int)),          this, SLOT (updateMouseLayout()));
  connect (ui->myZoomWithWheelCheck, SIGNAL (stateChanged (int)),           this, SLOT (updateMouseLayout()));
  connect (myKeyBindEdit,            SIGNAL (textChanged (const QString&)), this, SLOT (updateMouseLayout()));

  connect (ui->myPresetComboBox, SIGNAL (activated (int)), this, SLOT (choosePreset()));

  connect (ui->actionScreenshot, SIGNAL (triggered()), this, SLOT (makeScreenshot()));
  connect (ui->actionRotate,     SIGNAL (triggered()), this, SLOT (forceRotate()));
  connect (ui->actionMove,       SIGNAL (triggered()), this, SLOT (forceMove()));
  connect (ui->actionZoom,       SIGNAL (triggered()), this, SLOT (forceZoom()));

  connect (myRenderWindow, SIGNAL (resetForcedOperations()), this, SLOT (resetForcedOperations()));

  // Translate key press events to myRenderWindow to avoid window focus problems
  connect (ui->myTreeWidget, SIGNAL (keyPressed (QKeyEvent*)), myRenderWindow, SLOT (keyPressEvent (QKeyEvent*)));

  connect (ui->menubar, SIGNAL (triggered (QAction*)), myRenderWindow, SLOT (requestActivate()));

  connect (ui->myColorPickButton, SIGNAL (clicked()), this, SLOT (pickSelectionColor()));
  connect (ui->myColorLabel,      SIGNAL (clicked()), this, SLOT (pickSelectionColor()));

  setMouseTracking (true);

  // Enable multiple selection in Model browser
  ui->myTreeWidget->setSelectionMode (QAbstractItemView::ExtendedSelection);
  ui->myTreeWidget->setContextMenuPolicy (Qt::CustomContextMenu);

  connect (ui->myTreeWidget, SIGNAL (customContextMenuRequested (const QPoint&)),
           this, SLOT (showTreeContextMenu (const QPoint&)));

  connect (myRenderWindow, SIGNAL (contextMenuRequested (const QPoint&, bool&)),
           this, SLOT (showViewContextMenu (const QPoint&, bool&)));

  QTimer* aTimer = new QTimer (this);
  connect (aTimer, SIGNAL (timeout()), this, SLOT (updateStatusBar()));
  aTimer->start (50);

  updateMouseLayout();

  QTimer::singleShot (10, this, SLOT (checkCmdArgs(void)));
}

//=======================================================================
// function : saveSettings
// purpose  :
//=======================================================================
void JTGui_MainWindow::saveSettings()
{
  QSettings aSettings (QSettings::IniFormat, QSettings::UserScope, "OpenCASCADE", "JTAssistant");

  aSettings.setValue ("geometry", saveGeometry());
  aSettings.setValue ("windowState", saveState (LAYOUT_VERSION));

  aSettings.setValue ("state/showBrowser", ui->actionShowBrowser->isChecked());
  aSettings.setValue ("state/showToolbar", ui->actionShowToolbar->isChecked());

  aSettings.setValue ("state/axes",        ui->actionShowAxes->isChecked());
  aSettings.setValue ("state/perspective", ui->actionEnablePerspective->isChecked());

  aSettings.setValue ("settings/view_culling", ui->viewCullingCheckBox->isChecked());
  aSettings.setValue ("settings/size_culling", ui->sizeCullingCheckBox->isChecked());

  aSettings.setValue ("settings/lod_quality", ui->lodQualitySlider->value());

  aSettings.setValue ("settings/camera_anim",    ui->myAnimateCheck->isChecked());
  aSettings.setValue ("settings/camera_autofit", ui->myAutoFitCheck->isChecked());

  aSettings.setValue ("settings/osd_visible", ui->myOsdCheck->isChecked());

  aSettings.setValue ("settings/selection_button", ui->mySelectionButtons->checkedId());
  aSettings.setValue ("settings/rot_button",       ui->myRotationButtons->checkedId());
  aSettings.setValue ("settings/pan_button",       ui->myPanningButtons->checkedId());
  aSettings.setValue ("settings/zoom_button",      ui->myZoomingButtons->checkedId());
  aSettings.setValue ("settings/zoom_wheel",       ui->myZoomWithWheelCheck->isChecked());

  aSettings.setValue ("settings/interact_key",    myKeyBindEdit->bindedKey());
  aSettings.setValue ("settings/selection_color", myCurrentSelectionColor);
}

//=======================================================================
// function : loadSettings
// purpose  :
//=======================================================================
void JTGui_MainWindow::loadSettings()
{
  QSettings aSettings (QSettings::IniFormat, QSettings::UserScope, "OpenCASCADE", "JTAssistant");

  restoreGeometry (aSettings.value ("geometry").toByteArray());
  restoreState (aSettings.value ("windowState").toByteArray(), LAYOUT_VERSION);

  // Show/hide specific GUI items
  bool toShowBrowser = aSettings.value ("state/showBrowser", true).toBool();
  bool toShowToolbar = aSettings.value ("state/showToolbar", true).toBool();

  ui->actionShowBrowser->setChecked (toShowBrowser);
  ui->tabWidget->setVisible (toShowBrowser);

  ui->actionShowToolbar->setChecked (toShowToolbar);
  ui->myToolBar->setVisible (toShowToolbar);

  ui->actionShowAxes->setChecked (aSettings.value ("state/axes", false).toBool());
  ui->actionEnablePerspective->setChecked (aSettings.value ("state/perspective", false).toBool());

  ui->viewCullingCheckBox->setChecked (aSettings.value ("settings/view_culling", true).toBool());
  ui->sizeCullingCheckBox->setChecked (aSettings.value ("settings/size_culling", true).toBool());

  ui->lodQualitySlider->setValue (aSettings.value ("settings/lod_quality", 50).toInt());

  ui->myAnimateCheck->setChecked (aSettings.value ("settings/camera_anim", true).toBool());
  ui->myAutoFitCheck->setChecked (aSettings.value ("settings/camera_autofit", true).toBool());

  ui->myOsdCheck->setChecked (aSettings.value ("settings/osd_visible", false).toBool());

  const QString aLabelBgStyle ("QLabel { background-color : %1; }");

  myCurrentSelectionColor = aSettings.value ("settings/selection_color", QColor (0, 255, 255)).value<QColor>();

  ui->myColorLabel->setStyleSheet (aLabelBgStyle.arg (myCurrentSelectionColor.name()));

  ui->mySelectionButtons->button (aSettings.value ("settings/selection_button", -2).toInt())->setChecked (true);

  if (aSettings.contains ("settings/rot_button"))
  {
    ui->myRotationButtons->button (aSettings.value ("settings/rot_button", -2).toInt())->setChecked (true);
    ui->myPanningButtons->button (aSettings.value ("settings/pan_button", -2).toInt())->setChecked (true);
    ui->myZoomingButtons->button (aSettings.value ("settings/zoom_button", -2).toInt())->setChecked (true);

    ui->myZoomWithWheelCheck->setChecked (aSettings.value ("settings/zoom_wheel", true).toBool());

    int anInteractKey = aSettings.value ("settings/interact_key", JTGui_MouseLayout::NoKey).toInt();
    switch (anInteractKey)
    {
      case Qt::ControlModifier :
      case Qt::ShiftModifier :
      case Qt::AltModifier :
        myKeyBindEdit->setBindedKey ((Qt::KeyboardModifier) anInteractKey);
        break;

      default:
        myKeyBindEdit->setBindedKey (Qt::NoModifier);
        break;
    }
  }
  else
  {
    ui->myPresetComboBox->setCurrentIndex (0);
    choosePreset();
  }
}

//=======================================================================
// function : ~JTGui_MainWindow
// purpose  :
//=======================================================================
JTGui_MainWindow::~JTGui_MainWindow()
{
  delete ui;
}

//=======================================================================
// function : event
// purpose  :
//=======================================================================
bool JTGui_MainWindow::event (QEvent * theEvent)
{
  switch (theEvent->type())
  {
    case QEvent::WindowActivate :
      // gained focus
      myRenderWindow->requestActivate();
      break;

    case QEvent::WindowDeactivate :
      // lost focus
      break;

    default:
      break;
  }
  return QMainWindow::event (theEvent) ;
}

//=======================================================================
// function : showGUIItems
// purpose  :
//=======================================================================
void JTGui_MainWindow::showGUIItems()
{
  // Show / hide toolbar
  if (ui->actionShowToolbar->isChecked())
    ui->myToolBar->setVisible (true);
  else
    ui->myToolBar->setVisible (false);

  // Show / hide object browser
  if (ui->actionShowBrowser->isChecked())
    ui->tabWidget->setVisible (true);
  else
    ui->tabWidget->setVisible (false);
}

//=======================================================================
// function : fitAll
// purpose  :
//=======================================================================
void JTGui_MainWindow::fitAll ()
{
  if (!myRenderWindow->scene().isNull())
  {
    myRenderWindow->scene()->FitAll();
  }
}

//=======================================================================
// function : closeFile
// purpose  :
//=======================================================================
void JTGui_MainWindow::closeFile()
{
  myRenderWindow->setScene (JTVis_ScenePtr());

  ui->myTreeWidget->clear();

  myRenderWindow->needToUpdate();

  setWindowTitle ("JTAssistant");
}

//=======================================================================
// function : loadFile
// purpose  :
//=======================================================================
void JTGui_MainWindow::loadFile (const QString& theFileName)
{
  if (theFileName.isEmpty())
    return;
  setCursor (Qt::BusyCursor);

  QCoreApplication::processEvents();

  JTData_GeometrySourcePtr aGeometrySource (new JTData_GeometrySource);

  myRenderWindow->setScene (JTVis_ScenePtr());

  ui->myTreeWidget->clear();

  if (myCmdArgs.DoBenchmarking)
    JTCommon_Profiler::GetProfiler().Start();

  if (!aGeometrySource->Init (theFileName))
  {
    setCursor (Qt::ArrowCursor);

    QMessageBox aMessageBox (QMessageBox::Critical, "JTAssistant", "Error! Failed to load specified JT file");

    aMessageBox.exec();

    //close scene
    myRenderWindow->setScene (JTVis_ScenePtr());
    ui->myTreeWidget->clear();
    myRenderWindow->needToUpdate();
    setWindowTitle ("JTAssistant");
  }
  else
  {
    JTVis_ScenePtr aNewScene (new JTVis_Scene());
    aNewScene->SetGeometrySource (aGeometrySource);

    myRenderWindow->setScene (aNewScene);

    connect (aNewScene.data(), SIGNAL (RequestSelection (JTData_Node*)), this, SLOT (selectNode (JTData_Node*)));
    connect (aNewScene.data(), SIGNAL (RequestClearSelection()),         this, SLOT (clearSelection()));
    connect (aNewScene.data(), SIGNAL (LoadingComplete()),               this, SLOT (fileLoaded()));

    ui->myTreeWidget->insertTopLevelItem (0, JTGui_SceneGraphTree::CreateTree (aGeometrySource->SceneGraph())->Root());

    if (ui->myTreeWidget->topLevelItem (0)->childCount() < 35)
    {
      ui->myTreeWidget->topLevelItem (0)->setExpanded (true);

      Standard_Integer aRows = 0;

      for (Standard_Integer anIdx = 0; anIdx < ui->myTreeWidget->topLevelItem (0)->childCount(); ++anIdx)
        aRows += ui->myTreeWidget->topLevelItem (0)->child (anIdx)->childCount();

      if (aRows < 35)
      {
        for (Standard_Integer anIdx = 0; anIdx < ui->myTreeWidget->topLevelItem (0)->childCount(); ++anIdx)
          ui->myTreeWidget->topLevelItem (0)->child (anIdx)->setExpanded (true);
      }
    }

    QFileInfo anInfo (theFileName);
    setWindowTitle (anInfo.fileName() + " - JTAssistant");

    updateSettings();
  }

  if (myCmdArgs.DoBenchmarking)
  {
    JTCommon_Profiler::GetProfiler().WriteElapsed ("loading");
    JTVis_Settings& aSettings = myRenderWindow->scene()->ChangeSettings();
    aSettings.IsBenchmarkingMode = true;
  }

  myRenderWindow->requestActivate();
}

//=======================================================================
// function : openFile
// purpose  :
//=======================================================================
void JTGui_MainWindow::openFile()
{
  QSettings aSettings (QSettings::IniFormat, QSettings::UserScope, "OpenCASCADE", "JTAssistant");

  QString aFileName = QFileDialog::getOpenFileName (this, tr ("Open JT File"),
    aSettings.value ("DefaultDir").toString(), tr ("JT Files (*.jt)"));

  if (!aFileName.isEmpty()) 
  {
    QDir aDir = QFileInfo (aFileName).absoluteDir();
    aSettings.setValue ("DefaultDir", aDir.absolutePath());

    loadFile (aFileName);
  }
}

//=======================================================================
// function : selectNode
// purpose  :
//=======================================================================
void JTGui_MainWindow::selectNode (JTData_Node* theNode)
{
  JTGui_SceneGraphTreePtr aTree = dynamic_cast<JTGui_SceneGraphItem*> (ui->myTreeWidget->topLevelItem (0))->tree();
  JTGui_SceneGraphItem* anItem = aTree->Item (theNode);

  myIgnoreNextSelectionUpdate = true;

  ui->myTreeWidget->setItemSelected (anItem, true);
  ui->myTreeWidget->scrollTo (ui->myTreeWidget->indexFromItem (anItem));

  while (anItem != NULL)
  {
    anItem->setExpanded (true);
    anItem = anItem->parent();
  }
}

//=======================================================================
// function : showExitPromptDialog
// purpose  :
//=======================================================================
static QMessageBox::StandardButton showExitPromptDialog (JTGui_MainWindow* aParent)
{
  return QMessageBox::question (aParent,
                                "JTAssistant",
                                "Do you really want to quit?",
                                QMessageBox::Yes | QMessageBox::No);
}

//=======================================================================
// function : closeWindow
// purpose  :
//=======================================================================
void JTGui_MainWindow::closeWindow()
{
  if (showExitPromptDialog (this) == QMessageBox::Yes)
  {
    saveSettings();
    qApp->exit();
  }
}

//=======================================================================
// function : closeEvent
// purpose  :
//=======================================================================
void JTGui_MainWindow::closeEvent (QCloseEvent *theEvent)
{
  if (showExitPromptDialog (this) == QMessageBox::Yes)
  {
    saveSettings();
    QMainWindow::closeEvent (theEvent);
  }
  else
  {
    theEvent->ignore();
  }
}

//=======================================================================
// function : showTreeContextMenu
// purpose  :
//=======================================================================
void JTGui_MainWindow::showTreeContextMenu (const QPoint& thePoint)
{
  QPoint aGlobalPos = ui->myTreeWidget->mapToGlobal (thePoint);

  QMenu aMenu;
  aMenu.addAction (ui->actionHideItem);
  aMenu.addAction (ui->actionViewOnly);
  aMenu.addAction (ui->actionViewAll);
  aMenu.addSeparator();
  aMenu.addAction (ui->actionCollapseAll);

  aMenu.exec (aGlobalPos);
}

//=======================================================================
// function : showViewContextMenu
// purpose  :
//=======================================================================
void JTGui_MainWindow::showViewContextMenu (const QPoint& thePoint, bool& isSucceed)
{
  if (isRightMouseButtonUsed)
    return;

  QPoint aGlobalPos = myRenderWindow->mapToGlobal (thePoint);

  QMenu aMenu;
  aMenu.addAction (ui->actionFitAll);
  aMenu.addSeparator();
  aMenu.addAction (ui->actionHideItem);
  aMenu.addAction (ui->actionViewOnly);
  aMenu.addAction (ui->actionViewAll);

  aMenu.exec (aGlobalPos);

  myRenderWindow->requestActivate();

  isSucceed = true;
}

//=======================================================================
// function : updateSettings
// purpose  :
//=======================================================================
void JTGui_MainWindow::updateSettings()
{
  if (myRenderWindow->scene().isNull())
    return;

  JTVis_Settings& aSettings = myRenderWindow->scene()->ChangeSettings();
  aSettings.IsViewCullingEnabled = ui->viewCullingCheckBox->isChecked();
  aSettings.IsSizeCullingEnabled = ui->sizeCullingCheckBox->isChecked();
  aSettings.LodQuality = 0.5f + 4.f * ui->lodQualitySlider->value() / (float) ui->lodQualitySlider->maximum();
  aSettings.IsTrihedronVisible = ui->actionShowAxes->isChecked();
  aSettings.IsStatsOsdVisible = ui->myOsdCheck->isChecked();
  aSettings.IsCameraAnimated = ui->myAnimateCheck->isChecked();
  aSettings.SelectionColor = myCurrentSelectionColor;

  myRenderWindow->scene()->SetCameraMode (ui->actionEnablePerspective->isChecked() ? cmPerspective : cmOrthographic);
  myRenderWindow->needToUpdate();

  myRenderWindow->requestActivate();
}

//=======================================================================
// function : updateMouseLayout
// purpose  :
//=======================================================================
void JTGui_MainWindow::updateMouseLayout()
{
  struct Helper 
  {
    static Qt::MouseButton idToMouseButton (int theId)
    {
      switch (theId)
      {
      case -2:
        return Qt::LeftButton;
      case -3:
        return Qt::MiddleButton;
      case -4:
        return Qt::RightButton;

      default:
        return Qt::LeftButton;
      }
    }
  };

  JTGui_MouseLayout aMouseLayout = myRenderWindow->mouseLayout();

  aMouseLayout.SelectionButton = Helper::idToMouseButton (ui->mySelectionButtons->checkedId());
  aMouseLayout.RotateButton = Helper::idToMouseButton (ui->myRotationButtons->checkedId());
  aMouseLayout.PanButton = Helper::idToMouseButton (ui->myPanningButtons->checkedId());
  aMouseLayout.ZoomButton = Helper::idToMouseButton (ui->myZoomingButtons->checkedId());

  aMouseLayout.ZoomWithWheel = ui->myZoomWithWheelCheck->checkState() == Qt::Checked;

  aMouseLayout.InteractKey = myKeyBindEdit->bindedKey();

  isRightMouseButtonUsed = aMouseLayout.RotateButton == Qt::RightButton
                        || aMouseLayout.PanButton == Qt::RightButton
                        || (aMouseLayout.ZoomButton == Qt::RightButton && (!aMouseLayout.ZoomWithWheel));

  isRightMouseButtonUsed = (isRightMouseButtonUsed && aMouseLayout.InteractKey == JTGui_MouseLayout::NoKey)
                          || aMouseLayout.SelectionButton == Qt::RightButton;

  myRenderWindow->setMouseLayout (aMouseLayout);

  if (isRightMouseButtonUsed)
  {
    ui->myMenuWarningLabel->setVisible (true);
  }
  else
  {
    ui->myMenuWarningLabel->setVisible (false);
  }

  bool isFewActionsAssignedToSameButton = 
        aMouseLayout.RotateButton == aMouseLayout.PanButton ||
      ((aMouseLayout.RotateButton == aMouseLayout.ZoomButton) && !aMouseLayout.ZoomWithWheel) ||
      ((aMouseLayout.PanButton    == aMouseLayout.ZoomButton) && !aMouseLayout.ZoomWithWheel);

  if (isFewActionsAssignedToSameButton)
  {
    ui->mySameButtonWarningLabel->setVisible (true);
  }
  else
  {
    ui->mySameButtonWarningLabel->setVisible (false);
  }

  foreach (QAbstractButton* aButton, ui->myZoomingButtons->buttons())
  {
    aButton->setEnabled (ui->myZoomWithWheelCheck->checkState() != Qt::Checked);
  }

  myRenderWindow->requestActivate();
}

//=======================================================================
// function : updateStatusBar
// purpose  :
//=======================================================================
void JTGui_MainWindow::updateStatusBar()
{
  if (!myRenderWindow->scene().isNull())
  {
    JTVis_Stats aStats = myRenderWindow->scene()->RenderStats();
    float aRatio = aStats.FullTriangleCount == 0 ? 0 :
      100.f * (aStats.FullTriangleCount - aStats.VisibleTriangleCount) / aStats.FullTriangleCount;


    ui->statusbar->showMessage (tr ("Visible triangles: %1    Culled triangles: %2    %3% Culled    %4 Vis parts    %5% Small-part usage")
      .arg (aStats.VisibleTriangleCount)
      .arg (aStats.FullTriangleCount - aStats.VisibleTriangleCount)
      .arg ((int ) Round (aRatio))
      .arg (aStats.VisiblePartCount)
      .arg (aStats.SmallPartBufferUsage));
  }
  else
  {
    ui->statusbar->clearMessage();
  }
}

//=======================================================================
// function : clearSelection
// purpose  :
//=======================================================================
void JTGui_MainWindow::clearSelection()
{

  ui->myTreeWidget->clearSelection();

}

//=======================================================================
// function : selectionChanged
// purpose  :
//=======================================================================
void JTGui_MainWindow::selectionChanged()
{
  if (!myRenderWindow->scene().isNull())
  {
    if (myIgnoreNextSelectionUpdate)
    {
      myIgnoreNextSelectionUpdate = false;
      return;
    }

    myRenderWindow->scene()->ClearSelection();
    foreach (QTreeWidgetItem* anItem, ui->myTreeWidget->selectedItems())
    {
      QVariant aData = anItem->data (0, Qt::UserRole);

      if (aData.data() != NULL)
      {
        JTData_NodePtr aNode =
          anItem->data (0, Qt::UserRole).value<JTData_NodePtr>();

        myRenderWindow->scene()->SelectNode (JTData_NodePtr (aNode), true);
      }
    }
  }
}

//=======================================================================
// function : collapseAll
// purpose  :
//=======================================================================
void JTGui_MainWindow::collapseAll()
{
  ui->myTreeWidget->collapseAll();
}

//=======================================================================
// function : updateStatusBar
// purpose  :
//=======================================================================
void JTGui_MainWindow::hideItem()
{
  foreach (QTreeWidgetItem* anItem, ui->myTreeWidget->selectedItems())
  {
    anItem->setCheckState (0, Qt::Unchecked);
  }
}

//=======================================================================
// function : viewOnly
// purpose  :
//=======================================================================
void JTGui_MainWindow::viewOnly()
{
  if (ui->myTreeWidget->selectedItems().size() == 0)
    return;

  QTreeWidgetItem* aRoot = ui->myTreeWidget->invisibleRootItem()->child (0);

  aRoot->setCheckState (0, Qt::Unchecked);

  foreach (QTreeWidgetItem* anItem, ui->myTreeWidget->selectedItems())
  {
    anItem->setCheckState (0, Qt::Checked);
  }

  if (!myRenderWindow->scene().isNull() && ui->myAutoFitCheck->isChecked())
  {
    myRenderWindow->scene()->FitAll (fmFitSelected);
  }
}

//=======================================================================
// function : viewAll
// purpose  :
//=======================================================================
void JTGui_MainWindow::viewAll()
{
  if (!ui->myTreeWidget->topLevelItemCount ())
    return;

  QTreeWidgetItem* aRoot = ui->myTreeWidget->invisibleRootItem()->child (0);

  aRoot->setCheckState (0, Qt::Checked);

  if (!myRenderWindow->scene().isNull() && ui->myAutoFitCheck->isChecked())
  {
    myRenderWindow->scene()->FitAll (fmFitAll);
  }
}

//=======================================================================
// function : viewReset
// purpose  :
//=======================================================================
void JTGui_MainWindow::viewReset()
{
  if (myRenderWindow->scene().isNull())
    return;

  if (!myRenderWindow->scene().isNull())
  {
    myRenderWindow->scene()->FitAll (fmFitAll, svIso);
  }
}

//=======================================================================
// function : viewTop
// purpose  :
//=======================================================================
void JTGui_MainWindow::viewTop()
{
  if (myRenderWindow->scene().isNull())
    return;

  myRenderWindow->scene()->SetCameraStandardView (svTop);
}

//=======================================================================
// function : viewBottom
// purpose  :
//=======================================================================
void JTGui_MainWindow::viewBottom()
{
  if (myRenderWindow->scene().isNull())
    return;

  myRenderWindow->scene()->SetCameraStandardView (svBottom);
}

//=======================================================================
// function : viewLeft
// purpose  :
//=======================================================================
void JTGui_MainWindow::viewLeft()
{
  if (myRenderWindow->scene().isNull())
    return;

  myRenderWindow->scene()->SetCameraStandardView (svLeft);
}

//=======================================================================
// function : viewRight
// purpose  :
//=======================================================================
void JTGui_MainWindow::viewRight()
{
  if (myRenderWindow->scene().isNull())
    return;

  myRenderWindow->scene()->SetCameraStandardView (svRight);
}

//=======================================================================
// function : viewFront
// purpose  :
//=======================================================================
void JTGui_MainWindow::viewFront()
{
  if (myRenderWindow->scene().isNull())
    return;

  myRenderWindow->scene()->SetCameraStandardView (svFront);
}

//=======================================================================
// function : viewBack
// purpose  :
//=======================================================================
void JTGui_MainWindow::viewBack()
{
  if (myRenderWindow->scene().isNull())
    return;

  myRenderWindow->scene()->SetCameraStandardView (svBack);
}

//=======================================================================
// function : perspectiveMode
// purpose  :
//=======================================================================
void JTGui_MainWindow::perspectiveMode (bool isEnabled)
{
  if (myRenderWindow->scene().isNull())
    return;

  myRenderWindow->scene()->SetCameraMode (isEnabled ? cmPerspective : cmOrthographic);
}

//=======================================================================
// function : choosePreset
// purpose  :
//=======================================================================
void JTGui_MainWindow::choosePreset()
{
  struct Helper 
  {
    static int mouseButtonToId (Qt::MouseButton theButton)
    {
      switch (theButton)
      {
      case Qt::LeftButton:
        return -2;
      case Qt::MiddleButton:
        return -3;
      case Qt::RightButton:
        return -4;

      default:
        return -2;
      }
    }
  };

  // Exclude selection button from presets.
  Qt::MouseButton anOldSelectionButton = myRenderWindow->mouseLayout().SelectionButton;

  if (ui->myPresetComboBox->currentText() == "Default")
  {
    JTGui_MouseLayout aNewLayout;
    aNewLayout.SelectionButton = anOldSelectionButton;
    myRenderWindow->setMouseLayout (aNewLayout);
  }
  else if (ui->myPresetComboBox->currentText() == "OCCT")
  {
    JTGui_MouseLayout aNewLayout (Qt::LeftButton,
                                  Qt::RightButton,
                                  Qt::MiddleButton,
                                  Qt::LeftButton,
                                  Qt::ControlModifier,
                                  false);

    aNewLayout.SelectionButton = anOldSelectionButton;
    myRenderWindow->setMouseLayout (aNewLayout);
  }
  else
  {
    return;
  }

  const JTGui_MouseLayout& aMouseLayout = myRenderWindow->mouseLayout();

  ui->mySelectionButtons->button (Helper::mouseButtonToId (aMouseLayout.SelectionButton))->setChecked (true);
  ui->myRotationButtons->button (Helper::mouseButtonToId (aMouseLayout.RotateButton))->setChecked (true);
  ui->myPanningButtons->button (Helper::mouseButtonToId (aMouseLayout.PanButton))->setChecked (true);
  ui->myZoomingButtons->button (Helper::mouseButtonToId (aMouseLayout.ZoomButton))->setChecked (true);

  myKeyBindEdit->setBindedKey (aMouseLayout.InteractKey);

  ui->myZoomWithWheelCheck->setChecked (aMouseLayout.ZoomWithWheel);
  updateMouseLayout();

}

//=======================================================================
// function : makeScreenshot
// purpose  :
//=======================================================================
void JTGui_MainWindow::makeScreenshot()
{
  if (!myRenderWindow->scene().isNull())
  {
    QSettings aSettings (QSettings::IniFormat, QSettings::UserScope, "OpenCASCADE", "JTAssistant");

    // open native file dialog to query file path for saving the screenshot
    QString aFileName = QFileDialog::getSaveFileName (this, tr ("Save image to"),
      aSettings.value ("DefaultScreenshotDir").toString(), tr ("Image files (*.png)"));

    if (!aFileName.isEmpty())
    {
      // native file dialog does not append default suffix on Linux
      QString aFileSuffix = QFileInfo (aFileName).suffix();
      if (aFileSuffix != "png")
      {
        aFileName.append (".png");
      }

      QDir aDir;
      aSettings.setValue ("DefaultScreenshotDir", aDir.absoluteFilePath (aFileName));

      myRenderWindow->scene()->MakeScreenshot (aFileName);
    }
  }
}

//=======================================================================
// function : forceRotate
// purpose  :
//=======================================================================
void JTGui_MainWindow::forceRotate()
{
  if (myRenderWindow->scene().isNull())
    return;

  if (myRenderWindow->viewOperation() == voRotate)
  {
    myRenderWindow->setViewOperation (voNone);
    resetForcedOperations();
  }
  else
  {
    myRenderWindow->setViewOperation (voRotate);

    ui->actionRotate->setChecked (true);
    ui->actionMove->setChecked (false);
    ui->actionZoom->setChecked (false);
  }
}

//=======================================================================
// function : forceMove
// purpose  :
//=======================================================================
void JTGui_MainWindow::forceMove()
{
  if (myRenderWindow->scene().isNull())
    return;

  if (myRenderWindow->viewOperation() == voPan)
  {
    myRenderWindow->setViewOperation (voNone);
    resetForcedOperations();
  }
  else
  {
    myRenderWindow->setViewOperation (voPan);

    ui->actionMove->setChecked (true);
    ui->actionRotate->setChecked (false);
    ui->actionZoom->setChecked (false);
  }
}

//=======================================================================
// function : forceZoom
// purpose  :
//=======================================================================
void JTGui_MainWindow::forceZoom()
{
  if (myRenderWindow->scene().isNull())
    return;

  if (myRenderWindow->viewOperation() == voZoom)
  {
    myRenderWindow->setViewOperation (voNone);
    resetForcedOperations();
  }
  else
  {
    myRenderWindow->setViewOperation (voZoom);
    ui->actionZoom->setChecked (true);
    ui->actionRotate->setChecked (false);
    ui->actionMove->setChecked (false);
  }
}

//=======================================================================
// function : resetForcedOperations
// purpose  :
//=======================================================================
void JTGui_MainWindow::resetForcedOperations()
{
  ui->actionRotate->setChecked (false);
  ui->actionMove->setChecked (false);
  ui->actionZoom->setChecked (false);
}

//=======================================================================
// function : fileLoaded
// purpose  :
//=======================================================================
void JTGui_MainWindow::fileLoaded()
{
  setCursor (Qt::ArrowCursor);
}

//=======================================================================
// function : showAbout
// purpose  :
//=======================================================================
void JTGui_MainWindow::showAbout()
{
  JTGui_AboutDialog aDialog (JTData_GeometrySource::OcctVersion(), this);
  aDialog.exec();
}

//=======================================================================
// function : pickSelectionColor
// purpose  :
//=======================================================================
void JTGui_MainWindow::pickSelectionColor()
{
  QColorDialog aColorDialog (myCurrentSelectionColor, this);
  aColorDialog.setOption (QColorDialog::DontUseNativeDialog);

  if (aColorDialog.exec())
  {
    const QString aLabelBgStyle ("QLabel { background-color : %1; }");

    myCurrentSelectionColor = aColorDialog.selectedColor();
    ui->myColorLabel->setStyleSheet (aLabelBgStyle.arg (myCurrentSelectionColor.name()));
    updateSettings();
  }
}

//=======================================================================
// function : setParams
// purpose  :
//=======================================================================
void JTGui_MainWindow::setCmdArgs (const JTCommon_CmdArgs& theArgs)
{
  myCmdArgs = theArgs;
}

//=======================================================================
// function : checkCmdArgs
// purpose  :
//=======================================================================
void JTGui_MainWindow::checkCmdArgs()
{
  if (myCmdArgs.FileName != "")
  {
    loadFile (myCmdArgs.FileName);
  }
}
