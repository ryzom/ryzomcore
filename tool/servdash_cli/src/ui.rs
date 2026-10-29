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

//! The terminal view: the layout of the Win32 servdash, a column of process buttons with
//! a state light and a start/stop toggle, and the console of the selected process next to
//! it. For a NeL service the console shows the service's own toolbar (labels and command
//! buttons), its log and a command line, as the Win32 service window does.

use crate::proto::{self, ClientMsg, CommandInfo, CommandKind, DaemonMsg, ProcState, ProcessInfo, StyledLine};
use crossterm::event::{Event, KeyCode, KeyEvent, KeyEventKind, KeyModifiers, MouseButton, MouseEvent, MouseEventKind};
use ratatui::layout::{Constraint, Layout, Position, Rect};
use ratatui::style::{Color, Modifier, Style};
use ratatui::text::{Line, Span};
use ratatui::widgets::{Block, Paragraph, Scrollbar, ScrollbarOrientation, ScrollbarState};
use ratatui::Frame;
use std::collections::BTreeMap;
use std::sync::OnceLock;
use std::time::{Duration, Instant};

use regex::Regex;

const LIST_WIDTH: u16 = 30;
const MAX_CLIENT_LINES: usize = 10000;
const NOTICE_TIME: Duration = Duration::from_secs(6);
const QUIT_ARM_TIME: Duration = Duration::from_secs(4);

pub enum Action {
	Send(ClientMsg),
	Detach,
}

struct ProcView {
	info: ProcessInfo,
	/// Console lines, raw output and the daemon's notes, in arrival order.
	lines: Vec<StyledLine>,
	/// Lines scrolled up from the bottom; zero follows the output.
	scroll: usize,
	input: String,
	history: Vec<String>,
	/// Position in the history while browsing it, with the draft kept aside.
	history_pos: Option<usize>,
	draft: String,
	/// The command table of the service, by name.
	commands: BTreeMap<String, CommandInfo>,
	/// Which of the completions of the typed word is suggested; Shift+Tab moves it.
	completion_index: usize,
}

/// What the input line suggests: the commands whose name starts with the word being typed,
/// or the one command the arguments are being typed for.
struct Suggestion {
	candidates: Vec<CommandInfo>,
	/// The one shown as ghost text and in the hint.
	index: usize,
	/// Still typing the command name: the ghost text completes it.
	naming: bool,
}

impl Suggestion {
	fn current(&self) -> &CommandInfo {
		&self.candidates[self.index]
	}
}

impl ProcView {
	fn new(info: ProcessInfo) -> ProcView {
		ProcView {
			info,
			lines: Vec::new(),
			scroll: 0,
			input: String::new(),
			history: Vec::new(),
			history_pos: None,
			draft: String::new(),
			commands: BTreeMap::new(),
			completion_index: 0,
		}
	}

	/// The completion of the input line against the command table, when there is one.
	fn suggestion(&self) -> Option<Suggestion> {
		if !self.is_console() || self.commands.is_empty() || self.input.is_empty() {
			return None;
		}
		match self.input.find(' ') {
			None => {
				let word = self.input.as_str();
				let lowered = word.to_lowercase();
				// the names starting with the word; those matching its case first, each group in name order
				let mut exact: Vec<CommandInfo> = Vec::new();
				let mut loose: Vec<CommandInfo> = Vec::new();
				for command in self.commands.values() {
					if command.name.starts_with(word) {
						exact.push(command.clone());
					} else if command.name.to_lowercase().starts_with(&lowered) {
						loose.push(command.clone());
					}
				}
				exact.extend(loose);
				if exact.is_empty() {
					return None;
				}
				let index = self.completion_index % exact.len();
				Some(Suggestion { candidates: exact, index, naming: true })
			}
			Some(space) => {
				let word = &self.input[..space];
				let lowered = word.to_lowercase();
				let found = self.commands.get(word).or_else(|| self.commands.values().find(|c| c.name.to_lowercase() == lowered))?;
				Some(Suggestion { candidates: vec![found.clone()], index: 0, naming: false })
			}
		}
	}

