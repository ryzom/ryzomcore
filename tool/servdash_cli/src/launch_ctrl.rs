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

//! The launch control files shared with the admin executor service (AES). The AES writes
//! `<alias>.launch_ctrl` with LAUNCH or STOP, writes `<alias>.deferred_launch_ctrl` with
//! LAUNCH to restart a service it has told to quit, and polls `<alias>.state` for RUNNING or
//! STOPPED, together with `<alias>.start_count` for its crash chain detection. This is what
//! service_launcher.sh does under screen on Linux and what the Win32 servdash does through its
//! LaunchCtrl key, so the web admin shard controls work the same against this dashboard.
//!
//! Value indices follow the Win32 tool: 0 LAUNCH, 1 RUNNING, 2 STOP, 3 STOPPED.

use crate::config::LaunchCtrlCfg;
use crate::proto::ProcState;
use std::path::{Path, PathBuf};
use std::time::SystemTime;

pub const LAUNCH: usize = 0;
pub const RUNNING: usize = 1;
pub const STOP: usize = 2;
pub const STOPPED: usize = 3;

pub struct LaunchCtrl {
	cfg: LaunchCtrlCfg,
	deferred_file: Option<PathBuf>,
	start_count_file: Option<PathBuf>,
	start_count: u32,
	/// The last value this side applied, `LaunchCtrlValue` in the Win32 tool.
	pub value: usize,
	/// Content and modification time of the control file at the last poll.
	last_seen: Option<(String, Option<SystemTime>)>,
}

fn sibling_with_suffix(path: &Path, old_suffix: &str, new_suffix: &str) -> Option<PathBuf> {
	let name = path.file_name()?.to_string_lossy().to_string();
	let stem = name.strip_suffix(old_suffix)?;
	Some(path.with_file_name(format!("{}{}", stem, new_suffix)))
}

fn read_text(path: &Path) -> Option<String> {
	std::fs::read(path).ok().map(|bytes| String::from_utf8_lossy(&bytes).to_string())
}

fn write_text(path: &Path, text: &str) -> std::io::Result<()> {
	if let Some(parent) = path.parent() {
		if !parent.as_os_str().is_empty() {
			std::fs::create_dir_all(parent)?;
		}
	}
	std::fs::write(path, text)
}

impl LaunchCtrl {
	/// Applies the startup rules of the Win32 tool and returns whether the process should
	/// auto launch: a missing control file is created with LAUNCH or STOP from the AutoStart
	/// key, an existing one decides by itself. The state file starts as STOPPED and the start
	/// counter at zero, as the Linux launcher does.
	pub fn new(cfg: LaunchCtrlCfg, auto_start: bool) -> (LaunchCtrl, bool) {
		let deferred_file = sibling_with_suffix(&cfg.ctrl_file, ".launch_ctrl", ".deferred_launch_ctrl");
		let start_count_file = sibling_with_suffix(&cfg.state_file, ".state", ".start_count");
		let mut lc = LaunchCtrl { cfg, deferred_file, start_count_file, start_count: 0, value: STOPPED, last_seen: None };

		// a restart order left by the AES while no dashboard ran is an order like any other
		lc.promote_deferred();
		let auto_launch = match lc.read_ctrl() {
			Some(text) => lc.value_of(&text, ProcState::New) == LAUNCH,
			None => {
				let idx = if auto_start { LAUNCH } else { STOP };
				lc.write_ctrl(idx);
				auto_start
			}
		};
		lc.write_state(STOPPED);
		lc.write_start_count();
		lc.last_seen = lc.observe();
		(lc, auto_launch)
	}

	fn observe(&self) -> Option<(String, Option<SystemTime>)> {
		let text = read_text(&self.cfg.ctrl_file)?;
		let mtime = std::fs::metadata(&self.cfg.ctrl_file).ok().and_then(|m| m.modified().ok());
		Some((text, mtime))
	}

	pub fn read_ctrl(&self) -> Option<String> {
		read_text(&self.cfg.ctrl_file)
	}

