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

//! Starting child processes with their standard streams piped, and starting the daemon
//! itself detached from the terminal.

use std::path::Path;
use std::process::Stdio;
use tokio::process::{Child, Command};

/// Splits a command line the way a POSIX shell would for the simple cases: whitespace
/// separated words, single and double quotes, backslash escapes outside single quotes.
#[cfg(not(windows))]
pub fn split_args(text: &str) -> Vec<String> {
	let mut words = Vec::new();
	let mut current = String::new();
	let mut in_word = false;
	let mut quote: Option<char> = None;
	let mut chars = text.chars().peekable();
	while let Some(c) = chars.next() {
		match quote {
			Some('\'') => {
				if c == '\'' {
					quote = None;
				} else {
					current.push(c);
				}
			}
			Some(_) => {
				if c == '"' {
					quote = None;
				} else if c == '\\' {
					if let Some(&next) = chars.peek() {
						if next == '"' || next == '\\' || next == '$' || next == '`' {
							current.push(next);
							chars.next();
							continue;
						}
					}
					current.push(c);
				} else {
					current.push(c);
				}
			}
			None => {
				if c.is_whitespace() {
					if in_word {
						words.push(std::mem::take(&mut current));
						in_word = false;
					}
				} else if c == '\'' || c == '"' {
					quote = Some(c);
					in_word = true;
				} else if c == '\\' {
					if let Some(next) = chars.next() {
						current.push(next);
					}
					in_word = true;
				} else {
					current.push(c);
					in_word = true;
				}
			}
		}
	}
	if in_word {
		words.push(current);
	}
	words
}

#[cfg(windows)]
mod job {
	use std::sync::OnceLock;
	use windows_sys::Win32::Foundation::{CloseHandle, HANDLE};
	use windows_sys::Win32::System::JobObjects::{
		AssignProcessToJobObject, CreateJobObjectW, JobObjectExtendedLimitInformation, SetInformationJobObject, JOBOBJECT_EXTENDED_LIMIT_INFORMATION,
		JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE,
	};

	struct Job(HANDLE);
	unsafe impl Send for Job {}
	unsafe impl Sync for Job {}

	static JOB: OnceLock<Option<Job>> = OnceLock::new();

	/// A job object that takes the children down with the daemon, so a crashed or killed
	/// daemon leaves no orphan services behind.
	fn job() -> Option<HANDLE> {
		JOB.get_or_init(|| unsafe {
			let handle = CreateJobObjectW(std::ptr::null(), std::ptr::null());
			if handle.is_null() {
				return None;
			}
			let mut info: JOBOBJECT_EXTENDED_LIMIT_INFORMATION = std::mem::zeroed();
			info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			let ok = SetInformationJobObject(
				handle,
				JobObjectExtendedLimitInformation,
				&info as *const _ as *const std::ffi::c_void,
				std::mem::size_of::<JOBOBJECT_EXTENDED_LIMIT_INFORMATION>() as u32,
			);
			if ok == 0 {
				CloseHandle(handle);
				return None;
			}
			Some(Job(handle))
		})
		.as_ref()
		.map(|j| j.0)
	}

	pub fn assign(child: &tokio::process::Child) {
		let Some(job) = job() else {
			return;
		};
		let Some(handle) = child.raw_handle() else {
			return;
		};
		unsafe {
			if AssignProcessToJobObject(job, handle as HANDLE) == 0 {
				crate::log::note("cannot assign a child process to the job object");
			}
		}
	}
}

/// Starts a process with stdin, stdout and stderr piped. On Windows the argument string is
/// passed to the process as it stands, like the Win32 tool does; elsewhere it is split into
/// words. `extra_args` are appended as separate arguments.
pub fn spawn(program: &Path, args: &str, extra_args: &[String], cwd: Option<&Path>) -> std::io::Result<Child> {
	let mut command = Command::new(program);
	#[cfg(windows)]
	{
		const CREATE_NO_WINDOW: u32 = 0x08000000;
		if !args.trim().is_empty() {
			command.raw_arg(args.trim());
		}
		for arg in extra_args {
			command.arg(arg);
		}
		command.creation_flags(CREATE_NO_WINDOW);
	}
	#[cfg(not(windows))]
	{
		command.args(split_args(args));
		for arg in extra_args {
			command.arg(arg);
		}
		command.process_group(0);
	}
	#[cfg(target_os = "linux")]
	{
		// the counterpart of the Windows job object: a daemon that dies takes its services with
		// it, so the next daemon does not declare them stopped while they still run
		unsafe {
			command.pre_exec(|| {
				if libc::prctl(libc::PR_SET_PDEATHSIG, libc::SIGTERM) != 0 {
					return Err(std::io::Error::last_os_error());
				}
				Ok(())
			});
		}
	}
	if let Some(dir) = cwd {
		command.current_dir(dir);
	}
	command.stdin(Stdio::piped()).stdout(Stdio::piped()).stderr(Stdio::piped());
	command.kill_on_drop(false);
	let child = command.spawn()?;
	#[cfg(windows)]
	job::assign(&child);
	Ok(child)
}

/// Starts this executable as the daemon of an ini file, detached from the current terminal,
/// so it survives the client and the terminal closing.
pub fn spawn_daemon(ini: &Path) -> std::io::Result<()> {
	let exe = std::env::current_exe()?;
	let mut command = std::process::Command::new(exe);
	command.arg("--daemon").arg(ini);
	command.stdin(Stdio::null()).stdout(Stdio::null()).stderr(Stdio::null());
	#[cfg(windows)]
	{
		use std::os::windows::process::CommandExt;
		const DETACHED_PROCESS: u32 = 0x00000008;
		const CREATE_NEW_PROCESS_GROUP: u32 = 0x00000200;
		const CREATE_BREAKAWAY_FROM_JOB: u32 = 0x01000000;
		// detached from the console and from any job the terminal put us in, so closing the
		// terminal does not take the daemon down; a job that forbids breakaway refuses the flag
		command.creation_flags(DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP | CREATE_BREAKAWAY_FROM_JOB);
		match command.spawn() {
			Ok(_) => return Ok(()),
			Err(_) => {
				command.creation_flags(DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP);
			}
		}
	}
	#[cfg(not(windows))]
	{
		use std::os::unix::process::CommandExt;
		// a session of its own, so a hangup of the terminal never reaches the daemon
		unsafe {
			command.pre_exec(|| {
				if libc::setsid() < 0 {
					return Err(std::io::Error::last_os_error());
				}
				Ok(())
			});
		}
	}
	command.spawn()?;
	Ok(())
}
