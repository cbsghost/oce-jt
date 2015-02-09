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

#ifndef JTData_DataLoader_HeaderFile
#define JTData_DataLoader_HeaderFile

#pragma warning (push, 0)
#include <QSet>
#include <QList>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#pragma warning (pop)

#include <JtProperty_LateLoaded.hxx>


//! Work-item to push into loading queue.
class JTData_WorkItem : public QObject
{
  Q_OBJECT

public:

  //! Creates new work-item for loading triangulation data.
  JTData_WorkItem()
    : QObject (NULL),
      myFeedback (NULL)
  {
    //
  }

  //! Creates copy of existing work-item.
  JTData_WorkItem (const JTData_WorkItem& theItem)
    : QObject (NULL),
      myLateLoaded (theItem.myLateLoaded),
      myFeedback   (theItem.myFeedback)
  {
    if (myFeedback != NULL)
    {
      connect (this, SIGNAL (loaded (void)), myFeedback, SLOT (ForceUpdate (void)));
    }
  }

  //! Creates new work-item for loading triangulation data.
  JTData_WorkItem (const Handle(JtProperty_LateLoaded)& theLateLoaded, QObject* theFeedback)
    : QObject (NULL),
      myLateLoaded (theLateLoaded),
      myFeedback   (theFeedback)
  {
    if (myFeedback != NULL)
    {
      connect (this, SIGNAL (loaded (void)), myFeedback, SLOT (ForceUpdate (void)));
    }
  }

  //! Initializes work-item with data from existing one.
  JTData_WorkItem& operator= (const JTData_WorkItem& theItem)
  {
    myLateLoaded = theItem.myLateLoaded;
    myFeedback   = theItem.myFeedback;

    if (myFeedback != NULL)
    {
      connect (this, SIGNAL (loaded (void)), myFeedback, SLOT (ForceUpdate (void)));
    }

    return *this;
  }

  //! Returns late-loaded object.
  const Handle(JtProperty_LateLoaded)& LateLoaded() const
  {
    return myLateLoaded;
  }

  //! Perform loading shape triangulation data from JT file.
  Standard_Boolean Perform();

  //! Checks to see if two work-items are identical.
  Standard_Boolean IsEqual (const JTData_WorkItem& theItem) const
  {
    return myLateLoaded == theItem.myLateLoaded;
  }

signals:

  //! Emitted when task is completed.
  void loaded();

protected:

  Handle(JtProperty_LateLoaded) myLateLoaded; //!< Shape node to load.
  QObject* myFeedback;                        //!< Pointer to scene for update feedback.

};

//! Abstract command-queue for managing parallel threads.
class JTData_LoadingQueue : public QObject
{
  Q_OBJECT

public:

  //! Creates new loading queue.
  JTData_LoadingQueue()
   : QObject (NULL) {}

  //! Releases resources of the loading queue.
  ~JTData_LoadingQueue() {}

public:

  //! Returns size of the loading queue.
  Standard_Integer Size();

  //! Fetches work-item from the loading queue.
  JTData_WorkItem Fetch();

  //! Enqueues new work-item onto loading queue.
  void Enqueue (const JTData_WorkItem& theQuery);

  //! Checks if specific work-item is in loading queue.
  Standard_Boolean Enqueued (const JTData_WorkItem& theQuery);

signals:

  void workItemEnqueued();

protected:

  //! Set of loading tasks.
  QSet<const Standard_Transient*> mySet;

  //! Queue of loading tasks.
  QList<JTData_WorkItem> myQueue;

protected:

  //! Manages access serialization of loading threads.
  QMutex myMutex;

  //! Condition variable for synchronizing loading threads.
  QWaitCondition myCondition;
};

//! Separate thread for loading triangulation data.
class JTData_LoadingThread : public QThread
{
  Q_OBJECT

public:

  //! Creates new thread for loading triangulation data.
  JTData_LoadingThread (JTData_LoadingQueue& theQueue);

public slots:

  //! Awakes the loader to process queued work items.
  void Awake()
  {
    if (isRunning())
      return;

    start();
  }

protected:

  //! Executes loading thread.
  void run();

protected:

  //! Referenced loading queue.
  JTData_LoadingQueue& myQueue;
};

#endif // JTData_DataLoader_HeaderFile
