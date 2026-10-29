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

use crate::proto::{self, ClientMsg, DaemonMsg, ProcState, ProcessInfo, Stream, StyledLine};
use crossterm::event::{Event, KeyCode, KeyEvent, KeyEventKind, KeyModifiers, MouseButton, MouseEvent, MouseEventKind};
use ratatui::layout::{Constraint, Layout, Position, Rect};
use ratatui::style::{Color, Modifier, Style};
use ratatui::text::{Line, Span};
use ratatui::widgets::{Block, Paragraph};
use ratatui::Frame;
use std::time::{Duration, Instant};

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
	output: Vec<StyledLine>,
	console: Vec<StyledLine>,
	/// Lines scrolled up from the bottom; zero follows the output.
	scroll: usize,
	input: String,
	history: Vec<String>,
	/// Position in the history while browsing it, with the draft kept aside.
	history_pos: Option<usize>,
	draft: String,
	/// Show the raw process output instead of the console of a NeL service.
	raw: bool,
}

impl ProcView {
	fn new(info: ProcessInfo) -> ProcView {
		ProcView {
			info,
			output: Vec::new(),
			console: Vec::new(),
			scroll: 0,
			input: String::new(),
			history: Vec::new(),
			history_pos: None,
			draft: String::new(),
			raw: false,
		}
	}

	fn shows_console(&self) -> bool {
		self.info.control && !self.raw
	}

	fn buffer(&self, stream: Stream) -> &Vec<StyledLine> {
		match stream {
			Stream::Output => &self.output,
			Stream::Console => &self.console,
		}
	}

	fn buffer_mut(&mut self, stream: Stream) -> &mut Vec<StyledLine> {
		match stream {
			Stream::Output => &mut self.output,
			Stream::Console => &mut self.console,
		}
	}

