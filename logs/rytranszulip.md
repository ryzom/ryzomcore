# rytranszulip

## 2026-08-22 — 🐛 Retranslate edited Zulip messages, fix image links

- Added support for Zulip's `update_message` event: when a user edits a message, the fetcher now re-fetches its raw content and re-ingests it for translation (`ingestZulipMessage`), reusing the original message's stored source language (`Zulip-Msg-Lang-<id>` cache key) instead of mis-sourcing the language on re-translation. Edits made by the bot itself (i.e. the translation being appended) are ignored to avoid a translation loop, as are rendering-only edits.
- Registered the `update_message` event type alongside `message` in `zulip_service.py` (`registerMessages`/`manageMessages`) so the queue actually receives edit events.
- Fixed `convert_zulip_upload_links` in `ryzom_service.py`: the regex used to drop the surrounding `[alt](...)`/`![alt](...)` markdown brackets when making `/user_uploads/...` links absolute, leaving a bare `!https://...` that Zulip does not render as an image. The brackets are now preserved.
- In `zulip_fetcher.py`, the `!` is now stripped from image markdown (`![alt](url)` → `[alt](url)`) as soon as a Zulip message is ingested: Zulip still auto-embeds the image from a plain link, and this sidesteps every downstream issue caused by the `!` (DeepL typographic spacing, upload-link conversion, etc.) — this covers the chantier "Protéger la syntaxe d'image markdown avant traduction DeepL" via a simpler mechanism than the originally planned `<x>` tag wrapping.
- Added `RYZOM_DEBUG`-prefixed tracing prints across the pipeline (fetch, dispatch, translate, send) to help diagnose the translation/edit flow in production logs.
