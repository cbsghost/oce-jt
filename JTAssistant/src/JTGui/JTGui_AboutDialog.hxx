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

#ifndef JTGUI_ABOUT_HXX
#define JTGUI_ABOUT_HXX

#pragma warning (push, 0)
#include <QDialog>
#include <QString>
#pragma warning (pop)

class JTGui_AboutDialog : public QDialog
{
  Q_OBJECT

public:

  JTGui_AboutDialog (const QString& theOcctVersion, QWidget* theParent);

private:

};

#endif // JTGUI_ABOUT_HXX
