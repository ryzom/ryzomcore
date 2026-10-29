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

//! The daemon owns the processes, their output, the launch control files and the console
//! channels, and serves any number of attached clients. It is the part that stays alive when
//! the terminal goes away. The launch, stop and start order rules are those of the Win32
//! servdash (MainWindow.autoStart, ProcessHost, TerminalHost and ProcessControl), so a shard
//! behaves the same under both tools.
//!
//! Everything runs on one task that handles events; the reader tasks for pipes, console
//! channels and clients only forward what they read.

use crate::ansi;
use crate::config::{DashCfg, SectionCfg, PRIORITY_NONE};
use crate::ipc;
use crate::launch_ctrl::{self, LaunchCtrl};
use crate::log;
use crate::proto::{self, ClientMsg, Color, CommandInfo, CommandKind, DaemonMsg, ProcState, ProcessInfo, StyledLine};
use crate::session::Session;
use crate::spawn;
use anyhow::{Context, Result};
use regex::Regex;
use std::collections::{BTreeMap, HashMap, VecDeque};
use std::time::{Duration, Instant};
use tokio::io::{AsyncBufReadExt, AsyncRead, AsyncWriteExt, BufReader};
use tokio::process::Child;
use tokio::sync::mpsc::{self, UnboundedSender};

/// Lines kept per process and per stream.
const MAX_LINES: usize = 5000;
const TICK: Duration = Duration::from_millis(250);
/// A control service that has not connected by then counts as captured, so the start order goes on.
const CAPTURE_TIMEOUT: Duration = Duration::from_secs(30);

enum Event {
	ClientConnected(u64, UnboundedSender<DaemonMsg>),
	ClientMsg(u64, ClientMsg),
	ClientGone(u64),
	/// Process index, launch generation, line.
	Output(usize, u64, StyledLine),
	OutputClosed(usize, u64),
	/// Process index, connection id, sender towards the service.
	ControlConnected(usize, u64, UnboundedSender<String>),
	ControlLine(usize, u64, String),
	ControlGone(usize, u64),
	Tick,
}

struct Proc {
	cfg: SectionCfg,
	state: ProcState,
	/// The process title reported over the console channel, or the label.
	title: String,
	/// Whether the start button means stop. Set on launch, cleared on exit.
	started: bool,
	shutdown_requested: bool,
	restart_once: bool,
	ready_triggered: bool,
	ready_re: Option<Regex>,
	/// Incremented per launch; events from an earlier child are ignored.
	gen: u64,
	child: Option<Child>,
	stdin: Option<UnboundedSender<String>>,
	/// Console lines, raw output and notes, in arrival order.
	lines: VecDeque<StyledLine>,
	labels: Vec<String>,
	/// The command table of the service, by name, as it reports it over the console channel.
	commands: BTreeMap<String, CommandInfo>,
	control: Option<(u64, UnboundedSender<String>)>,
	control_endpoint: Option<String>,
	launch_ctrl: Option<LaunchCtrl>,
	launched_at: Option<Instant>,
}

impl Proc {
	fn running(&self) -> bool {
		self.child.is_some()
	}

	fn info(&self) -> ProcessInfo {
		let label = match &self.cfg.title {
			Some(title) => title.clone(),
			None => self.title.clone(),
		};
		ProcessInfo {
			name: self.cfg.name.clone(),
			label,
			title: self.title.clone(),
			state: self.state,
			started: self.started,
			control: self.cfg.control,
			control_connected: self.control.is_some(),
			labels: self.labels.clone(),
		}
	}
}

struct Daemon {
	title: String,
	procs: Vec<Proc>,
	/// Processes waiting for their turn, by index; `LaunchProcesses` in the Win32 tool.
	queue: VecDeque<usize>,
	clients: HashMap<u64, UnboundedSender<DaemonMsg>>,
	tx: UnboundedSender<Event>,
	shutting_down: bool,
	done: bool,
}

fn push_capped(buffer: &mut VecDeque<StyledLine>, line: StyledLine) {
	if buffer.len() >= MAX_LINES {
		buffer.pop_front();
	}
	buffer.push_back(line);
}

/// Field escaping of the console channel, see CSocketDisplayer.
fn escape_field(value: &str) -> String {
	let mut out = String::with_capacity(value.len());
	for c in value.chars() {
		match c {
			'\\' => out.push_str("\\\\"),
			'\t' => out.push_str("\\t"),
			'\n' => out.push_str("\\n"),
			'\r' => out.push_str("\\r"),
			_ => out.push(c),
		}
	}
	out
}

