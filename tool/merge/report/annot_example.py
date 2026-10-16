import importlib.util, os
_s = importlib.util.spec_from_file_location('areas', os.path.join(os.path.dirname(__file__), 'report_areas.py'))
_m = importlib.util.module_from_spec(_s); _s.loader.exec_module(_m)
AREAS = _m.AREAS
COUNT_ONLY = set()

TITLE = 'Atys-live to core4-atys merge'
LEFT = 'atys-live'
RIGHT = 'the merge'
SUBTITLE = ('What the feature/core4-atys branch (tip 9e43ae3e9, 16 October 2026) changes relative to '
            'main/atys-live (999e82147, 7 October 2026). Read it as the review of the merge request atys will receive.')
INTRO = """
<div class="notes">
<p>The branch replays every atys-live change since the last proper cross merge of November 2021 on top of core4, so
almost everything atys has is still here. What this page shows is the remainder: the core4 engine and tooling work of
2021 to 2026 that atys never got, the places where the branch keeps the core4 shape of something atys had rewritten,
and the handful of fixes made while bringing the whole tree through an MSVC build.</p>
<p>Lines marked <b>&minus;</b> exist only in atys-live, lines marked <b>+</b> only in the merge. Hunks are computed on
tokens, so the thousands of lines that differ only in indentation, wrapping, trailing whitespace or line endings (the
2022 autoformatted server import, the clang-formatted client files) do not appear; such files are listed as
<em>formatting only</em>. Hunks that are purely mechanical are counted per file instead of shown: the
<code>NL_OVERRIDE</code> annotations core4 added to overriding methods in July 2026, the <code>NULL</code> and
<code>0</code> to <code>nullptr</code> rewrites of the clang-tidy pass, copyright-year lines, and integer casts.
Whole files that exist on one side only are listed by name, one line each.</p>
<p>Three families of difference recur in every area and are not repeated below: the NL_OVERRIDE and nullptr
mechanics; the core4 copyright headers; and the include-order and <code>#include "stdpch.h"</code> details of the
clang-tidy pass. Everything else is annotated per area.</p>
</div>
"""