	/// Takes the suggested command: its name, then a dot for a handler instance (its commands
	/// follow) or a space for a command.
	fn accept_suggestion(&mut self) -> bool {
		let Some(suggestion) = self.suggestion() else {
			return false;
		};
		if !suggestion.naming {
			return false;
		}
		let command = suggestion.current();
		self.input = command.name.clone();
		self.input.push(if command.kind == CommandKind::Handler { '.' } else { ' ' });
		self.completion_index = 0;
		self.history_pos = None;
		true
	}

	/// A NeL service: the input line is a command and the toolbar shows.
	fn is_console(&self) -> bool {
		self.info.control
	}

	/// The command buttons of the toolbar in order: (text, command).
	fn buttons(&self) -> Vec<(String, String)> {
		self.info
			.labels
			.iter()
			.filter(|l| l.starts_with('@'))
			.map(|l| {
				let body = &l[1..];
				match body.find('|') {
					Some(pos) => (body[..pos].to_string(), body[pos + 1..].to_string()),
					None => (body.to_string(), body.to_string()),
				}
			})
			.collect()
	}
}

#[derive(Default)]
struct LayoutCache {
	rows: Vec<(u16, usize)>,
	list_inner: Rect,
	glyph_col: u16,
	buttons: Vec<(Rect, String)>,
	content: Rect,
	/// Rows the selected pane holds at the current width and wrap mode, for the scroll range.
	total_rows: usize,
	/// The command the input line suggests, with its arguments and help, for the status line.
	hint: Option<String>,
}

pub struct App {
	title: String,
	procs: Vec<ProcView>,
	selected: usize,
	notice: Option<(String, Instant)>,
	quit_armed: Option<Instant>,
	/// Compact the NeL log header of each line.
	compact_prefix: bool,
	/// Wrap long lines instead of cutting them.
	wrap: bool,
	/// Show the arrival time of each line.
	show_time: bool,
	layout: LayoutCache,
	pub dirty: bool,
}

fn truncate(text: &str, width: usize) -> String {
	if width == 0 {
		return String::new();
	}
	let count = text.chars().count();
	if count <= width {
		return text.to_string();
	}
	if width <= 1 {
		return text.chars().take(width).collect();
	}
	let mut out: String = text.chars().take(width - 1).collect();
	out.push('…');
	out
}

fn state_color(state: ProcState) -> Color {
	match state {
		ProcState::New => Color::DarkGray,
		ProcState::Launched => Color::Rgb(255, 140, 0),
		ProcState::Captured => Color::Yellow,
		ProcState::Ready => Color::Green,
		ProcState::Stopping => Color::Blue,
		ProcState::Stopped => Color::Red,
	}
}

fn to_color(color: proto::Color) -> Color {
	match color {
		proto::Color::Default => Color::Reset,
		proto::Color::Black => Color::Black,
		proto::Color::Red => Color::Red,
		proto::Color::Green => Color::Green,
		proto::Color::Yellow => Color::Yellow,
		proto::Color::Blue => Color::Blue,
		proto::Color::Magenta => Color::Magenta,
		proto::Color::Cyan => Color::Cyan,
		proto::Color::Gray => Color::Gray,
		proto::Color::DarkGray => Color::DarkGray,
		proto::Color::LightRed => Color::LightRed,
		proto::Color::LightGreen => Color::LightGreen,
		proto::Color::LightYellow => Color::LightYellow,
		proto::Color::LightBlue => Color::LightBlue,
		proto::Color::LightMagenta => Color::LightMagenta,
		proto::Color::LightCyan => Color::LightCyan,
		proto::Color::White => Color::White,
		proto::Color::Rgb(r, g, b) => Color::Rgb(r, g, b),
	}
}

/// Continuation rows of a wrapped line hang this many characters in.
const WRAP_INDENT: usize = 4;

/// Width of the shortened function name: three characters, an ellipsis, five characters.
const FUNCTION_WIDTH: usize = 9;

/// The NeL log header as CWindowDisplayer writes it, `INF  b8c  service.cpp  879 NLNET::IService::main: `,
/// or the `b8c: ` of a command log line; the thread id is hex and the function has no spaces.
fn prefix_regex() -> &'static Regex {
	static REGEX: OnceLock<Regex> = OnceLock::new();
	REGEX.get_or_init(|| Regex::new(r"^(?:([A-Z]{3}) +)?[0-9a-fA-F]{1,8}(?: +\S+ +\d+(?: +(\S+))?)?: ").expect("a valid regex"))
}