fn unescape_field(value: &str) -> String {
	let mut out = String::with_capacity(value.len());
	let mut chars = value.chars();
	while let Some(c) = chars.next() {
		if c == '\\' {
			match chars.next() {
				Some('t') => out.push('\t'),
				Some('n') => out.push('\n'),
				Some('r') => out.push('\r'),
				Some(other) => out.push(other),
				None => out.push('\\'),
			}
		} else {
			out.push(c);
		}
	}
	out
}

/// The colours CWindowDisplayer::doDisplay assigns per log type, as terminal colours.
fn console_color(hex: &str) -> Color {
	match u32::from_str_radix(hex, 16).unwrap_or(0) {
		0x000000 => Color::Default,
		0xFF0000 => Color::LightRed,
		0x800000 => Color::Yellow,
		0x808080 => Color::DarkGray,
		rgb => Color::Rgb(((rgb >> 16) & 0xFF) as u8, ((rgb >> 8) & 0xFF) as u8, (rgb & 0xFF) as u8),
	}
}

fn spawn_output_reader<R: AsyncRead + Unpin + Send + 'static>(tx: UnboundedSender<Event>, idx: usize, gen: u64, reader: R, reports_close: bool) {
	tokio::spawn(async move {
		let mut reader = BufReader::new(reader);
		let mut buf = Vec::new();
		loop {
			buf.clear();
			match reader.read_until(b'\n', &mut buf).await {
				Ok(0) | Err(_) => break,
				Ok(_) => {
					let text = String::from_utf8_lossy(&buf);
					let text = text.strip_suffix('\n').unwrap_or(&text);
					if tx.send(Event::Output(idx, gen, ansi::parse_line(text))).is_err() {
						return;
					}
				}
			}
		}
		if reports_close {
			let _ = tx.send(Event::OutputClosed(idx, gen));
		}
	});
}

fn spawn_stdin_writer(stdin: tokio::process::ChildStdin) -> UnboundedSender<String> {
	let (tx, mut rx) = mpsc::unbounded_channel::<String>();
	tokio::spawn(async move {
		let mut stdin = stdin;
		while let Some(line) = rx.recv().await {
			if stdin.write_all(line.as_bytes()).await.is_err() || stdin.flush().await.is_err() {
				break;
			}
		}
	});
	tx
}

fn spawn_client_listener(mut listener: ipc::Listener, tx: UnboundedSender<Event>) {
	tokio::spawn(async move {
		let mut next_id: u64 = 0;
		loop {
			match listener.accept().await {
				Ok(stream) => {
					next_id += 1;
					let id = next_id;
					let tx = tx.clone();
					tokio::spawn(async move {
						let (reader, mut writer) = tokio::io::split(stream);
						let (out_tx, mut out_rx) = mpsc::unbounded_channel::<DaemonMsg>();
						if tx.send(Event::ClientConnected(id, out_tx)).is_err() {
							return;
						}
						tokio::spawn(async move {
							while let Some(msg) = out_rx.recv().await {
								if proto::write_msg(&mut writer, &msg).await.is_err() {
									break;
								}
							}
						});
						let mut lines = BufReader::new(reader).lines();
						while let Ok(Some(line)) = lines.next_line().await {
							match proto::parse_msg::<ClientMsg>(&line) {
								Ok(msg) => {
									if tx.send(Event::ClientMsg(id, msg)).is_err() {
										return;
									}
								}
								Err(e) => log::note(&format!("client {}: bad message: {}", id, e)),
							}
						}
						let _ = tx.send(Event::ClientGone(id));
					});
				}
				Err(e) => {
					log::note(&format!("client accept failed: {}", e));
					tokio::time::sleep(Duration::from_millis(200)).await;
				}
			}
		}
	});
}

