## 2026-08-22 — ♻️ Reorganize www/ and harden login security

Restructured `ryzom/server/www/` for clarity and removed several legacy security issues in preparation for the codebase going public.

**Reorganization**
- Moved shared PHP includes used by both `login/` and `ring/` (`login_translations.php`, `login_service_itf.php`, `nel_message.php`, `domain_info.php`, `admin_modules_itf.php`) from `tools/` to a new `libs/` directory.
- Moved `login/`-only modules (`r2_login_db.php`, `r2_login_domain.php`, `r2_login_logincb.php`, `r2_login_logs.php`, `r2_login_user.php`) into `login/modules/`.
- Moved server status files (`server_open_csr`, `server_open_status`) into `login/status/`.
- Moved SQL schema references (`nel.sql`, `ring.sql`) into `login/schema/`.
- Moved `login/config.php` to `www/config.php` since it is used by `login/`, `ring/`, and `tools/`, not just `login/`.
- Added the project's standard ASCII banner header to all PHP files under `tools/`.
- Removed the unused legacy email system (`login/email/` and the related `mail` handling in `login_translations.php`), which had no remaining callers.

**PHP8 compatibility**
- Replaced all `split()` calls (removed since PHP 7.0) with `explode()` across `ring/`.
- Replaced all `mysql_*` calls (removed since PHP 7.0) with `mysqli_*`, adding explicit connections where the old code relied on the legacy implicit default link (`ring/edit_session.php`, `ring/invite_pioneer.php`, `ring/join_shard.php`, `ring/session_tools.php`, `tools/validate_cookie.php`).
- Replaced curly-brace string/array offset syntax (`$var{$i}`, removed in PHP8) with `$var[$i]` in `libs/nel_message.php`.

**Security hardening**
- Replaced the `$_SERVER['REMOTE_ADDR'] == ULUKYN_IP` bypass (which allowed passwordless login and skipped all access checks from a single hardcoded IP) with a proper admin impersonation flow in `login/modules/r2_login_user.php`: submitting a login as `targetLogin:adminLogin` authenticates normally against the admin account's real password, and if the admin's login is listed in the `ACCESS_IMPERSONATED` config entry, the session identity switches to the target account for the rest of the flow. Added error code 3015 for insufficient impersonation privilege.
- Replaced the hardcoded UID allowlist in `login/modules/r2_login_domain.php` (used to bypass the "restricted" server state) with the `ACCESS_WHEN_LOCK_USERS` config entry.
- Replaced the hardcoded IP ban list in `login/modules/r2_login_user.php` with the `ACCESS_BANNED` config entry.
- Removed the hardcoded RocketChat notification token in `login/modules/r2_login_logs.php`, now read from the `NOTIFY_TOKEN`/`NOTIFY_URL` config entries (already used elsewhere in `tools/`).
- Moved several file writes that used relative paths (landing inside the public webroot) to locations outside it: `tools/restore_char.php`'s intermediate character conversion files now go to `/home/nevrax/tmp/restore_char/` and are cleaned up after use; `ring/join_shard.php`'s debug log now goes to `/home/nevrax/tmp/`.
- Wired up `CWwwLog`'s log directory (`login/modules/r2_login_logs.php`) to the `SHARD_LOGS` config entry instead of a hardcoded path.

**Config cleanup**
- `config.php` now defines `ACCEPT_UNKNOWN_USER` and `AUTO_CREATE_RING_INFO` as constants instead of variables.
- Removed the redundant `$DBHost`/`$DBUserName`/`$DBPassword`/`$DBName`/`$RingDBName`/`$RingDBUserName`/`$RingDBPassword` variables from `config.php`; all call sites now use the `DB_NEL_HOST`/`DB_NEL_USER`/`DB_NEL_PASS`/`DB_NEL_NAME`/`DB_RING_NAME` constants already provided by `shard.ini`.
- Replaced the remaining `parse_ini_file('/etc/ryzom/shard.ini', ...)` calls in `tools/utils.php` and `tools/manage_shard.php` with the equivalent constants, since `config.php` already loads that file once.
