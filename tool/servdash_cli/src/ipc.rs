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

//! Local byte streams: named pipes on Windows, Unix domain sockets elsewhere. The daemon
//! listens on them for its clients and for the NeL services of the control sections.

use std::io;
use tokio::io::{AsyncRead, AsyncWrite};

pub trait AsyncRw: AsyncRead + AsyncWrite + Send + Unpin {}
impl<T: AsyncRead + AsyncWrite + Send + Unpin> AsyncRw for T {}

pub type Stream = Box<dyn AsyncRw>;

#[cfg(windows)]
mod imp {
	use super::Stream;
	use std::io;
	use std::time::Duration;
	use tokio::net::windows::named_pipe::{ClientOptions, NamedPipeServer, ServerOptions};

	const ERROR_FILE_NOT_FOUND: i32 = 2;
	const ERROR_PIPE_BUSY: i32 = 231;

	pub struct Listener {
		name: String,
		next: Option<NamedPipeServer>,
	}

	impl Listener {
		/// Fails with AddrInUse when another process already serves this name.
		pub fn bind(name: &str) -> io::Result<Listener> {
			let server = ServerOptions::new().first_pipe_instance(true).create(name).map_err(|e| {
				if e.raw_os_error() == Some(5) || e.raw_os_error() == Some(231) {
					io::Error::new(io::ErrorKind::AddrInUse, e)
				} else {
					e
				}
			})?;
			Ok(Listener { name: name.to_string(), next: Some(server) })
		}

		pub async fn accept(&mut self) -> io::Result<Stream> {
			let server = match self.next.take() {
				Some(server) => server,
				None => ServerOptions::new().create(&self.name)?,
			};
			server.connect().await?;
			// the next instance is created before this one is handed out, so a client arriving
			// in between finds a listener instead of a missing pipe
			self.next = ServerOptions::new().create(&self.name).ok();
			Ok(Box::new(server))
		}
	}

	pub async fn connect(name: &str) -> io::Result<Stream> {
		for attempt in 0..50 {
			match ClientOptions::new().open(name) {
				Ok(client) => return Ok(Box::new(client)),
				Err(e) if e.raw_os_error() == Some(ERROR_PIPE_BUSY) => tokio::time::sleep(Duration::from_millis(20)).await,
				// a QLocalServer (servdash_gui) has no listening instance between accepting a client
				// and creating the next one in its event loop; a short retry bridges that gap
				Err(e) if e.raw_os_error() == Some(ERROR_FILE_NOT_FOUND) && attempt < 5 => tokio::time::sleep(Duration::from_millis(20)).await,
				Err(e) => return Err(e),
			}
		}
		Err(io::Error::new(io::ErrorKind::TimedOut, "the pipe stayed busy"))
	}
}

#[cfg(not(windows))]
mod imp {
	use super::Stream;
	use std::io;
	use std::path::Path;
	use tokio::net::{UnixListener, UnixStream};

	pub struct Listener {
		listener: UnixListener,
	}

	impl Listener {
		/// Fails with AddrInUse when another process already serves this path. A socket file
		/// nobody listens on is a leftover of a crashed daemon and is replaced.
		pub fn bind(path: &str) -> io::Result<Listener> {
			let p = Path::new(path);
			if p.exists() {
				match std::os::unix::net::UnixStream::connect(p) {
					Ok(_) => return Err(io::Error::new(io::ErrorKind::AddrInUse, "a daemon is already listening")),
					Err(_) => {
						std::fs::remove_file(p)?;
					}
				}
			}
			let listener = UnixListener::bind(p)?;
			Ok(Listener { listener })
		}

		pub async fn accept(&mut self) -> io::Result<Stream> {
			let (stream, _) = self.listener.accept().await?;
			Ok(Box::new(stream))
		}
	}

	pub async fn connect(path: &str) -> io::Result<Stream> {
		Ok(Box::new(UnixStream::connect(path).await?))
	}
}

pub use imp::{connect, Listener};

/// Whether something listens at the endpoint right now.
pub async fn is_listening(endpoint: &str) -> bool {
	connect(endpoint).await.is_ok()
}

pub fn is_in_use(err: &io::Error) -> bool {
	err.kind() == io::ErrorKind::AddrInUse
}
