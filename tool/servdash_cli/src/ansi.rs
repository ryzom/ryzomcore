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

//! A line of process output with SGR colour sequences becomes a styled line. Other escape
//! sequences are dropped. The NeL std displayer on Linux writes errors as black on red; such
//! a background colour becomes the foreground, which reads on any terminal theme.

use crate::proto::{Color, Span, StyledLine};

fn basic_color(index: u32) -> Color {
	match index {
		0 => Color::Black,
		1 => Color::Red,
		2 => Color::Green,
		3 => Color::Yellow,
		4 => Color::Blue,
		5 => Color::Magenta,
		6 => Color::Cyan,
		7 => Color::Gray,
		8 => Color::DarkGray,
		9 => Color::LightRed,
		10 => Color::LightGreen,
		11 => Color::LightYellow,
		12 => Color::LightBlue,
		13 => Color::LightMagenta,
		14 => Color::LightCyan,
		15 => Color::White,
		_ => Color::Default,
	}
}

fn indexed_color(index: u32) -> Color {
	if index < 16 {
		return basic_color(index);
	}
	if index < 232 {
		let i = index - 16;
		let steps = [0u8, 95, 135, 175, 215, 255];
		return Color::Rgb(steps[(i / 36) as usize], steps[((i / 6) % 6) as usize], steps[(i % 6) as usize]);
	}
	if index < 256 {
		let v = (8 + (index - 232) * 10) as u8;
		return Color::Rgb(v, v, v);
	}
	Color::Default
}

#[derive(Clone, Copy, Default)]
struct Attrs {
	fg: Color,
	bg: Color,
	bold: bool,
}

impl Attrs {
	fn apply_sgr(&mut self, params: &[u32]) {
		if params.is_empty() {
			*self = Attrs::default();
			return;
		}
		let mut i = 0;
		while i < params.len() {
			let p = params[i];
			match p {
				0 => *self = Attrs::default(),
				1 => self.bold = true,
				22 => self.bold = false,
				30..=37 => self.fg = basic_color(p - 30),
				39 => self.fg = Color::Default,
				40..=47 => self.bg = basic_color(p - 40),
				49 => self.bg = Color::Default,
				90..=97 => self.fg = basic_color(p - 90 + 8),
				100..=107 => self.bg = basic_color(p - 100 + 8),
				38 | 48 => {
					let target = if p == 38 { &mut self.fg } else { &mut self.bg };
					if i + 1 < params.len() && params[i + 1] == 5 && i + 2 < params.len() {
						*target = indexed_color(params[i + 2]);
						i += 2;
					} else if i + 1 < params.len() && params[i + 1] == 2 && i + 4 < params.len() {
						*target = Color::Rgb(params[i + 2] as u8, params[i + 3] as u8, params[i + 4] as u8);
						i += 4;
					}
				}
				_ => {}
			}
			i += 1;
		}
	}

	fn span_color(&self) -> Color {
		if self.bg != Color::Default && (self.fg == Color::Default || self.fg == Color::Black) {
			self.bg
		} else {
			self.fg
		}
	}
}

/// Parses one line of output. Carriage returns inside the line keep only the text after the
/// last one, which shows the final state of a progress line. Tabs become spaces.
pub fn parse_line(raw: &str) -> StyledLine {
	let raw = raw.strip_suffix('\r').unwrap_or(raw);
	let raw = match raw.rfind('\r') {
		Some(pos) => &raw[pos + 1..],
		None => raw,
	};

	let mut spans: Vec<Span> = Vec::new();
	let mut attrs = Attrs::default();
	let mut text = String::new();
	let chars: Vec<char> = raw.chars().collect();
	let mut i = 0;

	let flush = |spans: &mut Vec<Span>, text: &mut String, attrs: &Attrs| {
		if !text.is_empty() {
			spans.push(Span { text: std::mem::take(text), fg: attrs.span_color(), bold: attrs.bold });
		}
	};

	while i < chars.len() {
		let c = chars[i];
		if c == '\u{1b}' {
			if i + 1 < chars.len() && chars[i + 1] == '[' {
				// CSI: parameters, then a final byte in 0x40..0x7e
				let mut j = i + 2;
				let mut params: Vec<u32> = Vec::new();
				let mut current: Option<u32> = None;
				let mut private = false;
				while j < chars.len() {
					let d = chars[j];
					if d.is_ascii_digit() {
						current = Some(current.unwrap_or(0).saturating_mul(10).saturating_add(d as u32 - '0' as u32));
					} else if d == ';' {
						params.push(current.unwrap_or(0));
						current = None;
					} else if ('\u{40}'..='\u{7e}').contains(&d) {
						break;
					} else {
						private = true;
					}
					j += 1;
				}
				if j < chars.len() {
					if let Some(v) = current {
						params.push(v);
					}
					if chars[j] == 'm' && !private {
						flush(&mut spans, &mut text, &attrs);
						attrs.apply_sgr(&params);
					}
					i = j + 1;
				} else {
					i = chars.len();
				}
				continue;
			}
			if i + 1 < chars.len() && chars[i + 1] == ']' {
				// OSC: until BEL or ESC backslash
				let mut j = i + 2;
				while j < chars.len() {
					if chars[j] == '\u{7}' {
						j += 1;
						break;
					}
					if chars[j] == '\u{1b}' && j + 1 < chars.len() && chars[j + 1] == '\\' {
						j += 2;
						break;
					}
					j += 1;
				}
				i = j;
				continue;
			}
			// a two character escape
			i += 2;
			continue;
		}
		if c == '\t' {
			text.push_str("    ");
		} else if !c.is_control() {
			text.push(c);
		}
		i += 1;
	}
	flush(&mut spans, &mut text, &attrs);
	StyledLine { spans, raw: true, time: String::new() }
}

#[cfg(test)]
mod tests {
	use super::*;

	#[test]
	fn plain_text() {
		let line = parse_line("hello world\r");
		assert_eq!(line.spans.len(), 1);
		assert_eq!(line.spans[0].text, "hello world");
	}

	#[test]
	fn nel_error_colors() {
		let line = parse_line("\u{1b}[0;30m\u{1b}[41mERR message\u{1b}[0m tail");
		assert_eq!(line.spans[0].text, "ERR message");
		assert_eq!(line.spans[0].fg, Color::Red);
		assert_eq!(line.spans[1].text, " tail");
		assert_eq!(line.spans[1].fg, Color::Default);
	}

	#[test]
	fn bright_and_progress() {
		let line = parse_line("10%\r50%\r\u{1b}[0;91mdone\u{1b}[0m");
		assert_eq!(line.text(), "done");
		assert_eq!(line.spans[0].fg, Color::LightRed);
	}
}