NOTES = {
'build': """
<p>The build system is core4's. That brings the target-based CMake of 2025 and 2026 (exported targets, <code>NeL::</code>
and <code>Ryzom::</code> aliases, presets, vcpkg ports and manifests, the hunter and emscripten paths, the reworked
FindMySQL, FindLuabind and FindGLIB2 modules), the GitHub Actions workflows with their composite actions, the dev
container, the docker files and the <code>tool/quick_start</code> scripts. The Mercurial leftovers (.hgtags, .hgignore,
.hgeol), the Travis file and the nelDashBuild scripts are gone as on core4.</p>
<p>Not taken from atys: the <code>WITH_RYZOM_GAMESHARE</code> / <code>FindPackage(RyzomGameShare)</code> plumbing of the
standalone server repository, the <code>IA_AGENT_DEBUG</code> Boost lookup in FindLuabind and the
<code>WITH_SERVER_TOOLS</code> option. The <code>BuildInfo.cmake</code> build-time version stamp and its
<code>ryzom_build_info</code> target are kept: the atys account and ARK services read the
<code>&lt;domain&gt; v&lt;yy&gt;.&lt;mm&gt;.&lt;revision&gt;</code> string from the client User-Agent and refuse
clients whose revision is too old. The atys <code>.gitlab-ci.yml</code> stays. The .gitignore keeps the atys entries and
restores the <code>build/*</code>, <code>build_*</code> and <code>install_*</code> rules that atys dropped in September 2026.
<code>CMakeModules/nel.cmake</code> gains two options the branch introduced to keep both sides buildable:
<code>WITH_MEMCACHED</code> and <code>WITH_BREAKPAD</code>, both off by default.</p>
""",
'nel-include': """
<p>Headers follow the engine work listed under nel/src. The biggest content changes are the shader and program
interfaces of the OpenGL 3 driver (program.h, fxaa_program.h, the new shader headers), the <code>CInetHost</code>
networking API that replaced <code>CInetAddress</code> for IPv6, the stereo passthrough display, GPU skinning, the
<code>NL_NO_LIBXML2</code> build and the atomics compatibility layer. The atys additions in these headers (the PACS
"stairs" trigger and Z offset on <code>UMovePrimitive</code>, a workaround for elevated platforms placed from Lua
scripts, plus the GUI and curl client extensions) are present on both sides, so they do not show.</p>
<p><code>nel/misc/ucstring.h</code> is identical on both sides: <code>RYZOM_LUA_UCSTRING</code> stays defined, because
the atys Lua scripts still use the ucstring API.</p>
""",
'nel-src': """
<p>This is the core4 engine of the last five years: the OpenGL 3 driver with its shader library (the 116 added files
are almost all under <code>nel/src/3d/driver/opengl3</code> and <code>nel/src/3d/shaders</code>), the program and
material rework that the existing OpenGL driver, water, MRM meshes and the wind tree vertex program were moved onto,
IPv6 networking in <code>inet_address.cpp</code> and the net layer, the stereo debugger and passthrough, the
libxml2-free configuration, and a long tail of smaller fixes.</p>
<p><code>nel/src/misc/debug.cpp</code> deserves a careful read. The branch keeps the core4 file and adds the atys
Breakpad minidump handler behind <code>NL_HAVE_BREAKPAD</code> (set by <code>WITH_BREAKPAD</code>, off by default):
the persistent <code>DumpHandler</code>, the process-wide unhandled-exception filter, the stack guarantee, the dump
path that follows <code>changeLogDirectory()</code> and the crashtest commands, as on the atys tip. What atys removed
around it (core4's EDebug report dialog and stack logging) is kept, so the atys build gets its dumps and core4's
flow is untouched. The atys Windows hunter build scripts pass <code>-DWITH_BREAKPAD=ON</code>; the Linux server
builds need <code>-DWITH_BREAKPAD=ON</code> and <code>-DWITH_MEMCACHED=ON</code> on their cmake line. <code>p_thread.cpp</code> shows the core4 cpu_set_t affinity fix where atys made
the equivalent change independently. <code>time_nl.cpp</code> keeps the atys <code>getSeconds64bSince1970()</code>.</p>
""",
'nel-tools': """
<p>Core4 tool work that atys never received: the unit tests converted to gtest (which is why
<code>ut_net_module.test.cpp</code> is the single largest diff on the page), the zone painter, the pipeline_max and
build_gamedata updates, the Qt5 georges editor and zviewer changes, plus the 348 added files that come with them. The
atys tools of 2026 that live here, <code>forgery</code> and <code>pynel</code>, are carried over unchanged. The atys
case-sensitive search-path fix in <code>build_rbank</code> is kept.</p>
""",
'nelns': """
<p>The nelns services and the admin web pages are core4's 2024 to 2026 state: the login service rework, the naming
service changes, docker support, and the admin PHP pages updated for current PHP. atys-live still has the 2021 state.</p>
""",
'game-share': """
<p>The protocol-defining files are the atys ones: <code>database.xml</code>, <code>msg.xml</code>,
<code>inventories.h</code> and <code>.cpp</code>, <code>item_type</code>, <code>continent</code>,
<code>outpost.h</code>, the CDB leaf code without the nullable bit, and <code>MAX_OTHER_PET</code>. None of them appear
here with content changes. What does appear is core4-side: the R2 <code>object.cpp</code> and scenario entry point
changes, the module interface generator (core4's current <code>generate_module_interface.xslt</code> with the atys
2021 load-check fix re-applied on top; the atys copy was an older state of the same file), the
<code>ring_session_manager_itf.h</code> regenerated from it, <code>generic_xml_msg_mngr</code>, and the leveldesign
HTML help pages for the <code>set_timer</code> actions. 34 files differ in formatting only.</p>
""",
'client-src': """
<p>All atys client features are in. The differences are core4 engine integration and tidy work: the driver selection
switch now knows OpenGL 3 and GPU skinning, the <code>CInetHost</code> login path, the stereo passthrough display, the
Luabind and lua_helper changes, and the clang-tidy rewrites, which account for the 2372 nullptr and 1419 NL_OVERRIDE
hunks that are only counted. Where clang-tidy had rewritten a block that atys had restructured, the branch kept the
atys code untouched (inventory manager, item action handlers, people list, chat window), so those spots read exactly
as atys-live.</p>
<p>Fixes made on the branch that atys should take note of: the duplicated <code>DrvAuto</code> case in the driver
switch is gone, <code>interface_manager.cpp</code> includes <code>nel/gui/url_parser.h</code> again (atys had dropped
it while core4's <code>setGroupHTMLWebServer()</code> needs it), and the client patcher keeps the atys xdelta and
unpack options next to core4's macro-based help text. <code>user_agent.cpp</code> is core4's plus the atys
<code>ryzom_build_info.h</code> include.</p>
""",
'client-data': """
<p>The UI XML and Lua are atys content in core4 formatting, which is why 92 files differ in formatting only. The
few real differences: <code>client_default.cfg</code> has the core4 <code>InterfaceScaleAuto</code> and
<code>GPUSkinning</code> keys next to the atys defaults; <code>game_config.xml</code> keeps the core4 scale_auto
parameter instead of the atys copy that carried stray scrollbar attributes; <code>login_keys.xml</code>,
<code>out_v2_keys.xml</code>, <code>outgame_keys.xml</code>, <code>help.xml</code> and <code>taskbar.xml</code> have
small core4 changes. The added <code>cacert.pem</code> under gamedev and the MSVC upgrade scripts are core4 files.</p>
""",
'server-egs': """
<p>The EGS is the atys service. 160 of the 291 differing files are NL_OVERRIDE-only and 68 are formatting-only.
The content differences are the egs_sheets changes that came with core4's shared sheet libraries
(<code>ryzom/server/src/sheets</code>, used by the shard sheet packer), the generated <code>database_guild.h</code>
regenerated for the GUILD:CHEST branch (atys-live only regenerated it in November 2025; the PLR and OUTPOST
regenerations matched the committed files exactly), and four fixes needed to build the atys code on MSVC:
<code>GuildChestSlots</code> is declared <code>extern CVariable&lt;sint32&gt;</code> to match its definition, the
character lookup in <code>CGuild::deleteMember()</code> moved out of the <code>HAVE_MEMCACHED</code> block,
<code>applyRegenAndClipCurrentValue()</code> is declared once, and the memcached setGuild block got its include.
<code>character.cpp</code> also shows the core4 pieces the positional merge kept where atys had reformatted around
them; they are listed in full so they can be checked one by one.</p>
""",
'server-ais': """
<p>The AI service is the atys one with core4's sheet-library split applied: <code>sheets.h</code> and
<code>registerScriptComp</code> (moved to ai_script_comp.cpp on core4, where the fight script components live), the
native function table in <code>script_compiler_native_func.cpp</code>, and the profile headers that carry the
NL_OVERRIDE annotations. The atys visual property parsing, pet manager and bot changes are present on both sides.</p>
""",
'server-other': """
<p>Core4 additions atys does not have: the QUIC transport in the frontend service (<code>quic_transceiver</code>,
<code>quic_selfsign</code>; it falls back to UDP automatically, so it is safe to ship), the shared sheet libraries
under <code>ryzom/server/src/sheets</code>, the patchman <code>log_report</code> files, and the IPv6 service
addressing. The nel admin and backup service code uses <code>CInetHost</code> but keeps the atys port numbers
(49997, 49990). The shard unifier keeps the atys permission query with core4's SQL escaping. The memcached wrapper
and the IOS chat manager changes are atys code and identical on both sides.</p>
""",
'server-tools': """
<p>The atys deployment tooling under <code>ryzom/server/tools</code> is carried as-is, with one exception: the atys
copies of <code>sheets_packer_shard</code> and <code>sheets_dump</code> placed here in July 2026, and the
<code>CMakeLists.txt</code> that wired them, are removed. They are an earlier revision of the same refactor that
core4 completed under <code>ryzom/tools/sheets_packer_shard</code> and <code>sheets_dump_shard</code>, which is the
shape the branch keeps. The three modified entries differ in line endings only.</p>
""",
'server-www': """<p>Identical to atys apart from line endings.</p>""",
'server-rest': """
<p>The 64 removed files are the scaffolding of the atys standalone server repository: <code>ryzom/server/CMakeModules</code>,
its README and COPYING, the patchman_cfg install trees and the shard start and stop scripts. The server builds from
the top-level CMake on this branch, as on core4.</p>
""",
'ryzom-tools': """
<p>Core4 tool work: the shard sheet packer and dumper rebuilt on the shared sheet libraries, the leveldesign and
world editor updates that came with the Qt5 move, and the 257 added files of those tools. The atys leveldesign
export fixes of 2026 are kept. The client patcher is discussed under client sources.</p>
""",
'web': """<p>The web tree is the atys one. The 22 modified entries differ in line endings only.</p>""",
'studio': """<p>Ryzom Studio and Snowballs are core4-only work (the Qt5 plugin rework); atys-live has the 2021 state.</p>""",
'ryzom-rest': """<p>Top-level ryzom files: the core4 CMake entry point.</p>""",
}
