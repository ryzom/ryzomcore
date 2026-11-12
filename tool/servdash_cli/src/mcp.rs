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

//! The MCP server: `servdash_cli --mcp [<ini>...]` speaks the Model Context Protocol over
//! stdio, for an agent that watches, drives and debugs the shards and the patchman services
//! of this machine. It is a client of the dashboards like the terminal is: it attaches to
//! every running dashboard of this user as it appears, mirrors their processes, logs and
//! command tables, and forwards the orders. The dashboards keep running when it exits.
//!
//! The ini files on the command line are started when they do not run yet, so a project can
//! name its shard in its MCP configuration and have it come up with the agent.
//!
//! Every tool call runs on its own task, so a call that waits (a log follow, a restart) does
//! not hold the others up. Requests and responses are one JSON-RPC document per line on stdin
//! and stdout; diagnostics go to stderr.

use crate::ipc;
use crate::log;
use crate::proto::{self, ClientMsg, CommandInfo, CommandKind, DaemonMsg, ProcState, ProcessInfo};
use crate::registry::{self, Record};
use crate::session::Session;
use anyhow::{anyhow, bail, Result};
use regex::Regex;
use serde_json::{json, Value};
use std::collections::{BTreeMap, HashMap, HashSet, VecDeque};
use std::path::{Path, PathBuf};
use std::sync::{Arc, Mutex, OnceLock};
use std::time::{Duration, Instant};
use tokio::io::{AsyncBufReadExt, AsyncWriteExt, BufReader};
use tokio::sync::{mpsc, watch};

const SERVER_NAME: &str = "servdash_cli";
/// Newest first; the client's version is echoed when it is one of these.
const PROTOCOL_VERSIONS: &[&str] = &["2025-06-18", "2025-03-26", "2024-11-05"];
const DISCOVERY_INTERVAL: Duration = Duration::from_secs(2);
/// A dashboard whose endpoint did not answer is tried again after this.
const RETRY_INTERVAL: Duration = Duration::from_secs(10);
/// The longest a tool call waits for anything.
const MAX_WAIT: f64 = 300.0;
const DEFAULT_LOG_LINES: usize = 100;
const MAX_LOG_LINES: usize = 5000;
/// Lines kept per process in the mirror; the daemon holds as many.
const MAX_MIRROR_LINES: usize = 10000;
const MAX_LINE_CHARS: usize = 2000;
/// send_input returns once the process has been quiet this long after answering.
const INPUT_IDLE: Duration = Duration::from_millis(400);
/// How long a tool call waits for a dashboard to send its state after the connection; a
/// dashboard that takes longer is reported as not answering instead of being served half known.
const SYNC_TIMEOUT: Duration = Duration::from_secs(15);
/// Run starts and ends kept per dashboard for the waits that watch them.
const MAX_EVENTS: usize = 256;
/// Lines of a process shown with a run that ended during a wait.
const EVENT_LINES: usize = 15;
/// A dashboard says what it does about an exit (a restart, a pending one) right after the exit
/// itself; a wait that ends on the exit gives that this long to arrive before it reports.
const EVENT_SETTLE: Duration = Duration::from_millis(200);
/// How long a dashboard older than the Synced message may take to send what follows its last Snapshot.
const SETTLE_WITHOUT_MARKER: Duration = Duration::from_millis(250);
/// The registry is read this often while a dashboard the server started has not shown up in it.
const EXPECTED_INTERVAL: Duration = Duration::from_millis(100);
/// The log types get_log filters by: those of NeL, plain output, dashboard notes, console lines without a prefix.
const LOG_TYPES: &[&str] = &["INF", "WRN", "ERR", "DBG", "AST", "STT", "UKN", "out", "dash", "log"];

const INSTRUCTIONS: &str = "Service dashboards of Ryzom Core on this machine, one per ini file: a shard \
(its services), or the patchman services of a server. The processes of a dashboard are the sections of \
its ini; a NeL service has a console channel with a command table (list_commands) and status labels, a \
plain program only its output. Log lines are indexed per process from 1 and never renumbered, and carry \
their arrival time; get_log selects by index range, time range or count and can keep following for a \
while, so a window of a log is reproducible. Run NeL commands with run_command and the arguments as a \
list: they reach the service already split, nothing is parsed, so values with spaces, quotes or \
semicolons are safe. To follow a shard coming up: wait_for_state without a process waits for every \
process of the dashboard and returns as soon as one exits, crashes or is relaunched, with its last \
lines; list_processes gives the time each took to get ready, its start count and its last exit; \
get_log without a process, with levels WRN, ERR, AST and since_start, gives the warnings and errors \
of every process, each line tagged with its process. The dashboard parameter can be left out while \
one dashboard runs.";

// *** the mirror ***

struct Line {
	idx: u64,
	ts: i64,
	raw: bool,
	text: String,
}

impl Line {
	fn from_proto(line: &proto::StyledLine) -> Line {
		Line { idx: line.idx, ts: line.ts, raw: line.raw, text: line.text() }
	}
}

struct ProcMirror {
	info: ProcessInfo,
	lines: VecDeque<Line>,
	commands: BTreeMap<String, CommandInfo>,
}

impl ProcMirror {
	fn new(info: ProcessInfo) -> ProcMirror {
		ProcMirror { info, lines: VecDeque::new(), commands: BTreeMap::new() }
	}

	/// The daemon reports next_idx and first_idx with the process state only; the lines that
	/// arrive in between move them on here.
	fn track_indices(&mut self) {
		if let Some(last) = self.lines.back() {
			self.info.next_idx = self.info.next_idx.max(last.idx + 1);
		}
		self.info.first_idx = self.lines.front().map(|l| l.idx).unwrap_or(0);
	}
}

/// A run of a process began or ended, as the mirror saw it in the process states. The waits
/// watch these, so an exit between two looks at the state is not missed.
struct ProcEvent {
	seq: u64,
	name: String,
	/// A run began; otherwise one ended.
	began: bool,
	/// How the run ended, in the words of the dashboard; empty when it did not say.
	how: String,
	/// How long the run that ended had been up, in milliseconds.
	up_ms: Option<i64>,
}

struct Dash {
	record: Record,
	title: String,
	daemon_version: u32,
	procs: Vec<ProcMirror>,
	/// Towards the daemon.
	tx: mpsc::UnboundedSender<ClientMsg>,
	/// The ExecDone answers by id, taken by the run_command call that waits for them.
	exec_results: HashMap<String, bool>,
	/// The state the dashboard sends to a new client has arrived in full. Until then the mirror
	/// is a part of the dashboard, and no tool answers from it.
	synced: bool,
	attached_at: Instant,
	/// The processes whose Snapshot has not come since the Processes message; a dashboard older
	/// than the Synced message counts as synced when this runs empty.
	awaiting: HashSet<String>,
	got_processes: bool,
	/// Such a dashboard sent its last Snapshot; see check_synced_without_marker.
	settling: bool,
	events: VecDeque<ProcEvent>,
	event_seq: u64,
}

/// How the last run of a process ended: what the dashboard says, or the exit code alone from
/// one older than that field.
fn exit_text(info: &ProcessInfo) -> Option<String> {
	if !info.last_exit.is_empty() {
		Some(info.last_exit.clone())
	} else {
		info.exit_code.map(|code| format!("code {}", code))
	}
}

impl Dash {
	fn new(record: Record, tx: mpsc::UnboundedSender<ClientMsg>) -> Dash {
		let title = record.title.clone();
		Dash {
			record,
			title,
			daemon_version: 0,
			procs: Vec::new(),
			tx,
			exec_results: HashMap::new(),
			synced: false,
			attached_at: Instant::now(),
			awaiting: HashSet::new(),
			got_processes: false,
			settling: false,
			events: VecDeque::new(),
			event_seq: 0,
		}
	}

	fn index_of(&self, name: &str) -> Option<usize> {
		self.procs.iter().position(|p| p.info.name == name)
	}

	fn push_event(&mut self, name: &str, began: bool, how: String, up_ms: Option<i64>) {
		self.event_seq += 1;
		self.events.push_back(ProcEvent { seq: self.event_seq, name: name.to_string(), began, how, up_ms });
		while self.events.len() > MAX_EVENTS {
			self.events.pop_front();
		}
	}

	/// A dashboard that sends no Synced message is at the end of its state once every process
	/// got its Snapshot, but for the command table of the last process, which follows its
	/// Snapshot when it has one. So the state counts as whole with the next message, whatever it
	/// is, or when none comes for SETTLE_WITHOUT_MARKER.
	fn check_synced_without_marker(&mut self) {
		if !self.synced && self.daemon_version < 3 && self.got_processes && self.awaiting.is_empty() {
			self.settling = true;
		}
	}

	fn apply(&mut self, msg: DaemonMsg) {
		let was_settling = self.settling;
		self.apply_message(msg);
		if was_settling {
			self.settled();
		}
	}

	/// The wait for what follows the last Snapshot is over.
	fn settled(&mut self) {
		if self.settling {
			self.settling = false;
			self.synced = true;
		}
	}

	fn apply_message(&mut self, msg: DaemonMsg) {
		match msg {
			DaemonMsg::Hello { version, title } => {
				self.daemon_version = version;
				self.title = title;
			}
			DaemonMsg::Processes { processes } => {
				let mut old = std::mem::take(&mut self.procs);
				for info in processes {
					let mut mirror = ProcMirror::new(info);
					if let Some(prev) = old.iter().position(|p| p.info.name == mirror.info.name).map(|pos| old.swap_remove(pos)) {
						mirror.lines = prev.lines;
						mirror.commands = prev.commands;
					}
					self.procs.push(mirror);
				}
				self.got_processes = true;
				self.awaiting = self.procs.iter().map(|p| p.info.name.clone()).collect();
				self.check_synced_without_marker();
			}
			DaemonMsg::Synced => self.synced = true,
			DaemonMsg::Process { info } => match self.index_of(&info.name) {
				Some(idx) => {
					// a run is known by its start time, which a dashboard reports while the process
					// runs; a change of it ends one run and begins another
					let old = &self.procs[idx].info;
					let changed = old.started_at != info.started_at;
					let ended = changed && old.started_at.is_some();
					let began = changed && info.started_at.is_some();
					let up_ms = old.started_at.map(|t| (now_ms() - t).max(0));
					let how = exit_text(&info).unwrap_or_default();
					let name = info.name.clone();
					self.procs[idx].info = info;
					self.procs[idx].track_indices();
					if ended {
						self.push_event(&name, false, how, up_ms);
					}
					if began {
						self.push_event(&name, true, String::new(), None);
					}
				}
				None => self.procs.push(ProcMirror::new(info)),
			},
			DaemonMsg::Label { name, index, value } => {
				if let Some(idx) = self.index_of(&name) {
					let labels = &mut self.procs[idx].info.labels;
					if labels.len() <= index {
						labels.resize(index + 1, String::new());
					}
					labels[index] = value;
				}
			}
			DaemonMsg::Snapshot { name, lines } => {
				if let Some(idx) = self.index_of(&name) {
					self.procs[idx].lines = lines.iter().map(Line::from_proto).collect();
					self.procs[idx].track_indices();
				}
				self.awaiting.remove(&name);
				self.check_synced_without_marker();
			}
			DaemonMsg::Lines { name, lines } => {
				if let Some(idx) = self.index_of(&name) {
					let buffer = &mut self.procs[idx].lines;
					buffer.extend(lines.iter().map(Line::from_proto));
					while buffer.len() > MAX_MIRROR_LINES {
						buffer.pop_front();
					}
					self.procs[idx].track_indices();
				}
			}
			DaemonMsg::Clear { name } => {
				if let Some(idx) = self.index_of(&name) {
					self.procs[idx].lines.clear();
				}
			}
			DaemonMsg::Commands { name, commands } => {
				if let Some(idx) = self.index_of(&name) {
					self.procs[idx].commands = commands.into_iter().map(|c| (c.name.clone(), c)).collect();
				}
			}
			DaemonMsg::Command { name, command } => {
				if let Some(idx) = self.index_of(&name) {
					self.procs[idx].commands.insert(command.name.clone(), command);
				}
			}
			DaemonMsg::Uncommand { name, command } => {
				if let Some(idx) = self.index_of(&name) {
					self.procs[idx].commands.remove(&command);
				}
			}
			DaemonMsg::ExecDone { id, ok, .. } => {
				self.exec_results.insert(id, ok);
			}
			DaemonMsg::Notice { .. } | DaemonMsg::Bye => {}
		}
	}
}

