# servdash_cli

The service dashboard of the quick start package as a terminal tool, for Windows and Linux.
It reads the same ini files as the Win32 servdash, launches the processes in the same order,
speaks the same launch control file protocol as the admin executor service (AES), and shows
the same layout: a column of process buttons with a state light, and the console of the
selected process next to it.

Like tmux, it is split in a daemon that owns the processes and a client that renders them.
Detaching, or closing the terminal, leaves the shard running. The same command attaches again,
and several terminals can attach at once. This replaces both servdash in development and the
`screen` scripts (`service_launcher.sh`, `ryzom_domain_screen_wrapper.sh`) in production.

```
servdash_cli start_mainland.ini      start the daemon if needed, then attach
servdash_cli -d start_mainland.ini   start the daemon without attaching
servdash_cli -a start_mainland.ini   attach only
servdash_cli --shutdown start_mainland.ini   stop all services and exit the daemon
servdash_cli --list                  the running dashboards of this user
servdash_cli 2                       attach to the second one in the list
servdash_cli Mainland                attach by title (or id, id prefix, ini path)
```

Each daemon registers itself in a per-user directory (`%LOCALAPPDATA%\servdash_cli\sessions`
on Windows, `$XDG_RUNTIME_DIR/servdash_cli.sessions` or `/tmp/servdash_cli.sessions.<uid>` on
Linux), so a running dashboard can be named from any directory without its ini path.

Keys: F5 start/stop, F6 kill, F7 start all, F8 stop all, F9 raw output or console,
F10 detach, F12 twice to stop everything and quit, Tab or Alt+Up/Down to select a process,
Alt+1..9 to press a console button, PgUp/PgDn to scroll, Enter to send the input line.
The mouse works too: click a process, its start/stop glyph, or a console button.

## NeL services and the console channel

A section with `Control=1` is a NeL service. The daemon listens on a local endpoint for it
and appends `--ctrl=<endpoint>` to its arguments. The service then runs a `CSocketDisplayer`
instead of its Win32 window and streams its log, labels, command buttons and title bar to the
dashboard, which executes the commands typed or clicked there. The `--ctrl` argument
overrides `WindowStyle`, so a service started by hand outside the dashboard still opens its
own window as its config file says. `ReadyPattern` applies to the title the service reports,
exactly as it applied to the window title before; the patch terminal keeps generating
`^[^*].+Service Console`. F9 shows the raw stdout and stderr of the service instead, which
is where an assert or a crash report ends up.

A control section is stopped with `quit` over the channel, or on stdin while the channel is
not connected; its `ShutdownCmd` and `ShutdownInput` keys are not used. A service that never
connects (an older binary that does not know `--ctrl`) counts as captured after 30 seconds, so
the start order of the shard goes on without it.

A service without `Control=1` is shown through its pipes. Its `ReadyPattern`, if any, is
matched against the output lines.

## The ini file

The format and keys of the Win32 servdash, which ignores the two new ones:

| Key | Meaning |
| --- | --- |
| `Title` | Fixed button text; without it the button follows the process title. In the unnamed first section: the dashboard title. |
| `WorkingDirectory` | Working directory of the process; a `LaunchCmd` starting with `.` is taken from it. Any other command is looked up on the PATH, not in the working directory. |
| `LaunchCmd`, `LaunchArgs` | The program and its arguments. On Windows the argument string is passed as it stands; on Linux it is split like a shell does. |
| `ShutdownCmd`, `ShutdownArgs` | A program run to stop the process. Without any stop method the process is killed. |
| `ShutdownInput` | New: a line written to the process stdin to stop it. |
| `ReadyPattern` | Regex for the ready state, see above. |
| `Priority` | Start order group, lowest first; processes without it start one at a time. |
| `AutoStart` | `0` keeps the process out of the start sequence. |
| `LaunchCtrl` | `ctrl,state,LAUNCH,RUNNING,STOP,STOPPED`: the launch control protocol below. |
| `Control` | New: `1` for a NeL service with the console channel. |
| `Terminal` | Accepted and ignored; every process is shown through its pipes. |

The daemon runs with the directory of the ini file as working directory, like the Win32 tool.
On Linux, backslashes in path values are accepted as separators. The processes are tied to
the daemon: a job object on Windows and the parent death signal on Linux take them down if the
daemon itself dies, so a restarted daemon never declares a service stopped while it still runs.

## Launch control files

The AES controls a service it registered through files in the service directory: it writes
`<alias>.launch_ctrl` with LAUNCH or STOP, writes `<alias>.deferred_launch_ctrl` with LAUNCH
to restart a service it told to quit, polls `<alias>.state` for RUNNING or STOPPED, and reads
`<alias>.start_count` for its crash chain detection. The web admin shard controls work through
this. servdash_cli does what `service_launcher.sh` does under screen: it starts and stops on
the orders, writes the state and the counter, consumes a LAUNCH order when the process exits
so a crash does not relaunch in a loop, and promotes a deferred order after the exit. The
start and stop buttons write the order file, as the Win32 tool does, so the AES stays in step.

## Building

Rust 1.75 or later. `cargo build --release` gives a single binary with no runtime files
(linked against the system C library on Linux; use a musl target for a fully static one).
Cross-building the Windows binary from Linux or WSL works with cargo-xwin and needs no
Visual Studio:

```
rustup target add x86_64-pc-windows-msvc
cargo install cargo-xwin
cargo xwin build --release --target x86_64-pc-windows-msvc
```

## Not done yet

- Processes are shown line by line through pipes. A program that needs a terminal (a shell,
  a curses tool) would need a pseudo terminal pane; none of the shard processes do.
- The patch terminal still writes the `start_<shard>.bat` files for the Win32 servdash; the
  quick start batch files have to point at this tool to use it.
