// main.cpp
//
/****************************************************************************
   Copyright (C) 2003-2008, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qsynthAbout.h"
#include "qsynthOptions.h"
#include "qsynthMainForm.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include <QLocale>


//-------------------------------------------------------------------------
// Single application instance stuff (Qt/X11 only atm.)
//

#if defined(Q_WS_X11)

#include <QX11Info>

#include <X11/Xatom.h>
#include <X11/Xlib.h>

#define QSYNTH_XUNIQUE "qsynthMainForm_xunique"

#endif

class qsynthApplication : public QApplication
{
public:

	// Constructor.
	qsynthApplication(int& argc, char **argv) : QApplication(argc, argv),
		m_pQtTranslator(0), m_pMyTranslator(0), m_pWidget(0)	
	{
		// Load translation support.
		QLocale loc;
		if (loc.language() != QLocale::C) {
			// Try own Qt translation...
			m_pQtTranslator = new QTranslator(this);
			QString sLocName = "qt_" + loc.name();
			QString sLocPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
			if (m_pQtTranslator->load(sLocName, sLocPath)) {
				QApplication::installTranslator(m_pQtTranslator);
			} else {
				delete m_pQtTranslator;
				m_pQtTranslator = 0;
		#ifdef CONFIG_DEBUG
				qWarning("Warning: no translation found for '%s' locale: %s/%s.qm",
					loc.name().toUtf8().constData(),
					sLocPath.toUtf8().constData(),
					sLocName.toUtf8().constData());
		#endif
			}
			// Try own application translation...
			m_pMyTranslator = new QTranslator(this);
			sLocName = "qsynth_" + loc.name();
			if (m_pMyTranslator->load(sLocName, sLocPath)) {
				QApplication::installTranslator(m_pMyTranslator);
			} else {
				sLocPath = CONFIG_PREFIX "/share/locale";
				if (m_pMyTranslator->load(sLocName, sLocPath)) {
					QApplication::installTranslator(m_pMyTranslator);
				} else {
					delete m_pMyTranslator;
					m_pMyTranslator = 0;
		#ifdef CONFIG_DEBUG
					qWarning("Warning: no translation found for '%s' locale: %s/%s.qm",
						loc.name().toUtf8().constData(),
						sLocPath.toUtf8().constData(),
						sLocName.toUtf8().constData());
		#endif
				}
			}
		}
	#if defined(Q_WS_X11)
		// Instance uniqueness initialization...
		m_pDisplay = QX11Info::display();
		m_aUnique  = XInternAtom(m_pDisplay, QSYNTH_XUNIQUE, false);
		XGrabServer(m_pDisplay);
		m_wOwner = XGetSelectionOwner(m_pDisplay, m_aUnique);
		XUngrabServer(m_pDisplay);
	#endif
	}

	// Destructor.
	~qsynthApplication()
	{
		if (m_pMyTranslator) delete m_pMyTranslator;
		if (m_pQtTranslator) delete m_pQtTranslator;
	}

	// Main application widget accessors.
	void setMainWidget(QWidget *pWidget)
	{
		m_pWidget = pWidget;
	#if defined(Q_WS_X11)
		XGrabServer(m_pDisplay);
		m_wOwner = m_pWidget->winId();
		XSetSelectionOwner(m_pDisplay, m_aUnique, m_wOwner, CurrentTime);
		XUngrabServer(m_pDisplay);
	#endif
	}

	QWidget *mainWidget() const { return m_pWidget; }

	// Check if another instance is running,
    // and raise its proper main widget...
	bool setup()
	{
	#if defined(Q_WS_X11)
		if (m_wOwner != None) {
			// First, notify any freedesktop.org WM
			// that we're about to show the main widget...
			Screen *pScreen = XDefaultScreenOfDisplay(m_pDisplay);
			int iScreen = XScreenNumberOfScreen(pScreen);
			XEvent ev;
			memset(&ev, 0, sizeof(ev));
			ev.xclient.type = ClientMessage;
			ev.xclient.display = m_pDisplay;
			ev.xclient.window = m_wOwner;
			ev.xclient.message_type = XInternAtom(m_pDisplay, "_NET_ACTIVE_WINDOW", false);
			ev.xclient.format = 32;
			ev.xclient.data.l[0] = 0; // Source indication.
			ev.xclient.data.l[1] = 0; // Timestamp.
			ev.xclient.data.l[2] = 0; // Requestor's currently active window (none)
			ev.xclient.data.l[3] = 0;
			ev.xclient.data.l[4] = 0;
			XSelectInput(m_pDisplay, m_wOwner, StructureNotifyMask);
			XSendEvent(m_pDisplay, RootWindow(m_pDisplay, iScreen), false,
				(SubstructureNotifyMask | SubstructureRedirectMask), &ev);
			XSync(m_pDisplay, false);
			XRaiseWindow(m_pDisplay, m_wOwner);
			// And then, let it get caught on destination
			// by QApplication::x11EventFilter...
			QByteArray value = QSYNTH_XUNIQUE;
			XChangeProperty(
				m_pDisplay,
				m_wOwner,
				m_aUnique,
				m_aUnique, 8,
				PropModeReplace,
				(unsigned char *) value.data(),
				value.length());
			// Done.
			return true;
		}
	#endif
		return false;
	}

#if defined(Q_WS_X11)
	bool x11EventFilter(XEvent *pEv)
	{
		if (m_pWidget && m_wOwner != None
			&& pEv->type == PropertyNotify
			&& pEv->xproperty.window == m_wOwner
			&& pEv->xproperty.state == PropertyNewValue) {
			// Always check whether our property-flag is still around...
			Atom aType;
			int iFormat = 0;
			unsigned long iItems = 0;
			unsigned long iAfter = 0;
			unsigned char *pData = 0;
			if (XGetWindowProperty(
					m_pDisplay,
					m_wOwner,
					m_aUnique,
					0, 1024,
					false,
					m_aUnique,
					&aType,
					&iFormat,
					&iItems,
					&iAfter,
					&pData) == Success
				&& aType == m_aUnique && iItems > 0 && iAfter == 0) {
				// Avoid repeating it-self...
				XDeleteProperty(m_pDisplay, m_wOwner, m_aUnique);
				// Just make it always shows up fine...
				m_pWidget->show();
				m_pWidget->raise();
				m_pWidget->activateWindow();
				// FIXME: Do our best speciality, although it should be
				// done iif configuration says so, we'll do it anyway!
				qsynthMainForm *pMainForm = qsynthMainForm::getInstance();
				if (pMainForm)
					pMainForm->startAllEngines();
			}
			// Free any left-overs...
			if (iItems > 0 && pData)
				XFree(pData);
		}
		return QApplication::x11EventFilter(pEv);
	}
#endif
	
private:

	// Translation support.
	QTranslator *m_pQtTranslator;
	QTranslator *m_pMyTranslator;

	// Instance variables.
	QWidget *m_pWidget;

#if defined(Q_WS_X11)
	Display *m_pDisplay;
	Atom     m_aUnique;
	Window   m_wOwner;
#endif
};


//-------------------------------------------------------------------------
// main - The main program trunk.
//

int main ( int argc, char **argv )
{
	qsynthApplication app(argc, argv);

	// Construct default settings; override with command line arguments.
	qsynthOptions settings;
	if (!settings.parse_args(app.argc(), app.argv())) {
		app.quit();
		return 1;
	}

	// Have another instance running?
	if (app.setup()) {
		app.quit();
		return 2;
	}

	// Set default base font...
	if (settings.iBaseFontSize > 0)
		app.setFont(QFont(app.font().family(), settings.iBaseFontSize));

	// What style do we create these forms?
	Qt::WindowFlags wflags = Qt::Window
#if QT_VERSION >= 0x040200
		| Qt::CustomizeWindowHint
#endif
		| Qt::WindowTitleHint
		| Qt::WindowSystemMenuHint
		| Qt::WindowMinMaxButtonsHint;
	if (settings.bKeepOnTop)
		wflags |= Qt::Tool;
	// Construct the main form, and show it to the world.
	qsynthMainForm w(0, wflags);
//	app.setMainWidget(&w);
	w.setup(&settings);
	// If we have a systray icon, we'll skip this.
	if (!settings.bSystemTray) {
		w.show();
		w.adjustSize();
	}

	// Settle this one as application main widget...
	app.setMainWidget(&w);

	// Register the quit signal/slot.
	// app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
	app.setQuitOnLastWindowClosed(false);

	return app.exec();
}

// end of main.cpp
