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

#include "JTGui_SceneGraphTree.hxx"

#include <typeinfo>

//=======================================================================
// function : JTGui_SceneGraphTree
// purpose  :
//=======================================================================
JTGui_SceneGraphTree::JTGui_SceneGraphTree()
: myRoot (NULL)
{
}

//=======================================================================
// function : JTGui_SceneGraphTree
// purpose  :
//=======================================================================
JTGui_SceneGraphTreePtr JTGui_SceneGraphTree::CreateTree (JTData_SceneGraph* theSceneGraph)
{
  JTGui_SceneGraphTreePtr aTree = JTGui_SceneGraphTreePtr (new JTGui_SceneGraphTree());

  aTree->myRoot = CreateTreeItem (aTree, theSceneGraph->Tree());

  return aTree;
}

// =======================================================================
// function : CreateTreeItem
// purpose  :
// =======================================================================
JTGui_SceneGraphItem* JTGui_SceneGraphTree::CreateTreeItem (const JTGui_SceneGraphTreePtr& theTree,
                                                            const JTData_NodePtr& theNode)
{
  JTGui_SceneGraphItem* anItem = new JTGui_SceneGraphItem (theTree, theNode);

  theTree->myItems.insert (theNode.data(), anItem);
  anItem->setData (QVariant::fromValue (theNode), Qt::UserRole);
  anItem->setCheckState (Qt::Checked); // checked by default
  anItem->setExpanded (false);

  // classify node to determine its text and icon
  if (typeid (*theNode.data()) == typeid (JTData_PartNode))
  {
    anItem->setText ("Part");
    anItem->setIcon (QIcon (":/desktop/res/icons/desktop/part.png"));
  }
  else if (typeid (*theNode.data()) == typeid (JTData_PartitionNode))
  {
    anItem->setText ("Partition");
    anItem->setIcon (QIcon (":/desktop/res/icons/desktop/group.png"));
  }
  else if (typeid (*theNode.data()) == typeid (JTData_GroupNode))
  {
    anItem->setText ("Group");
    anItem->setIcon (QIcon (":/desktop/res/icons/desktop/group.png"));
  }

  // merge part with its hidden subnodes - the part is a mininal element in tree
  if (JTData_PartNode* aPart = dynamic_cast<JTData_PartNode*> (theNode.data()))
  {
    JTData_NodeArray aSubnodes;

    CollectChildren (aPart, aSubnodes);

    for (size_t anIdx = 0; anIdx < aSubnodes.size(); ++anIdx)
    {
      theTree->myItems.insert (aSubnodes.at (anIdx).data(), anItem);
    }
  }
  else if (JTData_GroupNode* aGroup = dynamic_cast<JTData_GroupNode*> (theNode.data()))
  {
    for (size_t anIdx = 0; anIdx < aGroup->Children.size(); ++anIdx)
    {
      anItem->addChild (CreateTreeItem (theTree, aGroup->Children.at (anIdx)));
    }
  }

  if (!theNode->Name().isEmpty())
  {
    anItem->setText (theNode->Name());
  }

  return anItem;
}

//=======================================================================
// function : CollectChildren
// purpose  :
//=======================================================================
void JTGui_SceneGraphTree::CollectChildren (JTData_GroupNode* theGroup,
                                            JTData_NodeArray& theChildren)
{
  for (size_t anIdx = 0; anIdx < theGroup->Children.size(); ++anIdx)
  {
    JTData_NodePtr aChild = theGroup->Children.at (anIdx);
    JTData_GroupNode* aGroup = dynamic_cast<JTData_GroupNode*> (aChild.data());
    if (aGroup != NULL)
    {
      CollectChildren (aGroup, theChildren);
    }
    theChildren.push_back (aChild);
  }
}

//=======================================================================
// function : Node
// purpose  :
//=======================================================================
JTData_NodePtr JTGui_SceneGraphTree::Node (const JTGui_SceneGraphItem* theItem) const
{
  return theItem->node();
}

//=======================================================================
// function : updateCheckStates
// purpose  :
//=======================================================================
void JTGui_SceneGraphItem::updateCheckStates()
{
  updateChildren(this);
  updateParents (this);
}

//=======================================================================
// function : updateParents
// purpose  :
//=======================================================================
void JTGui_SceneGraphItem::updateParents (JTGui_SceneGraphItem* theItem)
{
  JTGui_SceneGraphItem* aParent = theItem->parent()
    ? dynamic_cast<JTGui_SceneGraphItem*> (theItem->parent())
    : NULL;

  if (!aParent)
  {
    return;
  }

  aParent->myCheckUpdatesOn = false;

  Standard_Integer aNbUncheck = 0;
  Standard_Integer aNbChecked = 0;

  for (Standard_Integer anIdx = 0; anIdx < aParent->childCount(); ++anIdx)
  {
    if (aParent->child (anIdx)->checkState() == Qt::Unchecked)
    {
      ++aNbUncheck;
    }
    else if (aParent->child (anIdx)->checkState() == Qt::Checked)
    {
      ++aNbChecked;
    }
  }

  if (aNbChecked == aParent->childCount()) // all items are checked
  {
    aParent->setCheckState (Qt::Checked);
  }
  else if (aNbUncheck == aParent->childCount()) // all items are unchecked
  {
    aParent->setCheckState (Qt::Unchecked);
  }
  else // node has children with mixed check status
  {
    aParent->setCheckState (Qt::PartiallyChecked);
  }

  Qt::CheckState aCheckState = aParent->checkState();

  JTData_NodePtr aNode = aParent->node();

  aNode->SetVisible (aCheckState != Qt::Unchecked);

  aParent->myCheckUpdatesOn = true;

  updateParents (aParent);
}

//=======================================================================
// function : updateChildren
// purpose  :
//=======================================================================
void JTGui_SceneGraphItem::updateChildren (JTGui_SceneGraphItem* theParent)
{
  Qt::CheckState aCheckState = theParent->checkState();

  JTData_NodePtr aNode = theParent->node();

  aNode->SetVisible (aCheckState != Qt::Unchecked);

  for (int anIdx = 0; anIdx < theParent->childCount(); ++anIdx)
  {
    JTGui_SceneGraphItem* aChild = theParent->child (anIdx);
    aChild->myCheckUpdatesOn = false;
    aChild->setCheckState (aCheckState);
    aChild->myCheckUpdatesOn = true;
    updateChildren (aChild);
  }
}
