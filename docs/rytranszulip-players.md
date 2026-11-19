# RyTransZulip: messages written in the name of players

`ryzom/server/tools/rytranszulip/zulip_dispatcher.py` sends the in-game
chat to Zulip. Each message is sent in the name of the player who wrote
it. Zulip's code is not modified for this: the bot only uses Zulip's own
API.

## How a message is sent

1. `ZulipClient.ensurePlayer` (`zulip_service.py`) gives the Zulip account
   of the player, `<name in lower case>@ig.ryzom.com`. It asks
   `GET /users/<email>` and, when the account doesn't exist, creates it
   with `POST /users` (a random password, players log in with Ryzom) and
   adds it to the `player_group` group. Accounts that already exist are
   not changed. The answer is remembered until the service restarts.
2. For a channel message, the dispatcher subscribes the player to the
   channel the first time it sees the pair (channel, player). That call
   creates the channel when it doesn't exist yet.
3. The message is sent with the `sender` parameter. A private message is
   also flagged `forged`, which Zulip requires when the bot is not one of
   the recipients. The name of a player of another shard, written
   `Name(Shard)` in the game, loses its `(Shard)` suffix in the address of
   the recipient.

`zulip_fetcher.py` doesn't send these messages back to the game, because
they carry the `ryzom-ig` local id, whatever their sender.

## Permissions of the bot

The account of the bot needs the organization administrator role (Zulip
only lets administrators create users) and the `can_forge_sender` and
`can_create_users` permissions:

```
./manage.py change_user_role -r <realm> <bot email> admin
./manage.py change_user_role -r <realm> <bot email> can_forge_sender
./manage.py change_user_role -r <realm> <bot email> can_create_users
```

On the Ryzom instance, logging in to the bot account with Ryzom applies
the role of the Ryzom groups of the person who logs in, which can lower
it. Run `change_user_role` again after such a login.

A player can only write in a channel when it belongs to the group that
the channel allows to send messages. Accounts created by the bot are
added to `player_group` for that reason.

## Instance settings

The ids of the groups and of the in-game folder differ between Zulip
instances. They are read from the `[zulip]` section of
`/etc/ryzom/rtz.ini`; the ids of the live instance are used when a key is
missing.

```
[zulip]
owner_group = 10
player_group = 73
ingame_folder_id = 2
```

## Limits

The bot is no longer the author of the messages of the players, so it
can't edit them. `sendTranslation` fails with "You don't have permission
to edit this message" until translations are done again on the Zulip side.
