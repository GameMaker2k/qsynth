// qsynthAboutForm.ui.h
//
// ui.h extension file, included from the uic-generated form implementation.
/****************************************************************************
   Copyright (C) 2003, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*****************************************************************************/

#include <qmessagebox.h>

#include "config.h"

#include "qsynthAbout.h"


// Kind of constructor.
void qsynthAboutForm::init (void)
{
    // Stuff the about box...
    QString sText = "<p align=\"center\"><br />\n";
    sText += "<b>" QSYNTH_TITLE " - " + tr(QSYNTH_SUBTITLE) + "</b><br />\n";
    sText += "<br />\n";
    sText += tr("Version") + ": <b>" QSYNTH_VERSION "</b><br />\n";
    sText += tr("Build") + ": " __DATE__ " " __TIME__ "<br />\n";
#ifdef CONFIG_DEBUG
    sText += "<small><font color=\"red\">";
    sText += tr("Debugging option enabled.");
    sText += "<br />\n";
    sText += "</font></small>";
#endif
#ifndef CONFIG_FLUID_SERVER
    sText += "<small><font color=\"red\">";
    sText += tr("Server option disabled.");
    sText += "<br />\n";
    sText += "</font></small>";
#endif
#ifndef CONFIG_FLUID_RESET
    sText += "<small><font color=\"red\">";
    sText += tr("System reset option disabled.");
    sText += "<br />\n";
    sText += "</font></small>";
#endif
    sText += "<br />\n";
    sText += tr("Website") + ": <a href=\"" QSYNTH_WEBSITE "\">" QSYNTH_WEBSITE "</a><br />\n";
    sText += "<br />\n";
    sText += "<small>";
    sText += QSYNTH_COPYRIGHT "<br />\n";
    sText += "<br />\n";
    sText += tr("This program is free software; you can redistribute it and/or modify it") + "<br />\n";
    sText += tr("under the terms of the GNU General Public License version 2 or later.");
    sText += "</small>";
    sText += "</p>\n";
    AboutTextView->setText(sText);
}


// Kind of destructor.
void qsynthAboutForm::destroy (void)
{
}


// About Qt request.
void qsynthAboutForm::aboutQt()
{
    QMessageBox::aboutQt(this);
}


// end of qsynthAboutForm.ui.h