#[derive(Default)]
struct State {
	/// The attached dashboards by record id.
	dashes: BTreeMap<String, Dash>,
	/// The registry has been read once, so the dashboards that run are attached or being attached.
	discovered: bool,
	/// The dashboards of the command line that are not attached yet, and until when to wait for them.
	expected: HashMap<String, Instant>,
	/// Records being connected to right now.
	attaching: HashSet<String>,
	/// Records whose endpoint did not answer, and when to try them again.
	retry_after: HashMap<String, Instant>,
	next_exec_id: u64,
}

#[derive(Clone)]
struct Shared {
	state: Arc<Mutex<State>>,
	/// Bumped on every change of the state; the waiting tool calls watch it.
	changes: Arc<watch::Sender<u64>>,
}

impl Shared {
	fn new() -> Shared {
		let (tx, _rx) = watch::channel(0u64);
		Shared { state: Arc::new(Mutex::new(State::default())), changes: Arc::new(tx) }
	}

	fn with<R>(&self, f: impl FnOnce(&mut State) -> R) -> R {
		let mut state = self.state.lock().unwrap_or_else(|e| e.into_inner());
		f(&mut state)
	}

	fn bump(&self) {
		self.changes.send_modify(|v| *v = v.wrapping_add(1));
	}

	/// True once the predicate holds, false when the deadline passes first.
	async fn wait_until(&self, deadline: Instant, mut pred: impl FnMut(&State) -> bool) -> bool {
		let mut rx = self.changes.subscribe();
		loop {
			if self.with(|s| pred(s)) {
				return true;
			}
			if Instant::now() >= deadline {
				return false;
			}
			tokio::select! {
				changed = rx.changed() => {
					if changed.is_err() {
						return false;
					}
				}
				_ = tokio::time::sleep_until(tokio::time::Instant::from_std(deadline)) => {
					return self.with(|s| pred(s));
				}
			}
		}
	}
}

/// Attaches to one dashboard and mirrors it until the connection ends.
async fn attach(shared: Shared, record: Record) {
	let id = record.id.clone();
	let stream = match ipc::connect(&record.endpoint).await {
		Ok(stream) => stream,
		Err(e) => {
			log::note(&format!("cannot reach the dashboard '{}' at {}: {}", record.title, record.endpoint, e));
			shared.with(|s| {
				s.attaching.remove(&id);
				s.retry_after.insert(id.clone(), Instant::now() + RETRY_INTERVAL);
			});
			// a tool call may be waiting for this connection to stand or fail
			shared.bump();
			return;
		}
	};
	let (reader, mut writer) = tokio::io::split(stream);
	let (tx, mut rx) = mpsc::unbounded_channel::<ClientMsg>();
	tokio::spawn(async move {
		while let Some(msg) = rx.recv().await {
			if proto::write_msg(&mut writer, &msg).await.is_err() {
				break;
			}
		}
	});
	let _ = tx.send(ClientMsg::Hello { version: proto::VERSION });
	shared.with(|s| {
		s.attaching.remove(&id);
		s.retry_after.remove(&id);
		s.dashes.insert(id.clone(), Dash::new(record, tx));
	});
	shared.bump();
	let mut lines = BufReader::new(reader).lines();
	loop {
		// a dashboard without the Synced message: what follows its last Snapshot, or nothing
		let settling = shared.with(|s| s.dashes.get(&id).is_some_and(|d| d.settling));
		let next = if settling {
			match tokio::time::timeout(SETTLE_WITHOUT_MARKER, lines.next_line()).await {
				Ok(next) => next,
				Err(_) => {
					shared.with(|s| {
						if let Some(dash) = s.dashes.get_mut(&id) {
							dash.settled();
						}
					});
					shared.bump();
					continue;
				}
			}
		} else {
			lines.next_line().await
		};
		let Ok(Some(line)) = next else {
			break;
		};
		let Ok(msg) = proto::parse_msg::<DaemonMsg>(&line) else {
			continue;
		};
		let bye = matches!(msg, DaemonMsg::Bye);
		shared.with(|s| {
			if let Some(dash) = s.dashes.get_mut(&id) {
				dash.apply(msg);
			}
		});
		shared.bump();
		if bye {
			break;
		}
	}
	shared.with(|s| {
		s.dashes.remove(&id);
	});
	shared.bump();
}

/// Attaches to every dashboard of the registry as it appears.
async fn discover(shared: Shared) {
	loop {
		for record in registry::records() {
			let attach_now = shared.with(|s| {
				if s.dashes.contains_key(&record.id) || s.attaching.contains(&record.id) {
					return false;
				}
				if s.retry_after.get(&record.id).is_some_and(|t| Instant::now() < *t) {
					return false;
				}
				s.attaching.insert(record.id.clone());
				true
			});
			if attach_now {
				tokio::spawn(attach(shared.clone(), record));
			}
		}
		// a dashboard this server started itself is looked for until it is there or its time is up
		let (changed, expecting) = shared.with(|s| {
			let before = s.expected.len();
			let State { dashes, expected, .. } = &mut *s;
			expected.retain(|id, until| !dashes.contains_key(id) && Instant::now() < *until);
			let first = !std::mem::replace(&mut s.discovered, true);
			(first || before != s.expected.len(), !s.expected.is_empty())
		});
		if changed {
			shared.bump();
		}
		tokio::time::sleep(if expecting { EXPECTED_INTERVAL } else { DISCOVERY_INTERVAL }).await;
	}
}

/// The dashboards a call still has to wait for, by the time each has left to send its state:
/// the one the call names, or all of them when it names none or its selector does not resolve.
/// One whose time is up is not waited for again.
fn sync_pending(state: &State, selector: Option<&str>) -> Vec<Instant> {
	let only = selector.and_then(|sel| find_dash_any(state, Some(sel)).ok());
	let now = Instant::now();
	state
		.dashes
		.values()
		.filter(|d| !d.synced && only.as_ref().is_none_or(|id| *id == d.record.id))
		.map(|d| d.attached_at + SYNC_TIMEOUT)
		.filter(|until| *until > now)
		.collect()
}

/// Holds a tool call until the mirror it answers from is whole: the registry read once, and
/// the dashboard the call names (every dashboard, when it names none) has sent its state, or
/// has had SYNC_TIMEOUT to do so. A server asked right after its start (a client that runs one
/// call per process does nothing else) would otherwise answer from the part of the state that
/// happened to have arrived.
async fn wait_for_sync(shared: &Shared, selector: Option<&str>) {
	let hard = Instant::now() + SYNC_TIMEOUT;
	// which dashboards run is known once the registry was read, the connections stand, and the
	// dashboards this server started itself have registered
	shared.wait_until(hard, |s| s.discovered && s.attaching.is_empty() && s.expected.is_empty()).await;
	loop {
		let Some(until) = shared.with(|s| sync_pending(s, selector).into_iter().max()) else {
			return;
		};
		let until = until.min(hard);
		if Instant::now() >= until {
			return;
		}
		shared.wait_until(until, |s| sync_pending(s, selector).is_empty()).await;
	}
}

// *** the server ***

pub async fn run(inis: Vec<PathBuf>) -> Result<()> {
	// the dashboards named on the command line come up with the server
	let shared = Shared::new();
	for ini in &inis {
		let session = Session::new(ini)?;
		if crate::ensure_daemon(&session).await? {
			log::note(&format!("started the dashboard for '{}'", session.ini_path.display()));
		}
		// its record may be a moment behind its endpoint; the first tool call waits for it
		shared.with(|s| s.expected.insert(session.id.clone(), Instant::now() + SYNC_TIMEOUT));
	}
	tokio::spawn(discover(shared.clone()));

	let (out_tx, mut out_rx) = mpsc::unbounded_channel::<Value>();
	tokio::spawn(async move {
		let mut stdout = tokio::io::stdout();
		while let Some(value) = out_rx.recv().await {
			let mut bytes = match serde_json::to_vec(&value) {
				Ok(bytes) => bytes,
				Err(_) => continue,
			};
			bytes.push(b'\n');
			if stdout.write_all(&bytes).await.is_err() || stdout.flush().await.is_err() {
				break;
			}
		}
	});

	let mut lines = BufReader::new(tokio::io::stdin()).lines();
	// the client closing stdin ends the server; the dashboards stay
	while let Ok(Some(line)) = lines.next_line().await {
		if line.trim().is_empty() {
			continue;
		}
		let msg: Value = match serde_json::from_str(&line) {
			Ok(value) => value,
			Err(e) => {
				let _ = out_tx.send(error_response(Value::Null, -32700, &format!("parse error: {}", e)));
				continue;
			}
		};
		let shared = shared.clone();
		let out_tx = out_tx.clone();
		tokio::spawn(async move {
			if let Some(response) = handle(shared, msg).await {
				let _ = out_tx.send(response);
			}
		});
	}
	Ok(())
}

fn error_response(id: Value, code: i64, message: &str) -> Value {
	json!({ "jsonrpc": "2.0", "id": id, "error": { "code": code, "message": message } })
}

async fn handle(shared: Shared, msg: Value) -> Option<Value> {
	let method = msg.get("method").and_then(Value::as_str).unwrap_or("").to_string();
	let params = msg.get("params").cloned().unwrap_or(Value::Null);
	// notifications need no answer; a message without a method is a response to a request we
	// never sent
	let id = msg.get("id").cloned()?;
	if method.is_empty() || method.starts_with("notifications/") {
		return None;
	}
	let result: Result<Value, (i64, String)> = match method.as_str() {
		"initialize" => Ok(initialize(&params)),
		"ping" => Ok(json!({})),
		"tools/list" => Ok(json!({ "tools": tool_list() })),
		"tools/call" => Ok(call_tool(&shared, &params).await),
		"resources/list" => Ok(json!({ "resources": [] })),
		"resources/templates/list" => Ok(json!({ "resourceTemplates": [] })),
		"prompts/list" => Ok(json!({ "prompts": [] })),
		other => Err((-32601, format!("method not found: {}", other))),
	};
	Some(match result {
		Ok(result) => json!({ "jsonrpc": "2.0", "id": id, "result": result }),
		Err((code, message)) => error_response(id, code, &message),
	})
}

fn initialize(params: &Value) -> Value {
	let requested = params.get("protocolVersion").and_then(Value::as_str).unwrap_or("");
	let version = PROTOCOL_VERSIONS.iter().find(|v| **v == requested).copied().unwrap_or(PROTOCOL_VERSIONS[0]);
	json!({
		"protocolVersion": version,
		"capabilities": { "tools": { "listChanged": false } },
		"serverInfo": { "name": SERVER_NAME, "version": env!("CARGO_PKG_VERSION") },
		"instructions": INSTRUCTIONS,
	})
}