fn spawn_control_listener(mut listener: ipc::Listener, idx: usize, tx: UnboundedSender<Event>) {
	tokio::spawn(async move {
		let mut next_id: u64 = 0;
		loop {
			match listener.accept().await {
				Ok(stream) => {
					next_id += 1;
					let conn = next_id;
					let tx = tx.clone();
					tokio::spawn(async move {
						let (reader, mut writer) = tokio::io::split(stream);
						let (out_tx, mut out_rx) = mpsc::unbounded_channel::<String>();
						if tx.send(Event::ControlConnected(idx, conn, out_tx)).is_err() {
							return;
						}
						tokio::spawn(async move {
							while let Some(line) = out_rx.recv().await {
								let mut bytes = line.into_bytes();
								bytes.push(b'\n');
								if writer.write_all(&bytes).await.is_err() {
									break;
								}
							}
						});
						let mut lines = BufReader::new(reader).lines();
						while let Ok(Some(line)) = lines.next_line().await {
							if tx.send(Event::ControlLine(idx, conn, line)).is_err() {
								return;
							}
						}
						let _ = tx.send(Event::ControlGone(idx, conn));
					});
				}
				Err(e) => {
					log::note(&format!("control accept failed: {}", e));
					tokio::time::sleep(Duration::from_millis(200)).await;
				}
			}
		}
	});
}

impl Daemon {
	fn new(session: &Session, cfg: DashCfg, tx: UnboundedSender<Event>) -> Result<Daemon> {
		let mut procs = Vec::new();
		let mut queue = VecDeque::new();
		for (idx, section) in cfg.sections.into_iter().enumerate() {
			let ready_re = match &section.ready_pattern {
				Some(pattern) => Some(Regex::new(pattern).with_context(|| format!("section [{}] has an invalid ReadyPattern", section.name))?),
				None => None,
			};
			let mut auto_launch = section.auto_start;
			let launch_ctrl = section.launch_ctrl.clone().map(|lc| {
				let (lc, decided) = LaunchCtrl::new(lc, section.auto_start);
				auto_launch = decided;
				lc
			});
			let control_endpoint = if section.control { Some(session.service_endpoint(&section.name)) } else { None };
			if let Some(endpoint) = &control_endpoint {
				let listener = ipc::Listener::bind(endpoint).with_context(|| format!("cannot listen on '{}'", endpoint))?;
				spawn_control_listener(listener, idx, tx.clone());
			}
			let title = section.label();
			procs.push(Proc {
				cfg: section,
				state: ProcState::New,
				title,
				started: false,
				shutdown_requested: false,
				restart_once: false,
				ready_triggered: false,
				ready_re,
				gen: 0,
				child: None,
				stdin: None,
				lines: VecDeque::new(),
				labels: Vec::new(),
				commands: BTreeMap::new(),
				control: None,
				control_endpoint,
				launch_ctrl,
				launched_at: None,
			});
			if auto_launch {
				queue.push_back(idx);
			}
		}
		Ok(Daemon { title: cfg.title, procs, queue, clients: HashMap::new(), tx, shutting_down: false, done: false })
	}

	fn index_of(&self, name: &str) -> Option<usize> {
		self.procs.iter().position(|p| p.cfg.name == name)
	}

	fn broadcast(&mut self, msg: DaemonMsg) {
		self.clients.retain(|_, tx| tx.send(msg.clone()).is_ok());
	}

	fn broadcast_info(&mut self, idx: usize) {
		let info = self.procs[idx].info();
		self.broadcast(DaemonMsg::Process { info });
	}

	/// A raw line: process output or a note of the daemon.
	fn push_output(&mut self, idx: usize, line: StyledLine) {
		self.push_lines(idx, vec![line.into_raw()]);
	}

	/// Console lines from the channel.
	fn push_console(&mut self, idx: usize, lines: Vec<StyledLine>) {
		self.push_lines(idx, lines);
	}

	fn push_lines(&mut self, idx: usize, mut lines: Vec<StyledLine>) {
		if lines.is_empty() {
			return;
		}
		let now = chrono::Local::now().format("%H:%M:%S").to_string();
		for line in &mut lines {
			line.time = now.clone();
		}
		for line in &lines {
			push_capped(&mut self.procs[idx].lines, line.clone());
		}
		let name = self.procs[idx].cfg.name.clone();
		self.broadcast(DaemonMsg::Lines { name, lines });
	}

	fn note(&mut self, idx: usize, text: String, color: Color) {
		log::note(&format!("[{}] {}", self.procs[idx].cfg.name, text));
		self.push_output(idx, StyledLine::colored(format!("> {}", text), color));
	}

	/// The state setters of ProcessControl: state, launch control value and state file, started flag.
	fn set_state(&mut self, idx: usize, state: ProcState) {
		let p = &mut self.procs[idx];
		p.state = state;
		let (value, started) = match state {
			ProcState::New => (None, false),
			ProcState::Launched | ProcState::Captured | ProcState::Ready => (Some(launch_ctrl::RUNNING), true),
			ProcState::Stopping => (Some(launch_ctrl::STOP), true),
			ProcState::Stopped => (Some(launch_ctrl::STOPPED), false),
		};
		p.started = started;
		if let (Some(lc), Some(value)) = (&mut p.launch_ctrl, value) {
			lc.set_value(value);
		}
		self.broadcast_info(idx);
	}

