#include <QDebug>
#include <QDir>
#include <QVector>
#include <QString>
#include <QFile>
#include "ftnoir_protocol_proton.h"

/*
 * Adds standard Steam library locations to vector and collects other Steam libraries on the system
 */
void get_steam_libs(QVector<QString> & outvector)
{
    /* Define input stuff */
    QString librarycontainer = QDir::homePath() + "/.steam/steam/steamapps/libraryfolders.vdf";

    /*
     * Scan for Steam libraries and add the paths to the Steam libraries vector
     */
    QFile libraryfile(librarycontainer);
    if (libraryfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        const QRegularExpression pathRegex("\"path\"\\s+\"(.*)\"");
        QTextStream in(&libraryfile);
        QString line;
        while (in.readLineInto(&line)) {
            const auto pathMatch = pathRegex.match(line);
            if(pathMatch.hasMatch())
                outvector.append(pathMatch.captured(1));
        }
    }
    for (int i = 0; i < outvector.size(); ++i)
        qDebug() << outvector[i];
}

/*
 *  Reads a property by regex from the input file and writes it to the output file
 */
QString read_proton_property(const QString &inputpath, const QString &inregex) {
    QFile inputfile(inputpath);
    if (inputfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QRegularExpression regex(inregex);
        QTextStream in(&inputfile);
        QString line;
        while (in.readLineInto(&line)) {
            const auto match = regex.match(line);
            if(match.hasMatch()) {
                return match.captured(1);
                break;
            }
        }
    }
    return {};
}

/*
 * Analyzes the Proton version string composition in the version file and writes it to the output variable
 */
bool analyze_version_string(const QString & inputstring, const QString & inregex, QString & outstring) {
    const QRegularExpression regex(inregex);
    const auto stringmatch = regex.match(inputstring);
    if(stringmatch.hasMatch()) {
        outstring = stringmatch.captured(1);
        /* qDebug() << inputstring << inregex << outstring; */
        return true;
    } else {
        return false;
    }
}

/*
 * Gets the Proton version and the path to the corresponding /dist or /files folder
 */
QString get_proton_version(const QString &inpath, const QVector<QString> & inlibrary)
{
    QString protonPath;
    QVector<QString> protonversions;
    /* Get Proton version from version file - Do not change output variable! */
    protonversions.append(read_proton_property(inpath + "/version", "(\\d.+)"));
    /* Get Proton version from config_info file */
    protonversions.append(read_proton_property(inpath + "/config_info", "(\\d.+)"));
    /* Information from version file and config_info plus matching Proton versions: Get proton runtime path from config_info */
    if ( ! protonversions[0].isEmpty() && protonversions[0] == protonversions[1]) {
            protonPath = read_proton_property(inpath + "/config_info", "(.*/dist)");
            if (protonPath == "")
                protonPath = read_proton_property(inpath + "/config_info", "(.*/files)"); /* Work around Proton Experimental's path */
            /* qDebug() << inpath << outpath; */
    }

    /* Information from only one of the two files (version file or config_info) or non-matching Proton versions: Get proton runtime path from version file and a search through library folders*/
    if (protonversions[0] != protonversions[1]) {
        QString versionstring;
        QString cleanfoldername;
        /* Regex-search path for any sign of GloriousEggroll Proton releases (folder name usually Proton-x.x-GE-somethingsomething */
        for (int j=0; j<protonversions.size();++j) {
            if ( ! protonversions[j].isEmpty()) {
                if (analyze_version_string(protonversions[j], "(\\d+.\\d+-\\D+.*)", versionstring)) {
                    cleanfoldername = "Proton-" + versionstring;
                } else if (analyze_version_string(protonversions[j], "(\\d+.\\d+)-\\d+", versionstring)) {
                    cleanfoldername = "Proton " + versionstring;
                }
            }
        }

        for (int l = 0; l < inlibrary.size(); ++l) {
            QString subsearchpath = inlibrary[l] + "/steamapps/common/" + cleanfoldername;
            QFile versionfile(subsearchpath + "/version");
            if (versionfile.exists()) {
                protonPath = subsearchpath + "/dist";
            }
        }

        /* Last chance to get a Proton runtime path from a version file only by looking in compatibilitytools.d */
        if (protonPath == "") {
            QString tempfilepath = QDir::homePath() + "/.steam/root/compatibilitytools.d/" + cleanfoldername;
            QFile customprotonversionfile(tempfilepath + "/version");
            if (customprotonversionfile.exists()) {
                protonPath = tempfilepath + "/dist";
            }
        }
    }

    return protonPath;
}

/*
 * Gets the Proton runtime's and Proton prefix' paths
 */
QVector<QString> get_proton_paths(const QString & input_path, const QString & input_appid, const QVector<QString> & inlibrary)
{
    QString configpath = input_path + '/' + input_appid;
    QDir prefixpath(configpath + "/pfx");
    QString runtimepath = get_proton_version(configpath,inlibrary);
    if (runtimepath != "" && prefixpath.exists()) {
        return {runtimepath, prefixpath.absolutePath()};
    }
    return {};
}
