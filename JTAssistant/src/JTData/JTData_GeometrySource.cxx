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

#pragma warning (push, 0)
#include <QStack>
#include <QFileInfo>
#pragma warning (pop)

#include "JTData_GeometrySource.hxx"

//=======================================================================
// function : JTData_GeometrySource
// purpose  :
//=======================================================================
JTData_GeometrySource::JTData_GeometrySource()
  : mySceneGraph (NULL)
{
  //
}

//=======================================================================
// function : ~JTData_GeometrySource
// purpose  :
//=======================================================================
JTData_GeometrySource::~JTData_GeometrySource()
{
  delete mySceneGraph;
}

//=======================================================================
// function : LoadPartition
// purpose  :
//=======================================================================
Standard_Boolean JTData_GeometrySource::LoadPartition (JTData_PartitionNode* thePartition)
{
  if (thePartition == NULL)
  {
    return Standard_False;
  }

  QMap<QString, Handle(JtNode_Partition)>::iterator anIter = myMap.find (thePartition->FileName());

  if (anIter == myMap.end())
  {
    TCollection_ExtendedString aFileName (thePartition->FileName().toUtf8().data(), Standard_True);

    Handle(JtData_Model) aModel = new JtData_Model (aFileName);

    Handle(JtNode_Partition) aPartitionRecord = aModel->Init();

    if (aPartitionRecord.IsNull())
    {
      return Standard_False;
    }

    myMap.insert (thePartition->FileName(), aPartitionRecord);
  }

  const Handle(JtNode_Partition)& aPartitionRecord = myMap.find (thePartition->FileName()).value();

  if (aPartitionRecord.IsNull())
  {
    return Standard_False;
  }

  return mySceneGraph->Init (aPartitionRecord, thePartition);
}

//=======================================================================
// function : LoadExternalPartitions
// purpose  :
//=======================================================================
Standard_Boolean JTData_GeometrySource::LoadExternalPartitions()
{
  if (mySceneGraph == NULL)
  {
    return Standard_True;
  }

  JTData_PartitionNode* aRoot =
    dynamic_cast<JTData_PartitionNode*> (mySceneGraph->Tree().data());

  if (aRoot == NULL)
  {
    return Standard_False;
  }

  QStack<JTData_Node*> aStack;

  for (size_t anIdx = 0; anIdx < aRoot->Children.size(); ++anIdx)
  {
    aStack.push (aRoot->Children.at (anIdx).data());
  }

  for (;;)
  {
    if (aStack.isEmpty())
      break;

    JTData_GroupNode* aGroup = dynamic_cast<JTData_GroupNode*> (aStack.pop());

    if (aGroup != NULL)
    {
      JTData_PartitionNode* aPartition = dynamic_cast<JTData_PartitionNode*> (aGroup);

      if (aPartition != NULL)
      {
        if (!LoadPartition (aPartition))
        {
          return Standard_False;
        }
      }
      else
      {
        for (size_t anIdx = 0; anIdx < aGroup->Children.size(); ++anIdx)
        {
          aStack.push (aGroup->Children.at (anIdx).data());
        }
      }
    }
  }

  return Standard_True;
}

//=======================================================================
// function : Init
// purpose  :
//=======================================================================
Standard_Boolean JTData_GeometrySource::Init (const QString& theFileName)
{
  myMap.clear();

  if (mySceneGraph != NULL)
  {
    delete mySceneGraph;
  }

#ifdef _WIN32
  ifstream aFile (theFileName.toStdWString().c_str(), ios::binary | ios::in);
#else
  ifstream aFile (theFileName.toUtf8().data(), ios::binary | ios::in);
#endif

  if (!aFile.is_open())
  {
    return Standard_False;
  }

#ifdef _WIN32
  TCollection_ExtendedString aFileName (theFileName.toUtf8().data(), Standard_True);
#else
  TCollection_ExtendedString aFileName (theFileName.toUtf8().data());
#endif

  Handle(JtData_Model) aModel = new JtData_Model (aFileName);

  if (aModel.IsNull())
  {
    return Standard_False;
  }

  mySceneGraph = new JTData_SceneGraph;

  if (!mySceneGraph->Init (aModel, theFileName))
  {
    return Standard_False;
  }

  return LoadExternalPartitions();
}
