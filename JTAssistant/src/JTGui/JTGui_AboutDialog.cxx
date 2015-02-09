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

#include "JTGui_AboutDialog.hxx"

#pragma warning (push, 0)
#include <QLabel>
#include <QLayout>
#include <QBoxLayout>
#include <QPushButton>
#pragma warning (pop)

// =======================================================================
// function : checkUpdateState
// purpose  :
// =======================================================================
JTGui_AboutDialog::JTGui_AboutDialog (const QString& theOcctVersion, QWidget* theParent)
  : QDialog (theParent)
{
  this->setWindowTitle (tr ("Open CASCADE JT Assistant"));

  QBoxLayout* aLayout = new QBoxLayout (QBoxLayout::Down, this);
  this->setLayout (aLayout);

  QString anAboutText = tr (
    "Open CASCADE JT Assistant\n"
    "\n"
    "\n"
    "powered by\n"
    "Open CASCADE Technology %1.dev\n"
    "\n"
    "This software is provided 'as-is', without any express or\n"
    "implied warranty of any kind. Use it on your own risk. In\n"
    "no event OPEN CASCADE SAS shall be held liable for any\n"
    " damages arising from the use of this software.\n"
    "\n"
    "This software is free of charge and may be used for any purpose. The\n"
    "modifications and redistribution are allowed provided that you keep\n"
    "this copyright statement and disclaimer, and provide prominent notice\n"
    "of your modifications.\n"
    "\n"
    "See licensing information in README.txt\n"
    "file in application folder\n"

  ).arg (theOcctVersion);
  QLabel* aLabel = new QLabel (anAboutText, this);
  aLabel->setAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
  aLayout->addWidget (aLabel);

  aLayout->addStretch (1);

  QLabel* aLinkLabel = new QLabel (tr (
    "Copyright (C) 2013-2015, OPEN CASCADE SAS<br>"
    "<a href = http://www.opencascade.org>http://www.opencascade.org</a>"), this);
  aLinkLabel->setTextFormat (Qt::RichText);
  aLinkLabel->setAlignment (Qt::AlignHCenter | Qt::AlignBottom);
  aLinkLabel->setTextInteractionFlags (Qt::TextBrowserInteraction);
  aLinkLabel->setOpenExternalLinks (true);
  aLayout->addWidget (aLinkLabel);

  QPushButton* anOkButton = new QPushButton (tr ("Ok"), this);
  aLayout->addWidget (anOkButton);

  connect (anOkButton, SIGNAL (clicked()), this, SLOT (accept()));

  setMinimumSize (250, 200);
}