	// *** the launch order ***

	/// MainWindow.autoStart: unprioritised processes start one at a time, waiting for the
	/// previous one to be captured; prioritised ones start when nothing of a lower priority
	/// value is still queued, starting or stopping.
	fn auto_start(&mut self) {
		if self.shutting_down {
			return;
		}
		loop {
			if self.procs.iter().any(|p| p.state == ProcState::Launched && p.cfg.priority == PRIORITY_NONE) {
				return;
			}
			let mut lowest = PRIORITY_NONE;
			for (idx, p) in self.procs.iter().enumerate() {
				let waiting = matches!(p.state, ProcState::Stopped | ProcState::New) && self.queue.contains(&idx);
				let busy = matches!(p.state, ProcState::Launched | ProcState::Captured | ProcState::Stopping);
				if (waiting || busy) && p.cfg.priority < lowest {
					lowest = p.cfg.priority;
				}
			}
			let next = self.queue.iter().position(|&idx| {
				let p = &self.procs[idx];
				matches!(p.state, ProcState::Stopped | ProcState::New) && p.cfg.priority <= lowest
			});
			match next {
				Some(pos) => {
					let idx = self.queue.remove(pos).expect("position came from the queue");
					self.launch(idx);
				}
				None => return,
			}
		}
	}

	/// ProcessControl.LaunchWanted: queue the process at the front and run the launch order.
	fn launch_wanted(&mut self, idx: usize) {
		if self.shutting_down {
			return;
		}
		if !self.queue.contains(&idx) {
			self.queue.push_front(idx);
		}
		self.auto_start();
	}

	/// ProcessHost.Launch / TerminalHost.Launch.
	fn launch(&mut self, idx: usize) {
		if self.shutting_down {
			return;
		}
		let p = &mut self.procs[idx];
		if p.running() {
			return;
		}
		if p.shutdown_requested {
			// Restart(): the exit of the stopping process relaunches it
			p.shutdown_requested = false;
			p.restart_once = true;
			return;
		}
		let program = p.cfg.program(&p.cfg.launch_cmd);
		let mut extra = Vec::new();
		if let Some(endpoint) = &p.control_endpoint {
			extra.push(format!("--ctrl={}", endpoint));
		}
		let cwd = p.cfg.working_directory.clone();
		match spawn::spawn(&program, &p.cfg.launch_args, &extra, cwd.as_deref()) {
			Ok(mut child) => {
				p.gen += 1;
				let gen = p.gen;
				p.stdin = child.stdin.take().map(spawn_stdin_writer);
				if let Some(stdout) = child.stdout.take() {
					spawn_output_reader(self.tx.clone(), idx, gen, stdout, true);
				}
				if let Some(stderr) = child.stderr.take() {
					spawn_output_reader(self.tx.clone(), idx, gen, stderr, false);
				}
				p.child = Some(child);
				p.ready_triggered = false;
				p.launched_at = Some(Instant::now());
				if let Some(lc) = &mut p.launch_ctrl {
					lc.count_launch();
				}
				let control = p.cfg.control;
				let has_pattern = p.ready_re.is_some();
				self.push_output(idx, StyledLine::plain("> Process Started"));
				self.set_state(idx, ProcState::Launched);
				if !control {
					// the pipes are the console: captured at once, like TerminalHost
					self.set_state(idx, ProcState::Captured);
					if !has_pattern {
						self.procs[idx].ready_triggered = true;
						self.set_state(idx, ProcState::Ready);
					}
				}
			}
			Err(e) => {
				self.note(idx, format!("Cannot start '{}': {}", program.display(), e), Color::LightRed);
			}
		}
	}

	/// MainWindow.stopClicked: leave the queue and ask the process to stop.
	fn stop(&mut self, idx: usize) {
		self.queue.retain(|&i| i != idx);
		self.shutdown(idx);
	}

