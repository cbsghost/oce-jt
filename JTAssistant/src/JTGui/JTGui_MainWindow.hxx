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

#ifndef JTGUI_MAINWINDOW_H
#define JTGUI_MAINWINDOW_H

#pragma warning (push, 0)
#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QTreeWidget>
#include <QLabel>
#pragma warning (pop)

#include <JTCommon_Utils.hxx>

class JTGui_Window;

namespace Ui {
class MainWindow;
}

class JTData_Node;

//! Custom tree widget which is able to translate its key press events to another widget.
class JTGui_TreeWidget : public QTreeWidget
{
  Q_OBJECT

public:

  JTGui_TreeWidget (QWidget* theWidget = NULL)
    : QTreeWidget (theWidget)
  {}

  QModelIndex indexFromItem (QTreeWidgetItem* theItem, int theColumn = 0) const
  {
    return QTreeWidget::indexFromItem (theItem, theColumn);
  }

signals:

  void keyPressed (QKeyEvent* theEvent);

private:

  //! Handle key press events.
  void keyPressEvent (QKeyEvent* theEvent)
  {
    emit keyPressed (theEvent);

    QTreeWidget::keyPressEvent (theEvent);
  }
};

//! QLineEdit adapted for key binding.
class JTGui_KeyBindEdit : public QLineEdit
{
  Q_OBJECT

public:

  JTGui_KeyBindEdit();

  //! Handle key press events.
  void keyPressEvent (QKeyEvent* theEvent);

  //! Sets binded key and updates text.
  void setBindedKey (const Qt::KeyboardModifier theKey);

  //! Returns binded key.
  Qt::KeyboardModifier bindedKey() const { return myBindedKey; }

private:

  //! Cached binded key.
  Qt::KeyboardModifier myBindedKey;

};

//! Clickable label class.
class JTGui_ClickableLabel : public QLabel
{
  Q_OBJECT

public:
    explicit JTGui_ClickableLabel (QWidget * parent = 0);

signals:
    void clicked();

protected:
    void mouseReleaseEvent (QMouseEvent * event);
};

//! Main window object.
class JTGui_MainWindow : public QMainWindow
{
  Q_OBJECT

public:

  //! Initializes main window content.
  explicit JTGui_MainWindow (QWidget *parent = 0);

  //! Releases ui.
  ~JTGui_MainWindow();

  //! Passes command line args to main window.
  void setCmdArgs (const JTCommon_CmdArgs& theArgs);

public slots:

  //! Expands ans selects tree item representing given node.
  void selectNode (JTData_Node* theNode);

  //! Clears selection in Model browser.
  void clearSelection();

private slots:

  //! Closes JT file;
  void closeFile();

  //! Opens JT file.
  void openFile();

  //! Closes main window.
  void closeWindow();

  //! Shows / hides specific GUI items
  void showGUIItems();

  //! Calls FitAll method of scene.
  void fitAll();

  //! Updates info in a status bar.
  void updateStatusBar();

  //! Reacts to selection changing in Model browser.
  void selectionChanged();

  //! Collapses items in Model browser.
  void collapseAll();

  //! Hides selected items.
  void hideItem();

  //! Shows only selected items.
  void viewOnly();

  //! Shows all items.
  void viewAll();

  //! Turns camera to standard view.
  void viewReset();

  //! Turns camera to top view.
  void viewTop();

  //! Turns camera to bottom view.
  void viewBottom();

  //! Turns camera to left view.
  void viewLeft();

  //! Turns camera to right view.
  void viewRight();

  //! Turns camera to front view.
  void viewFront();

  //! Turns camera to back view.
  void viewBack();

  //! Reacts to settings changes.
  void updateSettings();

  //! Reacts to mouse layout changes.
  void updateMouseLayout();

  //! Changes camera mode to perspective and back to orthographic.
  void perspectiveMode (bool isEnabled);

  //! Reacts to preset selecting.
  void choosePreset();

  //! Makes screenshot and stores it to file specified in save dialog.
  void makeScreenshot();

  //! Enables forced rotate mode.
  void forceRotate();

  //! Enables forced pan mode.
  void forceMove();

  //! Enables forced zoom mode.
  void forceZoom();

  //! Resets forced modes.
  void resetForcedOperations();

  //! Shows "About" dialog.
  void showAbout();

  //! Reacts to selection color picking.
  void pickSelectionColor();

  //! Handles cmd arguments.
  void checkCmdArgs();

  //! Called then visible part of file is fully loaded.
  void fileLoaded();

  //! Shows context menu for Model browser.
  void showTreeContextMenu (const QPoint& thePoint);

  //! Shows context menu for 3d View (myRenderWindow).
  void showViewContextMenu (const QPoint& thePoint, bool& isSucceed);

private:

  //! Handles window events.
  bool event (QEvent * theEvent);

  //! Loads JT file into new scenegraph and creates 3d scene to visualize it.
  void loadFile (const QString& theFileName);

  //! Called when user attempts to close window.
  //! Stores application settings.
  void closeEvent (QCloseEvent *theEvent);

  //! Saves settings, GUI layout and GUI geometry.
  void saveSettings();

  //! Loads settings, GUI layout and GUI geometry.
  void loadSettings();

private:

  Ui::MainWindow *ui; //!< User inter face object generated from MainWindow.ui file.

  JTGui_Window* myRenderWindow; //!< Window for displaying 3d scene with OpenGL.

  bool myIgnoreNextSelectionUpdate; //!< If set to true forbids autoupdate of 3d scene selection. 
  bool isListeningItemChanges;      //!< If set to true forbids handling tree item changes. 
  bool isRightMouseButtonUsed;      //!< Indicates if right mouse button is used in current layout.

  JTGui_KeyBindEdit* myKeyBindEdit; //!< QLineEdit adapted for key binding.

  QColor myCurrentSelectionColor;   //!< Cached color from selection color picker.

  JTCommon_CmdArgs myCmdArgs;       //!< Command line arguments.
};

#endif // JTGUI_MAINWINDOW_H
