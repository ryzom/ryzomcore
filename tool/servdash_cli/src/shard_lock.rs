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

//! The lock that keeps a shard from being launched twice, shared with servdash_gui: a file
//! next to the ini, `<stem>.servdash.lock`, with three lines, the pid of the dashboard that
//! holds it, its tool name and its host name. A holder whose process is gone is stale and
//! taken over; a live one is reported. The file is created exclusively, so two dashboards
//! starting at once cannot both win.

use std::fmt;
use std::path::{Path, PathBuf};

pub const TOOL_NAME: &str = "servdash_cli";

#[derive(Clone, Debug)]
pub struct Holder {
	pub pid: u32,
	pub tool: String,
	pub host: String,
}

impl fmt::Display for Holder {
	fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
		write!(f, "{} (pid {}) on {}", self.tool, self.pid, self.host)
	}
}

pub struct ShardLock {
	path: PathBuf,
}

pub fn lock_path(ini: &Path) -> PathBuf {
	let stem = ini.file_stem().map(|s| s.to_string_lossy().to_string()).unwrap_or_else(|| "shard".to_string());
	ini.with_file_name(format!("{}.servdash.lock", stem))
}

#[cfg(windows)]
fn host_name() -> String {
	std::env::var("COMPUTERNAME").unwrap_or_else(|_| "localhost".to_string())
}

#[cfg(not(windows))]
fn host_name() -> String {
	let mut buffer = [0u8; 256];
	let ok = unsafe { libc::gethostname(buffer.as_mut_ptr() as *mut libc::c_char, buffer.len()) } == 0;
	let name = if ok { String::from_utf8_lossy(&buffer[..buffer.iter().position(|&b| b == 0).unwrap_or(buffer.len())]).to_string() } else { String::new() };
	if name.is_empty() {
		"localhost".to_string()
	} else {
		name
	}
}

/// Host names differ in case between tools and systems (COMPUTERNAME is upper case, the DNS
/// name lower case), so the comparison ignores it.
fn same_host(a: &str, b: &str) -> bool {
	a.eq_ignore_ascii_case(b)
}

#[cfg(windows)]
fn process_alive(pid: u32) -> bool {
	use windows_sys::Win32::Foundation::{CloseHandle, STILL_ACTIVE};
	use windows_sys::Win32::System::Threading::{GetExitCodeProcess, OpenProcess, PROCESS_QUERY_LIMITED_INFORMATION};
	unsafe {
		let handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, pid);
		if handle.is_null() {
			return false;
		}
		let mut code: u32 = 0;
		let alive = GetExitCodeProcess(handle, &mut code) != 0 && code == STILL_ACTIVE as u32;
		CloseHandle(handle);
		alive
	}
}

#[cfg(not(windows))]
fn process_alive(pid: u32) -> bool {
	unsafe { libc::kill(pid as libc::pid_t, 0) == 0 || *libc::__errno_location() == libc::EPERM }
}

fn read_holder(path: &Path) -> Option<Holder> {
	let text = std::fs::read_to_string(path).ok()?;
	let mut lines = text.lines();
	let pid = lines.next()?.trim().parse().ok()?;
	let tool = lines.next().unwrap_or("").trim().to_string();
	let host = lines.next().unwrap_or("").trim().to_string();
	Some(Holder { pid, tool, host })
}

/// The dashboard holding the shard right now, if any; a stale lock reads as none.
pub fn holder(ini: &Path) -> Option<Holder> {
	let path = lock_path(ini);
	let holder = read_holder(&path)?;
	if same_host(&holder.host, &host_name()) && !process_alive(holder.pid) {
		return None;
	}
	Some(holder)
}

impl ShardLock {
	/// Takes the lock for this process, or reports who holds it.
	pub fn acquire(ini: &Path) -> Result<ShardLock, Holder> {
		let path = lock_path(ini);
		for _ in 0..2 {
			match std::fs::OpenOptions::new().write(true).create_new(true).open(&path) {
				Ok(mut file) => {
					use std::io::Write;
					let _ = write!(file, "{}\n{}\n{}\n", std::process::id(), TOOL_NAME, host_name());
					return Ok(ShardLock { path });
				}
				Err(_) => match read_holder(&path) {
					Some(holder) if !same_host(&holder.host, &host_name()) || process_alive(holder.pid) => return Err(holder),
					// stale or unreadable: remove it and try once more
					_ => {
						let _ = std::fs::remove_file(&path);
					}
				},
			}
		}
		match read_holder(&path) {
			Some(holder) => Err(holder),
			None => Err(Holder { pid: 0, tool: "an unknown dashboard".to_string(), host: host_name() }),
		}
	}
}

impl Drop for ShardLock {
	fn drop(&mut self) {
		// only our own lock goes
		if read_holder(&self.path).map(|h| h.pid == std::process::id()).unwrap_or(false) {
			let _ = std::fs::remove_file(&self.path);
		}
	}
}