	/// ProcessHost.Shutdown / TerminalHost.Shutdown: a second request kills.
	fn shutdown(&mut self, idx: usize) {
		let p = &mut self.procs[idx];
		if !p.running() {
			return;
		}
		if p.shutdown_requested {
			self.kill(idx);
			return;
		}
		p.shutdown_requested = true;
		let mut via = "";
		if p.cfg.control {
			if let Some((_, tx)) = &p.control {
				let _ = tx.send("cmd\tquit".to_string());
				via = "quit over the console channel";
			} else if let Some(stdin) = &p.stdin {
				let _ = stdin.send("quit\n".to_string());
				via = "quit on stdin";
			}
		} else if let Some(input) = &p.cfg.shutdown_input {
			if let Some(stdin) = &p.stdin {
				let _ = stdin.send(format!("{}\n", input));
				via = "the shutdown input";
			}
		} else if let Some(cmd) = p.cfg.shutdown_cmd.clone() {
			let program = p.cfg.program(&cmd);
			let args = p.cfg.shutdown_args.clone();
			let cwd = p.cfg.working_directory.clone();
			let gen = p.gen;
			match spawn::spawn(&program, &args, &[], cwd.as_deref()) {
				Ok(mut child) => {
					if let Some(stdout) = child.stdout.take() {
						spawn_output_reader(self.tx.clone(), idx, gen, stdout, false);
					}
					if let Some(stderr) = child.stderr.take() {
						spawn_output_reader(self.tx.clone(), idx, gen, stderr, false);
					}
					// the shutdown command runs on its own; nothing waits for it
					tokio::spawn(async move {
						let _ = child.wait().await;
					});
					via = "the shutdown command";
				}
				Err(e) => {
					self.note(idx, format!("Cannot run the shutdown command '{}': {}", program.display(), e), Color::LightRed);
					self.kill_child(idx);
					via = "kill";
				}
			}
		}
		if via.is_empty() {
			self.kill_child(idx);
			via = "kill";
		}
		self.push_output(idx, StyledLine::plain(format!("> Stopping ({})", via)));
		self.set_state(idx, ProcState::Stopping);
		self.auto_start();
	}

	fn kill_child(&mut self, idx: usize) {
		if let Some(child) = &mut self.procs[idx].child {
			if let Err(e) = child.start_kill() {
				log::note(&format!("[{}] kill failed: {}", self.procs[idx].cfg.name, e));
			}
		}
	}

	fn kill(&mut self, idx: usize) {
		self.queue.retain(|&i| i != idx);
		if !self.procs[idx].running() {
			return;
		}
		self.procs[idx].shutdown_requested = true;
		self.push_output(idx, StyledLine::colored("> Killing", Color::LightRed));
		self.kill_child(idx);
		if self.procs[idx].state != ProcState::Stopping {
			self.set_state(idx, ProcState::Stopping);
		}
	}

	/// ProcessHost.processExitedInvoked.
	fn on_exit(&mut self, idx: usize, code: Option<i32>) {
		let p = &mut self.procs[idx];
		p.child = None;
		p.stdin = None;
		let text = match code {
			Some(code) => format!("> Process Exited (code {})", code),
			None => "> Process Exited".to_string(),
		};
		self.push_output(idx, StyledLine::plain(text));
		self.set_state(idx, ProcState::Stopped);
		let p = &mut self.procs[idx];
		if let Some(lc) = &mut p.launch_ctrl {
			lc.after_exit();
		}
		let relaunch = p.restart_once && !p.shutdown_requested;
		p.restart_once = false;
		p.shutdown_requested = false;
		if relaunch {
			self.launch(idx);
		}
		if self.shutting_down {
			self.check_done();
		} else {
			self.auto_start();
		}
	}

	fn check_done(&mut self) {
		if self.shutting_down && !self.procs.iter().any(|p| p.running()) {
			self.done = true;
			self.broadcast(DaemonMsg::Bye);
		}
	}

	/// A stdout EOF usually means the process is gone; the tick catches the rest.
	fn reap(&mut self, idx: usize) {
		let exited = match &mut self.procs[idx].child {
			Some(child) => match child.try_wait() {
				Ok(Some(status)) => Some(status.code()),
				Ok(None) => None,
				Err(_) => Some(None),
			},
			None => None,
		};
		if let Some(code) = exited {
			self.on_exit(idx, code);
		}
	}

	// *** launch control files ***