/// The line with its NeL log header shortened: the type stays, thread, file and line go, and
/// the function is cut to a fixed width, `NLNET::cbRegisterBroadcast` to `NLN…dcast`, so the
/// messages line up. The thread of a command log line goes. Spans are consumed from the start.
fn compact_prefix(line: &StyledLine) -> StyledLine {
	let text = line.text();
	let Some(found) = prefix_regex().captures(&text) else {
		return line.clone();
	};
	let whole = found.get(0).expect("the whole match");
	let mut head = String::new();
	if let Some(function) = found.get(2) {
		let function = function.as_str();
		let chars: Vec<char> = function.chars().collect();
		let short: String = if chars.len() > FUNCTION_WIDTH {
			let mut s: String = chars[..3].iter().collect();
			s.push('…');
			s.extend(chars[chars.len() - 5..].iter());
			s
		} else {
			format!("{:<width$}", function, width = FUNCTION_WIDTH)
		};
		if let Some(kind) = found.get(1) {
			head.push_str(kind.as_str());
			head.push(' ');
		}
		head.push_str(&short);
		head.push_str(": ");
	}
	// the header's bytes leave the spans; the shortened one takes the style of the first
	let mut remove = whole.end();
	let mut spans = Vec::new();
	let first_style = line.spans.first().cloned().unwrap_or_default();
	if !head.is_empty() {
		spans.push(proto::Span { text: head, ..first_style });
	}
	for span in &line.spans {
		if remove == 0 {
			spans.push(span.clone());
			continue;
		}
		let len = span.text.len();
		if remove >= len {
			remove -= len;
			continue;
		}
		let mut shortened = span.clone();
		shortened.text = span.text[remove..].to_string();
		remove = 0;
		spans.push(shortened);
	}
	StyledLine { spans, raw: line.raw, time: line.time.clone(), ..Default::default() }
}

/// The line with its arrival time in front, `[hh:mm:ss] `, in grey.
fn with_time(mut line: StyledLine) -> StyledLine {
	if line.time.is_empty() {
		return line;
	}
	line.spans.insert(0, proto::Span { text: format!("[{}] ", line.time), fg: proto::Color::DarkGray, bold: false });
	line
}

/// A line split into rows of at most `width` characters, continuations indented.
fn wrap_line(line: &StyledLine, width: usize) -> Vec<StyledLine> {
	let width = width.max(WRAP_INDENT + 1);
	let mut rows: Vec<StyledLine> = Vec::new();
	let mut current = StyledLine { spans: Vec::new(), raw: line.raw, time: line.time.clone(), ..Default::default() };
	let mut used = 0usize;
	for span in &line.spans {
		let mut piece = proto::Span { text: String::new(), ..span.clone() };
		for c in span.text.chars() {
			let limit = if rows.is_empty() { width } else { width - WRAP_INDENT };
			if used >= limit {
				if !piece.text.is_empty() {
					current.spans.push(std::mem::replace(&mut piece, proto::Span { text: String::new(), ..span.clone() }));
				}
				rows.push(std::mem::replace(
					&mut current,
					StyledLine {
						spans: vec![proto::Span { text: " ".repeat(WRAP_INDENT), ..Default::default() }],
						raw: line.raw,
						time: line.time.clone(),
						..Default::default()
					},
				));
				used = 0;
			}
			piece.text.push(c);
			used += 1;
		}
		if !piece.text.is_empty() {
			current.spans.push(piece);
		}
	}
	if !current.spans.is_empty() || rows.is_empty() {
		rows.push(current);
	}
	rows
}

fn to_line(line: &StyledLine, console: bool) -> Line<'static> {
	let spans: Vec<Span<'static>> = line
		.spans
		.iter()
		.map(|s| {
			// stdout and stderr beside a console log read apart in grey
			let fg = if console && line.raw && s.fg == proto::Color::Default { Color::DarkGray } else { to_color(s.fg) };
			let mut style = Style::default().fg(fg);
			if s.bold {
				style = style.add_modifier(Modifier::BOLD);
			}
			Span::styled(s.text.clone(), style)
		})
		.collect();
	Line::from(spans)
}

