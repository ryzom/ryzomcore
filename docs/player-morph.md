# Player morphs

A GM gives a player a creature body: the creature's model, hit points, speed, size and AI actions.
The player's own look and values are kept aside and come back on unmorph.

Server code: `ryzom/server/src/entities_game_service/player_morph*.{h,cpp}`.
Client code: `ryzom/client/src/player_morph.{h,cpp}`, panel template
`ryzom/client/data/gamedev/interfaces_v3/player_morph.xml`.

## Commands

All are admin commands (`/a` on oneself, `/b` on the target, `/c <player>` on a named player),
privileges `:DEV:SGM:GM:` in `client_commands_privileges*.txt`.

| Command | Arguments |
|---|---|
| `morph` | `<role, or creature sheet name or id> [<attackable 0\|1> [<duration in seconds, 0 for none>]]` |
| `unmorph` | |
| `morphRole` | `<role> <creature sheet> [<attackable 0\|1> [<hit points, 0 for the creature ones> [<loot table, - for none> [<next role> <at hit points %>]]]]` |
| `morphRoleAbilities` | `<role> [<ai action sheet>...]`, no action for all actions of the creature |
| `removeMorphRole` | `<role>` |
| `listMorphRoles` | |

`morph` with a creature sheet gives the plain creature: its hit points, all its actions, no loot.
`morph` with a role uses the values of the role.

A morph with a duration ends on its own (`checkMorphEnd`, called from `CCharacter::tickUpdate`).
The end is fixed when the morph is asked for, so the phases of a role keep it. A request whose
client has not answered by then is dropped; a dead morph waits for its respawn or resurrection,
which end the morph anyway.

## Ark

Ark scripts morph the player with the signed web commands of `webExecCommand` (`admin.cpp`), so
no GM privilege is needed:

- `morph!<role or creature sheet>[!<attackable 0|1>[!<duration in seconds>]]`, for example
  `morph!potion_gingo!0!300` for a potion that turns the player into the role `potion_gingo`
  for five minutes. An unknown role, a dead or mounted player fails the command.
- `unmorph`

Both commands and `/a morph` / `/a unmorph` share `morphPlayer` and `unmorphPlayer`.

## Roles

A role (`CPlayerMorphRole`, `player_morph_roles.h`) has exactly one creature sheet, an attackable
flag, hit points (at most `INT_MAX / 1023`, because the hp bar is computed as 1023 * current hit points in a
sint32),
an optional subset of the creature's actions, an optional loot table and an optional next role.

Phases of an event creature are a chain of roles: when the hit points of a role morph fall to
`<at hit points %>`, it turns into the next role (`checkMorphTransformation`). The next role may
use another creature sheet or the same one with other values. The hit points keep their
percentage of the new maximum.

A role with a loot table puts the loot into the bags of the killer and the killer's team when the
morph dies (`dropMorphLoot`). A plain morph drops nothing.

Roles are saved through the backup service in `morph_roles.bin` and loaded when the EGS starts. A
role whose creature sheet no longer exists is dropped at load.

## Morph handshake

1. The command stores the request on the character (`setPendingMorph`) and sends
   `USER:MORPH_PREPARE <sheet>` to the client.
2. The client loads the creature shape and animations. Only if it can show them, it answers
   `COMMAND:MORPH_READY <sheet>`; otherwise it logs a warning and keeps the current look.
3. On `MORPH_READY` for the pending sheet the EGS sets the mirror property `MorphSheet`, the
   creature body and the creature speeds, and sends `USER:MORPH_ABILITIES` (sheet, action sheet
   ids, translation keys, attackable flag) for the panel.

`MorphSheet` (`fe_temp.dataset`, not persistent) goes to every client as a visual property. 0
means the homin look, so unmorph only clears it and the client shows the saved homin appearance
again. It is cleared on login, so a morph ends on relog.

## Creature body

- Hit points: `CCharacter::setMorphBody` keeps the homin scores aside (`_UnmorphedScores`) and
  sets the creature maximum. The character file always stores the homin values
  (`swapMorphBodyForStore`).
- Speed: the creature walk and run speed, sent to the client in `MOUNT_WALK_SPEED` and
  `MOUNT_RUN_SPEED` like the speeds of a mount. The client moves a morph at these speeds, also in
  water, and never further than its run speed in third person, so the GPMS speed check does not
  set it back (see [gpms-player-speed-check.md](gpms-player-speed-check.md)).
- Size: range and collision follow the creature size in the EGS, the GPMS and the client.
- Combat and magic use the creature form (level, range, latency, behaviours, defense).
  `MorphRangeAttackRange` (EGS variable, mm) is the range of creature range attacks.
- No XP, death penalty or faction points from morph fights. The damage the homin did before the
  morph goes to nobody, as when a player enters water, so the creature can't finish a kill for
  the homin's XP. The pvp relation follows the attackable flag.
- Death, respawn and resurrection end the morph. A morph can't mount.

While morphed, the player uses only the creature's actions (`COMMAND:MORPH_ACTION`). Own phrases
and emotes are refused, and equipped items are locked (`isMorphLockedItem`).

## Moods

The panel offers the rest, eat, alert and hungry modes the creature has (`COMMAND:MORPH_MODE`).
Moving, acting or a new form ends the mood. The rest mood uses `MBEHAV::REST` like afk, so the
client treats a morph as afk only through the afk contextual property (`CPlayerCL::isAFK`).

## Translation keys

The panel uses `uiMorph*` keys. They live in the central online translations.