	/// ProcessControl.launchCtrlChanged.
	fn apply_ctrl_value(&mut self, idx: usize, value: usize) {
		let p = &self.procs[idx];
		let Some(lc) = &p.launch_ctrl else {
			return;
		};
		if value == lc.value {
			return;
		}
		if value == launch_ctrl::LAUNCH && !p.started {
			self.launch_wanted(idx);
		} else if value == launch_ctrl::STOP && p.state != ProcState::Stopping && p.started {
			self.stop(idx);
		} else {
			let current = lc.value;
			if let Some(lc) = &mut self.procs[idx].launch_ctrl {
				lc.set_value(current);
			}
		}
	}

	/// A service binary that does not know `--ctrl` opens its own window and never connects;
	/// it must not hold the rest of the shard in the start order forever.
	fn check_capture_timeouts(&mut self) {
		for idx in 0..self.procs.len() {
			let p = &self.procs[idx];
			let overdue = p.cfg.control && p.state == ProcState::Launched && p.running() && p.launched_at.is_some_and(|t| t.elapsed() > CAPTURE_TIMEOUT);
			if overdue {
				self.note(idx, format!("No console connection after {} s; treating the service as started", CAPTURE_TIMEOUT.as_secs()), Color::Yellow);
				self.set_state(idx, ProcState::Captured);
				self.procs[idx].ready_triggered = true;
				self.set_state(idx, ProcState::Ready);
				self.auto_start();
			}
		}
	}

	fn poll_launch_ctrl(&mut self) {
		for idx in 0..self.procs.len() {
			let state = self.procs[idx].state;
			let changed = match &mut self.procs[idx].launch_ctrl {
				Some(lc) => lc.poll(state),
				None => None,
			};
			if let Some(value) = changed {
				self.apply_ctrl_value(idx, value);
			}
		}
	}

	// *** console channel ***

	fn on_control_connected(&mut self, idx: usize, conn: u64, tx: UnboundedSender<String>) {
		let p = &mut self.procs[idx];
		p.control = Some((conn, tx));
		// the service replays its history on connection, so its earlier lines go; the raw ones stay
		p.lines.retain(|line| line.raw);
		p.labels.clear();
		p.commands.clear();
		let name = p.cfg.name.clone();
		let lines = p.lines.iter().cloned().collect();
		self.broadcast(DaemonMsg::Snapshot { name: name.clone(), lines });
		self.broadcast(DaemonMsg::Commands { name, commands: Vec::new() });
		if self.procs[idx].state == ProcState::Launched {
			self.set_state(idx, ProcState::Captured);
			if self.procs[idx].ready_re.is_none() {
				self.procs[idx].ready_triggered = true;
				self.set_state(idx, ProcState::Ready);
			}
		} else {
			self.broadcast_info(idx);
		}
		self.auto_start();
	}

	fn on_control_gone(&mut self, idx: usize, conn: u64) {
		let p = &mut self.procs[idx];
		if matches!(&p.control, Some((id, _)) if *id == conn) {
			p.control = None;
			self.broadcast_info(idx);
		}
	}

	fn on_control_line(&mut self, idx: usize, conn: u64, line: String) {
		if !matches!(&self.procs[idx].control, Some((id, _)) if *id == conn) {
			return;
		}
		let fields: Vec<String> = line.split('\t').map(unescape_field).collect();
		match fields[0].as_str() {
			"hello" => {}
			"title" => {
				let title = fields.get(1).cloned().unwrap_or_default();
				self.procs[idx].title = title.clone();
				let p = &self.procs[idx];
				let ready = !p.ready_triggered
					&& matches!(p.state, ProcState::Launched | ProcState::Captured)
					&& p.ready_re.as_ref().is_some_and(|re| re.is_match(&title));
				if ready {
					self.procs[idx].ready_triggered = true;
					self.set_state(idx, ProcState::Ready);
					self.auto_start();
				} else {
					self.broadcast_info(idx);
				}
			}
			"label" => {
				let index: usize = fields.get(1).and_then(|v| v.parse().ok()).unwrap_or(usize::MAX);
				let value = fields.get(2).cloned().unwrap_or_default();
				if index < 256 {
					let labels = &mut self.procs[idx].labels;
					if labels.len() <= index {
						labels.resize(index + 1, String::new());
					}
					labels[index] = value.clone();
					let name = self.procs[idx].cfg.name.clone();
					self.broadcast(DaemonMsg::Label { name, index, value });
				}
			}
			"command" => {
				let name = fields.get(1).cloned().unwrap_or_default();
				if name.is_empty() {
					return;
				}
				let kind = match fields.get(2).map(String::as_str) {
					Some("o") => CommandKind::Handler,
					Some("m") => CommandKind::Member,
					_ => CommandKind::Command,
				};
				let command = CommandInfo {
					name: name.clone(),
					kind,
					args: fields.get(3).cloned().unwrap_or_default(),
					help: fields.get(4).cloned().unwrap_or_default(),
				};
				let p = &mut self.procs[idx];
				if p.commands.len() < 10000 || p.commands.contains_key(&name) {
					p.commands.insert(name, command.clone());
					let name = p.cfg.name.clone();
					self.broadcast(DaemonMsg::Command { name, command });
				}
			}
			"uncommand" => {
				let command = fields.get(1).cloned().unwrap_or_default();
				let p = &mut self.procs[idx];
				if p.commands.remove(&command).is_some() {
					let name = p.cfg.name.clone();
					self.broadcast(DaemonMsg::Uncommand { name, command });
				}
			}
			"log" => {
				let color = console_color(fields.get(1).map(String::as_str).unwrap_or("0"));
				let text = fields.get(2).cloned().unwrap_or_default();
				// every message ends with a newline; a message may hold several lines
				let text = text.strip_suffix('\n').unwrap_or(&text);
				let lines = text.split('\n').map(|part| StyledLine::colored(part.strip_suffix('\r').unwrap_or(part), color)).collect();
				self.push_console(idx, lines);
			}
			other => log::note(&format!("[{}] unknown console message '{}'", self.procs[idx].cfg.name, other)),
		}
	}

