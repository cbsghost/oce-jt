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

#ifndef JTDATA_GEOMETRYSOURCE_H
#define JTDATA_GEOMETRYSOURCE_H

#pragma warning (push, 0)
#include <QMap>
#include <QString>
#pragma warning (pop)

#include <Standard_Version.hxx>

#include "JTData_Node.hxx"
#include "JTData_SceneGraph.hxx"


//! Tool object for loading scene geometry from JT file and
//! building logical scene graph.
class JTData_GeometrySource
{
public:

  //! Creates uninitialized geometry source.
  JTData_GeometrySource();

  //! Releases resources of geometry source.
  ~JTData_GeometrySource();

public:

  //! Initialized geometry source from specified file.
  Standard_Boolean Init (const QString& theFileName);

  //! Returns logical scene graph.
  JTData_SceneGraph* SceneGraph()
  {
    return mySceneGraph;
  }

  static const char* OcctVersion() { return OCC_VERSION_COMPLETE; }

protected:

  //! Loads specified partition.
  Standard_Boolean LoadPartition (JTData_PartitionNode* thePartition);

  //! Loads external partitions and connect it to LSG.
  Standard_Boolean LoadExternalPartitions();

protected:

  //! Logical scene graph.
  JTData_SceneGraph* mySceneGraph;

  //! Loaded JT data models for given file names.
  QMap<QString, Handle(JtNode_Partition)> myMap;

public:

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

typedef QSharedPointer<JTData_GeometrySource> JTData_GeometrySourcePtr;

#endif // JTDATA_GEOMETRYSOURCE_H
