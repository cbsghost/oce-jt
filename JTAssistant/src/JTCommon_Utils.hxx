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

#ifndef JTCOMMON_UTILS_H
#define JTCOMMON_UTILS_H

#include <vector>
#include <numeric>

#pragma warning (push, 0)
#include <QString>
#include <QSharedPointer>
#include <QElapsedTimer>
#include <QMap>
#pragma warning (pop)

#include <BVH/BVH_Box.hxx>

#include <JtElement_ShapeLOD_TriStripSet.hxx>

#include <Eigen/Core>

#include <NCollection_Vec4.hxx>

typedef BVH_Box<float, 4> JTCommon_AABB;

typedef std::vector<Eigen::Vector2f, Eigen::aligned_allocator<Eigen::Vector2f> > Array2f;
typedef std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f> > Array3f;
typedef std::vector<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f> > Array4f;

typedef NCollection_Vec4<Standard_ShortReal> Vec4f;

class JTCommon_TriangleData
{
public:

  Handle(JtElement_ShapeLOD_TriStripSet) Data;
};

typedef QSharedPointer<JTCommon_TriangleData> JTCommon_TriangleDataPtr;

template <typename T>
class JTCommon_Callback
{
public:

  virtual ~JTCommon_Callback() {}

  virtual void Execute (T theParam) = 0;
};

struct JTCommon_CmdArgs
{
  QString FileName;

  bool DoBenchmarking;
};

class JTCommon_Profiler
{
private:

  JTCommon_Profiler()
  {
    //
  }

public:

  static JTCommon_Profiler& GetProfiler()
  {
    static JTCommon_Profiler* aProfiler = new JTCommon_Profiler();

    return *aProfiler;
  }

  void Start()
  {
    myTimer.start();
  }

  void WriteElapsed (QString theTag)
  {
    myValues.insert (theTag, myTimer.elapsed());
  }

  const QMap<QString, qint64>& Values() const
  {
    return myValues;
  }

protected:

  QElapsedTimer myTimer;
  QMap<QString, qint64> myValues;

};

/*
 * The methods below will be required for porting to OCCT version of BVH.
 *
static Standard_ShortReal norm (Vec4f theVector)
{
  return (Standard_ShortReal)sqrt(theVector[0] * theVector[0] +
                                  theVector[1] * theVector[1] +
                                  theVector[2] * theVector[2] +
                                  theVector[3] * theVector[3]);
}

static Standard_ShortReal maxCoeff (Vec4f theVector)
{
  return std::max (theVector.x(), std::max (theVector.y(), std::max (theVector.z(), theVector.w())));
}

static Standard_ShortReal minCoeff (Vec4f theVector)
{
  return std::min (theVector.x(), std::min (theVector.y(), std::min (theVector.z(), theVector.w())));
}

static Vec4f cwiseAbs (Vec4f theVector)
{
  return Vec4f (fabs(theVector.x()),
                fabs(theVector.y()),
                fabs(theVector.z()),
                fabs(theVector.w()));
}

static Vec4f cwiseInverse (Vec4f theVector)
{
  return Vec4f (1 / theVector.x(),
                1 / theVector.y(),
                1 / theVector.z(),
                1 / theVector.w());
}

static Vec4f cwiseProduct (Vec4f theVec1, Vec4f theVec2)
{
  return Vec4f (theVec1.x() * theVec2.x(),
                theVec1.y() * theVec2.y(),
                theVec1.z() * theVec2.z(),
                theVec1.w() * theVec2.w());
}
*/

#endif // JTCOMMON_UTILS_H