	/// The value index of a control file text; an unknown text reads as the current state.
	/// Surrounding whitespace is ignored, so `echo LAUNCH > x.launch_ctrl` works as it did with
	/// the shell launcher.
	pub fn value_of(&self, text: &str, state: ProcState) -> usize {
		let text = text.trim();
		if let Some(idx) = self.cfg.values.iter().position(|v| v == text) {
			return idx;
		}
		match state {
			ProcState::Stopping => STOP,
			ProcState::Stopped => STOPPED,
			ProcState::Launched | ProcState::Captured | ProcState::Ready => RUNNING,
			ProcState::New => LAUNCH,
		}
	}

	/// The name of the value this side applied last, as the control files spell it.
	pub fn value_name(&self) -> &str {
		self.cfg.values.get(self.value).map(String::as_str).unwrap_or("")
	}

	pub fn ctrl_is(&self, idx: usize) -> bool {
		self.read_ctrl().map(|t| t.trim() == self.cfg.values[idx]).unwrap_or(false)
	}

	/// Moves a deferred order onto the control file, as the shell launcher does on each round.
	fn promote_deferred(&mut self) {
		if let Some(deferred) = &self.deferred_file {
			if deferred.exists() {
				if let Err(e) = std::fs::rename(deferred, &self.cfg.ctrl_file) {
					crate::log::note(&format!("cannot move '{}' to '{}': {}", deferred.display(), self.cfg.ctrl_file.display(), e));
				}
			}
		}
	}

	pub fn write_ctrl(&mut self, idx: usize) {
		if let Err(e) = write_text(&self.cfg.ctrl_file, &self.cfg.values[idx]) {
			crate::log::note(&format!("cannot write '{}': {}", self.cfg.ctrl_file.display(), e));
		}
		// what we wrote ourselves is not a change to react to
		self.last_seen = self.observe();
	}

	/// Writes the state file for RUNNING and STOPPED; the other values are not states.
	pub fn write_state(&self, idx: usize) {
		if idx != RUNNING && idx != STOPPED {
			return;
		}
		if let Err(e) = write_text(&self.cfg.state_file, &self.cfg.values[idx]) {
			crate::log::note(&format!("cannot write '{}': {}", self.cfg.state_file.display(), e));
		}
	}

	fn write_start_count(&self) {
		if let Some(path) = &self.start_count_file {
			if let Err(e) = write_text(path, &self.start_count.to_string()) {
				crate::log::note(&format!("cannot write '{}': {}", path.display(), e));
			}
		}
	}

	/// Sets the value this side applied and keeps the state file in step with it.
	pub fn set_value(&mut self, idx: usize) {
		self.value = idx;
		if (idx == RUNNING || idx == STOPPED) && read_text(&self.cfg.state_file).as_deref() != Some(self.cfg.values[idx].as_str()) {
			self.write_state(idx);
		}
	}

	pub fn count_launch(&mut self) {
		self.start_count += 1;
		self.write_start_count();
	}

	/// Returns the new value index when the control file changed since the last poll. A
	/// rewrite with the same content counts, since the AES rewrites LAUNCH after a crash.
	pub fn poll(&mut self, state: ProcState) -> Option<usize> {
		if matches!(state, ProcState::Stopped | ProcState::New) {
			// the AES writes the deferred order before it asks the service to quit; it may
			// already be stopped, or the daemon may have started after the order was written
			self.promote_deferred();
		}
		let seen = self.observe();
		if seen == self.last_seen {
			return None;
		}
		self.last_seen = seen.clone();
		let (text, _) = seen?;
		Some(self.value_of(&text, state))
	}

	/// After the process exited: a LAUNCH order is consumed so a crash does not relaunch in
	/// a loop, and a deferred order becomes the next order. This is what the Linux launcher does.
	pub fn after_exit(&mut self) {
		if self.ctrl_is(LAUNCH) {
			if let Err(e) = std::fs::remove_file(&self.cfg.ctrl_file) {
				crate::log::note(&format!("cannot remove '{}': {}", self.cfg.ctrl_file.display(), e));
			}
		}
		self.promote_deferred();
		// the next poll reports the resulting order, if any
		self.last_seen = None;
		if self.cfg.ctrl_file.exists() {
			// keep a STOP or an unchanged file from reading as a change
			let text = self.read_ctrl().unwrap_or_default();
			if self.value_of(&text, ProcState::Stopped) != LAUNCH {
				self.last_seen = self.observe();
			}
		}
	}
}