impl App {
	pub fn new() -> App {
		App {
			title: "Service Dashboard".to_string(),
			procs: Vec::new(),
			selected: 0,
			notice: None,
			quit_armed: None,
			compact_prefix: true,
			wrap: false,
			show_time: true,
			layout: LayoutCache::default(),
			dirty: true,
		}
	}

	fn notice(&mut self, text: impl Into<String>) {
		self.notice = Some((text.into(), Instant::now()));
		self.dirty = true;
	}

	fn index_of(&self, name: &str) -> Option<usize> {
		self.procs.iter().position(|p| p.info.name == name)
	}

	// *** daemon messages ***

	pub fn apply(&mut self, msg: DaemonMsg) {
		self.dirty = true;
		match msg {
			DaemonMsg::Hello { title, .. } => self.title = title,
			DaemonMsg::Processes { processes } => {
				let old = std::mem::take(&mut self.procs);
				for info in processes {
					let mut view = ProcView::new(info);
					if let Some(prev) = old.iter().find(|p| p.info.name == view.info.name) {
						view.input = prev.input.clone();
						view.history = prev.history.clone();
						view.commands = prev.commands.clone();
					}
					self.procs.push(view);
				}
				if self.selected >= self.procs.len() {
					self.selected = 0;
				}
			}
			DaemonMsg::Process { info } => match self.index_of(&info.name) {
				Some(idx) => self.procs[idx].info = info,
				None => self.procs.push(ProcView::new(info)),
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
					let view = &mut self.procs[idx];
					view.lines = lines;
					view.scroll = view.scroll.min(view.lines.len());
				}
			}
			DaemonMsg::Lines { name, lines } => {
				if let Some(idx) = self.index_of(&name) {
					let view = &mut self.procs[idx];
					let follows = view.scroll == 0;
					let count = lines.len();
					let buffer = &mut view.lines;
					buffer.extend(lines);
					if buffer.len() > MAX_CLIENT_LINES {
						let excess = buffer.len() - MAX_CLIENT_LINES;
						buffer.drain(..excess);
					}
					let total = buffer.len();
					if !follows {
						// keep the lines on screen where they are
						view.scroll = (view.scroll + count).min(total);
					}
				}
			}
			DaemonMsg::Clear { name } => {
				if let Some(idx) = self.index_of(&name) {
					self.procs[idx].lines.clear();
					self.procs[idx].scroll = 0;
				}
			}
			DaemonMsg::Commands { name, commands } => {
				if let Some(idx) = self.index_of(&name) {
					let view = &mut self.procs[idx];
					view.commands = commands.into_iter().map(|c| (c.name.clone(), c)).collect();
					view.completion_index = 0;
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
			DaemonMsg::Notice { text } => self.notice(text),
			DaemonMsg::ExecDone { .. } => {}
			DaemonMsg::Bye => {}
		}
	}

	// *** input ***

	fn select(&mut self, idx: usize) {
		if idx < self.procs.len() {
			self.selected = idx;
			self.dirty = true;
		}
	}

	fn selected_name(&self) -> Option<String> {
		self.procs.get(self.selected).map(|p| p.info.name.clone())
	}

	fn scroll_by(&mut self, delta: isize) {
		if let Some(view) = self.procs.get_mut(self.selected) {
			let total = self.layout.total_rows.max(view.lines.len());
			let height = self.layout.content.height.max(1) as usize;
			let max = total.saturating_sub(height);
			let next = (view.scroll as isize + delta).clamp(0, max as isize) as usize;
			view.scroll = next;
			self.dirty = true;
		}
	}

	fn history_move(&mut self, up: bool) {
		let Some(view) = self.procs.get_mut(self.selected) else {
			return;
		};
		if view.history.is_empty() {
			return;
		}
		match (view.history_pos, up) {
			(None, true) => {
				view.draft = view.input.clone();
				view.history_pos = Some(view.history.len() - 1);
			}
			(Some(pos), true) if pos > 0 => view.history_pos = Some(pos - 1),
			(Some(pos), false) if pos + 1 < view.history.len() => view.history_pos = Some(pos + 1),
			(Some(_), false) => {
				view.history_pos = None;
				view.input = std::mem::take(&mut view.draft);
				self.dirty = true;
				return;
			}
			_ => return,
		}
		if let Some(pos) = view.history_pos {
			view.input = view.history[pos].clone();
		}
		self.dirty = true;
	}

	fn submit(&mut self) -> Option<Action> {
		let view = self.procs.get_mut(self.selected)?;
		let text = std::mem::take(&mut view.input);
		view.history_pos = None;
		view.completion_index = 0;
		view.draft.clear();
		self.dirty = true;
		if text.is_empty() {
			return None;
		}
		if view.history.last() != Some(&text) {
			view.history.push(text.clone());
			if view.history.len() > 200 {
				view.history.remove(0);
			}
		}
		let name = view.info.name.clone();
		if view.is_console() {
			if text == "CLEAR" {
				return Some(Action::Send(ClientMsg::ClearConsole { name }));
			}
			Some(Action::Send(ClientMsg::Command { name, text }))
		} else {
			Some(Action::Send(ClientMsg::Input { name, text }))
		}
	}

	fn toggle_selected(&mut self) -> Option<Action> {
		let view = self.procs.get(self.selected)?;
		let name = view.info.name.clone();
		if view.info.started {
			Some(Action::Send(ClientMsg::Stop { name }))
		} else {
			Some(Action::Send(ClientMsg::Start { name }))
		}
	}

	fn press_button(&mut self, number: usize) -> Option<Action> {
		let view = self.procs.get(self.selected)?;
		if !view.is_console() {
			return None;
		}
		let (text, command) = view.buttons().into_iter().nth(number.checked_sub(1)?)?;
		self.button_action(&view.info.name.clone(), &text, &command)
	}

	fn button_action(&mut self, name: &str, text: &str, command: &str) -> Option<Action> {
		if command == "CLEAR" {
			return Some(Action::Send(ClientMsg::ClearConsole { name: name.to_string() }));
		}
		self.notice(format!("{}: {}", text, command));
		Some(Action::Send(ClientMsg::Command { name: name.to_string(), text: command.to_string() }))
	}

	fn handle_key(&mut self, key: KeyEvent) -> Vec<Action> {
		let mut actions = Vec::new();
		let ctrl = key.modifiers.contains(KeyModifiers::CONTROL);
		let alt = key.modifiers.contains(KeyModifiers::ALT);
		match key.code {
			KeyCode::F(2) => {
				self.compact_prefix = !self.compact_prefix;
				self.dirty = true;
			}
			KeyCode::F(3) => {
				self.wrap = !self.wrap;
				self.dirty = true;
			}
			KeyCode::F(4) => {
				self.show_time = !self.show_time;
				self.dirty = true;
			}
			KeyCode::F(5) => actions.extend(self.toggle_selected()),
			KeyCode::F(6) => {
				if let Some(name) = self.selected_name() {
					actions.push(Action::Send(ClientMsg::Kill { name }));
				}
			}
			KeyCode::F(7) => actions.push(Action::Send(ClientMsg::StartAll)),
			KeyCode::F(8) => actions.push(Action::Send(ClientMsg::StopAll)),
			KeyCode::F(10) => actions.push(Action::Detach),
			KeyCode::F(12) => {
				let armed = self.quit_armed.is_some_and(|t| t.elapsed() < QUIT_ARM_TIME);
				if armed {
					self.quit_armed = None;
					actions.push(Action::Send(ClientMsg::Shutdown));
				} else {
					self.quit_armed = Some(Instant::now());
					self.notice("F12 again stops all services and closes the dashboard (F10 detaches and keeps them running)");
				}
			}
			KeyCode::Tab => {
				if let Some(view) = self.procs.get_mut(self.selected) {
					if view.accept_suggestion() {
						self.dirty = true;
					}
				}
			}
			KeyCode::BackTab => {
				if let Some(view) = self.procs.get_mut(self.selected) {
					if view.suggestion().is_some_and(|s| s.naming) {
						view.completion_index += 1;
						self.dirty = true;
					}
				}
			}
			KeyCode::Up if alt || ctrl => self.select((self.selected + self.procs.len().max(1) - 1) % self.procs.len().max(1)),
			KeyCode::Down if alt || ctrl => self.select((self.selected + 1) % self.procs.len().max(1)),
			KeyCode::Up => self.history_move(true),
			KeyCode::Down => self.history_move(false),
			KeyCode::PageUp => {
				let page = self.layout.content.height.saturating_sub(1).max(1) as isize;
				self.scroll_by(page);
			}
			KeyCode::PageDown => {
				let page = self.layout.content.height.saturating_sub(1).max(1) as isize;
				self.scroll_by(-page);
			}
			KeyCode::Home if ctrl => self.scroll_by(isize::MAX / 2),
			KeyCode::End if ctrl => self.scroll_by(-(isize::MAX / 2)),
			KeyCode::Enter => actions.extend(self.submit()),
			KeyCode::Esc => {
				if let Some(view) = self.procs.get_mut(self.selected) {
					view.input.clear();
					view.history_pos = None;
					view.completion_index = 0;
					self.dirty = true;
				}
			}
			KeyCode::Backspace => {
				if let Some(view) = self.procs.get_mut(self.selected) {
					view.input.pop();
					view.history_pos = None;
					view.completion_index = 0;
					self.dirty = true;
				}
			}
			KeyCode::Char('l') | KeyCode::Char('L') if ctrl => {
				if let Some(view) = self.procs.get(self.selected) {
					if view.is_console() {
						actions.push(Action::Send(ClientMsg::ClearConsole { name: view.info.name.clone() }));
					}
				}
			}
			KeyCode::Char(c) if alt && c.is_ascii_digit() && c != '0' => {
				actions.extend(self.press_button(c as usize - '0' as usize));
			}
			KeyCode::Char(c) if !ctrl && !alt => {
				if let Some(view) = self.procs.get_mut(self.selected) {
					view.input.push(c);
					view.history_pos = None;
					view.completion_index = 0;
					self.dirty = true;
				}
			}
			_ => {}
		}
		actions
	}

	fn handle_mouse(&mut self, mouse: MouseEvent) -> Vec<Action> {
		let mut actions = Vec::new();
		let pos = Position { x: mouse.column, y: mouse.row };
		match mouse.kind {
			MouseEventKind::Down(MouseButton::Left) => {
				if self.layout.list_inner.contains(pos) {
					if let Some(&(_, idx)) = self.layout.rows.iter().find(|(y, _)| *y == mouse.row) {
						self.select(idx);
						if mouse.column >= self.layout.glyph_col {
							actions.extend(self.toggle_selected());
						}
					}
					return actions;
				}
				let hit = self.layout.buttons.iter().find(|(rect, _)| rect.contains(pos)).cloned();
				if let Some((_, command)) = hit {
					if let Some(view) = self.procs.get(self.selected) {
						let name = view.info.name.clone();
						let text = view.buttons().into_iter().find(|(_, c)| *c == command).map(|(t, _)| t).unwrap_or_default();
						actions.extend(self.button_action(&name, &text, &command));
					}
				}
			}
			MouseEventKind::ScrollUp => self.scroll_by(3),
			MouseEventKind::ScrollDown => self.scroll_by(-3),
			_ => {}
		}
		actions
	}

	pub fn handle(&mut self, event: Event) -> Vec<Action> {
		match event {
			Event::Key(key) if key.kind == KeyEventKind::Press => self.handle_key(key),
			Event::Mouse(mouse) => self.handle_mouse(mouse),
			Event::Resize(_, _) => {
				self.dirty = true;
				Vec::new()
			}
			_ => Vec::new(),
		}
	}

	// *** rendering ***

	pub fn render(&mut self, frame: &mut Frame) {
		self.dirty = false;
		let area = frame.area();
		let [main, status] = Layout::vertical([Constraint::Min(3), Constraint::Length(1)]).areas(area);
		let list_width = LIST_WIDTH.min(main.width / 2).max(12);
		let [list, pane] = Layout::horizontal([Constraint::Length(list_width), Constraint::Min(10)]).areas(main);

		self.render_list(frame, list);
		self.render_pane(frame, pane);
		self.render_status(frame, status);
	}

	fn render_list(&mut self, frame: &mut Frame, area: Rect) {
		let block = Block::bordered().title(format!(" {} ", truncate(&self.title, area.width.saturating_sub(4) as usize)));
		let inner = block.inner(area);
		frame.render_widget(block, area);
		self.layout.list_inner = inner;
		self.layout.rows.clear();
		self.layout.glyph_col = inner.right().saturating_sub(2);

		let label_width = inner.width.saturating_sub(5) as usize;
		let mut lines = Vec::new();
		for (idx, view) in self.procs.iter().enumerate() {
			if idx as u16 >= inner.height {
				break;
			}
			let y = inner.y + idx as u16;
			self.layout.rows.push((y, idx));
			let label = truncate(&view.info.label, label_width);
			let pad = label_width.saturating_sub(label.chars().count());
			let glyph = if view.info.started { "■" } else { "▶" };
			let row_style = if idx == self.selected { Style::default().add_modifier(Modifier::REVERSED) } else { Style::default() };
			lines.push(Line::from(vec![
				Span::styled("● ", Style::default().fg(state_color(view.info.state))),
				Span::styled(format!("{}{} ", label, " ".repeat(pad)), row_style),
				Span::styled(glyph, Style::default().fg(if view.info.started { Color::LightRed } else { Color::LightGreen })),
			]));
		}
		frame.render_widget(Paragraph::new(lines), inner);
	}

	fn render_pane(&mut self, frame: &mut Frame, area: Rect) {
		self.layout.buttons.clear();
		let Some(view) = self.procs.get(self.selected) else {
			let block = Block::bordered().title(" no processes ");
			frame.render_widget(block, area);
			self.layout.content = Rect::default();
			return;
		};

		let mut title = view.info.title.clone();
		if view.info.control {
			title.push_str(if view.info.control_connected { "  [console]" } else { "  [console: not connected]" });
		}
		let block = Block::bordered().title(format!(" {} ", truncate(&title, area.width.saturating_sub(4) as usize)));
		let inner = block.inner(area);
		frame.render_widget(block, area);

		// toolbar rows: labels split on the empty separator
		let mut rows: Vec<Vec<String>> = vec![Vec::new()];
		if view.is_console() {
			for label in &view.info.labels {
				if label.is_empty() {
					rows.push(Vec::new());
				} else {
					rows.last_mut().expect("there is always a row").push(label.clone());
				}
			}
		}
		rows.retain(|r| !r.is_empty());
		let toolbar_height = (rows.len() as u16).min(inner.height.saturating_sub(2));

		let [toolbar, content, input] = Layout::vertical([Constraint::Length(toolbar_height), Constraint::Min(1), Constraint::Length(1)]).areas(inner);
		self.layout.content = content;

		// as CWinDisplayer::resizeLabels: the width is divided equally among the items of a row and
		// each item sits in its own column, so a value that changes width does not move its neighbours
		let mut toolbar_lines = Vec::new();
		for (row_idx, row) in rows.iter().enumerate().take(toolbar_height as usize) {
			let y = toolbar.y + row_idx as u16;
			let column = (toolbar.width / row.len().max(1) as u16).max(1);
			let mut spans = Vec::new();
			for (j, label) in row.iter().enumerate() {
				let x = toolbar.x + j as u16 * column;
				let room = column.saturating_sub(1) as usize;
				if let Some(body) = label.strip_prefix('@') {
					let (text, command) = match body.find('|') {
						Some(pos) => (&body[..pos], &body[pos + 1..]),
						None => (body, body),
					};
					let shown = truncate(&format!("[{}]", text), room);
					let width = shown.chars().count() as u16;
					self.layout.buttons.push((Rect { x, y, width, height: 1 }, command.to_string()));
					spans.push(Span::styled(shown, Style::default().fg(Color::Black).bg(Color::Gray).add_modifier(Modifier::BOLD)));
					spans.push(Span::raw(" ".repeat(column as usize - width as usize)));
				} else {
					let shown = truncate(label, room);
					let width = shown.chars().count();
					spans.push(Span::styled(shown, Style::default().fg(Color::Cyan)));
					spans.push(Span::raw(" ".repeat(column as usize - width)));
				}
			}
			toolbar_lines.push(Line::from(spans));
		}
		if toolbar_height > 0 {
			frame.render_widget(Paragraph::new(toolbar_lines), toolbar);
		}

		let height = content.height as usize;
		// the rows to show: the lines, their headers compacted and wrapped as asked; the scrollbar
		// takes the right column once there is more than fits
		let console = view.is_console();
		let shown: Vec<StyledLine> = view
			.lines
			.iter()
			.map(|line| {
				let line = if self.compact_prefix { compact_prefix(line) } else { line.clone() };
				if self.show_time {
					with_time(line)
				} else {
					line
				}
			})
			.collect();
		let rows: Vec<StyledLine> = if self.wrap {
			let width = content.width.saturating_sub(1).max(1) as usize;
			shown.iter().flat_map(|line| wrap_line(line, width)).collect()
		} else {
			shown
		};
		let total = rows.len();
		self.layout.total_rows = total;
		let end = total.saturating_sub(view.scroll.min(total));
		let start = end.saturating_sub(height);
		let [text_area, bar_area] = Layout::horizontal([Constraint::Min(1), Constraint::Length(if total > height { 1 } else { 0 })]).areas(content);
		let lines: Vec<Line> = rows[start..end].iter().map(|line| to_line(line, console)).collect();
		frame.render_widget(Paragraph::new(lines), text_area);
		if total > height && bar_area.width > 0 {
			let mut state = ScrollbarState::new(total.saturating_sub(height)).position(start).viewport_content_length(height);
			frame.render_stateful_widget(
				Scrollbar::new(ScrollbarOrientation::VerticalRight).begin_symbol(Some("▲")).end_symbol(Some("▼")),
				bar_area,
				&mut state,
			);
		}

		let prompt = "> ";
		let avail = input.width.saturating_sub(prompt.len() as u16) as usize;
		let shown: String = {
			let count = view.input.chars().count();
			if count > avail {
				view.input.chars().skip(count - avail).collect()
			} else {
				view.input.clone()
			}
		};
		// the rest of the suggested command name, and its arguments, in grey after the cursor
		let suggestion = view.suggestion();
		let ghost: String = match &suggestion {
			Some(s) if s.naming => {
				let command = s.current();
				let rest: String = command.name.chars().skip(view.input.chars().count()).collect();
				let args = if command.kind == CommandKind::Handler {
					".".to_string()
				} else if command.args.is_empty() {
					String::new()
				} else {
					format!(" {}", command.args)
				};
				truncate(&format!("{}{}", rest, args), avail.saturating_sub(shown.chars().count()))
			}
			_ => String::new(),
		};
		self.layout.hint = suggestion.map(|s| {
			let command = s.current();
			let mut hint = command.name.clone();
			if !command.args.is_empty() {
				hint.push(' ');
				hint.push_str(&command.args);
			}
			if !command.help.is_empty() {
				hint.push_str(": ");
				hint.push_str(&command.help);
			}
			if s.naming && s.candidates.len() > 1 {
				hint.push_str(&format!("  [{}/{} Shift+Tab]", s.index + 1, s.candidates.len()));
			}
			hint
		});
		let cursor_x = input.x + prompt.len() as u16 + shown.chars().count() as u16;
		frame.render_widget(
			Paragraph::new(Line::from(vec![
				Span::styled(prompt, Style::default().fg(Color::LightGreen)),
				Span::raw(shown),
				Span::styled(ghost, Style::default().fg(Color::DarkGray)),
			])),
			input,
		);
		if input.width > 0 {
			frame.set_cursor_position(Position { x: cursor_x.min(input.right().saturating_sub(1)), y: input.y });
		}
	}

	fn render_status(&mut self, frame: &mut Frame, area: Rect) {
		let notice = match &self.notice {
			Some((text, at)) if at.elapsed() < NOTICE_TIME => Some(text.clone()),
			_ => None,
		};
		let line = match (notice, &self.layout.hint) {
			(Some(text), _) => Line::from(Span::styled(truncate(&text, area.width as usize), Style::default().fg(Color::Black).bg(Color::Yellow))),
			(None, Some(hint)) => Line::from(Span::styled(truncate(hint, area.width as usize), Style::default().fg(Color::Cyan))),
			(None, None) => {
				let keys = "F5 Start/Stop  F6 Kill  F7 Start all  F8 Stop all  F2 Compact  F3 Wrap  F4 Time  F10 Detach  F12 Quit  Alt+Up/Down Select  Tab Complete  Alt+1..9 Buttons  PgUp/PgDn Scroll";
				Line::from(Span::styled(truncate(keys, area.width as usize), Style::default().fg(Color::DarkGray)))
			}
		};
		frame.render_widget(Paragraph::new(line), area);
	}
}
