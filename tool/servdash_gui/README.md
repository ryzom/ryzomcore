# servdash_gui

The service dashboard of the quick start package as a Qt application, for development
shards on Windows, Linux and macOS. It reads the same ini files as the Win32 servdash and as
servdash_cli, starts the processes in the same order with the same priorities, speaks the
launch control file protocol of the admin executor service, and shows the same layout: the
column of process buttons with a state light and a start/stop button, and the console of the
selected process next to it.

Unlike servdash_cli there is no daemon: the window owns the processes, and closing it stops
every process and waits for them before it goes, as the .NET original did. Closing a second
time while they stop kills what is left. This is the development tool; production shards use
servdash_cli, which keeps running when its terminal goes away.

```
servdash_gui start_mainland.ini
```

The class transport sample in `tool/servdash_cli/examples/class_transport` runs under both
dashboards; its ini is the shared example for the format.

Without an argument a file dialog asks for the ini. The directory of the ini file becomes the
working directory. A shard is opened once: a lock file next to the ini, `<stem>.servdash.lock`,
names the dashboard that holds it, servdash_gui or servdash_cli, and a second one is refused
while that process lives. The Dashboard menu repeats Launch All and Shutdown All and adds Exit, which
closes the window the same way.

## The console

A section with `Control=1` is a NeL service. The dashboard listens on a local socket for it
and passes `--ctrl=<endpoint>`; the service then streams its log, labels, command buttons and
title to the pane instead of opening its Win32 window, and the pane sends back the commands
typed or clicked. The labels sit in fixed columns per row, as the Win32 service window laid
them out. Whatever the service writes to stdout or stderr besides its log, an assert or a
crash report for instance, shows in the same pane in grey, as do the dashboard's own notes
about the process; the service does not echo its log on stdout while it runs under a
dashboard. Up and Down in the command line walk its history; `CLEAR` or the Clear button
empties the log. A right click on the log offers to compact the NeL log header (the type stays,
thread, file and line go, the function is cut to `NLN…dcast`), to show the arrival time of
each line (both on by default), and to wrap long lines with a hanging indent, and switches the theme
between the system's, light and dark; the choices are kept across runs.

The command line completes the commands of the service: the rest of the first matching name
shows in grey after the cursor, with the arguments the command takes, and its help at the right.
Tab takes the suggestion (a handler instance such as `moduleManager` completes to
`moduleManager.` and its commands follow), Shift+Tab moves to the next match. The table comes
from the service itself, live: every command, variable and module command it registers, as it
registers them, so a module loaded later completes too.

A service without `Control=1` is shown through its pipes, and the input line goes to its stdin.

Stopping a service whose console is not connected yet sends `quit` on its stdin, which a NeL
service with its own window does not read; closing the window a second time kills what is
left. On Linux the `LaunchArgs` string is split like a POSIX shell does (single and double
quotes, backslashes), the same way as servdash_cli; on Windows it is passed as it stands.

## The ini file

The same keys as servdash_cli; see its README for the table. In short: `Title`,
`WorkingDirectory`, `LaunchCmd`, `LaunchArgs`, `ShutdownCmd`, `ShutdownArgs`, `ShutdownInput`,
`ReadyPattern`, `Priority`, `AutoStart`, `LaunchCtrl` and `Control`. The launch control files
(`launch_ctrl`, `deferred_launch_ctrl`, `state`, `start_count`) are handled as servdash_cli
and the Linux launcher handle them, so the web admin shard controls stay in step. The
processes are tied to the dashboard with a job object on Windows and the parent death signal
on Linux, so a crash of the dashboard does not leave them behind.

## In the quick start package

The tools build of the quick start builds servdash_gui when Qt is found, and the shard scripts
(`shard_mainland.bat` and the others) open it on their ini; `"Headless": true` in
`.nel/config.json` makes them run servdash_cli in the console instead.

## Building

In the Ryzom Core tree the tool builds with `WITH_QT5` or `WITH_QT6`. On its own:

```
cmake -S tool/servdash_gui -B build_gui -DCMAKE_PREFIX_PATH=<Qt install prefix>
cmake --build build_gui
```

Qt 6.12 or later with the Core, Gui, Widgets and Network modules (the 2026q4 externals).
The icons, including the application icon, are painted from code; `servdash_gui --write-icon
<dir>` writes the application icon as PNG files per size, from which `servdash_gui.ico` for
the Windows executable is packed. The
tool also builds with Qt 5.15 and Qt 6.2, but their Fusion style drops one pixel lines at a
fractional display scale, so frames and scrollbars come out without edges at 125 percent.

The GitHub Actions workflow `.github/workflows/tools.yml` builds the tool on its own against
the Qt 5 and Qt 6 of Ubuntu and against Qt 6.8 on Windows, where the deployed executable
is kept as a run artifact; the Linux Ryzom job of `ci.yml` builds it in the tree with Qt 5.
