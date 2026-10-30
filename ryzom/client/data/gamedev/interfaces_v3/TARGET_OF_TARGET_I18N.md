# Ryzom Target-of-Target translations

Target-of-Target is a standard Ryzom interface feature, not a separate addon or PadUI module.

## Native NeL I18N keys

These nine unique keys use the feature prefix `uiTargetOfTarget`:

- `uiTargetOfTargetTitle`
- `uiTargetOfTargetNoVisibleTarget`
- `uiTargetOfTargetLoadingName`
- `uiTargetOfTargetSettingTargetHP`
- `uiTargetOfTargetSettingTargetSAP`
- `uiTargetOfTargetSettingTargetStamina`
- `uiTargetOfTargetSettingHP`
- `uiTargetOfTargetSettingSAP`
- `uiTargetOfTargetSettingStamina`

## Language resources

Files: `target_of_target_en.uxt`, `target_of_target_de.uxt`, `target_of_target_fr.uxt`, `target_of_target_es.uxt`, `target_of_target_ru.uxt`.

The client already loads its base locale with `CI18N::load(ClientCfg.LanguageCode)`.
Afterwards `init.cpp` adds these feature strings via `CI18N::loadFromFilename(..., true)`.
The addon file is selected by the same client language, with English-file fallback.
`LanguageName` is intentionally present in each .uxt file (NeL requires it).
No existing Ryzom language file is overwritten.

XML labels are resolved natively via `hardtext="uiTargetOfTarget..."`;
Lua uses `i18n.get()`; C++ uses `CI18N::get()`.
There is no independent language switch and no duplicated Lua dictionary.
New UI translations belong in these five locale files.

## Isolated validation (profile 9903)

Pull the branch, rebuild the independent ToT client, copy the updated
`interaction.xml`, `interaction.lua`, `game_config.xml`,
`game_config.lua`, `taskbar.xml` and all five
`target_of_target_*.uxt` files to the isolated profile's `user` directory.
Back up any existing files and **remove obsolete `padui_tot_*.uxt` files
only from test profile 9903**, after verifying they are ordinary files, not links.
Restart the client. Verify English and German labels, and names for both
players and fauna without P6 string IDs.
Do not touch the Omega baseline or profile 9902.

The historical worktree and feature branch names continue to be used for
development; the translation keys and shipped resources remain neutral.