fn text_result(text: String, is_error: bool) -> Value {
	json!({ "content": [{ "type": "text", "text": text }], "isError": is_error })
}

async fn call_tool(shared: &Shared, params: &Value) -> Value {
	let name = params.get("name").and_then(Value::as_str).unwrap_or("");
	let empty = json!({});
	let args = params.get("arguments").unwrap_or(&empty);
	wait_for_sync(shared, arg_str(args, "dashboard").as_deref()).await;
	let result = match name {
		"list_dashboards" => tool_list_dashboards(shared),
		"start_dashboard" => tool_start_dashboard(shared, args).await,
		"shutdown_dashboard" => tool_shutdown_dashboard(shared, args).await,
		"list_processes" => tool_list_processes(shared, args),
		"get_process" => tool_get_process(shared, args),
		"start_process" => tool_process_order(shared, args, Order::Start).await,
		"stop_process" => tool_process_order(shared, args, Order::Stop).await,
		"kill_process" => tool_process_order(shared, args, Order::Kill).await,
		"restart_process" => tool_restart_process(shared, args).await,
		"start_all" => tool_all(shared, args, true).await,
		"stop_all" => tool_all(shared, args, false).await,
		"wait_for_state" => tool_wait_for_state(shared, args).await,
		"list_commands" => tool_list_commands(shared, args),
		"run_command" => tool_run_command(shared, args).await,
		"send_input" => tool_send_input(shared, args).await,
		"get_log" => tool_get_log(shared, args).await,
		"clear_log" => tool_clear_log(shared, args),
		other => Err(anyhow!("unknown tool '{}'", other)),
	};
	match result {
		Ok(text) => text_result(text, false),
		Err(e) => text_result(format!("{:#}", e), true),
	}
}

// *** tool schemas ***

fn prop(kind: &str, description: &str) -> Value {
	json!({ "type": kind, "description": description })
}

fn dashboard_prop() -> Value {
	prop("string", "Which dashboard: its title, id or id prefix, ini path or the end of it. Optional while one dashboard runs.")
}

fn process_prop() -> Value {
	prop("string", "The process: its section name in the ini, or its label.")
}

fn wait_prop(what: &str) -> Value {
	prop("number", &format!("Seconds to wait for {}, 0 to return at once. At most 300.", what))
}

fn tool(name: &str, description: &str, properties: Value, required: &[&str]) -> Value {
	json!({
		"name": name,
		"description": description,
		"inputSchema": { "type": "object", "properties": properties, "required": required },
	})
}

fn tool_list() -> Vec<Value> {
	let prefix_prop = json!({
		"type": "string",
		"enum": ["none", "compact", "full"],
		"description": "How much of the NeL log prefix (type, thread, file, line, function) to show: none keeps the type only (default), compact adds file:line and function, full keeps the line as logged."
	});
	vec![
		tool("list_dashboards", "The dashboards running on this machine for this user: shards and patchman services, with their processes counted.", json!({}), &[]),
		tool(
			"start_dashboard",
			"Starts the dashboard daemon of an ini file (a shard) when none runs for it, and waits until it answers. Its auto start processes come up in order.",
			json!({
				"ini_path": prop("string", "Path of the dashboard ini file."),
				"wait_seconds": wait_prop("the dashboard to answer (default 15)"),
			}),
			&["ini_path"],
		),
		tool(
			"shutdown_dashboard",
			"Stops every process of a dashboard and exits its daemon, like the Shutdown All button followed by closing the window.",
			json!({ "dashboard": dashboard_prop(), "wait_seconds": wait_prop("the daemon to exit (default 60)") }),
			&[],
		),
		tool(
			"list_processes",
			"The processes of a dashboard, one per line: state, pid, uptime, console and launch control, how often the dashboard started it (start count), how long it took from start to ready, how its last run ended, and the log line range.",
			json!({ "dashboard": dashboard_prop() }),
			&[],
		),
		tool(
			"get_process",
			"Everything about one process: configuration, state, start count, time from start to ready, how the last run ended, the service's status labels and buttons, its log range and command table size.",
			json!({ "dashboard": dashboard_prop(), "process": process_prop() }),
			&["process"],
		),
		tool(
			"start_process",
			"Starts a process, through its launch control file when it has one so the admin executor stays in step; the Start button.",
			json!({ "dashboard": dashboard_prop(), "process": process_prop(), "wait_seconds": wait_prop("the process to be ready (default 0)") }),
			&["process"],
		),
		tool(
			"stop_process",
			"Asks a process to stop (quit over its console channel, or its shutdown command); the Stop button. A second stop kills it.",
			json!({ "dashboard": dashboard_prop(), "process": process_prop(), "wait_seconds": wait_prop("the process to exit (default 0)") }),
			&["process"],
		),
		tool("kill_process", "Kills a process at once.", json!({ "dashboard": dashboard_prop(), "process": process_prop(), "wait_seconds": wait_prop("the process to exit (default 0)") }), &["process"]),
		tool(
			"restart_process",
			"Stops a process, waits for it to exit, starts it again and waits for it to be ready.",
			json!({ "dashboard": dashboard_prop(), "process": process_prop(), "wait_seconds": wait_prop("each of the two steps (default 60)") }),
			&["process"],
		),
		tool("start_all", "Starts every process of a dashboard in the configured order; the Launch All button.", json!({ "dashboard": dashboard_prop(), "wait_seconds": wait_prop("every process to be ready (default 0)") }), &[]),
		tool("stop_all", "Stops every running process of a dashboard; the Shutdown All button.", json!({ "dashboard": dashboard_prop(), "wait_seconds": wait_prop("every process to exit (default 0)") }), &[]),
		tool(
			"wait_for_state",
			"Waits until a process, or without the process parameter every process of the dashboard, reaches a state: ready (the service is up), running (started, console attached or not), stopped. Returns early when a watched process exits, crashes or is relaunched meanwhile (for stopped: when one is started again), naming it with the way it ended and its last log lines.",
			json!({
				"dashboard": dashboard_prop(),
				"process": prop("string", "The process: its section name in the ini, or its label. Leave out to wait for every process of the dashboard."),
				"state": { "type": "string", "enum": ["ready", "running", "stopped"], "description": "The state to wait for." },
				"wait_seconds": wait_prop("the state (default 60)"),
				"stop_on_exit": prop("boolean", "Return as soon as a watched process exits or is relaunched while waiting (default true); false keeps waiting for the state."),
				"prefix": prefix_prop,
			}),
			&["state"],
		),
		tool(
			"list_commands",
			"The command table of a NeL service as it reports it: commands and variables with their argument syntax and help, named handler instances and their member commands (instance.command). Filter by prefix or a word of the name or help.",
			json!({
				"dashboard": dashboard_prop(),
				"process": process_prop(),
				"prefix": prop("string", "Only names starting with this (case sensitive, as the commands are)."),
				"contains": prop("string", "Only entries whose name or help contains this (case insensitive)."),
				"limit": prop("integer", "At most this many entries (default 300)."),
			}),
			&["process"],
		),
		tool(
			"run_command",
			"Runs a NeL command on a service over its console channel and returns what it logged. The arguments go as a list and reach the command as given, nothing is parsed. A variable is read by its name alone and set with one argument.",
			json!({
				"dashboard": dashboard_prop(),
				"process": process_prop(),
				"command": prop("string", "The command or variable name, instance.command for a member command."),
				"args": { "type": "array", "items": { "type": "string" }, "description": "The arguments, one per entry." },
				"wait_seconds": wait_prop("the command to complete (default 10)"),
				"prefix": prefix_prop,
			}),
			&["process", "command"],
		),
		tool(
			"send_input",
			"Writes a line to the stdin of a process (a plain program, or a NeL service without its console channel) and returns what it printed in response.",
			json!({
				"dashboard": dashboard_prop(),
				"process": process_prop(),
				"text": prop("string", "The line to write, without the newline."),
				"wait_seconds": wait_prop("output to settle (default 3)"),
				"prefix": prefix_prop,
			}),
			&["process", "text"],
		),
		tool(
			"get_log",
			"Lines of a process log, or without the process parameter of every process of the dashboard merged by time, each line tagged with its process. Select by index range (from_idx, to_idx), by time range (from_time, to_time), since the process started (since_start), or the last N lines, and filter by log type (levels) and regular expression (match): levels WRN, ERR, AST with since_start and no process gives every warning and error of a shard since it came up. Then optionally keep following for follow_seconds, up to max_lines in total. Each line: index, time, [process], log type, message. Indices are per process, from 1, never reused.",
			json!({
				"dashboard": dashboard_prop(),
				"process": prop("string", "The process; leave out for every process of the dashboard, each line then names its process."),
				"last": prop("integer", "The last N lines of the selection, per process (default 100 when no range is given)."),
				"from_idx": prop("integer", "First line index, inclusive."),
				"to_idx": prop("integer", "Last line index, inclusive."),
				"from_time": prop("string", "Earliest arrival time: HH:MM[:SS[.mmm]] today, an ISO 8601 date and time, Unix seconds or milliseconds, or relative such as -90s, -5m, -2h."),
				"to_time": prop("string", "Latest arrival time, same forms."),
				"since_start": prop("boolean", "Only the lines since the last start of each process, as far as they are still held."),
				"follow_seconds": prop("number", "After the selection, keep collecting new lines for this long (at most 300), stopping early at max_lines."),
				"max_lines": prop("integer", "At most this many lines in the answer, the newest of the selection (default 200, at most 5000)."),
				"prefix": prefix_prop,
				"levels": { "type": "array", "items": { "type": "string" }, "description": "Only these log types: INF, WRN, ERR, DBG, AST, STT, UKN of NeL; out (plain output of the process), dash (notes of the dashboard), log (a console line without a NeL prefix)." },
				"match": prop("string", "Only lines matching this regular expression (case insensitive)."),
			}),
			&[],
		),
		tool("clear_log", "Drops the held lines of a process; indices keep counting.", json!({ "dashboard": dashboard_prop(), "process": process_prop() }), &["process"]),
	]
}

// *** arguments ***

fn arg_str(args: &Value, key: &str) -> Option<String> {
	match args.get(key) {
		Some(Value::String(s)) if !s.trim().is_empty() => Some(s.clone()),
		Some(Value::Number(n)) => Some(n.to_string()),
		_ => None,
	}
}

fn arg_f64(args: &Value, key: &str) -> Option<f64> {
	match args.get(key) {
		Some(Value::Number(n)) => n.as_f64(),
		Some(Value::String(s)) => s.trim().parse().ok(),
		_ => None,
	}
}

fn arg_u64(args: &Value, key: &str) -> Option<u64> {
	arg_f64(args, key).filter(|v| *v >= 0.0).map(|v| v as u64)
}

fn arg_bool(args: &Value, key: &str) -> Option<bool> {
	match args.get(key) {
		Some(Value::Bool(b)) => Some(*b),
		Some(Value::Number(n)) => n.as_f64().map(|v| v != 0.0),
		Some(Value::String(s)) => match s.trim().to_lowercase().as_str() {
			"true" | "1" | "yes" => Some(true),
			"false" | "0" | "no" => Some(false),
			_ => None,
		},
		_ => None,
	}
}

fn arg_strs(args: &Value, key: &str) -> Vec<String> {
	match args.get(key) {
		Some(Value::Array(items)) => items
			.iter()
			.map(|v| match v {
				Value::String(s) => s.clone(),
				other => other.to_string(),
			})
			.collect(),
		Some(Value::String(s)) => vec![s.clone()],
		_ => Vec::new(),
	}
}

