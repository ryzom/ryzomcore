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

//! servdash_cli: the service dashboard of the quick start package as a terminal tool, for
//! Windows and Linux, with the daemon and client split of tmux so a shard keeps running
//! when the terminal goes away.

mod ansi;
mod client;
mod config;
mod daemon;
mod ipc;
mod launch_ctrl;
mod log;
mod proto;
mod registry;
mod session;
mod spawn;
mod ui;

use anyhow::{bail, Context, Result};
use session::Session;
use std::path::{Path, PathBuf};
use std::time::{Duration, Instant};

const USAGE: &str = "servdash_cli, service dashboard for Ryzom Core

Usage:
  servdash_cli [options] <dashboard.ini | running dashboard>
  servdash_cli --list

Starts the dashboard daemon for the ini file when none runs, then attaches
to it. Detaching (F10) or closing the terminal leaves the services running.
A running dashboard can be given instead of the ini file, from any directory:
its number in --list, its title, its id or a prefix of it, or the ini path.

Options:
  -l, --list       list the running dashboards of this user
  -d, --detach     start the daemon without attaching
  -a, --attach     attach only, fail when no daemon runs
      --shutdown   stop all services and exit the daemon
      --status     print the state of every process and exit
      --endpoint   print the local endpoint of the daemon and exit
  -h, --help       this text
  -V, --version    the version

Keys:
  F5 start/stop  F6 kill  F7 start all  F8 stop all  F10 detach  F12 twice: stop all and quit  Tab/Shift+Tab or Alt+Up/Down select
  Alt+1..9 press a console button  PgUp/PgDn scroll  Enter send the input line
";

struct Options {
	ini: Option<PathBuf>,
	daemon: bool,
	detach: bool,
	attach: bool,
	shutdown: bool,
	status: bool,
	endpoint: bool,
	list: bool,
}

fn parse_args() -> Result<Options> {
	let mut options = Options { ini: None, daemon: false, detach: false, attach: false, shutdown: false, status: false, endpoint: false, list: false };
	for arg in std::env::args().skip(1) {
		match arg.as_str() {
			"-h" | "--help" => {
				print!("{}", USAGE);
				std::process::exit(0);
			}
			"-V" | "--version" => {
				println!("servdash_cli {}", env!("CARGO_PKG_VERSION"));
				std::process::exit(0);
			}
			"--daemon" => options.daemon = true,
			"-d" | "--detach" => options.detach = true,
			"-a" | "--attach" => options.attach = true,
			"--shutdown" => options.shutdown = true,
			"--status" => options.status = true,
			"--endpoint" => options.endpoint = true,
			"-l" | "--list" => options.list = true,
			other if other.starts_with('-') => bail!("unknown option '{}'\n\n{}", other, USAGE),
			other => {
				if options.ini.is_some() {
					bail!("only one dashboard can be given\n\n{}", USAGE);
				}
				options.ini = Some(PathBuf::from(other));
			}
		}
	}
	if options.ini.is_none() && !options.list {
		bail!("{}", USAGE);
	}
	Ok(options)
}

/// The dashboard the command line names: an ini file, or a running dashboard by number,
/// title, id or path. Starting a daemon needs the file itself.
async fn resolve_session(target: &Path, options: &Options) -> Result<Session> {
	if target.is_file() {
		return Session::new(target);
	}
	let selector = target.to_string_lossy();
	match registry::resolve(&selector).await {
		Ok(record) => Session::from_record(&record.id, &record.ini_path),
		Err(e) => {
			if options.daemon || options.endpoint {
				bail!("cannot find '{}'", selector);
			}
			bail!("'{}' is not an ini file and {}", selector, e)
		}
	}
}

async fn run_daemon(session: Session) -> Result<()> {
	log::set_file(session.log_path());
	if let Err(e) = daemon::run(session).await {
		log::note(&format!("daemon failed: {:#}", e));
		return Err(e);
	}
	Ok(())
}

