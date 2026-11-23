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

//! The servdash ini file.
//!
//! The format is the one the Win32 servdash reads: `;` comments, `[section]` per process,
//! `Key=Value` split at the first `=`, no trimming, and the unnamed section before the first
//! header holds the dashboard `Title`. The patch terminal generates these files for each shard,
//! so the keys it writes keep their meaning here. Keys that the Win32 tool does not know
//! (`Control`, `ShutdownInput`) are ignored by it, which keeps one ini usable by both.

use anyhow::{bail, Context, Result};
use std::path::{Path, PathBuf};
use std::time::Duration;

/// Priority of a section without a `Priority` key. Such sections start one at a time.
pub const PRIORITY_NONE: i32 = i32::MAX;

/// The `LaunchCtrl` key: the launch control protocol shared with the admin executor service.
#[derive(Clone, Debug)]
pub struct LaunchCtrlCfg {
	pub ctrl_file: PathBuf,
	pub state_file: PathBuf,
	/// LAUNCH, RUNNING, STOP, STOPPED in the generated files; indexed by `LaunchCtrlValue`.
	pub values: [String; 4],
}

#[derive(Clone, Debug)]
pub struct SectionCfg {
	pub name: String,
	/// `Title`: fixed button text. Without it the button follows the process title.
	pub title: Option<String>,
	pub working_directory: Option<PathBuf>,
	pub launch_cmd: String,
	pub launch_args: String,
	pub shutdown_cmd: Option<String>,
	pub shutdown_args: String,
	/// `ShutdownInput`: a line written to the process stdin to stop it, such as `quit` for a NeL service.
	pub shutdown_input: Option<String>,
	/// `ReadyPattern`: a regex on the process title (control sections) or on the output lines.
	pub ready_pattern: Option<String>,
	pub priority: i32,
	pub auto_start: bool,
	/// `Control=1`: a NeL service; the dashboard opens a console channel and passes it as `--ctrl`.
	pub control: bool,
	pub launch_ctrl: Option<LaunchCtrlCfg>,
	/// `RestartDelay`: seconds after which a process that exited on its own starts again. For a
	/// process without launch control, such as a patchman: what the relaunch loops of the screen
	/// setup do. A process stopped or killed from the dashboard stays down.
	pub restart_delay: Option<Duration>,
}

#[derive(Clone, Debug)]
pub struct DashCfg {
	pub title: String,
	pub sections: Vec<SectionCfg>,
}

struct RawSection {
	name: String,
	keys: Vec<(String, String)>,
}

impl RawSection {
	fn get(&self, key: &str) -> Option<&str> {
		// the last assignment wins, as with a dictionary store
		self.keys.iter().rev().find(|(k, _)| k == key).map(|(_, v)| v.as_str())
	}
}

fn parse_raw(text: &str) -> Vec<RawSection> {
	let mut sections: Vec<RawSection> = vec![RawSection { name: String::new(), keys: Vec::new() }];
	for line in text.lines() {
		let line = line.strip_suffix('\r').unwrap_or(line);
		if line.starts_with(';') {
			continue;
		}
		if let Some(rest) = line.strip_prefix('[') {
			let name = match rest.find(']') {
				Some(end) => rest[..end].to_string(),
				None => rest.to_string(),
			};
			if let Some(existing) = sections.iter_mut().find(|s| s.name == name) {
				// a repeated header replaces the earlier section, keeping its place
				existing.keys.clear();
			} else {
				sections.push(RawSection { name, keys: Vec::new() });
			}
			continue;
		}
		if line.is_empty() {
			continue;
		}
		let (key, value) = match line.find('=') {
			Some(pos) => (&line[..pos], &line[pos + 1..]),
			None => (line, ""),
		};
		let current = sections.last_mut().expect("there is always a current section");
		current.keys.push((key.to_string(), value.to_string()));
	}
	sections
}

/// Windows path separators in a path-like value, for inis written on Windows and used on Unix.
fn native_path(value: &str) -> PathBuf {
	if cfg!(windows) {
		PathBuf::from(value)
	} else {
		PathBuf::from(value.replace('\\', "/"))
	}
}