fn wait_seconds(args: &Value, default: f64) -> Duration {
	Duration::from_secs_f64(arg_f64(args, "wait_seconds").unwrap_or(default).clamp(0.0, MAX_WAIT))
}

#[derive(Clone, Copy, PartialEq, Eq)]
enum PrefixMode {
	None,
	Compact,
	Full,
}

fn prefix_mode(args: &Value) -> PrefixMode {
	match arg_str(args, "prefix").as_deref() {
		Some("compact") => PrefixMode::Compact,
		Some("full") => PrefixMode::Full,
		_ => PrefixMode::None,
	}
}

// *** resolution ***

fn dash_summary(dash: &Dash) -> String {
	let tool = if dash.record.tool.is_empty() { "servdash_cli" } else { dash.record.tool.as_str() };
	format!("'{}' (id {}, {}, {})", dash.title, dash.record.id, tool, dash.record.ini_path)
}

/// The id of the dashboard a selector names; the only one when the selector is empty. A
/// dashboard that has not sent its state is refused: its mirror is a part of it at best.
fn find_dash(state: &State, selector: Option<&str>) -> Result<String> {
	let id = find_dash_any(state, selector)?;
	let dash = &state.dashes[&id];
	if !dash.synced {
		bail!(
			"the dashboard {} is connected but has not sent its state in the {} s since; it may be busy or hung, ask again in a moment",
			dash_summary(dash),
			dash.attached_at.elapsed().as_secs()
		);
	}
	Ok(id)
}

/// The same for a dashboard in any condition, for what needs no mirror of it.
fn find_dash_any(state: &State, selector: Option<&str>) -> Result<String> {
	let mut dashes: Vec<&Dash> = state.dashes.values().collect();
	dashes.sort_by_key(|d| (d.record.started, d.record.id.clone()));
	if dashes.is_empty() {
		bail!("no dashboard is running; start_dashboard starts one from its ini file");
	}
	let Some(selector) = selector else {
		if dashes.len() == 1 {
			return Ok(dashes[0].record.id.clone());
		}
		let names: Vec<String> = dashes.iter().map(|d| dash_summary(d)).collect();
		bail!("several dashboards run, name one with the dashboard parameter:\n{}", names.join("\n"));
	};
	let lowered = selector.trim().to_lowercase();
	let matches: Vec<&&Dash> = dashes
		.iter()
		.filter(|d| {
			d.record.id.starts_with(&lowered)
				|| d.title.to_lowercase() == lowered
				|| d.record.title.to_lowercase() == lowered
				|| d.record.ini_path.to_lowercase() == lowered
				|| d.record.ini_path.to_lowercase().replace('\\', "/").ends_with(&lowered.replace('\\', "/"))
		})
		.collect();
	match matches.len() {
		1 => Ok(matches[0].record.id.clone()),
		0 => {
			let names: Vec<String> = dashes.iter().map(|d| dash_summary(d)).collect();
			bail!("no running dashboard matches '{}'; running:\n{}", selector, names.join("\n"))
		}
		_ => bail!("'{}' matches several dashboards; use the id", selector),
	}
}

fn find_proc(dash: &Dash, name: &str) -> Result<usize> {
	let wanted = name.trim();
	if let Some(idx) = dash.procs.iter().position(|p| p.info.name == wanted) {
		return Ok(idx);
	}
	let lowered = wanted.to_lowercase();
	let matches: Vec<usize> = dash
		.procs
		.iter()
		.enumerate()
		.filter(|(_, p)| p.info.name.to_lowercase() == lowered || p.info.label.to_lowercase() == lowered || p.info.title.to_lowercase() == lowered)
		.map(|(i, _)| i)
		.collect();
	match matches.len() {
		1 => Ok(matches[0]),
		0 => {
			let names: Vec<&str> = dash.procs.iter().map(|p| p.info.name.as_str()).collect();
			bail!("no process '{}' in {}; processes: {}", name, dash_summary(dash), names.join(", "))
		}
		_ => bail!("'{}' names several processes of {}; use the section name", name, dash_summary(dash)),
	}
}

/// Dashboard id and process index for the usual pair of arguments.
fn find_target(state: &State, args: &Value) -> Result<(String, usize)> {
	let dash_id = find_dash(state, arg_str(args, "dashboard").as_deref())?;
	let process = arg_str(args, "process").ok_or_else(|| anyhow!("the process parameter is required"))?;
	let dash = &state.dashes[&dash_id];
	let idx = find_proc(dash, &process)?;
	Ok((dash_id, idx))
}

// *** formatting ***

fn now_ms() -> i64 {
	chrono::Local::now().timestamp_millis()
}

fn format_time(ts: i64) -> String {
	match chrono::DateTime::from_timestamp_millis(ts) {
		Some(utc) => utc.with_timezone(&chrono::Local).format("%H:%M:%S%.3f").to_string(),
		None => "??:??:??.???".to_string(),
	}
}

fn format_date_time(ts: i64) -> String {
	match chrono::DateTime::from_timestamp_millis(ts) {
		Some(utc) => utc.with_timezone(&chrono::Local).format("%Y-%m-%d %H:%M:%S").to_string(),
		None => "unknown".to_string(),
	}
}

fn format_duration(d: Duration) -> String {
	let secs = d.as_secs();
	if secs >= 3600 {
		format!("{}h{:02}m", secs / 3600, (secs % 3600) / 60)
	} else if secs >= 60 {
		format!("{}m{:02}s", secs / 60, secs % 60)
	} else {
		format!("{}s", secs)
	}
}

fn state_name(state: ProcState) -> &'static str {
	match state {
		ProcState::New => "new",
		ProcState::Launched => "launched",
		ProcState::Captured => "captured",
		ProcState::Ready => "ready",
		ProcState::Stopping => "stopping",
		ProcState::Stopped => "stopped",
	}
}

/// The NeL log prefix as CWindowDisplayer::stringifyMessage writes it: type, thread id, file,
/// line and function, each optional, then `: ` and the message.
struct ParsedLine<'a> {
	level: Option<&'a str>,
	file: Option<&'a str>,
	line: Option<&'a str>,
	func: Option<&'a str>,
	msg: &'a str,
}

fn parse_prefix(text: &str) -> Option<ParsedLine<'_>> {
	static RE: OnceLock<Regex> = OnceLock::new();
	let re =
		RE.get_or_init(|| Regex::new(r"^(?:(INF|WRN|ERR|DBG|AST|STT|UKN) +)?[0-9a-fA-F]{1,8}(?: +(\S+) +(\d+)(?: +(\S+))?)?: (.*)$").expect("a valid regex"));
	let caps = re.captures(text)?;
	Some(ParsedLine {
		level: caps.get(1).map(|m| m.as_str()),
		file: caps.get(2).map(|m| m.as_str()),
		line: caps.get(3).map(|m| m.as_str()),
		func: caps.get(4).map(|m| m.as_str()),
		msg: caps.get(5).map(|m| m.as_str()).unwrap_or(""),
	})
}

/// The log type of a line as get_log shows and filters it: the NeL type, `out` for plain output,
/// `dash` for a note of the dashboard, `log` for a console line without a prefix.
fn line_level(line: &Line) -> &'static str {
	if line.raw && line.text.starts_with("> ") {
		return "dash";
	}
	match parse_prefix(&line.text).and_then(|p| p.level) {
		Some("INF") => "INF",
		Some("WRN") => "WRN",
		Some("ERR") => "ERR",
		Some("DBG") => "DBG",
		Some("AST") => "AST",
		Some("STT") => "STT",
		Some("UKN") => "UKN",
		_ => {
			if line.raw {
				"out"
			} else {
				"log"
			}
		}
	}
}

fn truncate_chars(text: &str, max: usize) -> String {
	if text.chars().count() <= max {
		return text.to_string();
	}
	let mut out: String = text.chars().take(max).collect();
	out.push_str(" [...]");
	out
}

fn format_line(line: &Line, process: Option<&str>, mode: PrefixMode) -> String {
	let mut out = format!("{} {}", line.idx, format_time(line.ts));
	if let Some(process) = process {
		out.push_str(&format!(" [{}]", process));
	}
	let level = line_level(line);
	let body: String = match mode {
		PrefixMode::Full => {
			out.push_str(&format!(" {:<4}", level));
			line.text.clone()
		}
		PrefixMode::Compact => {
			out.push_str(&format!(" {:<4}", level));
			match parse_prefix(&line.text) {
				Some(parsed) => {
					let mut body = String::new();
					if let (Some(file), Some(number)) = (parsed.file, parsed.line) {
						body.push_str(&format!("{}:{} ", file, number));
					}
					if let Some(func) = parsed.func {
						body.push_str(&format!("{}: ", func));
					}
					body.push_str(parsed.msg);
					body
				}
				None => line.text.clone(),
			}
		}
		PrefixMode::None => {
			out.push_str(&format!(" {:<4}", level));
			match parse_prefix(&line.text) {
				Some(parsed) => parsed.msg.to_string(),
				None if level == "dash" => line.text[2..].to_string(),
				None => line.text.clone(),
			}
		}
	};
	out.push(' ');
	out.push_str(&truncate_chars(&body, MAX_LINE_CHARS));
	out
}

/// An absolute time in milliseconds from the forms get_log accepts.
fn parse_time(text: &str) -> Result<i64> {
	use chrono::{Local, NaiveDateTime, NaiveTime, TimeZone};
	let text = text.trim();
	if let Some(rest) = text.strip_prefix('-') {
		// relative to now: -90s, -5m, -2h, -1d
		let (number, unit) = rest.split_at(rest.len().saturating_sub(1));
		let value: f64 = number.parse().map_err(|_| anyhow!("cannot read the relative time '{}'", text))?;
		let secs = match unit {
			"s" => value,
			"m" => value * 60.0,
			"h" => value * 3600.0,
			"d" => value * 86400.0,
			_ => bail!("a relative time ends in s, m, h or d: '{}'", text),
		};
		return Ok(now_ms() - (secs * 1000.0) as i64);
	}
	if let Ok(number) = text.parse::<f64>() {
		// Unix seconds, or milliseconds when too large for seconds
		return Ok(if number > 1e11 { number as i64 } else { (number * 1000.0) as i64 });
	}
	if let Ok(fixed) = chrono::DateTime::parse_from_rfc3339(text) {
		return Ok(fixed.timestamp_millis());
	}
	for format in ["%Y-%m-%dT%H:%M:%S%.f", "%Y-%m-%d %H:%M:%S%.f", "%Y-%m-%dT%H:%M:%S", "%Y-%m-%d %H:%M:%S", "%Y-%m-%d %H:%M", "%Y-%m-%dT%H:%M"] {
		if let Ok(naive) = NaiveDateTime::parse_from_str(text, format) {
			if let Some(local) = Local.from_local_datetime(&naive).earliest() {
				return Ok(local.timestamp_millis());
			}
		}
	}
	for format in ["%H:%M:%S%.f", "%H:%M:%S", "%H:%M"] {
		if let Ok(time) = NaiveTime::parse_from_str(text, format) {
			let today = Local::now().date_naive();
			if let Some(local) = Local.from_local_datetime(&today.and_time(time)).earliest() {
				return Ok(local.timestamp_millis());
			}
		}
	}
	bail!("cannot read the time '{}': use HH:MM[:SS[.mmm]], an ISO 8601 date and time, Unix seconds or a relative -30s, -5m, -2h", text)
}

/// Seconds with the precision a boot time needs.
fn format_secs(ms: i64) -> String {
	format!("{:.1} s", ms.max(0) as f64 / 1000.0)
}

