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
//! besides, and the daemon's notes, in arrival order; raw lines are tagged.

use serde::{Deserialize, Serialize};
use tokio::io::{AsyncWrite, AsyncWriteExt};

pub const VERSION: u32 = 1;

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
}

impl StyledLine {
	pub fn plain(text: impl Into<String>) -> StyledLine {
		StyledLine { spans: vec![Span { text: text.into(), fg: Color::Default, bold: false }], raw: false, time: String::new() }
	}

	pub fn colored(text: impl Into<String>, fg: Color) -> StyledLine {
		StyledLine { spans: vec![Span { text: text.into(), fg, bold: false }], raw: false, time: String::new() }
	}

	pub fn into_raw(mut self) -> StyledLine {
		self.raw = true;
		self
	}

	pub fn text(&self) -> String {
		self.spans.iter().map(|s| s.text.as_str()).collect()
	}
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
	Notice {
		text: String,
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