	fn visible_stream(&self) -> Stream {
		if self.shows_console() {
			Stream::Console
		} else {
			Stream::Output
		}
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
}

pub struct App {
	title: String,
	procs: Vec<ProcView>,
	selected: usize,
	notice: Option<(String, Instant)>,
	quit_armed: Option<Instant>,
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

fn to_line(line: &StyledLine) -> Line<'static> {
	let spans: Vec<Span<'static>> = line
		.spans
		.iter()
		.map(|s| {
			let mut style = Style::default().fg(to_color(s.fg));
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
						view.raw = prev.raw;
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
			DaemonMsg::Snapshot { name, stream, lines } => {
				if let Some(idx) = self.index_of(&name) {
					*self.procs[idx].buffer_mut(stream) = lines;
				}
			}
			DaemonMsg::Lines { name, stream, lines } => {
				if let Some(idx) = self.index_of(&name) {
					let view = &mut self.procs[idx];
					let follows = view.scroll == 0 || view.visible_stream() != stream;
					let count = lines.len();
					let buffer = view.buffer_mut(stream);
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
			DaemonMsg::Clear { name, stream } => {
				if let Some(idx) = self.index_of(&name) {
					self.procs[idx].buffer_mut(stream).clear();
					self.procs[idx].scroll = 0;
				}
			}
			DaemonMsg::Notice { text } => self.notice(text),
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
			let total = view.buffer(view.visible_stream()).len();
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
		if view.shows_console() {
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
		if !view.shows_console() {
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
			KeyCode::F(5) => actions.extend(self.toggle_selected()),
			KeyCode::F(6) => {
				if let Some(name) = self.selected_name() {
					actions.push(Action::Send(ClientMsg::Kill { name }));
				}
			}
			KeyCode::F(7) => actions.push(Action::Send(ClientMsg::StartAll)),
			KeyCode::F(8) => actions.push(Action::Send(ClientMsg::StopAll)),
			KeyCode::F(9) => {
				if let Some(view) = self.procs.get_mut(self.selected) {
					if view.info.control {
						view.raw = !view.raw;
						view.scroll = 0;
						self.dirty = true;
					}
				}
			}
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
			KeyCode::Tab => self.select((self.selected + 1) % self.procs.len().max(1)),
			KeyCode::BackTab => self.select((self.selected + self.procs.len().max(1) - 1) % self.procs.len().max(1)),
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
					self.dirty = true;
				}
			}
			KeyCode::Backspace => {
				if let Some(view) = self.procs.get_mut(self.selected) {
					view.input.pop();
					view.history_pos = None;
					self.dirty = true;
				}
			}
			KeyCode::Char('l') | KeyCode::Char('L') if ctrl => {
				if let Some(view) = self.procs.get(self.selected) {
					if view.shows_console() {
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
			title.push_str(if view.raw {
				"  [raw output]"
			} else if view.info.control_connected {
				"  [console]"
			} else {
				"  [console: not connected]"
			});
		}
		let block = Block::bordered().title(format!(" {} ", truncate(&title, area.width.saturating_sub(4) as usize)));
		let inner = block.inner(area);
		frame.render_widget(block, area);

		// toolbar rows: labels split on the empty separator
		let mut rows: Vec<Vec<String>> = vec![Vec::new()];
		if view.shows_console() {
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

		let mut toolbar_lines = Vec::new();
		for (row_idx, row) in rows.iter().enumerate().take(toolbar_height as usize) {
			let y = toolbar.y + row_idx as u16;
			let mut x = toolbar.x;
			let mut spans = Vec::new();
			for label in row {
				if let Some(body) = label.strip_prefix('@') {
					let (text, command) = match body.find('|') {
						Some(pos) => (&body[..pos], &body[pos + 1..]),
						None => (body, body),
					};
					let shown = format!("[{}]", text);
					let width = shown.chars().count() as u16;
					self.layout.buttons.push((Rect { x, y, width, height: 1 }, command.to_string()));
					spans.push(Span::styled(shown, Style::default().fg(Color::Black).bg(Color::Gray).add_modifier(Modifier::BOLD)));
					x += width;
				} else {
					let width = label.chars().count() as u16;
					spans.push(Span::styled(label.clone(), Style::default().fg(Color::Cyan)));
					x += width;
				}
				spans.push(Span::raw("  "));
				x += 2;
			}
			toolbar_lines.push(Line::from(spans));
		}
		if toolbar_height > 0 {
			frame.render_widget(Paragraph::new(toolbar_lines), toolbar);
		}

		let buffer = view.buffer(view.visible_stream());
		let height = content.height as usize;
		let total = buffer.len();
		let end = total.saturating_sub(view.scroll.min(total));
		let start = end.saturating_sub(height);
		let lines: Vec<Line> = buffer[start..end].iter().map(to_line).collect();
		frame.render_widget(Paragraph::new(lines), content);

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
		let cursor_x = input.x + prompt.len() as u16 + shown.chars().count() as u16;
		frame.render_widget(Paragraph::new(Line::from(vec![Span::styled(prompt, Style::default().fg(Color::LightGreen)), Span::raw(shown)])), input);
		if input.width > 0 {
			frame.set_cursor_position(Position { x: cursor_x.min(input.right().saturating_sub(1)), y: input.y });
		}
	}

	fn render_status(&mut self, frame: &mut Frame, area: Rect) {
		let notice = match &self.notice {
			Some((text, at)) if at.elapsed() < NOTICE_TIME => Some(text.clone()),
			_ => None,
		};
		let line = match notice {
			Some(text) => Line::from(Span::styled(truncate(&text, area.width as usize), Style::default().fg(Color::Black).bg(Color::Yellow))),
			None => {
				let keys =
					"F5 Start/Stop  F6 Kill  F7 Start all  F8 Stop all  F9 Raw/Console  F10 Detach  F12 Quit  Tab Next  Alt+1..9 Buttons  PgUp/PgDn Scroll";
				Line::from(Span::styled(truncate(keys, area.width as usize), Style::default().fg(Color::DarkGray)))
			}
		};
		frame.render_widget(Paragraph::new(line), area);
	}
}
