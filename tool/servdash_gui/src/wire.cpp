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

#include "wire.h"

#include <QJsonDocument>

#include <climits>

namespace Wire {

QString stateName(CProcessEntry::TState state)
{
	switch (state)
	{
	case CProcessEntry::New: return "New";
	case CProcessEntry::Launched: return "Launched";
	case CProcessEntry::Captured: return "Captured";
	case CProcessEntry::Ready: return "Ready";
	case CProcessEntry::Stopping: return "Stopping";
	case CProcessEntry::Stopped: return "Stopped";
	}
	return "New";
}

QString commandKindName(CProcessEntry::TCommandKind kind)
{
	switch (kind)
	{
	case CProcessEntry::CommandKind: return "Command";
	case CProcessEntry::HandlerKind: return "Handler";
	case CProcessEntry::MemberKind: return "Member";
	}
	return "Command";
}

QJsonValue colorValue(const QColor &color)
{
	if (!color.isValid())
		return "Default";
	// the colours CWindowDisplayer::doDisplay assigns per log type, as the servdash_cli daemon names them
	switch (color.rgb() & 0xFFFFFF)
	{
	case 0xFF0000: return "LightRed";
	case 0x800000: return "Yellow";
	case 0x808080: return "DarkGray";
	default: break;
	}
	QJsonArray rgb;
	rgb.append(color.red());
	rgb.append(color.green());
	rgb.append(color.blue());
	QJsonObject wrapped;
	wrapped["Rgb"] = rgb;
	return wrapped;
}

QJsonObject lineObject(const CProcessEntry::CLine &line)
{
	QJsonObject span;
	span["text"] = line.Text;
	span["fg"] = colorValue(line.Color);
	QJsonArray spans;
	spans.append(span);
	QJsonObject o;
	o["spans"] = spans;
	o["raw"] = line.Raw;
	o["time"] = line.Time;
	o["idx"] = (qint64)line.Idx;
	o["ts"] = line.Ts;
	return o;
}

QJsonObject commandObject(const CProcessEntry::CCommand &command)
{
	QJsonObject o;
	o["name"] = command.Name;
	o["kind"] = commandKindName(command.Kind);
	o["args"] = command.Args;
	o["help"] = command.Help;
	return o;
}

QJsonArray commandsArray(const QMap<QString, CProcessEntry::CCommand> &commands)
{
	QJsonArray array;
	for (const CProcessEntry::CCommand &command : commands)
		array.append(commandObject(command));
	return array;
}

QJsonObject processInfo(const CProcessEntry *entry)
{
	const CSectionConfig &cfg = entry->config();
	QJsonObject o;
	o["name"] = entry->name();
	o["label"] = entry->label();
	o["title"] = entry->title();
	o["state"] = stateName(entry->state());
	o["started"] = entry->started();
	o["control"] = cfg.Control;
	o["control_connected"] = entry->controlConnected();
	QJsonArray labels;
	for (const QString &label : entry->labels())
		labels.append(label);
	o["labels"] = labels;
	o["pid"] = entry->running() ? QJsonValue(entry->pid()) : QJsonValue();
	o["started_at"] = entry->running() ? QJsonValue(entry->startedAt()) : QJsonValue();
	o["exit_code"] = entry->hasExitCode() ? QJsonValue(entry->exitCode()) : QJsonValue();
	o["relaunch_pending"] = entry->relaunchPending();
	o["crash_chain"] = entry->crashChain();
	o["has_launch_ctrl"] = entry->hasLaunchCtrl();
	o["ctrl_value"] = entry->hasLaunchCtrl() ? QJsonValue(entry->ctrlValueName()) : QJsonValue();
	o["priority"] = cfg.Priority == INT_MAX ? QJsonValue() : QJsonValue(cfg.Priority);
	o["auto_start"] = cfg.AutoStart;
	const QString args = cfg.LaunchArgs.trimmed();
	o["command"] = args.isEmpty() ? cfg.LaunchCmd : cfg.LaunchCmd + " " + args;
	o["working_directory"] = cfg.WorkingDirectory;
	o["next_idx"] = (qint64)entry->nextIdx();
	o["first_idx"] = entry->lines().isEmpty() ? 0 : (qint64)entry->lines().first().Idx;
	o["channel_version"] = entry->controlConnected() ? entry->channelVersion() : 0;
	return o;
}

QJsonObject message(const QString &variant, const QJsonObject &fields)
{
	QJsonObject o;
	o[variant] = fields;
	return o;
}

QByteArray encode(const QJsonValue &msg)
{
	QJsonDocument doc;
	if (msg.isObject())
		doc.setObject(msg.toObject());
	else if (msg.isArray())
		doc.setArray(msg.toArray());
	else
	{
		// a bare variant is a JSON string, which a document cannot hold on its own
		QByteArray text = QJsonDocument(QJsonArray { msg }).toJson(QJsonDocument::Compact);
		// strip the array brackets
		text = text.mid(1, text.size() - 2);
		text.append('\n');
		return text;
	}
	QByteArray text = doc.toJson(QJsonDocument::Compact);
	text.append('\n');
	return text;
}

bool decode(const QJsonValue &msg, QString &variant, QJsonObject &fields)
{
	fields = QJsonObject();
	if (msg.isString())
	{
		variant = msg.toString();
		return !variant.isEmpty();
	}
	if (!msg.isObject())
		return false;
	const QJsonObject o = msg.toObject();
	if (o.size() != 1)
		return false;
	variant = o.begin().key();
	fields = o.begin().value().toObject();
	return true;
}

} // namespace Wire
