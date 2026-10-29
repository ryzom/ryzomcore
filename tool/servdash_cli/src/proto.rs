// Service dashboard for Ryzom Core
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

//! Messages between the daemon and its attached clients: one JSON document per line.
//! The daemon streams every process to every client, and a client keeps the lines of all
//! of them, so switching panes needs no round trip and a reattach gets a full snapshot.
//! Each process has one stream: its console log, whatever it writes to stdout or stderr
//! besides, and the daemon's notes, in arrival order; raw lines are tagged. Every line carries
//! the index the daemon gave it, counting from 1 per process and never reused, and its time of
//! arrival, so a client can name a range of the log exactly.
//!
//! Version 2 added the line index and time stamp, the process details in ProcessInfo, and
//! Exec with ExecDone: a command with its arguments already split, answered when it ran.

use serde::{Deserialize, Serialize};
use tokio::io::{AsyncWrite, AsyncWriteExt};

pub const VERSION: u32 = 2;

/// The process states of the Win32 tool, with the same meaning.
#[derive(Serialize, Deserialize, Clone, Copy, PartialEq, Eq, Debug)]
pub enum ProcState {
	New,
	/// Process started.
	Launched,
	/// The console is attached: control channel connected, or output pipes for a plain process.
	Captured,
	/// The ready pattern matched, or no pattern was given.
	Ready,
	Stopping,
	Stopped,
}

#[derive(Serialize, Deserialize, Clone, Copy, PartialEq, Eq, Debug, Default)]
pub enum Color {
	#[default]
	Default,
	Black,
	Red,
	Green,
	Yellow,
	Blue,
	Magenta,
	Cyan,
	Gray,
	DarkGray,
	LightRed,
	LightGreen,
	LightYellow,
	LightBlue,
	LightMagenta,
	LightCyan,
	White,
	Rgb(u8, u8, u8),
}

#[derive(Serialize, Deserialize, Clone, Debug, Default, PartialEq, Eq)]
pub struct Span {
	pub text: String,
	#[serde(default, skip_serializing_if = "is_default_color")]
	pub fg: Color,
	#[serde(default, skip_serializing_if = "std::ops::Not::not")]
	pub bold: bool,
}

fn is_default_color(c: &Color) -> bool {
	*c == Color::Default
}

#[derive(Serialize, Deserialize, Clone, Debug, Default, PartialEq, Eq)]
pub struct StyledLine {
	pub spans: Vec<Span>,
	/// From stdout or stderr, or a note of the daemon, rather than from the console channel.
	#[serde(default, skip_serializing_if = "std::ops::Not::not")]
	pub raw: bool,
	/// Local time of arrival at the daemon, `hh:mm:ss`.
	#[serde(default, skip_serializing_if = "String::is_empty")]
	pub time: String,
	/// Index of the line in the stream of its process, from 1; dropped lines leave a gap.
	#[serde(default, skip_serializing_if = "is_zero_u64")]
	pub idx: u64,
	/// Time of arrival at the daemon, milliseconds since the Unix epoch.
	#[serde(default, skip_serializing_if = "is_zero_i64")]
	pub ts: i64,
}

fn is_zero_u64(v: &u64) -> bool {
	*v == 0
}

fn is_zero_i64(v: &i64) -> bool {
	*v == 0
}

impl StyledLine {
	pub fn plain(text: impl Into<String>) -> StyledLine {
		StyledLine { spans: vec![Span { text: text.into(), fg: Color::Default, bold: false }], ..Default::default() }
	}

	pub fn colored(text: impl Into<String>, fg: Color) -> StyledLine {
		StyledLine { spans: vec![Span { text: text.into(), fg, bold: false }], ..Default::default() }
	}

	pub fn into_raw(mut self) -> StyledLine {
		self.raw = true;
		self
	}

	pub fn text(&self) -> String {
		self.spans.iter().map(|s| s.text.as_str()).collect()
	}
}

/// What an entry of the command table is, as the service reports it.
#[derive(Serialize, Deserialize, Clone, Copy, PartialEq, Eq, Debug)]
pub enum CommandKind {
	/// A command, or a variable (its name starts with an upper case letter).
	Command,
	/// A named command handler instance; its commands follow as `instance.command`.
	Handler,
	/// A command of a handler instance.
	Member,
}

/// A command of a NeL service, for completion; `instance.command` for a member command.
#[derive(Serialize, Deserialize, Clone, PartialEq, Eq, Debug)]
pub struct CommandInfo {
	pub name: String,
	pub kind: CommandKind,
	/// The argument help, such as `<serviceName> [sid=<serviceId>]`.
	#[serde(default, skip_serializing_if = "String::is_empty")]
	pub args: String,
	#[serde(default, skip_serializing_if = "String::is_empty")]
	pub help: String,
}

