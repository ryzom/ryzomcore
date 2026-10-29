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

//! An attached client: the terminal in front of the daemon. It renders what the daemon
//! streams and forwards what the user does; nothing is lost when it detaches.

use crate::ipc;
use crate::proto::{self, ClientMsg, DaemonMsg};
use crate::session::Session;
use crate::ui::{Action, App};
use anyhow::{Context, Result};
use crossterm::event::{DisableMouseCapture, EnableMouseCapture};
use std::time::Duration;
use tokio::io::{AsyncBufReadExt, AsyncWriteExt, BufReader};
use tokio::sync::mpsc;

pub enum Outcome {
	Detached,
	DaemonGone,
}

pub async fn attach(session: &Session) -> Result<Outcome> {
	let stream = ipc::connect(&session.control_endpoint()).await.context("cannot connect to the dashboard daemon")?;
	let (reader, mut writer) = tokio::io::split(stream);
	proto::write_msg(&mut writer, &ClientMsg::Hello { version: proto::VERSION }).await?;

	let (daemon_tx, mut daemon_rx) = mpsc::unbounded_channel::<DaemonMsg>();
	tokio::spawn(async move {
		let mut lines = BufReader::new(reader).lines();
		while let Ok(Some(line)) = lines.next_line().await {
			match proto::parse_msg::<DaemonMsg>(&line) {
				Ok(msg) => {
					if daemon_tx.send(msg).is_err() {
						return;
					}
				}
				Err(_) => continue,
			}
		}
		let _ = daemon_tx.send(DaemonMsg::Bye);
	});

	// crossterm reads are blocking; a thread forwards them
	let (input_tx, mut input_rx) = mpsc::unbounded_channel::<crossterm::event::Event>();
	std::thread::spawn(move || {
		while let Ok(event) = crossterm::event::read() {
			if input_tx.send(event).is_err() {
				break;
			}
		}
	});

	let mut terminal = ratatui::init();
	let _ = crossterm::execute!(std::io::stdout(), EnableMouseCapture);
	let mut app = App::new();
	let mut frame_tick = tokio::time::interval(Duration::from_millis(40));

	let outcome: Result<Outcome> = async {
		loop {
			tokio::select! {
				msg = daemon_rx.recv() => match msg {
					None | Some(DaemonMsg::Bye) => return Ok(Outcome::DaemonGone),
					Some(msg) => app.apply(msg),
				},
				event = input_rx.recv() => match event {
					None => return Ok(Outcome::Detached),
					Some(event) => {
						for action in app.handle(event) {
							match action {
								Action::Send(msg) => proto::write_msg(&mut writer, &msg).await.context("the connection to the daemon is gone")?,
								Action::Detach => return Ok(Outcome::Detached),
							}
						}
					}
				},
				_ = frame_tick.tick() => {
					if app.dirty {
						terminal.draw(|frame| app.render(frame))?;
					}
				}
			}
		}
	}
	.await;

	let _ = crossterm::execute!(std::io::stdout(), DisableMouseCapture);
	ratatui::restore();
	let _ = writer.shutdown().await;
	outcome
}
