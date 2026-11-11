# Ryzom Core web services

The web side of a Ryzom Core shard. `public_php` is the web root: `setup` (install and upgrade), `login` (the login scripts the client talks to), `account` (the account tool), `admin` (the shard admin tool), `ring` (the Ring pages), `webig` (the in-game mail and forum pages), `stats` and `stats_query` (the client install statistics and their viewer), `crash_report` (the crash report receiver) and `tools` (code that these share). `private_php` stays outside the web root and holds the configuration template and the SQL scripts of the setup and the `webig` page templates; `docs` holds the shard admin how-to.

## Upgrade notes

### The AMS account management system is removed

The old account management system (AMS) is no longer part of the web stack. The account tool in `public_php/account` is the only account front end. A web server keeps serving files that the repository no longer has, so after updating the web installation of a shard:

* Delete the deployed `ams` folder from the web root (`public_php/ams`) and from the private folder (`private_php/ams`).
* Point links and bookmarks at `/account/` instead of `/ams/`. The web root redirects there, and the client configuration defaults do the same.
* The AMS databases (`nel_ams` and `nel_ams_lib` by default) are left as they are. The setup no longer creates or upgrades them, and nothing drops them. A `config.php` that a later upgrade regenerates no longer has their names, the AMS mail settings or the AMS crypt keys, so keep a copy of the old `config.php` if that data still matters to you.
* The `role_support`, `db_version_web` and `db_version_lib` files in the web root are no longer read and can be deleted.
* The web installer no longer has the Support role or the AMS panel. `setup_cli.php` lost `--no-role-support`, `--ams-db` and `--ams-lib-db`: it refuses them with a message that says so, as it now refuses any option it does not know. That includes `--role-service` and `--role-domain`, which never did anything. Take them out of install scripts.
* A quick start root that was configured before keeps its `localhost_ams.lnk` shortcut, which leads nowhere now. Delete it; `configure.bat` creates `localhost_account.lnk` in its place.
* The client account URLs are CMake cache entries (`RYZOM_CLIENT_CREATE_ACCOUNT_URL`, `RYZOM_CLIENT_EDIT_ACCOUNT_URL`, `RYZOM_CLIENT_FORGET_PASSWORD_URL`), so an existing build directory keeps the `/ams/` values it was configured with. Pass the new values with `-D`, or remove the three entries from `CMakeCache.txt` and configure again. A quick start `.nel/config.json` that sets `ClientCreateAccountUrl`, `ClientEditAccountUrl` or `ClientForgetPasswordUrl` likewise keeps its own values.
