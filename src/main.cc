// This is copyrighted software. More information is at the end of this file.
#include <QDebug>
#include <QFileDialog>
#include <QTimer>

#include "qtadssound.h"
#include "settings.h"
#include "sysframe.h"

#include "dapglobals.h" // CHANGES: This file defines the global EnableDAP variable
#if _DEBUG
  #include <Windows.h>
#endif
// On some platforms, SDL redefines main in order to provide a
// platform-specific main() implementation.  However, Qt handles this too,
// so things can get weird.  We need to make sure main is not redefined so
// that Qt can find our own implementation and SDL will not try to do
// platform-specific initialization work (like launching the Cocoa event-loop
// or setting up the application menu on OS X, or redirecting stdout and stderr
// to text files on Windows), which would break things.
#if !defined(NO_AUDIO) && defined(main)
    #undef main
#endif

auto main(int argc, char** argv) -> int
{
    // CHANGES: On Windows, wait for the debugger to attach
//#if _DEBUG
//    while (!IsDebuggerPresent()) {
//        _sleep(1);
//    }
//    _sleep(1);
//#endif
    CHtmlResType::add_basic_types();
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    CHtmlSysFrameQt* app = new CHtmlSysFrameQt(
        argc, argv, "QTads", QTADS_VERSION, "Nikos Chantziaras", {});
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    QApplication::setDesktopFileName("nikos.chantziaras.qtads");
#endif

    // Filename of the game to run.
    QString gameFileName;

    const QStringList& args = app->arguments();

    // CHANGES: Added a new -d argument. When it is passed QTads starts the DAP extension
    // If it is passed, we will expect the filename of the story to debug
    QString tempGameFileName;
    if (args.size() == 1 || args.size() == 2) {
        QString tempArgument = args.at(0);
        // The -d argument was passed first
        if (tempArgument == "-d" || tempArgument == "/d") {
            EnableDAP = true;
            // If there was three args, the gamefile name is the third
            if (args.size() == 2) {
                tempGameFileName = args.at(1);
            }
        } else if (args.size() == 2) { // The -d argument was passed second
            tempArgument = args.at(1);
            EnableDAP = (tempArgument == "-d" || tempArgument == "/d");
        }
    }

    // Changed args.at(1) to tempGameFileName
    if (!tempGameFileName.isEmpty()) {
        if (QFile::exists(tempGameFileName)) {
            gameFileName = tempGameFileName;
        } else if (QFile::exists(tempGameFileName + ".gam")) {
            gameFileName = tempGameFileName + ".gam";
        } else if (QFile::exists(tempGameFileName + ".t3")) {
            gameFileName = tempGameFileName + ".t3";
        } else {
            qWarning() << "File" << tempGameFileName << "not found.";
        }
    } else if (EnableDAP == false) {
        qWarning() << "The -d was passed but not the filename of the game to debug.";
    }
    // END CHANGES:


    if (gameFileName.isEmpty() and app->settings().askForGameFile) {
        gameFileName = QFileDialog::getOpenFileName(
            nullptr, QObject::tr("Choose the TADS game you wish to run"), {},
            QObject::tr("TADS Games") + "(*.gam *.Gam *.GAM *.t3 *.T3)");
    }
    // END CHANGES:

    // CHANGES: Added a new -d argument. When it is passed QTads starts the DAP extension
    // TODO: Start the DAP
    // END CHANGES:

#ifndef NO_AUDIO
    if (not initSound()) {
        delete app;
        return 1;
    }
#endif

    QTimer::singleShot(0, app, [app, gameFileName] { app->entryPoint(gameFileName); });
    int ret = CHtmlSysFrameQt::exec();

    delete app;
    quitSound();
    return ret;
}

/*
    Copyright 2003-2020 Nikos Chantziaras <realnc@gmail.com>

    This file is part of QTads.

    QTads is free software: you can redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the Free Software
    Foundation, either version 3 of the License, or (at your option) any later
    version.

    QTads is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with QTads. If not, see <https://www.gnu.org/licenses/>.
*/
