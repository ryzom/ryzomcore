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

//! The daemon's own notes go to a file next to the ini, since it has no terminal.

use std::io::Write;
use std::path::PathBuf;
use std::sync::Mutex;
use std::time::{SystemTime, UNIX_EPOCH};

static TARGET: Mutex<Option<PathBuf>> = Mutex::new(None);

pub fn set_file(path: PathBuf) {
	*TARGET.lock().unwrap() = Some(path);
}

pub fn note(text: &str) {
	let target = TARGET.lock().unwrap().clone();
	let secs = SystemTime::now().duration_since(UNIX_EPOCH).map(|d| d.as_secs()).unwrap_or(0);
	let line = format!("{} {}\n", secs, text);
	match target {
		Some(path) => {
			if let Ok(mut f) = std::fs::OpenOptions::new().create(true).append(true).open(path) {
				let _ = f.write_all(line.as_bytes());
			}
		}
		None => {
			let _ = std::io::stderr().write_all(line.as_bytes());
		}
	}
}