/// `daemon_version` tells whether the dashboard reports the start count at all.
fn process_line(p: &ProcMirror, daemon_version: u32) -> String {
	let info = &p.info;
	let mut flags = Vec::new();
	if let Some(pid) = info.pid {
		flags.push(format!("pid {}", pid));
	}
	if let Some(started_at) = info.started_at {
		let up = Duration::from_millis((now_ms() - started_at).max(0) as u64);
		flags.push(format!("up {}", format_duration(up)));
	}
	if info.control {
		flags.push(if info.control_connected { "console connected".to_string() } else { "console not connected".to_string() });
	}
	if let Some(ctrl) = &info.ctrl_value {
		flags.push(format!("launch_ctrl {}", ctrl));
	}
	if info.relaunch_pending {
		flags.push(format!("restart pending ({} quick exits)", info.crash_chain));
	}
	if daemon_version >= 3 {
		flags.push(format!("start count {}", info.start_count));
	}
	if let Some(ms) = info.ready_ms {
		flags.push(format!("ready after {}", format_secs(ms)));
	}
	if let Some(exit) = exit_text(info) {
		flags.push(format!("last exit: {}", exit));
	}
	let (first, last) = (p.lines.front().map(|l| l.idx).unwrap_or(0), p.lines.back().map(|l| l.idx).unwrap_or(0));
	flags.push(if first == 0 { "no lines".to_string() } else { format!("lines {}..{}", first, last) });
	format!(
		"{:<20} {:<9} {}  {}",
		info.name,
		state_name(info.state),
		flags.join(", "),
		if info.label == info.name { String::new() } else { info.label.clone() }
	)
}

// *** dashboard tools ***

fn tool_list_dashboards(shared: &Shared) -> Result<String> {
	Ok(shared.with(|s| {
		if s.dashes.is_empty() {
			return "No dashboard is running. start_dashboard starts one from its ini file.".to_string();
		}
		let mut dashes: Vec<&Dash> = s.dashes.values().collect();
		dashes.sort_by_key(|d| (d.record.started, d.record.id.clone()));
		let mut out = format!("{} dashboard(s):\n", dashes.len());
		for dash in dashes {
			if !dash.synced {
				out.push_str(&format!(
					"- {}: connected, but it has not sent its state in the {} s since; dashboard pid {}\n",
					dash_summary(dash),
					dash.attached_at.elapsed().as_secs(),
					dash.record.pid
				));
				continue;
			}
			let running = dash.procs.iter().filter(|p| p.info.started).count();
			let ready = dash.procs.iter().filter(|p| p.info.state == ProcState::Ready).count();
			out.push_str(&format!(
				"- {}: {} processes, {} started, {} ready; dashboard pid {}, since {}\n",
				dash_summary(dash),
				dash.procs.len(),
				running,
				ready,
				dash.record.pid,
				format_date_time((dash.record.started as i64) * 1000)
			));
		}
		out
	}))
}

async fn tool_start_dashboard(shared: &Shared, args: &Value) -> Result<String> {
	let ini = arg_str(args, "ini_path").ok_or_else(|| anyhow!("ini_path is required"))?;
	let session = Session::new(Path::new(&ini))?;
	let started = crate::ensure_daemon(&session).await?;
	let wait = wait_seconds(args, 15.0);
	let id = session.id.clone();
	let found = shared.wait_until(Instant::now() + wait, |s| s.dashes.get(&id).is_some_and(|d| d.synced)).await;
	let summary = shared.with(|s| s.dashes.get(&id).map(dash_summary));
	match (found, summary) {
		(true, Some(summary)) => Ok(format!("{} the dashboard {}.", if started { "Started" } else { "Already running:" }, summary)),
		_ => Ok(format!(
			"{} the dashboard for '{}', but it did not answer within {} s; see '{}'.",
			if started { "Started" } else { "Found" },
			session.ini_path.display(),
			wait.as_secs_f64(),
			session.log_path().display()
		)),
	}
}

async fn tool_shutdown_dashboard(shared: &Shared, args: &Value) -> Result<String> {
	let (id, summary, endpoint) = shared.with(|s| {
		let id = find_dash_any(s, arg_str(args, "dashboard").as_deref())?;
		let dash = &s.dashes[&id];
		let _ = dash.tx.send(ClientMsg::Shutdown);
		Ok::<_, anyhow::Error>((id, dash_summary(dash), dash.record.endpoint.clone()))
	})?;
	let wait = wait_seconds(args, 60.0);
	if wait.is_zero() {
		return Ok(format!("Asked {} to stop its processes and exit.", summary));
	}
	let gone = shared.wait_until(Instant::now() + wait, |s| !s.dashes.contains_key(&id)).await;
	if gone {
		// the daemon closes its endpoint and leaves the registry right after the Bye
		let closed = Instant::now() + Duration::from_secs(3);
		while Instant::now() < closed && ipc::is_listening(&endpoint).await {
			tokio::time::sleep(Duration::from_millis(50)).await;
		}
		Ok(format!("The dashboard {} stopped its processes and exited.", summary))
	} else {
		// what still runs is known of a dashboard that has sent its state
		let left: Option<Vec<String>> =
			shared.with(|s| s.dashes.get(&id).filter(|d| d.synced).map(|d| d.procs.iter().filter(|p| p.info.started).map(|p| p.info.name.clone()).collect()));
		Ok(match left {
			Some(left) => format!(
				"The dashboard {} is still stopping after {} s; still running: {}. It exits when they are gone.",
				summary,
				wait.as_secs_f64(),
				left.join(", ")
			),
			None => {
				format!("The dashboard {} has not exited after {} s; it has not sent its state, so what still runs is not known.", summary, wait.as_secs_f64())
			}
		})
	}
}

// *** process tools ***

fn tool_list_processes(shared: &Shared, args: &Value) -> Result<String> {
	shared.with(|s| {
		let id = find_dash(s, arg_str(args, "dashboard").as_deref())?;
		let dash = &s.dashes[&id];
		let mut out = format!("{}: {} processes\n", dash_summary(dash), dash.procs.len());
		for p in &dash.procs {
			out.push_str(&process_line(p, dash.daemon_version));
			out.push('\n');
		}
		Ok(out)
	})
}

fn tool_get_process(shared: &Shared, args: &Value) -> Result<String> {
	shared.with(|s| {
		let (dash_id, idx) = find_target(s, args)?;
		let dash = &s.dashes[&dash_id];
		let p = &dash.procs[idx];
		let info = &p.info;
		let mut out = format!("Process '{}' of {}\n", info.name, dash_summary(dash));
		out.push_str(&format!("state: {}{}\n", state_name(info.state), if info.started { " (started)" } else { "" }));
		out.push_str(&format!("label: {}\ntitle: {}\n", info.label, info.title));
		out.push_str(&format!("command: {}\n", info.command));
		if !info.working_directory.is_empty() {
			out.push_str(&format!("working directory: {}\n", info.working_directory));
		}
		if let Some(pid) = info.pid {
			out.push_str(&format!("pid: {}\n", pid));
		}
		if let Some(started_at) = info.started_at {
			out.push_str(&format!(
				"started: {} (up {})\n",
				format_date_time(started_at),
				format_duration(Duration::from_millis((now_ms() - started_at).max(0) as u64))
			));
		}
		if dash.daemon_version >= 3 {
			out.push_str(&format!(
				"start count: {}{}\n",
				info.start_count,
				if info.has_launch_ctrl {
					" (since the dashboard opened; it writes the count to the start_count file of the launch control)"
				} else {
					" (since the dashboard opened)"
				}
			));
			match info.ready_ms {
				Some(ms) => out.push_str(&format!("ready after: {}{}\n", format_secs(ms), if info.pid.is_some() { "" } else { " (the last run)" })),
				None if info.pid.is_some() => out.push_str("ready after: not ready yet\n"),
				None => {}
			}
		}
		if let Some(exit) = exit_text(info) {
			out.push_str(&format!("last exit: {}\n", exit));
		}
		out.push_str(&format!("priority: {}\n", info.priority.map(|p| p.to_string()).unwrap_or_else(|| "none (starts one at a time)".to_string())));
		out.push_str(&format!("auto start: {}\n", info.auto_start));
		if info.control {
			out.push_str(&format!(
				"console channel: {}\n",
				if info.control_connected {
					format!(
						"connected (protocol {}, exec {})",
						info.channel_version,
						if info.channel_version >= 2 { "supported" } else { "not supported, run_command needs a service built with it" }
					)
				} else {
					"not connected".to_string()
				}
			));
		}
		if info.has_launch_ctrl {
			out.push_str(&format!("launch control: yes, value {}\n", info.ctrl_value.clone().unwrap_or_default()));
		}
		if info.relaunch_pending {
			out.push_str(&format!("restart pending after {} quick exits\n", info.crash_chain));
		}
		let (first, last) = (p.lines.front().map(|l| l.idx).unwrap_or(0), p.lines.back().map(|l| l.idx).unwrap_or(0));
		out.push_str(&format!("log: {} lines held, idx {}..{}, next {}\n", p.lines.len(), first, last, info.next_idx));
		out.push_str(&format!("commands known: {}\n", p.commands.len()));
		let mut values = Vec::new();
		let mut buttons = Vec::new();
		for label in &info.labels {
			if label.is_empty() {
				continue;
			}
			if let Some(rest) = label.strip_prefix('@') {
				match rest.split_once('|') {
					Some((name, command)) => buttons.push(format!("{} ({})", name, command)),
					None => buttons.push(rest.to_string()),
				}
			} else {
				values.push(label.clone());
			}
		}
		if !values.is_empty() {
			out.push_str(&format!("status labels: {}\n", values.join(" | ")));
		}
		if !buttons.is_empty() {
			out.push_str(&format!("buttons (name (command), run with run_command): {}\n", buttons.join(", ")));
		}
		Ok(out)
	})
}

#[derive(Clone, Copy, PartialEq, Eq)]
enum Order {
	Start,
	Stop,
	Kill,
}

fn is_ready(info: &ProcessInfo) -> bool {
	info.state == ProcState::Ready
}

fn is_running(info: &ProcessInfo) -> bool {
	info.started && matches!(info.state, ProcState::Launched | ProcState::Captured | ProcState::Ready)
}

fn is_stopped(info: &ProcessInfo) -> bool {
	!info.started && matches!(info.state, ProcState::Stopped | ProcState::New)
}

async fn tool_process_order(shared: &Shared, args: &Value, order: Order) -> Result<String> {
	let (dash_id, idx, name, before) = shared.with(|s| {
		let (dash_id, idx) = find_target(s, args)?;
		let dash = &s.dashes[&dash_id];
		let name = dash.procs[idx].info.name.clone();
		let before = dash.procs[idx].info.clone();
		let msg = match order {
			Order::Start => ClientMsg::Start { name: name.clone() },
			Order::Stop => ClientMsg::Stop { name: name.clone() },
			Order::Kill => ClientMsg::Kill { name: name.clone() },
		};
		let _ = dash.tx.send(msg);
		Ok::<_, anyhow::Error>((dash_id, idx, name, before))
	})?;
	let verb = match order {
		Order::Start => "start",
		Order::Stop => "stop",
		Order::Kill => "kill",
	};
	let was = if before.relaunch_pending {
		", restart pending"
	} else if before.started {
		", started"
	} else {
		""
	};
	let mut out = format!("Sent {} to '{}' (was {}{}).", verb, name, state_name(before.state), was);
	let wait = wait_seconds(args, 0.0);
	if wait.is_zero() {
		return Ok(out);
	}
	let deadline = Instant::now() + wait;
	let reached = shared
		.wait_until(deadline, |s| {
			let Some(dash) = s.dashes.get(&dash_id) else {
				return true;
			};
			let Some(p) = dash.procs.get(idx) else {
				return true;
			};
			match order {
				Order::Start => is_ready(&p.info),
				Order::Stop | Order::Kill => is_stopped(&p.info),
			}
		})
		.await;
	let after = shared.with(|s| s.dashes.get(&dash_id).and_then(|d| d.procs.get(idx)).map(|p| p.info.clone()));
	match after {
		None => out.push_str(" The dashboard went away meanwhile."),
		Some(after) => {
			if reached {
				out.push_str(&format!(" Now {}{}.", state_name(after.state), after.pid.map(|p| format!(", pid {}", p)).unwrap_or_default()));
			} else {
				out.push_str(&format!(" Still {} after {} s.", state_name(after.state), wait.as_secs_f64()));
			}
		}
	}
	Ok(out)
}