	// *** output ***

	fn on_output(&mut self, idx: usize, gen: u64, line: StyledLine) {
		if self.procs[idx].gen != gen {
			return;
		}
		let p = &self.procs[idx];
		let ready = !p.cfg.control
			&& !p.ready_triggered
			&& matches!(p.state, ProcState::Launched | ProcState::Captured)
			&& p.ready_re.as_ref().is_some_and(|re| re.is_match(&line.text()));
		self.push_output(idx, line);
		if ready {
			self.procs[idx].ready_triggered = true;
			self.set_state(idx, ProcState::Ready);
			self.auto_start();
		}
	}

	// *** clients ***

	fn on_client_connected(&mut self, id: u64, tx: UnboundedSender<DaemonMsg>) {
		let send = |msg: DaemonMsg| tx.send(msg).is_ok();
		if !send(DaemonMsg::Hello { version: proto::VERSION, title: self.title.clone() }) {
			return;
		}
		let processes = self.procs.iter().map(Proc::info).collect();
		send(DaemonMsg::Processes { processes });
		for p in &self.procs {
			send(DaemonMsg::Snapshot { name: p.cfg.name.clone(), lines: p.lines.iter().cloned().collect() });
			if !p.commands.is_empty() {
				send(DaemonMsg::Commands { name: p.cfg.name.clone(), commands: p.commands.values().cloned().collect() });
			}
		}
		self.clients.insert(id, tx);
	}

	fn send_input(&mut self, idx: usize, text: String) {
		let p = &self.procs[idx];
		match &p.stdin {
			Some(stdin) => {
				let _ = stdin.send(format!("{}\n", text));
			}
			None => self.note(idx, "The process is not running".to_string(), Color::Yellow),
		}
	}

