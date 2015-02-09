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

#ifndef JTGui_SceneGraphTree_HeaderFile
#define JTGui_SceneGraphTree_HeaderFile

#pragma warning (push, 0)
#include <QStandardItem>
#include <QStandardItemModel>
#include <QPersistentModelIndex>
#include <QSet>
#pragma warning (pop)

#include <JTData_SceneGraph.hxx>
#include <JTData_Node.hxx>

//! Forward declaration.
class JTGui_SceneGraphItem;
class JTGui_SceneGraphTree;

//! Define short-cut for smart pointer type.
typedef QSharedPointer<JTGui_SceneGraphTree> JTGui_SceneGraphTreePtr;


//! This class provides Qt items to represent scene graph (LSG) in tree widget.
//! It supplies general information about the tree : root item, complete item list,
//! map of item to data node associations. This tree is shared between the items
//! and can be accessed through them.
class JTGui_SceneGraphTree
{
public:

  //! Creates item tree class.
  JTGui_SceneGraphTree();

public:

  //! Creates Qt items tree for the given scene graph.
  static JTGui_SceneGraphTreePtr CreateTree (JTData_SceneGraph* theGraph);

  //! Returns complete list of Qt tree view items.
  QSet<JTGui_SceneGraphItem*> Items() const
  {
    return myItems.values().toSet();
  }

  //! Returns Qt tree view item created for a give scene graph node.
  JTGui_SceneGraphItem* Item (const JTData_NodePtr& theNode) const
  {
    return myItems.value (theNode.data(), NULL);
  }

  //! Returns Qt tree view item created for a give scene graph node or a sub-part.
  JTGui_SceneGraphItem* Item (JTData_Node* theNode) const
  {
    return myItems.value (theNode, NULL);
  }

  //! Returns top level Qt tree view item.
  JTGui_SceneGraphItem* Root() const
  {
    return myRoot;
  }

  //! Returns top level Qt tree view item.
  operator JTGui_SceneGraphItem*() const
  {
    return Root();
  }

  //! Returns scene graph node associated with the Qt tree view item.
  JTData_NodePtr Node (const JTGui_SceneGraphItem* theItem) const;

private:

  //! Recursively creates tree of Qt tree view items based on scenegraph.
  static JTGui_SceneGraphItem* CreateTreeItem (const JTGui_SceneGraphTreePtr& theTree,
                                               const JTData_NodePtr& theNode);

  //! Recursively collects all children for a given group.
  static void CollectChildren (JTData_GroupNode* theGroup, JTData_NodeArray& theChildren);

private:

  JTGui_SceneGraphItem*                     myRoot;  //!< Root tree view item.
  QMap<JTData_Node*, JTGui_SceneGraphItem*> myItems; //!< Map of nodes to items for quick lookup.
};

//! Qt item for specific tree view model. The item represents
//! scene graph node and contains custom attributes specific
//! for user interface implementation: "is selected" and
//! "is expanded" properties.
class JTGui_SceneGraphItem : public QTreeWidgetItem
{

public:

  //! Creates new tree view item for a given scene graph node.
  JTGui_SceneGraphItem (JTGui_SceneGraphTreePtr theTree,
                        JTData_NodePtr theNode)
    : mySceneGraphTree (theTree),
      mySceneGraphNode (theNode),
      myCheckUpdatesOn (true)
  {}

public:

  //! Returns reference to an items tree.
  const JTGui_SceneGraphTreePtr& tree() const
  {
    return mySceneGraphTree;
  }

  //! Returns reference to an associated scene graph node.
  const JTData_NodePtr& node() const
  {
    return mySceneGraphNode;
  }

  //! Sets data for the item.
  virtual void setData (int theColumn, int theRole, const QVariant &theValue)
  {
    QTreeWidgetItem::setData (theColumn, theRole, theValue);

    if (myCheckUpdatesOn && theRole == Qt::CheckStateRole)
    {
      updateCheckStates();
    }
  }

  //! Updates selection of the items in Model Tree
  void updateCheckStates();

  //! Sets data for the item.
  void setData (const QVariant &theValue, const int theRole)
  {
    QTreeWidgetItem::setData (0, theRole, theValue);
  }

  //! Return item data.
  QVariant data (const int theRole) const
  {
    return QTreeWidgetItem::data (0, theRole);
  }

  //! Sets item text.
  void setText (const QString& theText)
  {
    QTreeWidgetItem::setText (0, theText);
  }

  //! Returns item text.
  QString text() const
  {
    return QTreeWidgetItem::text (0);
  }

  //! Sets item icon.
  void setIcon (const QIcon& theIcon)
  {
    QTreeWidgetItem::setIcon (0, theIcon);
  }

  //! Returns item icon.
  QIcon icon() const
  {
    return QTreeWidgetItem::icon (0);
  }

  //! Sets item enabled state.
  void setEnabled (const bool theIsEnabled)
  {
    QTreeWidgetItem::setDisabled (!theIsEnabled);
  }

  //! Checks if the item is enabled.
  bool isEnabled() const
  {
    return !QTreeWidgetItem::isDisabled();
  }

  //! Sets check state for the item.
  void setCheckState (const Qt::CheckState theState)
  {
    QTreeWidgetItem::setCheckState (0, theState);
  }

  //! Returns check state of the item.
  Qt::CheckState checkState() const
  {
    return QTreeWidgetItem::checkState (0);
  }

  //! Returns parent of an item.
  JTGui_SceneGraphItem* parent() const
  {
    return dynamic_cast<JTGui_SceneGraphItem*> (QTreeWidgetItem::parent());
  }

  //! Returns a child of an item.
  JTGui_SceneGraphItem* child(const int theIndex) const
  {
    return dynamic_cast<JTGui_SceneGraphItem*> (QTreeWidgetItem::child (theIndex));
  }

private:

  //! Updates parent of the specified Model Tree item
  static void updateParents (JTGui_SceneGraphItem* theItem);

  //! Updates children of the specified Model Tree item
  static void updateChildren (JTGui_SceneGraphItem* theParent);

private:

  JTGui_SceneGraphTreePtr mySceneGraphTree; //!< Reference to an owner scene graph tree.
  JTData_NodePtr          mySceneGraphNode; //!< Reference to an associated scene graph node.
  bool                    myCheckUpdatesOn; //!< Turns on / off automatic check state updates.
};

#endif