async fn shutdown(session: &Session) -> Result<()> {
	let endpoint = session.control_endpoint();
	if !ipc::is_listening(&endpoint).await {
		println!("No dashboard daemon runs for '{}'.", session.ini_path.display());
		return Ok(());
	}
	let stream = ipc::connect(&endpoint).await.context("cannot connect to the dashboard daemon")?;
	let (reader, mut writer) = tokio::io::split(stream);
	proto::write_msg(&mut writer, &proto::ClientMsg::Hello { version: proto::VERSION }).await?;
	proto::write_msg(&mut writer, &proto::ClientMsg::Shutdown).await?;
	println!("Stopping all services...");
	use tokio::io::AsyncBufReadExt;
	let mut lines = tokio::io::BufReader::new(reader).lines();
	let wait = async {
		while let Ok(Some(line)) = lines.next_line().await {
			if let Ok(proto::DaemonMsg::Bye) = proto::parse_msg::<proto::DaemonMsg>(&line) {
				break;
			}
		}
	};
	match tokio::time::timeout(Duration::from_secs(120), wait).await {
		Ok(()) => println!("The dashboard daemon exited."),
		Err(_) => println!("The services are still stopping; the daemon exits when they are gone."),
	}
	Ok(())
}

async fn status(session: &Session) -> Result<()> {
	let endpoint = session.control_endpoint();
	if !ipc::is_listening(&endpoint).await {
		println!("No dashboard daemon runs for '{}'.", session.ini_path.display());
		return Ok(());
	}
	let stream = ipc::connect(&endpoint).await.context("cannot connect to the dashboard daemon")?;
	let (reader, mut writer) = tokio::io::split(stream);
	proto::write_msg(&mut writer, &proto::ClientMsg::Hello { version: proto::VERSION }).await?;
	use tokio::io::AsyncBufReadExt;
	let mut lines = tokio::io::BufReader::new(reader).lines();
	let wait = async {
		while let Ok(Some(line)) = lines.next_line().await {
			match proto::parse_msg::<proto::DaemonMsg>(&line) {
				Ok(proto::DaemonMsg::Hello { title, .. }) => println!("{}", title),
				Ok(proto::DaemonMsg::Processes { processes }) => {
					for p in processes {
						println!("{:<24} {:<9} {}", p.name, format!("{:?}", p.state), p.title);
					}
					break;
				}
				_ => {}
			}
		}
	};
	if tokio::time::timeout(Duration::from_secs(10), wait).await.is_err() {
		bail!("the dashboard daemon did not answer");
	}
	Ok(())
}

async fn start_and_attach(session: &Session, options: &Options) -> Result<()> {
	let endpoint = session.control_endpoint();
	let running = ipc::is_listening(&endpoint).await;
	if !running {
		if options.attach {
			bail!("no dashboard daemon runs for '{}'", session.ini_path.display());
		}
		// the daemon reads the ini itself; a broken file is better reported here than in its log
		config::read(&session.ini_path)?;
		spawn::spawn_daemon(&session.ini_path).context("cannot start the dashboard daemon")?;
		let deadline = Instant::now() + Duration::from_secs(10);
		while !ipc::is_listening(&endpoint).await {
			if Instant::now() > deadline {
				bail!("the dashboard daemon did not start; see '{}'", session.log_path().display());
			}
			tokio::time::sleep(Duration::from_millis(100)).await;
		}
		if options.detach {
			println!("Started the dashboard for '{}'.", session.ini_path.display());
		}
	} else if options.detach {
		println!("The dashboard for '{}' is already running.", session.ini_path.display());
	}
	if options.detach {
		return Ok(());
	}
	match client::attach(session).await? {
		client::Outcome::Detached => println!("Detached. The services keep running; run the same command to attach again."),
		client::Outcome::DaemonGone => println!("The dashboard daemon exited."),
	}
	Ok(())
}

fn main() -> Result<()> {
	let options = parse_args()?;
	let runtime = tokio::runtime::Runtime::new()?;
	runtime.block_on(async {
		if options.list {
			registry::print_list(&registry::running().await);
			return Ok(());
		}
		let target = options.ini.clone().expect("checked by parse_args");
		let session = resolve_session(&target, &options).await?;
		if options.endpoint {
			println!("{}", session.control_endpoint());
			Ok(())
		} else if options.daemon {
			run_daemon(session).await
		} else if options.shutdown {
			shutdown(&session).await
		} else if options.status {
			status(&session).await
		} else {
			start_and_attach(&session, &options).await
		}
	})
}
