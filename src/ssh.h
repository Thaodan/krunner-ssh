/*
 *   Copyright (C) 2009 Edward "Hades" Toroshchin <kde@hades.name>
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SSHRUNNER_H
#define SSHRUNNER_H

#include <KRunner/AbstractRunner>
#include <krunner_version.h>

class SSHConfigReader;

class KRunnerSSH : public Plasma::AbstractRunner
{
public:
    KRunnerSSH(QObject *parent,
#if KRUNNER_VERSION >= QT_VERSION_CHECK(5, 77, 0)
               const KPluginMetaData &metadata,
#endif
               const QVariantList &args);
    ~KRunnerSSH() override;

    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

protected:
    void init() override;

private:
    bool isRunning(const QString name);
    Plasma::QueryMatch constructMatch(QString host, Plasma::QueryMatch::Type priority);
    SSHConfigReader *rd;
    QList<QAction *> matchActionList;
};

K_EXPORT_PLASMA_RUNNER(krunner - ssh, KRunnerSSH)

#endif
