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

#include <JTGui/JTGui_MainWindow.hxx>

#ifdef _WIN32
  #include <Windows.h>
#endif

#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <Message_PrinterOStream.hxx>

#pragma warning (push, 0)
#include <QApplication>

#include <QStyleFactory>
#include <QCommandLineOption>
#include <QCommandLineParser>
#pragma warning (pop)

int main (int argc, char **argv)
{
  QApplication app (argc, argv);
  QCoreApplication::setApplicationName ("JTAssistant");

  QStringList paths = QCoreApplication::libraryPaths();
  paths.append (".");
  paths.append ("platforms");
  QCoreApplication::setLibraryPaths (paths);

  // command line tools

  QCommandLineParser aParser;
  aParser.setApplicationDescription (QCoreApplication::translate ("main",
    "Viewer for JT files."));

  aParser.addHelpOption();

  aParser.addPositionalArgument ("filename", QCoreApplication::translate ("main",
    "JT file to open (8 or 9 JT format version)."));

  QCommandLineOption aBenchmarkOption (QStringList() << "b" << "benchmark",
    QCoreApplication::translate ("main",
    "Measure loading time."));
  aParser.addOption (aBenchmarkOption);

  QCommandLineOption aLogOption (QStringList() << "l" << "log",
    QCoreApplication::translate ("main",
    "Enables logging."));
  aParser.addOption (aLogOption);

  aParser.process (app);

  const QStringList anArgs = aParser.positionalArguments();

  Handle(Message_PrinterOStream) aCoutPrinter
    = Handle(Message_PrinterOStream)::DownCast (::Message::DefaultMessenger()->ChangePrinters().First());
  if (!aCoutPrinter.IsNull())
  {
#ifdef _DEBUG
    aCoutPrinter->SetTraceLevel (Message_Trace);
#else
    aCoutPrinter->SetTraceLevel (aParser.isSet (aLogOption) ? Message_Trace : Message_Alarm);
#endif
  }

  // window

  QApplication::setStyle (QStyleFactory::create ("Fusion"));

  JTGui_MainWindow aMainWindow;

#if !defined (_DEBUG) && defined (_WIN32)
  if (anArgs.size() == 0)
  {
    if (!aParser.isSet (aLogOption))
    {
      FreeConsole();
    }
  }
#endif

  JTCommon_CmdArgs aParams = {anArgs.size() == 0 ? "" : anArgs.at(0), aParser.isSet (aBenchmarkOption)};
  
  aMainWindow.setCmdArgs (aParams);

  aMainWindow.show();

  return app.exec();
}
