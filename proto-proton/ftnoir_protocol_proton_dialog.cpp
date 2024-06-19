#include "ftnoir_protocol_proton.h"
#include <QDebug>
#include <QDir>
#include <QVector>

#include "api/plugin-api.hpp"

QVector< QVector<QString> > steamapps;
QVector<QString> steam_libraries;

FTControls::FTControls()
{
    ui.setupUi(this);
    /* Clear all items from Vectors */
    steamapps.clear();
    steam_libraries.clear();
    /* Get all Steam libraries on the system - see proton_handling.cpp */
    get_steam_libs(steam_libraries);
    /*
     * Collect Steam app information. ID, name, path to Proton runtime, path to Proton prefix
     */
    for (int i = 0; i < steam_libraries.size(); ++i) {
        QString searchpathSteamapps = steam_libraries[i] + "/steamapps";
        QString searchpathCompatdata = steam_libraries[i] + "/steamapps/compatdata/";

        /* Look for appmanifest files and iterate through them */
        QDir dir(searchpathSteamapps);
        dir.setFilter(QDir::Files);
        dir.setNameFilters({ "appmanifest_*" });
        QFileInfoList list = dir.entryInfoList();
        for (int j = 0; j < list.size(); ++j) {
            QFileInfo fileInfo = list.at(j);
            QFile manifestfile(fileInfo.filePath());
            /* Extract Steam app ID and name from manifest file */
            if (manifestfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                const QRegularExpression reSteamAppId("\"appid\"\\s+\"([0-9]*)\""); /* steamapps[0]: Steam app ID */
                const QRegularExpression reSteamAppName("\"name\"\\s+\"(.*)\""); /* steamapps[1]: Steam app name */
                QTextStream in(&manifestfile);
                QString line;
                QString appId;
                QString appName;
                while (in.readLineInto(&line)) {
                    const auto appIdMatch = reSteamAppId.match(line);
                    const auto appNameMatch = reSteamAppName.match(line);
                    if(appId.isEmpty() && appIdMatch.hasMatch()) {
                        appId = appIdMatch.captured(1);
                        if (! appName.isEmpty())
                          break; // We now have both name and id. Stop processing rest of the file.
                    }
                    if(appName.isEmpty() && appNameMatch.hasMatch()) {
                        appName = appNameMatch.captured(1);
                        if (! appId.isEmpty())
                          break; // We now have both name and id. Stop processing rest of the file.
                    }
                }
                if (! appName.isEmpty() && ! appId.isEmpty()) {
                    QVector<QString> ret = get_proton_paths(searchpathCompatdata, appId, steam_libraries);
                    if (ret.size() == 2) {
                        steamapps.push_back(QVector<QString> {appId, appName, ret[0], ret[1]});
                    }
                }
            }
        }
    }
    /*
     * Add Steam apps to UI combobox
     */
    for (int k = 0; k < steamapps.size(); ++k) {
        /* qDebug() << steamapps[k]; */
        ui.steamapp->addItem(steamapps[k][1]+" ("+steamapps[k][0]+")", QVariant{steamapps[k][0]});
        if (s.proton_appid == steamapps[k][0]) {
            /*qDebug() << "Index:" << k;*/
            ui.steamapp->setCurrentIndex(k);
        }
    }

    /*
     * WINE wrapper detection
     */
    QDir wrapperdir;
    wrapperdir.currentPath(); /* Starts in [opentrack install dir]/bin */
    wrapperdir.cdUp();
    wrapperdir.cd("libexec/opentrack/");
    if (wrapperdir.exists()) {
        wrapperdir.setFilter(QDir::Files);
        wrapperdir.setNameFilters({ "opentrack-wrapper-proton*.exe.so" });
        QFileInfoList wrapperlist = wrapperdir.entryInfoList();
        for(int i=0;i<wrapperlist.size();++i) {
            QFileInfo fileInfo = wrapperlist.at(i);
            QString wrapperfile = fileInfo.fileName();
            ui.wrapper_selection->addItem(wrapperfile, QVariant{wrapperfile});
            if(s.wrapper == wrapperfile)
                ui.wrapper_selection->setCurrentIndex(i);
        }
    } else {
     qDebug() << "\"[opentrack install directory]/libexec/opentrack\" not found";
    }
    tie_setting(s.wineprefix, ui.wineprefix);
    tie_setting(s.wineruntime, ui.wineruntime);
    tie_setting(s.variant_wine, ui.variant_wine);
    tie_setting(s.variant_proton, ui.variant_proton);
    tie_setting(s.esync, ui.esync);
    tie_setting(s.fsync, ui.fsync);
    tie_setting(s.protocol, ui.protocol_selection);
    tie_setting(s.wrapper, ui.wrapper_selection);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &FTControls::doOK);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &FTControls::doCancel);
}

void FTControls::doOK()
{
    //if (ui.variant_wine) {
    //}
    /* Save Steam app id */
    if (ui.variant_proton) {
        s.proton_appid = ui.steamapp->itemData(ui.steamapp->currentIndex()).toString();
    }
    s.b->save();
    close();
}

void FTControls::doCancel()
{
    s.b->reload();
    close();
}
