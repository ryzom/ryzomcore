// Ryzom Core - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SERVDASH_WIRE_H
#define SERVDASH_WIRE_H

#include "process_entry.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

/// The JSON line protocol between a dashboard and its clients, as servdash_cli defines it in
/// its proto module: one document per line, an enum message as `{"Variant": {fields}}` or the
/// bare `"Variant"` string for a message without fields, which is how serde writes it. The
/// dashboard serves it on a local endpoint for the terminal client and the MCP servers of
/// both tools; every message a servdash_cli daemon sends, this writes the same way.
namespace Wire {

/// Protocol version 2: line indices and times, process details, Exec and ExecDone.
const int Version = 2;

QString stateName(CProcessEntry::TState state);
QString commandKindName(CProcessEntry::TCommandKind kind);
/// `"Default"`, a named terminal colour for the console colours of CWindowDisplayer, or `{"Rgb": [r, g, b]}`.
QJsonValue colorValue(const QColor &color);
QJsonObject lineObject(const CProcessEntry::CLine &line);
QJsonObject commandObject(const CProcessEntry::CCommand &command);
QJsonArray commandsArray(const QMap<QString, CProcessEntry::CCommand> &commands);
/// The ProcessInfo of the protocol.
QJsonObject processInfo(const CProcessEntry *entry);
QJsonObject message(const QString &variant, const QJsonObject &fields);
/// Compact JSON and the newline that ends the message.
QByteArray encode(const QJsonValue &msg);

/// Splits a message into its variant name and its fields (empty for a bare variant).
bool decode(const QJsonValue &msg, QString &variant, QJsonObject &fields);

} // namespace Wire

#endif // SERVDASH_WIRE_H
