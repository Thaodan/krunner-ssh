/*
 *   Copyright (C) 2009 Edward "Hades" Toroshchin <kde@hades.name>
 *   Copyright (C) 2008 Sun Microsystems, Inc.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. */

#include <QAction>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QIcon>
#include <QMutexLocker>
#include <QProcess>
#include <QTextStream>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KShell>
#include <kio_version.h>
#if KIO_VERSION >= QT_VERSION_CHECK(5, 71, 0)
#include <KIO/CommandLauncherJob>
#include <KJobUiDelegate>
#include <KMessageBox>
#endif
#include <krun.h>

#include <iostream>

#include "ssh.h"

struct SSHHost {
    QString name;
};

class SSHConfigReader
{
    QList<SSHHost> *list;
    QDateTime lastChecked;
    QMutex mutex;

    QString sshdir;

public:
    SSHConfigReader()
        : list(0)
    {
        sshdir = QString(QDir::home().filePath(".ssh"));
    }

    ~SSHConfigReader()
    {
        if (list)
            delete list;
        list = 0;
    }

    void updateAsNeccessary()
    {
        QMutexLocker _ml(&mutex);
        QDir dir(sshdir);

        QString config_path = dir.filePath("config");

        if (list && lastChecked >= QFileInfo(config_path).lastModified()) {
            return;
        }

        if (list)
            delete list;

        list = new QList<SSHHost>;

        QFile config(config_path);
        if (!config.open(QIODevice::ReadOnly)) {
            return;
        }

        QTextStream stream(&config);
        stream.setCodec("UTF-8");

        while (!stream.atEnd()) {
            QString line = stream.readLine();

            line = line.trimmed();
            if (line.isEmpty()) {
                continue;
            }

            if (line.startsWith("host ", Qt::CaseInsensitive)) {
                QString hostname = line.mid(5).trimmed();

                SSHHost host;
                host.name = hostname;

                (*list) << host;
            }
        }

        config.close();

        lastChecked = QDateTime::currentDateTime();
    }

    QList<SSHHost> hosts()
    {
        updateAsNeccessary();
        if (!list)
            return QList<SSHHost>();

        return *list;
    }
};

KRunnerSSH::KRunnerSSH(QObject *parent,
#if KRUNNER_VERSION >= QT_VERSION_CHECK(5, 77, 0)
                       const KPluginMetaData &metaData,
#endif
                       const QVariantList &args)
    : Plasma::AbstractRunner(parent, metaData, args)
    , rd(0)
{
    rd = new SSHConfigReader;
    setSpeed(AbstractRunner::SlowSpeed);
}

KRunnerSSH::~KRunnerSSH()
{
    if (rd)
        delete rd;
    rd = 0;
}

void KRunnerSSH::init()
{
    auto action = new QAction(QIcon::fromTheme("utilities-terminal"), i18n("SSH to remote host"));
    action->setData("ssh");

    matchActionList << action;
}

Plasma::QueryMatch KRunnerSSH::constructMatch(QString host, Plasma::QueryMatch::Type priority)
{
    Plasma::QueryMatch match(this);
    match.setText(QString("SSH to host %1").arg(host));
    match.setType(priority);
    match.setIcon(QIcon::fromTheme("utilities-terminal"));
    match.setData(QVariant(host));
    return match;
}

void KRunnerSSH::match(Plasma::RunnerContext &context)
{
    QString request = context.query();

    bool startsWithSSH = false;
    bool exactMatchFound = false;
    if (request.startsWith("ssh ", Qt::CaseInsensitive)) {
        request.remove(0, 4);
        startsWithSSH = true;
    }

    if (request.isEmpty())
        return;

    QList<Plasma::QueryMatch> matches;
    foreach (SSHHost h, rd->hosts()) {
        if (h.name.compare(request, Qt::CaseInsensitive) == 0) {
            matches << constructMatch(h.name, Plasma::QueryMatch::ExactMatch);
            exactMatchFound = true;
        } else if (request.length() > 2) {
            if (h.name.startsWith(request, Qt::CaseInsensitive)) {
                matches << constructMatch(h.name, Plasma::QueryMatch::PossibleMatch);
            } else if (h.name.contains(request, Qt::CaseInsensitive)) {
                matches << constructMatch(h.name, Plasma::QueryMatch::CompletionMatch);
            }
        }
    }

    if (startsWithSSH && !exactMatchFound) {
        Plasma::QueryMatch match(this);

        match.setType(Plasma::QueryMatch::CompletionMatch);

        match.setText(QString("SSH to %1").arg(request));
        match.setIcon(QIcon::fromTheme("utilities-terminal"));
        match.setData(QVariant(request));
        match.setActions(matchActionList);

        matches << match;
    }

    context.addMatches(matches);
}

void KRunnerSSH::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context);

    QString host = match.data().toString();
    KShell::quoteArg(host);

    QString command = QString("ssh %1").arg(host);

    KConfigGroup config(KSharedConfig::openConfig(QStringLiteral("kdeglobals")), "General");
    QString terminal = config.readPathEntry("TerminalApplication", QStringLiteral("konsole"));
    QString konsole_command = QString(terminal + " -e %1").arg(command);

#if KIO_VERSION >= QT_VERSION_CHECK(5, 69, 0)
    auto job = new KIO::CommandLauncherJob(konsole_command);
    connect(job, &KJob::finished, this, [this, command](KJob *job) {
        if (job->error())
            KMessageBox::error(nullptr, i18n("Failed to launch command: %1").arg(command), i18n("Failed to lanch"));
    });
    job->start();
#else
    KRun::runCommand(konsole_command, 0);
#endif
}

bool KRunnerSSH::isRunning(const QString name)
{
    Q_UNUSED(name);
    // TODO Work out if there is an active connection to a host
    return false;
}

K_PLUGIN_CLASS_WITH_JSON(KRunnerSSH, "KRunnerSSH.json")

#include "ssh.moc"