async fn tool_restart_process(shared: &Shared, args: &Value) -> Result<String> {
	let wait = wait_seconds(args, 60.0);
	let (dash_id, idx, name, running) = shared.with(|s| {
		let (dash_id, idx) = find_target(s, args)?;
		let p = &s.dashes[&dash_id].procs[idx];
		Ok::<_, anyhow::Error>((dash_id, idx, p.info.name.clone(), p.info.started))
	})?;
	let mut out = String::new();
	if running {
		shared.with(|s| {
			if let Some(dash) = s.dashes.get(&dash_id) {
				let _ = dash.tx.send(ClientMsg::Stop { name: name.clone() });
			}
		});
		let stopped =
			shared.wait_until(Instant::now() + wait, |s| s.dashes.get(&dash_id).and_then(|d| d.procs.get(idx)).is_none_or(|p| is_stopped(&p.info))).await;
		if !stopped {
			let state = shared.with(|s| s.dashes.get(&dash_id).and_then(|d| d.procs.get(idx)).map(|p| state_name(p.info.state)).unwrap_or("gone"));
			bail!("'{}' did not stop within {} s (still {}); kill_process ends it", name, wait.as_secs_f64(), state);
		}
		out.push_str(&format!("'{}' stopped. ", name));
	} else {
		out.push_str(&format!("'{}' was not running. ", name));
	}
	shared.with(|s| {
		if let Some(dash) = s.dashes.get(&dash_id) {
			let _ = dash.tx.send(ClientMsg::Start { name: name.clone() });
		}
	});
	let ready = shared.wait_until(Instant::now() + wait, |s| s.dashes.get(&dash_id).and_then(|d| d.procs.get(idx)).is_none_or(|p| is_ready(&p.info))).await;
	let after = shared.with(|s| s.dashes.get(&dash_id).and_then(|d| d.procs.get(idx)).map(|p| p.info.clone()));
	match after {
		None => out.push_str("The dashboard went away meanwhile."),
		Some(after) if ready => out.push_str(&format!("Started again, now ready{}.", after.pid.map(|p| format!(" with pid {}", p)).unwrap_or_default())),
		Some(after) => out.push_str(&format!("Started again, still {} after {} s.", state_name(after.state), wait.as_secs_f64())),
	}
	Ok(out)
}

async fn tool_all(shared: &Shared, args: &Value, start: bool) -> Result<String> {
	let (dash_id, summary) = shared.with(|s| {
		let id = find_dash(s, arg_str(args, "dashboard").as_deref())?;
		let dash = &s.dashes[&id];
		let _ = dash.tx.send(if start { ClientMsg::StartAll } else { ClientMsg::StopAll });
		Ok::<_, anyhow::Error>((id, dash_summary(dash)))
	})?;
	let mut out = format!("Sent {} to {}.", if start { "start all" } else { "stop all" }, summary);
	let wait = wait_seconds(args, 0.0);
	if wait.is_zero() {
		return Ok(out);
	}
	let done = shared
		.wait_until(Instant::now() + wait, |s| {
			let Some(dash) = s.dashes.get(&dash_id) else {
				return true;
			};
			dash.procs.iter().all(|p| if start { is_ready(&p.info) } else { is_stopped(&p.info) })
		})
		.await;
	let states: Vec<String> = shared.with(|s| {
		s.dashes.get(&dash_id).map(|d| d.procs.iter().map(|p| format!("{} {}", p.info.name, state_name(p.info.state))).collect()).unwrap_or_default()
	});
	if done {
		out.push_str(&format!(" Every process is {}.", if start { "ready" } else { "stopped" }));
	} else {
		out.push_str(&format!(" After {} s: {}.", wait.as_secs_f64(), states.join(", ")));
	}
	Ok(out)
}

/// What a process is doing now, for the report of a run that ended or began during a wait.
fn now_text(info: &ProcessInfo, daemon_version: u32) -> String {
	let mut out = format!("now {}", state_name(info.state));
	if let Some(pid) = info.pid {
		out.push_str(&format!(", pid {}", pid));
	}
	if info.relaunch_pending {
		out.push_str(", restart pending");
	}
	if daemon_version >= 3 {
		out.push_str(&format!(", start count {}", info.start_count));
	}
	out
}

async fn tool_wait_for_state(shared: &Shared, args: &Value) -> Result<String> {
	let wanted = arg_str(args, "state").ok_or_else(|| anyhow!("the state parameter is required"))?.to_lowercase();
	let pred: fn(&ProcessInfo) -> bool = match wanted.as_str() {
		"ready" => is_ready,
		"running" => is_running,
		"stopped" => is_stopped,
		other => bail!("unknown state '{}'; ready, running or stopped", other),
	};
	let stop_on_exit = arg_bool(args, "stop_on_exit").unwrap_or(true);
	let mode = prefix_mode(args);
	// waiting for the stop, a run that begins is the surprise; otherwise one that ends
	let watch_began = wanted == "stopped";
	let (dash_id, summary, indices, names, whole, seq0) = shared.with(|s| {
		let dash_id = find_dash(s, arg_str(args, "dashboard").as_deref())?;
		let dash = &s.dashes[&dash_id];
		let (indices, whole): (Vec<usize>, bool) = match arg_str(args, "process") {
			Some(name) => (vec![find_proc(dash, &name)?], false),
			None => ((0..dash.procs.len()).collect(), true),
		};
		let names: Vec<String> = indices.iter().map(|&i| dash.procs[i].info.name.clone()).collect();
		Ok::<_, anyhow::Error>((dash_id.clone(), dash_summary(dash), indices, names, whole, dash.event_seq))
	})?;
	let wait = wait_seconds(args, 60.0);
	let begun = Instant::now();
	let relevant = |e: &ProcEvent| e.seq > seq0 && e.began == watch_began && names.contains(&e.name);
	shared
		.wait_until(begun + wait, |s| {
			let Some(dash) = s.dashes.get(&dash_id) else {
				return true;
			};
			if stop_on_exit && dash.events.iter().any(relevant) {
				return true;
			}
			indices.iter().all(|&i| dash.procs.get(i).is_none_or(|p| pred(&p.info)))
		})
		.await;
	let waited = begun.elapsed().as_secs_f64();
	if stop_on_exit && shared.with(|s| s.dashes.get(&dash_id).is_some_and(|d| d.events.iter().any(relevant))) {
		tokio::time::sleep(EVENT_SETTLE).await;
	}
	shared.with(|s| {
		let Some(dash) = s.dashes.get(&dash_id) else {
			return Ok(format!("The dashboard {} went away while waiting.", summary));
		};
		let procs: Vec<&ProcMirror> = indices.iter().filter_map(|&i| dash.procs.get(i)).collect();
		let reached = procs.iter().all(|p| pred(&p.info));
		let mut out = String::new();
		// the runs that ended (or began) meanwhile come first: they are why the wait is over early
		let mut shown: Vec<&str> = Vec::new();
		for e in dash.events.iter().filter(|e| relevant(e)) {
			let now =
				dash.procs.iter().find(|p| p.info.name == e.name).map(|p| now_text(&p.info, dash.daemon_version)).unwrap_or_else(|| "now gone".to_string());
			if e.began {
				out.push_str(&format!("'{}' was started while waiting; {}.\n", e.name, now));
			} else {
				out.push_str(&format!(
					"'{}' exited while waiting{}{}; {}.\n",
					e.name,
					if e.how.is_empty() { String::new() } else { format!(" ({})", e.how) },
					e.up_ms.map(|ms| format!(", after {} up", format_duration(Duration::from_millis(ms as u64)))).unwrap_or_default(),
					now
				));
			}
			if !shown.contains(&e.name.as_str()) {
				shown.push(&e.name);
			}
		}
		// a process that is down comes with the way its last run ended: an exit from before the
		// wait is no event of it, and this is where it shows
		let not_there: Vec<String> = procs
			.iter()
			.filter(|p| !pred(&p.info))
			.map(|p| {
				let exit = exit_text(&p.info).filter(|_| !watch_began && p.info.pid.is_none());
				format!("{} {}{}", p.info.name, state_name(p.info.state), exit.map(|e| format!(" (last exit: {})", e)).unwrap_or_default())
			})
			.collect();
		if whole {
			let count = procs.len();
			if reached {
				out.push_str(&format!("Every process of {} is {} ({} processes, waited {:.1} s).\n", summary, wanted, count, waited));
			} else {
				out.push_str(&format!(
					"Not every process of {} is {} after {:.1} s: {} of {} are; not {}: {}.\n",
					summary,
					wanted,
					waited,
					count - not_there.len(),
					count,
					wanted,
					not_there.join(", ")
				));
			}
		} else {
			let state = procs.first().map(|p| state_name(p.info.state)).unwrap_or("gone");
			// a stop that was waited for comes with the way the run ended
			let exit = procs.first().and_then(|p| exit_text(&p.info)).filter(|_| watch_began);
			if reached {
				out.push_str(&format!("'{}' is {} (state {}{}).\n", names[0], wanted, state, exit.map(|e| format!(", last exit: {}", e)).unwrap_or_default()));
			} else {
				out.push_str(&format!("'{}' is not {} after {:.1} s (state {}).\n", names[0], wanted, waited, state));
			}
		}
		for name in shown.iter().take(3) {
			if let Some(p) = dash.procs.iter().find(|p| p.info.name == *name) {
				let skip = p.lines.len().saturating_sub(EVENT_LINES);
				out.push_str(&format!("Last lines of '{}' (idx time type message):\n", name));
				for line in p.lines.iter().skip(skip) {
					out.push_str(&format_line(line, None, mode));
					out.push('\n');
				}
			}
		}
		if shown.len() > 3 {
			out.push_str(&format!("The lines of {} are in get_log.\n", shown[3..].join(", ")));
		}
		Ok(out)
	})
}

// *** command tools ***