fn parse_section(raw: &RawSection) -> Result<SectionCfg> {
	let launch_cmd = raw.get("LaunchCmd").unwrap_or("").to_string();
	if launch_cmd.is_empty() {
		bail!("section [{}] has no LaunchCmd", raw.name);
	}
	let working_directory = raw.get("WorkingDirectory").filter(|v| !v.is_empty()).map(native_path);
	let priority = match raw.get("Priority") {
		Some(v) => v.trim().parse::<i32>().with_context(|| format!("section [{}] has an invalid Priority '{}'", raw.name, v))?,
		None => PRIORITY_NONE,
	};
	let launch_ctrl = match raw.get("LaunchCtrl") {
		Some(v) => {
			let parts: Vec<&str> = v.split(',').collect();
			if parts.len() != 6 {
				bail!("section [{}] LaunchCtrl needs 6 comma separated values, got {}", raw.name, parts.len());
			}
			let combine = |file: &str| -> PathBuf {
				match &working_directory {
					Some(wd) => wd.join(native_path(file)),
					None => native_path(file),
				}
			};
			Some(LaunchCtrlCfg {
				ctrl_file: combine(parts[0]),
				state_file: combine(parts[1]),
				values: [parts[2].to_string(), parts[3].to_string(), parts[4].to_string(), parts[5].to_string()],
			})
		}
		None => None,
	};
	let restart_delay = match raw.get("RestartDelay").filter(|v| !v.trim().is_empty()) {
		Some(v) => {
			let seconds: f64 = v.trim().parse().with_context(|| format!("section [{}] has an invalid RestartDelay '{}'", raw.name, v))?;
			if seconds < 0.0 {
				bail!("section [{}] has a negative RestartDelay", raw.name);
			}
			Some(Duration::from_secs_f64(seconds))
		}
		None => None,
	};
	Ok(SectionCfg {
		name: raw.name.clone(),
		title: raw.get("Title").map(str::to_string),
		working_directory,
		launch_cmd,
		launch_args: raw.get("LaunchArgs").unwrap_or("").to_string(),
		shutdown_cmd: raw.get("ShutdownCmd").filter(|v| !v.is_empty()).map(str::to_string),
		shutdown_args: raw.get("ShutdownArgs").unwrap_or("").to_string(),
		shutdown_input: raw.get("ShutdownInput").filter(|v| !v.is_empty()).map(str::to_string),
		ready_pattern: raw.get("ReadyPattern").filter(|v| !v.is_empty()).map(str::to_string),
		priority,
		auto_start: raw.get("AutoStart") != Some("0"),
		control: raw.get("Control") == Some("1"),
		launch_ctrl,
		restart_delay,
	})
}

pub fn read(path: &Path) -> Result<DashCfg> {
	let bytes = std::fs::read(path).with_context(|| format!("cannot read '{}'", path.display()))?;
	let text = String::from_utf8_lossy(&bytes);
	let text = text.strip_prefix('\u{feff}').unwrap_or(&text);
	let raw = parse_raw(text);
	let title = raw[0].get("Title").unwrap_or("Service Dashboard").to_string();
	let mut sections = Vec::new();
	for section in raw.iter().skip(1) {
		sections.push(parse_section(section)?);
	}
	Ok(DashCfg { title, sections })
}

impl SectionCfg {
	/// The button text, as the Win32 tool shows it before a process title arrives.
	pub fn label(&self) -> String {
		self.title.clone().unwrap_or_else(|| self.launch_cmd.clone())
	}

	/// The program to run: a relative command starting with `.` is taken from the working directory.
	pub fn program(&self, command: &str) -> PathBuf {
		match &self.working_directory {
			Some(wd) if command.starts_with('.') => {
				// the child runs in the working directory too, so the joined path is made absolute
				// here; otherwise the kernel would resolve it against that directory a second time
				let joined = wd.join(native_path(command));
				std::env::current_dir().map(|cwd| cwd.join(&joined)).unwrap_or(joined)
			}
			_ => PathBuf::from(command),
		}
	}
}

#[cfg(test)]
mod tests {
	use super::*;

	#[test]
	fn parses_generated_shard_ini() {
		let text = "[]\r\nTitle=Mainland\r\n\r\n[egs_mainland]\r\nTitle=egs_mainland\r\nReadyPattern=^[^*].+Service Console\r\nWorkingDirectory=.\\egs_mainland\r\nLaunchCmd=ryzom_entities_game_service\r\nLaunchArgs=-C. -L.\r\nLaunchCtrl=.\\egs_mainland.launch_ctrl,.\\egs_mainland.state,LAUNCH,RUNNING,STOP,STOPPED\r\nControl=1\r\nPriority=3\r\n";
		let raw = parse_raw(text);
		assert_eq!(raw.len(), 2);
		let s = parse_section(&raw[1]).unwrap();
		assert_eq!(s.name, "egs_mainland");
		assert_eq!(s.priority, 3);
		assert!(s.control);
		assert!(s.auto_start);
		let lc = s.launch_ctrl.unwrap();
		assert_eq!(lc.values[2], "STOP");
		assert!(lc.ctrl_file.to_string_lossy().contains("egs_mainland.launch_ctrl"));
	}

	#[test]
	fn restart_delay_in_seconds() {
		let raw = parse_raw("[x]\nLaunchCmd=a\nRestartDelay=2.5\n");
		let s = parse_section(&raw[1]).unwrap();
		assert_eq!(s.restart_delay, Some(Duration::from_millis(2500)));
		let raw = parse_raw("[x]\nLaunchCmd=a\n");
		assert_eq!(parse_section(&raw[1]).unwrap().restart_delay, None);
		let raw = parse_raw("[x]\nLaunchCmd=a\nRestartDelay=soon\n");
		assert!(parse_section(&raw[1]).is_err());
	}

	#[test]
	fn value_keeps_equals_sign() {
		let raw = parse_raw("[x]\nLaunchCmd=a\nLaunchArgs=--ctrl=b=c\n");
		let s = parse_section(&raw[1]).unwrap();
		assert_eq!(s.launch_args, "--ctrl=b=c");
	}
}
