# Chat sharing, quotes, mentions and emoji

Players can put items, actions, positions, map locations, landmarks and macros into a chat message, quote an earlier message and mention players or groups. The receiving clients show these as links. The protocol lives in `ryzom/common/src/game_share/chat_message.h`.

## Message flow

1. The client keeps each inserted link as a text tag of the chat edit box (`CGroupEditBox::CTextTag`); the tag stores a serialized `CChatMessageReference`. On send, `CHAT_SHARE::buildRequest` (`ryzom/client/src/interface_v3/chat_link_ui.cpp`) turns text and tags into a `CChatMessageRequest`.
2. The client sends `STRING:CHAT_SHARE` with chat mode, dynamic channel, tell receiver and the request (`CClientChatManager::chat` / `tell`). The FS forwards it to the EGS as `CLIENT:STRING:CHAT_SHARE`.
3. The EGS (`cbClientChatShare`, `ryzom/server/src/entities_game_service/client_messages.cpp`) builds the message parts from server state: items from the inventory slot, actions from the known phrases or the phrase sheet, positions from the character or the requested map point. Macros are client data and are only checked against their limits. The resulting `CChatMessage` goes to the IOS as `CHAT_SHARE`.
4. The IOS (`cbChatShare`, `ryzom/server/src/input_output_service/messages.cpp`, and `CChatManager`) filters the text, resolves mentions, attaches the quote and sends the regular `CHAT` / `TELL` impulse. A `CChatMessageTrailer` is appended to that impulse; clients without support ignore the remaining bits. Item names are resolved per receiver before sending.
5. The sender gets `STRING:CHAT_SHARE_RESULT` with its request id. The edit box is only cleared once the server accepted the message.

Far tells to another shard go through the shard unifier as `sendFarTellShared` / `recvFarTellShared` (`chat_unifier_itf.xml`).

## Parts and references

| Part (`CChatMessagePart`) | Built from reference (`CChatMessageReference`) |
|---|---|
| `Text` | text between references |
| `Item` | `Item`: inventory slot |
| `Phrase` | `KnownPhrase`: known phrase index, `PhraseSheet`: rolemaster phrase sheet |
| `Position` | `Position`: own position, `MapPosition`: map location or user landmark |
| `Macro` | `Macro`: name, icon, display text and commands |
| `Reaction` | `Reaction`: reacted message id, emoji name, add or remove |

`Mention` references do not become parts: `@team`, `@guild` and `@all` stay in the text and are mapped to a `CChatMessageMention` with their scope.

Limits are in `CHAT_MESSAGE` (`chat_message.h`): 255 characters of text, 8 references per message, 32 commands per macro and 64 KiB of serialized data.

## Mentions

- `@name` mentions a player; the IOS resolves the name, also across shards.
- `@team` and `@guild` follow the client language (`uiTeam`, `uiGuild`) and only apply in the team or guild channel.
- `@all` is a command word in every language. It needs account privileges, like copying a sheet id, and does not apply in tells.

## Positions

- A `Position` carries the coordinates in millimetres, the place, region and continent names for the receiver, the sender and the time.
- `Continent` holds the continent's place name (for example `continent_fyros`), used for the localized display. Requests name the continent like the client maps (`fyros`), the EGS compares by the `CONTINENT` id.
- "Show on map" finds the continent from the coordinates, opens the most detailed map containing the point and shows a marker for 30 seconds. "Save as landmark" opens the normal landmark name dialog, so the landmark file is only written by the existing landmark code.

## Macros

A shared macro opens prefilled in the macro editor as a new macro. Commands the receiving client does not know are left out; the key shortcut is never shared. The macro is only saved when the player confirms the editor.

## Reactions

A reaction is a message of its own with a single `Reaction` part, sent to the channel of the reacted message ("React" in the chat line menu, or a click on an emoji of the reaction row). The IOS accepts it where a quote of the same message would be accepted and does not keep it in its message history. Player reactions are logged as `chat_reaction` lines; the bridge adds or removes the matching Zulip reaction, once per emoji with the bot account, or for the player's account when the Zulip server supports `on_behalf_of` (see below), and Zulip reactions come back through the IOS command `bridgeReaction`. Clients do not display it as a chat line: they add or remove the sender in the reaction row below the reacted message, whose emoji tooltip lists the players. Reactions to messages no longer shown, and reactions sent before the client received the message, are not kept.

## Chat window

- Messages with links, quotes or group mentions leave the input when they are sent and come back if the server refuses them.
- Jump buttons to the newest message and to the first unread one (`button_newest`, `button_unread` of `CGroupScrollText`), with a line above the first message that arrived in an inactive tab.
- Emoji with a picker beside each chat input; the emoji data and its tooling are described in `ryzom/tools/emoji/README.md`.

## Zulip bridge

The IOS logs shared messages with their parts (`chat_meta` lines), so `ryzom/server/tools/rytranszulip` can forward links and quotes to Zulip. `quote_bridge.py` validates quotes written in Zulip and renders game quotes there.

### Acting for players in Zulip

The bridge bot sends game messages, quotes and reactions for the player's Zulip account `<name>@ig.ryzom.com` instead of writing them itself with a "Name:" prefix. Plain Zulip does not support this; the server needs to accept one extra parameter from the bridge bot:

- `POST /api/v1/messages` with `on_behalf_of=<email>`: the message is stored with that user as sender.
- `POST` and `DELETE /api/v1/messages/{message_id}/reactions` with `on_behalf_of=<email>`: the reaction is added or removed for that user.
- Only the bridge bot may use it, and only for `@ig.ryzom.com` accounts.
- The bot must still be able to edit these messages, because translations are added by editing.

When a request with `on_behalf_of` fails, the bridge falls back to the bot. A server that does not know the parameter reports it in `ignored_parameters_unsupported`; the bridge then uses the bot for an hour before trying again, and adds the "Name:" prefix to the message the bot just posted. Messages and reactions it sent for a player are marked in memcached for 60 seconds, so the fetcher does not send them back to the game.

## Translation keys

The UI keys used by these features are maintained in the online translations, among them `uiChatLinkMenu*`, `uiChatLinkClipboard`, `uiChatLinkDoesNotFit`, `uiChatQuoteSourceChannelOnly`, `uiChatLocation`, `uiChatPositionShowOnMap`, `uiChatPositionSaveLandmark`, `uittChatJumpNewest`, `uittChatJumpUnread`, `uiChatReact` and `uiCopySheetId`.

## Credits

- Chat sharing, quotes, mentions and chat window: Xiao
- Emoji: Riasan