fn tool_list_commands(shared: &Shared, args: &Value) -> Result<String> {
	shared.with(|s| {
		let (dash_id, idx) = find_target(s, args)?;
		let p = &s.dashes[&dash_id].procs[idx];
		if !p.info.control {
			bail!("'{}' is not a NeL service with a console channel", p.info.name);
		}
		if p.commands.is_empty() {
			return Ok(format!("No command table from '{}' yet{}.", p.info.name, if p.info.control_connected { "" } else { " (console not connected)" }));
		}
		let prefix = arg_str(args, "prefix");
		let contains = arg_str(args, "contains").map(|c| c.to_lowercase());
		let limit = arg_u64(args, "limit").unwrap_or(300) as usize;
		let mut shown = 0;
		let mut total = 0;
		let mut out = String::new();
		for command in p.commands.values() {
			if prefix.as_ref().is_some_and(|pre| !command.name.starts_with(pre.as_str())) {
				continue;
			}
			if contains.as_ref().is_some_and(|c| !command.name.to_lowercase().contains(c.as_str()) && !command.help.to_lowercase().contains(c.as_str())) {
				continue;
			}
			total += 1;
			if shown >= limit {
				continue;
			}
			shown += 1;
			let kind = match command.kind {
				CommandKind::Command => "",
				CommandKind::Handler => " [handler instance]",
				CommandKind::Member => " [member]",
			};
			let mut line = command.name.clone();
			if !command.args.is_empty() {
				line.push(' ');
				line.push_str(&command.args);
			}
			line.push_str(kind);
			if !command.help.is_empty() {
				line.push_str(" -- ");
				line.push_str(&command.help);
			}
			out.push_str(&line);
			out.push('\n');
		}
		let header =
			format!("{} of {} commands of '{}'{}:\n", shown, total, p.info.name, if total > shown { " (narrow with prefix, contains or limit)" } else { "" });
		Ok(header + &out)
	})
}

/// Lines of a process from an index on, formatted.
fn lines_from(p: &ProcMirror, from_idx: u64, mode: PrefixMode) -> Vec<String> {
	p.lines.iter().filter(|l| l.idx >= from_idx).map(|l| format_line(l, None, mode)).collect()
}

async fn tool_run_command(shared: &Shared, args: &Value) -> Result<String> {
	let command = arg_str(args, "command").ok_or_else(|| anyhow!("the command parameter is required"))?;
	let command_args = arg_strs(args, "args");
	let mode = prefix_mode(args);
	let (dash_id, idx, name, exec_id, start_idx) = shared.with(|s| {
		let (dash_id, idx) = find_target(s, args)?;
		s.next_exec_id += 1;
		let exec_id = format!("mcp-{}-{}", std::process::id(), s.next_exec_id);
		let dash = &s.dashes[&dash_id];
		let p = &dash.procs[idx];
		if !p.info.control {
			bail!("'{}' has no console channel; send_input writes to its stdin", p.info.name);
		}
		if !p.info.control_connected {
			bail!("the console of '{}' is not connected ({}); send_input reaches its stdin", p.info.name, state_name(p.info.state));
		}
		let start_idx = p.info.next_idx.max(p.lines.back().map(|l| l.idx + 1).unwrap_or(1));
		let _ = dash.tx.send(ClientMsg::Exec { name: p.info.name.clone(), id: exec_id.clone(), command: command.clone(), args: command_args.clone() });
		Ok::<_, anyhow::Error>((dash_id, idx, p.info.name.clone(), exec_id, start_idx))
	})?;
	let wait = wait_seconds(args, 10.0);
	let done = shared.wait_until(Instant::now() + wait, |s| s.dashes.get(&dash_id).is_none_or(|d| d.exec_results.contains_key(&exec_id))).await;
	let (result, lines) = shared.with(|s| {
		let Some(dash) = s.dashes.get_mut(&dash_id) else {
			return (None, Vec::new());
		};
		let result = dash.exec_results.remove(&exec_id);
		let lines = dash.procs.get(idx).map(|p| lines_from(p, start_idx, mode)).unwrap_or_default();
		(result, lines)
	});
	let shown: Vec<String> = if command_args.is_empty() {
		vec![command.clone()]
	} else {
		std::iter::once(command.clone()).chain(command_args.iter().map(|a| format!("{:?}", a))).collect()
	};
	let header = match result {
		Some(true) => format!("Command {} on '{}': ok", shown.join(" "), name),
		Some(false) => format!("Command {} on '{}': FAILED (unknown command, bad usage, or no console channel)", shown.join(" "), name),
		None if done => format!("Command {} on '{}': the dashboard went away", shown.join(" "), name),
		None => format!(
			"Command {} on '{}': no completion marker within {} s; the lines so far follow (a service built before the exec message never answers one, its output still shows)",
			shown.join(" "),
			name,
			wait.as_secs_f64()
		),
	};
	Ok(format!("{}\n{}", header, lines.join("\n")))
}

async fn tool_send_input(shared: &Shared, args: &Value) -> Result<String> {
	let text = arg_str(args, "text").ok_or_else(|| anyhow!("the text parameter is required"))?;
	let mode = prefix_mode(args);
	let (dash_id, idx, name, start_idx) = shared.with(|s| {
		let (dash_id, idx) = find_target(s, args)?;
		let dash = &s.dashes[&dash_id];
		let p = &dash.procs[idx];
		if !p.info.started {
			bail!("'{}' is not running", p.info.name);
		}
		let start_idx = p.info.next_idx.max(p.lines.back().map(|l| l.idx + 1).unwrap_or(1));
		let _ = dash.tx.send(ClientMsg::Input { name: p.info.name.clone(), text: text.clone() });
		Ok::<_, anyhow::Error>((dash_id, idx, p.info.name.clone(), start_idx))
	})?;
	let wait = wait_seconds(args, 3.0);
	let deadline = Instant::now() + wait;
	// return once the process answered and went quiet, or when the time is up
	let mut seen = start_idx;
	loop {
		let now = Instant::now();
		if now >= deadline {
			break;
		}
		let until = if seen > start_idx { (now + INPUT_IDLE).min(deadline) } else { deadline };
		let more = shared
			.wait_until(until, |s| s.dashes.get(&dash_id).and_then(|d| d.procs.get(idx)).is_none_or(|p| p.lines.back().is_some_and(|l| l.idx >= seen)))
			.await;
		if !more {
			if seen > start_idx {
				break;
			}
			continue;
		}
		let last = shared.with(|s| s.dashes.get(&dash_id).and_then(|d| d.procs.get(idx)).and_then(|p| p.lines.back().map(|l| l.idx)));
		match last {
			Some(last) if last >= seen => seen = last + 1,
			Some(_) => {}
			None => break,
		}
	}
	let lines = shared.with(|s| s.dashes.get(&dash_id).and_then(|d| d.procs.get(idx)).map(|p| lines_from(p, start_idx, mode)).unwrap_or_default());
	Ok(format!("Wrote {:?} to the stdin of '{}'; {} line(s) followed:\n{}", text, name, lines.len(), lines.join("\n")))
}

// *** log tools ***

struct LogFilter {
	from_idx: Option<u64>,
	to_idx: Option<u64>,
	from_ts: Option<i64>,
	to_ts: Option<i64>,
	levels: Vec<String>,
	pattern: Option<Regex>,
}

/// The log types of the levels argument: a list, or one string with commas or spaces; NeL
/// types in upper case, the others in lower case. An unknown one is an error, since it would
/// match nothing and read as a quiet log.
fn parse_levels(args: &Value) -> Result<Vec<String>> {
	let mut levels = Vec::new();
	for entry in arg_strs(args, "levels") {
		for word in entry.split(|c: char| c == ',' || c.is_whitespace()).filter(|w| !w.is_empty()) {
			match LOG_TYPES.iter().find(|t| t.eq_ignore_ascii_case(word)) {
				Some(known) => levels.push(known.to_string()),
				None => bail!("unknown log type '{}' in levels; one of {}", word, LOG_TYPES.join(", ")),
			}
		}
	}
	Ok(levels)
}

/// The index of the note the dashboard wrote at the last start of the process; the lines of
/// the current run follow it. When that note is no longer held, every held line is of this run.
fn run_start_idx(p: &ProcMirror) -> u64 {
	p.lines.iter().rev().find(|l| l.raw && l.text == "> Process Started").map(|l| l.idx).unwrap_or(0)
}

impl LogFilter {
	fn accepts(&self, line: &Line) -> bool {
		if self.from_idx.is_some_and(|v| line.idx < v) || self.to_idx.is_some_and(|v| line.idx > v) {
			return false;
		}
		if self.from_ts.is_some_and(|v| line.ts < v) || self.to_ts.is_some_and(|v| line.ts > v) {
			return false;
		}
		if !self.levels.is_empty() && !self.levels.iter().any(|l| l == line_level(line)) {
			return false;
		}
		if self.pattern.as_ref().is_some_and(|re| !re.is_match(&line.text)) {
			return false;
		}
		true
	}
}

struct Picked {
	ts: i64,
	idx: u64,
	proc_idx: usize,
	text: String,
}

async fn tool_get_log(shared: &Shared, args: &Value) -> Result<String> {
	let mode = prefix_mode(args);
	let filter = LogFilter {
		from_idx: arg_u64(args, "from_idx"),
		to_idx: arg_u64(args, "to_idx"),
		from_ts: arg_str(args, "from_time").map(|t| parse_time(&t)).transpose()?,
		to_ts: arg_str(args, "to_time").map(|t| parse_time(&t)).transpose()?,
		levels: parse_levels(args)?,
		pattern: match arg_str(args, "match") {
			Some(p) => Some(Regex::new(&format!("(?i){}", p)).map_err(|e| anyhow!("bad match expression: {}", e))?),
			None => None,
		},
	};
	let since_start = arg_bool(args, "since_start").unwrap_or(false);
	let has_range = since_start || filter.from_idx.is_some() || filter.to_idx.is_some() || filter.from_ts.is_some() || filter.to_ts.is_some();
	let max_lines = (arg_u64(args, "max_lines").unwrap_or(200) as usize).clamp(1, MAX_LOG_LINES);
	let last = arg_u64(args, "last").map(|v| v as usize).or(if has_range { None } else { Some(DEFAULT_LOG_LINES) });
	let follow = Duration::from_secs_f64(arg_f64(args, "follow_seconds").unwrap_or(0.0).clamp(0.0, MAX_WAIT));

	// the selection, under the lock
	let (dash_id, proc_indices, mut picked, mut header, mut cursors) = shared.with(|s| {
		let dash_id = find_dash(s, arg_str(args, "dashboard").as_deref())?;
		let dash = &s.dashes[&dash_id];
		let proc_indices: Vec<usize> = match arg_str(args, "process") {
			Some(name) => vec![find_proc(dash, &name)?],
			None => (0..dash.procs.len()).collect(),
		};
		let several = proc_indices.len() > 1;
		let mut picked: Vec<Picked> = Vec::new();
		let mut header = String::new();
		let mut cursors: Vec<u64> = Vec::new();
		let mut without: Vec<&str> = Vec::new();
		for &pi in &proc_indices {
			let p = &dash.procs[pi];
			let run_start = if since_start { run_start_idx(p) } else { 0 };
			let matching: Vec<&Line> = p.lines.iter().filter(|l| l.idx >= run_start && filter.accepts(l)).collect();
			let total = matching.len();
			let take: Vec<&Line> = match last {
				Some(n) => matching.into_iter().rev().take(n).collect::<Vec<_>>().into_iter().rev().collect(),
				None => matching,
			};
			let (first_held, last_held) = (p.lines.front().map(|l| l.idx).unwrap_or(0), p.lines.back().map(|l| l.idx).unwrap_or(0));
			if several && total == 0 {
				// the processes a selection over the dashboard finds nothing in share one line
				without.push(p.info.name.as_str());
			} else {
				header.push_str(&format!(
					"# {}: {} of {} matching lines, held {}..{}, next idx {}\n",
					if several { format!("[{}]", p.info.name) } else { p.info.name.clone() },
					take.len(),
					total,
					first_held,
					last_held,
					p.info.next_idx
				));
			}
			for l in take {
				picked.push(Picked { ts: l.ts, idx: l.idx, proc_idx: pi, text: format_line(l, if several { Some(&p.info.name) } else { None }, mode) });
			}
			cursors.push(last_held.max(p.info.next_idx.saturating_sub(1)));
		}
		if !without.is_empty() {
			header.push_str(&format!("# no matching lines: {}\n", without.join(", ")));
		}
		Ok::<_, anyhow::Error>((dash_id, proc_indices, picked, header, cursors))
	})?;
	let several = proc_indices.len() > 1;
	if several {
		picked.sort_by_key(|p| (p.ts, p.proc_idx, p.idx));
	}
	// to_time in the past makes following pointless; a to_idx too
	let can_follow = !follow.is_zero() && filter.to_idx.is_none() && filter.to_ts.is_none_or(|t| t > now_ms());
	let mut followed = 0usize;
	if can_follow && picked.len() < max_lines {
		let deadline = Instant::now() + follow;
		loop {
			if picked.len() >= max_lines || Instant::now() >= deadline {
				break;
			}
			let cursors_now = cursors.clone();
			let more = shared
				.wait_until(deadline, |s| {
					let Some(dash) = s.dashes.get(&dash_id) else {
						return true;
					};
					proc_indices.iter().enumerate().any(|(i, &pi)| dash.procs.get(pi).and_then(|p| p.lines.back()).is_some_and(|l| l.idx > cursors_now[i]))
				})
				.await;
			if !more {
				break;
			}
			let gone = shared.with(|s| {
				let Some(dash) = s.dashes.get(&dash_id) else {
					return true;
				};
				for (i, &pi) in proc_indices.iter().enumerate() {
					let Some(p) = dash.procs.get(pi) else {
						continue;
					};
					let cursor = cursors[i];
					for l in p.lines.iter().filter(|l| l.idx > cursor) {
						cursors[i] = l.idx;
						if filter.accepts(l) && picked.len() < max_lines {
							picked.push(Picked {
								ts: l.ts,
								idx: l.idx,
								proc_idx: pi,
								text: format_line(l, if several { Some(&p.info.name) } else { None }, mode),
							});
							followed += 1;
						}
					}
				}
				false
			});
			if gone {
				break;
			}
		}
	}
	if picked.len() > max_lines {
		// the selection itself was larger than the cap: keep the newest
		let excess = picked.len() - max_lines;
		picked.drain(..excess);
		header.push_str(&format!("# {} older lines of the selection left out by max_lines\n", excess));
	}
	if can_follow {
		header.push_str(&format!("# followed for {} s: {} new line(s)\n", follow.as_secs_f64(), followed));
	}
	header.push_str(&format!("# now {}; columns: idx time type message\n", format_date_time(now_ms())));
	let body: Vec<String> = picked.into_iter().map(|p| p.text).collect();
	Ok(header + &body.join("\n"))
}

