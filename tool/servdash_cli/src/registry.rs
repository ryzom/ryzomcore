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

//! The running dashboards of this user. Each daemon writes a record while it runs, so a
//! client can list them and attach by number, title or id without knowing the ini path or
//! the working directory. A record whose endpoint no longer answers is a leftover of a
//! crashed daemon and is dropped when listing.
//!
//! The directory is shared with servdash_gui, which writes the same records and serves the
//! same protocol on its endpoint, so one list and one MCP server cover both tools.

use crate::ipc;
use crate::session::Session;
use anyhow::{bail, Result};
use serde::{Deserialize, Serialize};
use std::path::PathBuf;
use std::time::{SystemTime, UNIX_EPOCH};

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct Record {
	pub id: String,
	pub ini_path: String,
	pub title: String,
	pub endpoint: String,
	pub pid: u32,
	pub started: u64,
	/// The dashboard tool that wrote the record: servdash_cli or servdash_gui.
	#[serde(default)]
	pub tool: String,
}

#[cfg(windows)]
fn dir() -> PathBuf {
	let base = std::env::var_os("LOCALAPPDATA").map(PathBuf::from).unwrap_or_else(std::env::temp_dir);
	base.join("servdash").join("sessions")
}

#[cfg(not(windows))]
fn dir() -> PathBuf {
	match std::env::var_os("XDG_RUNTIME_DIR") {
		Some(base) => PathBuf::from(base).join("servdash.sessions"),
		None => PathBuf::from(format!("/tmp/servdash.sessions.{}", unsafe { libc::getuid() })),
	}
}

fn record_path(id: &str) -> PathBuf {
	dir().join(format!("{}.json", id))
}

pub fn register(session: &Session, title: &str) {
	let record = Record {
		id: session.id.clone(),
		ini_path: session.ini_path.to_string_lossy().to_string(),
		title: title.to_string(),
		endpoint: session.control_endpoint(),
		pid: std::process::id(),
		started: SystemTime::now().duration_since(UNIX_EPOCH).map(|d| d.as_secs()).unwrap_or(0),
		tool: crate::shard_lock::TOOL_NAME.to_string(),
	};
	let dir = dir();
	if let Err(e) = std::fs::create_dir_all(&dir) {
		crate::log::note(&format!("cannot create '{}': {}", dir.display(), e));
		return;
	}
	#[cfg(not(windows))]
	{
		use std::os::unix::fs::PermissionsExt;
		let _ = std::fs::set_permissions(&dir, std::fs::Permissions::from_mode(0o700));
	}
	match serde_json::to_vec_pretty(&record) {
		Ok(bytes) => {
			if let Err(e) = std::fs::write(record_path(&session.id), bytes) {
				crate::log::note(&format!("cannot write the session record: {}", e));
			}
		}
		Err(e) => crate::log::note(&format!("cannot encode the session record: {}", e)),
	}
}

pub fn unregister(session: &Session) {
	let _ = std::fs::remove_file(record_path(&session.id));
}

/// Every record as it stands, without asking the endpoints; stale ones included. For a poll
/// that runs often, since each liveness check costs the daemon a client connection.
pub fn records() -> Vec<Record> {
	let mut records = Vec::new();
	let Ok(entries) = std::fs::read_dir(dir()) else {
		return records;
	};
	for entry in entries.flatten() {
		let path = entry.path();
		if path.extension().and_then(|e| e.to_str()) != Some("json") {
			continue;
		}
		if let Some(record) = std::fs::read(&path).ok().and_then(|bytes| serde_json::from_slice::<Record>(&bytes).ok()) {
			records.push(record);
		}
	}
	records.sort_by_key(|r| (r.started, r.id.clone()));
	records
}

/// The dashboards that answer right now, oldest first. Stale records are removed.
pub async fn running() -> Vec<Record> {
	let mut records = Vec::new();
	let Ok(entries) = std::fs::read_dir(dir()) else {
		return records;
	};
	for entry in entries.flatten() {
		let path = entry.path();
		if path.extension().and_then(|e| e.to_str()) != Some("json") {
			continue;
		}
		let record: Option<Record> = std::fs::read(&path).ok().and_then(|bytes| serde_json::from_slice(&bytes).ok());
		match record {
			Some(record) if ipc::is_listening(&record.endpoint).await => records.push(record),
			_ => {
				let _ = std::fs::remove_file(&path);
			}
		}
	}
	records.sort_by_key(|r| (r.started, r.id.clone()));
	records
}

/// Finds a running dashboard by list number, id or id prefix, title, or ini path.
pub async fn resolve(selector: &str) -> Result<Record> {
	let records = running().await;
	if records.is_empty() {
		bail!("no dashboard is running");
	}
	if let Ok(number) = selector.trim_start_matches('#').parse::<usize>() {
		if number >= 1 && number <= records.len() {
			return Ok(records[number - 1].clone());
		}
	}
	let lowered = selector.to_lowercase();
	let matches: Vec<&Record> = records
		.iter()
		.filter(|r| {
			r.id.starts_with(&lowered)
				|| r.title.to_lowercase() == lowered
				|| r.ini_path.to_lowercase() == lowered
				|| r.ini_path.to_lowercase().ends_with(&lowered)
		})
		.collect();
	match matches.len() {
		1 => Ok(matches[0].clone()),
		0 => bail!("no running dashboard matches '{}'; see --list", selector),
		_ => bail!("'{}' matches several running dashboards; see --list and use the number or the id", selector),
	}
}

pub fn print_list(records: &[Record]) {
	if records.is_empty() {
		println!("No dashboard is running.");
		return;
	}
	println!("  #  id                title                         tool          ini");
	for (i, r) in records.iter().enumerate() {
		let tool = if r.tool.is_empty() { "servdash_cli" } else { r.tool.as_str() };
		println!("{:>3}  {:<16}  {:<28}  {:<12}  {}", i + 1, r.id, r.title, tool, r.ini_path);
	}
}
