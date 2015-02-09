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

#include "JTData_DataLoader.hxx"

#include <JtNode_Shape_TriStripSet.hxx>
#include <JtElement_ShapeLOD_TriStripSet.hxx>

#pragma warning (push, 0)
#include <QThread>
#pragma warning (pop)

// =======================================================================
// function : Perform
// purpose  :
// =======================================================================
Standard_Boolean JTData_WorkItem::Perform()
{
  if (myLateLoaded->DefferedObject().IsNull())
  {
    myLateLoaded->Load();
  }

  bool isLoaded = !myLateLoaded->DefferedObject().IsNull();

  if (isLoaded)
  {
    emit loaded();
  }

  return isLoaded;
}

// =======================================================================
// function : Enqueue
// purpose  :
// =======================================================================
void JTData_LoadingQueue::Enqueue (const JTData_WorkItem& theQuery)
{
  myMutex.lock();
  {
    myQueue.push_back (theQuery);

    mySet.insert (theQuery.LateLoaded().Access());

    myCondition.wakeOne();
  }
  myMutex.unlock();

  emit workItemEnqueued();
}

// =======================================================================
// function : Fetch
// purpose  :
// =======================================================================
JTData_WorkItem JTData_LoadingQueue::Fetch()
{
  JTData_WorkItem aQuery;

  myMutex.lock();
  {
    while (myQueue.size() == 0)
    {
      myCondition.wait (&myMutex);
    }

    aQuery = myQueue.takeFirst();

    mySet.remove (aQuery.LateLoaded().Access());
  }
  myMutex.unlock();

  return aQuery;
}

// =======================================================================
// function : Size
// purpose  :
// =======================================================================
Standard_Integer JTData_LoadingQueue::Size()
{
  Standard_Integer aSize;

  myMutex.lock();
  {
    aSize = myQueue.size();
  }
  myMutex.unlock();

  return aSize;
}

// =======================================================================
// function : Enqueued
// purpose  :
// =======================================================================
Standard_Boolean JTData_LoadingQueue::Enqueued (const JTData_WorkItem& theQuery)
{
  Standard_Boolean anIsEnqueued = Standard_False;

  myMutex.lock();
  {
    if (mySet.find (theQuery.LateLoaded().Access()) != mySet.end())
    {
      anIsEnqueued = Standard_True;
    }
  }
  myMutex.unlock();

  return anIsEnqueued;
}

// =======================================================================
// function : Constructor
// purpose  :
// =======================================================================
JTData_LoadingThread::JTData_LoadingThread (JTData_LoadingQueue& theQueue)
    : myQueue (theQueue)
{
  connect (&myQueue, SIGNAL (workItemEnqueued()), this, SLOT (Awake()));
}

// =======================================================================
// function : run
// purpose  :
// =======================================================================
void JTData_LoadingThread::run()
{
  while (myQueue.Size() > 0)
  {
    myQueue.Fetch().Perform();
  }
}