fn tool_clear_log(shared: &Shared, args: &Value) -> Result<String> {
	shared.with(|s| {
		let (dash_id, idx) = find_target(s, args)?;
		let dash = &s.dashes[&dash_id];
		let name = dash.procs[idx].info.name.clone();
		let _ = dash.tx.send(ClientMsg::ClearConsole { name: name.clone() });
		Ok(format!("Cleared the held lines of '{}'.", name))
	})
}

#[cfg(test)]
mod tests {
	use super::*;

	#[test]
	fn prefix_parsing() {
		let p = parse_prefix("INF 0001           service.cpp  879                 main: SERVICE: Starting").expect("parsed");
		assert_eq!(p.level, Some("INF"));
		assert_eq!(p.file, Some("service.cpp"));
		assert_eq!(p.line, Some("879"));
		assert_eq!(p.func, Some("main"));
		assert_eq!(p.msg, "SERVICE: Starting");
		let p = parse_prefix("0001: Executing command : 'quit'").expect("parsed");
		assert_eq!(p.level, None);
		assert_eq!(p.msg, "Executing command : 'quit'");
		assert!(parse_prefix("plain output").is_none());
	}

	#[test]
	fn levels_of_lines() {
		let line = Line { idx: 1, ts: 0, raw: true, text: "> Process Started".to_string() };
		assert_eq!(line_level(&line), "dash");
		let line = Line { idx: 2, ts: 0, raw: true, text: "hello".to_string() };
		assert_eq!(line_level(&line), "out");
		let line = Line { idx: 3, ts: 0, raw: false, text: "WRN 0001 a.cpp 1 f: careful".to_string() };
		assert_eq!(line_level(&line), "WRN");
	}

	#[test]
	fn relative_and_clock_times() {
		let before = now_ms();
		let t = parse_time("-90s").expect("relative");
		assert!(t <= before - 89_000 && t >= before - 92_000);
		assert!(parse_time("12:34").is_ok());
		assert!(parse_time("12:34:56.789").is_ok());
		assert!(parse_time("2026-10-30 12:00:00").is_ok());
		assert!(parse_time("nonsense").is_err());
		assert_eq!(parse_time("1700000000").expect("seconds"), 1_700_000_000_000);
		assert_eq!(parse_time("1700000000123").expect("millis"), 1_700_000_000_123);
	}

	fn test_dash() -> Dash {
		let record = Record {
			id: "0".to_string(),
			ini_path: "x.ini".to_string(),
			title: "Test".to_string(),
			endpoint: String::new(),
			pid: 1,
			started: 0,
			tool: String::new(),
		};
		let (tx, _rx) = mpsc::unbounded_channel::<ClientMsg>();
		Dash::new(record, tx)
	}

	fn test_info(name: &str, pid: Option<u32>, started_at: Option<i64>) -> ProcessInfo {
		let json = json!({
			"name": name, "label": name, "title": name, "state": if pid.is_some() { "Ready" } else { "Stopped" }, "started": pid.is_some(),
			"control": false, "control_connected": false, "labels": [], "pid": pid, "started_at": started_at,
		});
		serde_json::from_value(json).expect("a process info with only the fields of version 1")
	}

	fn snapshot(name: &str) -> DaemonMsg {
		DaemonMsg::Snapshot { name: name.to_string(), lines: Vec::new() }
	}

	#[test]
	fn synced_by_the_marker() {
		let mut dash = test_dash();
		dash.apply(DaemonMsg::Hello { version: 3, title: "Test".to_string() });
		dash.apply(DaemonMsg::Processes { processes: vec![test_info("a", Some(1), Some(10)), test_info("b", None, None)] });
		dash.apply(snapshot("a"));
		dash.apply(snapshot("b"));
		// the command table of the last process may still follow
		assert!(!dash.synced);
		dash.apply(DaemonMsg::Synced);
		assert!(dash.synced);
	}

	#[test]
	fn synced_without_the_marker() {
		// a dashboard of protocol version 2 ends its state with the Snapshot of its last process
		// and, when that process has one, its command table
		let mut dash = test_dash();
		dash.apply(DaemonMsg::Hello { version: 2, title: "Test".to_string() });
		// a line reset that was broadcast before the state does not count
		dash.apply(snapshot("b"));
		assert!(!dash.synced && !dash.settling);
		dash.apply(DaemonMsg::Processes { processes: vec![test_info("a", Some(1), Some(10)), test_info("b", None, None)] });
		dash.apply(snapshot("a"));
		assert!(!dash.synced && !dash.settling);
		dash.apply(snapshot("b"));
		// the command table of b may follow: not yet
		assert!(!dash.synced && dash.settling);
		dash.apply(DaemonMsg::Commands { name: "b".to_string(), commands: Vec::new() });
		assert!(dash.synced && !dash.settling);
		// one whose last process has no command table: when nothing follows for a moment
		let mut quiet = test_dash();
		quiet.apply(DaemonMsg::Hello { version: 2, title: "Test".to_string() });
		quiet.apply(DaemonMsg::Processes { processes: vec![test_info("a", None, None)] });
		quiet.apply(snapshot("a"));
		assert!(!quiet.synced && quiet.settling);
		quiet.settled();
		assert!(quiet.synced);
		// and one without processes after the list itself
		let mut empty = test_dash();
		empty.apply(DaemonMsg::Hello { version: 2, title: "Test".to_string() });
		empty.apply(DaemonMsg::Processes { processes: Vec::new() });
		assert!(empty.settling);
		empty.apply(DaemonMsg::Notice { text: String::new() });
		assert!(empty.synced);
		// the marker of a newer dashboard is not anticipated
		let mut marked = test_dash();
		marked.apply(DaemonMsg::Hello { version: 3, title: "Test".to_string() });
		marked.apply(DaemonMsg::Processes { processes: Vec::new() });
		assert!(!marked.synced && !marked.settling);
	}

	#[test]
	fn the_synced_marker_is_a_bare_variant() {
		assert!(matches!(proto::parse_msg::<DaemonMsg>("\"Synced\"").expect("parsed"), DaemonMsg::Synced));
		assert_eq!(serde_json::to_string(&DaemonMsg::Synced).expect("written"), "\"Synced\"");
	}

	#[test]
	fn runs_that_end_and_begin_are_events() {
		let mut dash = test_dash();
		dash.apply(DaemonMsg::Processes { processes: vec![test_info("a", Some(1), Some(10)), test_info("b", None, None)] });
		assert!(dash.events.is_empty());
		// a state change of the same run is none
		dash.apply(DaemonMsg::Process { info: test_info("a", Some(1), Some(10)) });
		assert!(dash.events.is_empty());
		// the exit
		let mut gone = test_info("a", None, None);
		gone.exit_code = Some(3);
		dash.apply(DaemonMsg::Process { info: gone });
		assert_eq!(dash.events.len(), 1);
		assert!(!dash.events[0].began);
		assert_eq!(dash.events[0].name, "a");
		assert_eq!(dash.events[0].how, "code 3");
		// the relaunch, and the first start of another
		dash.apply(DaemonMsg::Process { info: test_info("a", Some(2), Some(20)) });
		dash.apply(DaemonMsg::Process { info: test_info("b", Some(3), Some(30)) });
		assert_eq!(dash.events.len(), 3);
		assert!(dash.events[1].began && dash.events[1].name == "a");
		assert!(dash.events[2].began && dash.events[2].name == "b");
		// a run replaced within one message is an end and a beginning
		let mut again = test_info("a", Some(4), Some(40));
		again.last_exit = "crashed, 0xC0000005".to_string();
		dash.apply(DaemonMsg::Process { info: again });
		assert_eq!(dash.events.len(), 5);
		assert!(!dash.events[3].began && dash.events[3].how == "crashed, 0xC0000005");
		assert!(dash.events[4].began);
		assert_eq!(dash.events[4].seq, 5);
		// a pid that goes missing while the start time stays is the same run
		dash.apply(DaemonMsg::Process { info: test_info("a", None, Some(40)) });
		assert_eq!(dash.events.len(), 5);
	}

	#[test]
	fn levels_argument() {
		assert_eq!(parse_levels(&json!({ "levels": ["wrn", "ERR", "Out"] })).expect("a list"), vec!["WRN", "ERR", "out"]);
		assert_eq!(parse_levels(&json!({ "levels": "WRN, err ast" })).expect("a string"), vec!["WRN", "ERR", "AST"]);
		assert!(parse_levels(&json!({})).expect("none").is_empty());
		assert!(parse_levels(&json!({ "levels": ["WARN"] })).is_err());
	}

	#[test]
	fn lines_since_the_last_start() {
		let mut p = ProcMirror::new(test_info("a", Some(1), Some(10)));
		let texts = ["> Process Started", "one", "> Process Exited (code 0)", "> Process Started", "two"];
		for (i, text) in texts.iter().enumerate() {
			p.lines.push_back(Line { idx: i as u64 + 1, ts: 0, raw: true, text: text.to_string() });
		}
		assert_eq!(run_start_idx(&p), 4);
		// the start note rolled out of the held lines: all of them are of this run
		p.lines.drain(..4);
		assert_eq!(run_start_idx(&p), 0);
	}
}