#[derive(Serialize, Deserialize, Clone, Debug, PartialEq, Eq)]
pub struct ProcessInfo {
	pub name: String,
	/// Button text: the `Title` key, else the process title once known, else the command.
	pub label: String,
	/// Window title reported by the process, or the label.
	pub title: String,
	pub state: ProcState,
	/// Whether the start button would start (false) or stop (true) the process.
	pub started: bool,
	pub control: bool,
	pub control_connected: bool,
	/// Console labels as the service created them: empty is a row separator, `@Name|cmd` a button.
	pub labels: Vec<String>,
	/// The process id while it runs.
	#[serde(default)]
	pub pid: Option<u32>,
	/// When the current run started, milliseconds since the Unix epoch.
	#[serde(default)]
	pub started_at: Option<i64>,
	/// The exit code of the last run, when it ended by itself with one.
	#[serde(default)]
	pub exit_code: Option<i32>,
	/// A RestartDelay relaunch is scheduled.
	#[serde(default)]
	pub relaunch_pending: bool,
	/// Quick exits in a row, the RestartDelay backoff.
	#[serde(default)]
	pub crash_chain: u32,
	#[serde(default)]
	pub has_launch_ctrl: bool,
	/// The launch control value this side applied last: LAUNCH, RUNNING, STOP or STOPPED.
	#[serde(default)]
	pub ctrl_value: Option<String>,
	/// The `Priority` key; none for a section that starts one at a time.
	#[serde(default)]
	pub priority: Option<i32>,
	#[serde(default)]
	pub auto_start: bool,
	/// The command line as the ini gives it.
	#[serde(default)]
	pub command: String,
	#[serde(default)]
	pub working_directory: String,
	/// The index the next line of the process gets; the lines so far are 1..next_idx.
	#[serde(default)]
	pub next_idx: u64,
	/// The oldest line the daemon still holds, 0 when it holds none.
	#[serde(default)]
	pub first_idx: u64,
	/// The console channel protocol version the service announced, 0 when none is connected.
	#[serde(default)]
	pub channel_version: u32,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub enum ClientMsg {
	Hello {
		version: u32,
	},
	Start {
		name: String,
	},
	Stop {
		name: String,
	},
	Kill {
		name: String,
	},
	StartAll,
	StopAll,
	/// A line for the process stdin.
	Input {
		name: String,
		text: String,
	},
	/// A NeL command for the control channel; falls back to stdin when it is not connected.
	Command {
		name: String,
		text: String,
	},
	ClearConsole {
		name: String,
	},
	/// A NeL command with its arguments already split, over the control channel; nothing is
	/// parsed. ExecDone with the same id follows once the service ran it.
	Exec {
		name: String,
		id: String,
		command: String,
		args: Vec<String>,
	},
	/// Stop every process and exit the daemon.
	Shutdown,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub enum DaemonMsg {
	Hello {
		version: u32,
		title: String,
	},
	Processes {
		processes: Vec<ProcessInfo>,
	},
	Process {
		info: ProcessInfo,
	},
	/// One console label changed; they update several times a second, so not a whole ProcessInfo.
	Label {
		name: String,
		index: usize,
		value: String,
	},
	/// The whole pane of a process: console lines, raw output and the daemon's notes in arrival order.
	Snapshot {
		name: String,
		lines: Vec<StyledLine>,
	},
	Lines {
		name: String,
		lines: Vec<StyledLine>,
	},
	Clear {
		name: String,
	},
	/// The whole command table of a NeL service: on attach, and when a service connects anew.
	Commands {
		name: String,
		commands: Vec<CommandInfo>,
	},
	/// A command the service registered, or changed.
	Command {
		name: String,
		command: CommandInfo,
	},
	/// A command the service unregistered.
	Uncommand {
		name: String,
		command: String,
	},
	Notice {
		text: String,
	},
	/// An Exec ran: its output is in the lines before this message. Not ok when the command
	/// failed, or when the service has no console channel to run it on.
	ExecDone {
		name: String,
		id: String,
		ok: bool,
	},
	Bye,
}

pub async fn write_msg<W: AsyncWrite + Unpin, T: Serialize>(writer: &mut W, msg: &T) -> std::io::Result<()> {
	let mut bytes = serde_json::to_vec(msg).map_err(|e| std::io::Error::new(std::io::ErrorKind::InvalidData, e))?;
	bytes.push(b'\n');
	writer.write_all(&bytes).await
}

pub fn parse_msg<'a, T: Deserialize<'a>>(line: &'a str) -> std::io::Result<T> {
	serde_json::from_str(line).map_err(|e| std::io::Error::new(std::io::ErrorKind::InvalidData, e))
}
