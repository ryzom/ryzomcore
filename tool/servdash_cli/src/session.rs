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

//! A session is one ini file. Its identity names the local endpoints the daemon listens on,
//! so a second `servdash_cli start_mainland.ini` finds the running daemon for that file.

use anyhow::{Context, Result};
use std::path::{Path, PathBuf};

#[derive(Clone, Debug)]
pub struct Session {
	/// Absolute path of the ini file.
	pub ini_path: PathBuf,
	/// Directory of the ini file; the daemon runs with it as working directory, like the Win32 tool.
	pub dir: PathBuf,
	/// Stable hash of the ini path.
	pub id: String,
}

fn fnv1a64(text: &str) -> u64 {
	let mut hash: u64 = 0xcbf29ce484222325;
	for byte in text.bytes() {
		hash ^= byte as u64;
		hash = hash.wrapping_mul(0x100000001b3);
	}
	hash
}

impl Session {
	pub fn new(ini: &Path) -> Result<Session> {
		let ini_path = std::fs::canonicalize(ini).with_context(|| format!("cannot find '{}'", ini.display()))?;
		// canonicalize gives a verbatim path on Windows; the plain form reads better and works the same
		let ini_path = match ini_path.to_str() {
			Some(text) if text.starts_with(r"\\?\") && !text.starts_with(r"\\?\UNC\") => PathBuf::from(&text[4..]),
			_ => ini_path,
		};
		let dir = ini_path.parent().map(Path::to_path_buf).context("the ini file has no parent directory")?;
		let mut key = ini_path.to_string_lossy().to_string();
		if cfg!(windows) {
			key = key.to_lowercase();
		}
		Ok(Session { ini_path, dir, id: format!("{:016x}", fnv1a64(&key)) })
	}

	/// A session from a registry record of a running daemon: the file does not need to exist.
	pub fn from_record(id: &str, ini_path: &str) -> Result<Session> {
		let ini_path = PathBuf::from(ini_path);
		let dir = ini_path.parent().map(Path::to_path_buf).context("the ini file has no parent directory")?;
		Ok(Session { ini_path, dir, id: id.to_string() })
	}

	#[cfg(windows)]
	fn runtime_dir(&self) -> PathBuf {
		PathBuf::new()
	}

	#[cfg(not(windows))]
	fn runtime_dir(&self) -> PathBuf {
		let base = std::env::var_os("XDG_RUNTIME_DIR").map(PathBuf::from).unwrap_or_else(|| PathBuf::from("/tmp"));
		base.join(format!("servdash_cli.{}", self.id))
	}

	/// Creates the directory that holds the Unix sockets. Nothing to do on Windows.
	pub fn prepare_runtime_dir(&self) -> Result<()> {
		#[cfg(not(windows))]
		{
			let dir = self.runtime_dir();
			std::fs::create_dir_all(&dir).with_context(|| format!("cannot create '{}'", dir.display()))?;
			use std::os::unix::fs::PermissionsExt;
			std::fs::set_permissions(&dir, std::fs::Permissions::from_mode(0o700)).ok();
		}
		Ok(())
	}

	/// Removes the Unix socket directory when the daemon exits. Nothing to do on Windows.
	pub fn cleanup_runtime_dir(&self) {
		#[cfg(not(windows))]
		{
			let _ = std::fs::remove_dir_all(self.runtime_dir());
		}
	}

	/// Where the dashboard clients connect.
	pub fn control_endpoint(&self) -> String {
		if cfg!(windows) {
			format!(r"\\.\pipe\servdash_cli.{}", self.id)
		} else {
			self.runtime_dir().join("dash.sock").to_string_lossy().to_string()
		}
	}

	/// Where the NeL service of a control section connects with `--ctrl`. The section name is
	/// reduced to characters that are safe in a pipe name, a socket path and a service command line.
	pub fn service_endpoint(&self, name: &str) -> String {
		let safe: String = name.chars().map(|c| if c.is_ascii_alphanumeric() || c == '_' || c == '-' || c == '.' { c } else { '_' }).collect();
		if cfg!(windows) {
			format!(r"\\.\pipe\servdash_cli.{}.{}", self.id, safe)
		} else {
			self.runtime_dir().join(format!("{}.sock", safe)).to_string_lossy().to_string()
		}
	}

	/// The daemon's own diagnostics, next to the ini file.
	pub fn log_path(&self) -> PathBuf {
		let stem = self.ini_path.file_stem().map(|s| s.to_string_lossy().to_string()).unwrap_or_else(|| "servdash".to_string());
		self.dir.join(format!("{}.servdash_cli.log", stem))
	}
}