	fn on_client_msg(&mut self, client: u64, msg: ClientMsg) {
		match msg {
			ClientMsg::Hello { .. } => {}
			ClientMsg::Start { name } => {
				if let Some(idx) = self.index_of(&name) {
					// ProcessControl.startButton_Click
					let has_ctrl = self.procs[idx].launch_ctrl.is_some();
					if has_ctrl {
						if let Some(lc) = &mut self.procs[idx].launch_ctrl {
							lc.write_ctrl(launch_ctrl::LAUNCH);
						}
						self.apply_ctrl_value(idx, launch_ctrl::LAUNCH);
					} else {
						self.launch(idx);
					}
				}
			}
			ClientMsg::Stop { name } => {
				if let Some(idx) = self.index_of(&name) {
					let write_stop = self.procs[idx].launch_ctrl.as_ref().is_some_and(|lc| !lc.ctrl_is(launch_ctrl::STOP));
					if write_stop {
						if let Some(lc) = &mut self.procs[idx].launch_ctrl {
							lc.write_ctrl(launch_ctrl::STOP);
						}
						self.apply_ctrl_value(idx, launch_ctrl::STOP);
					} else {
						self.stop(idx);
					}
				}
			}
			ClientMsg::Kill { name } => {
				if let Some(idx) = self.index_of(&name) {
					self.kill(idx);
				}
			}
			ClientMsg::StartAll => {
				if self.shutting_down {
					return;
				}
				self.queue = (0..self.procs.len()).collect();
				self.auto_start();
			}
			ClientMsg::StopAll => {
				self.queue.clear();
				for idx in 0..self.procs.len() {
					self.shutdown(idx);
				}
			}
			ClientMsg::Input { name, text } => {
				if let Some(idx) = self.index_of(&name) {
					self.send_input(idx, text);
				}
			}
			ClientMsg::Command { name, text } => {
				if let Some(idx) = self.index_of(&name) {
					let sent = match &self.procs[idx].control {
						Some((_, tx)) => tx.send(format!("cmd\t{}", escape_field(&text))).is_ok(),
						None => false,
					};
					if !sent {
						self.send_input(idx, text);
					}
				}
			}
			ClientMsg::ClearConsole { name } => {
				if let Some(idx) = self.index_of(&name) {
					self.procs[idx].lines.clear();
					self.broadcast(DaemonMsg::Clear { name });
				}
			}
			ClientMsg::Shutdown => {
				log::note(&format!("client {} asked for the shutdown", client));
				self.shutting_down = true;
				self.queue.clear();
				self.broadcast(DaemonMsg::Notice { text: "Stopping all services, the dashboard exits when they are gone".to_string() });
				for idx in 0..self.procs.len() {
					self.shutdown(idx);
				}
				self.check_done();
			}
		}
	}

	fn handle(&mut self, event: Event) {
		match event {
			Event::ClientConnected(id, tx) => self.on_client_connected(id, tx),
			Event::ClientMsg(id, msg) => self.on_client_msg(id, msg),
			Event::ClientGone(id) => {
				self.clients.remove(&id);
			}
			Event::Output(idx, gen, line) => self.on_output(idx, gen, line),
			Event::OutputClosed(idx, gen) => {
				if self.procs[idx].gen == gen {
					self.reap(idx);
				}
			}
			Event::ControlConnected(idx, conn, tx) => self.on_control_connected(idx, conn, tx),
			Event::ControlLine(idx, conn, line) => self.on_control_line(idx, conn, line),
			Event::ControlGone(idx, conn) => self.on_control_gone(idx, conn),
			Event::Tick => {
				for idx in 0..self.procs.len() {
					if self.procs[idx].running() {
						self.reap(idx);
					}
				}
				self.poll_launch_ctrl();
				self.check_capture_timeouts();
			}
		}
	}
}

pub async fn run(session: Session) -> Result<()> {
	std::env::set_current_dir(&session.dir).with_context(|| format!("cannot enter '{}'", session.dir.display()))?;
	let cfg = crate::config::read(&session.ini_path)?;
	// one dashboard per shard, whichever tool it is; released when the daemon exits
	let _lock = crate::shard_lock::ShardLock::acquire(&session.ini_path)
		.map_err(|holder| anyhow::anyhow!("'{}' is already open in {}", session.ini_path.display(), holder))?;
	session.prepare_runtime_dir()?;
	let listener = ipc::Listener::bind(&session.control_endpoint()).map_err(|e| {
		if ipc::is_in_use(&e) {
			anyhow::anyhow!("a dashboard daemon already runs for '{}'", session.ini_path.display())
		} else {
			anyhow::anyhow!("cannot listen on '{}': {}", session.control_endpoint(), e)
		}
	})?;

	let (tx, mut rx) = mpsc::unbounded_channel::<Event>();
	let mut daemon = Daemon::new(&session, cfg, tx.clone())?;
	spawn_client_listener(listener, tx.clone());
	{
		let tx = tx.clone();
		tokio::spawn(async move {
			let mut interval = tokio::time::interval(TICK);
			loop {
				interval.tick().await;
				if tx.send(Event::Tick).is_err() {
					break;
				}
			}
		});
	}
	log::note(&format!("daemon started for '{}' with {} sections", session.ini_path.display(), daemon.procs.len()));
	crate::registry::register(&session, &daemon.title);

	daemon.auto_start();
	while let Some(event) = rx.recv().await {
		daemon.handle(event);
		if daemon.done {
			break;
		}
	}
	// give the Bye a moment to reach the clients
	tokio::time::sleep(Duration::from_millis(100)).await;
	crate::registry::unregister(&session);
	session.cleanup_runtime_dir();
	log::note("daemon exited");
	Ok(())
}
