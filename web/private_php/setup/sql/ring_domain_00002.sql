ALTER TABLE `characters`
  CHANGE `ring_access` `ring_access` VARCHAR(63) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  CHANGE `race` `race` ENUM('r_fyros','r_matis','r_tryker','r_zorai') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'r_fyros',
  CHANGE `civilisation` `civilisation` ENUM('c_neutral','c_fyros','c_matis','c_tryker','c_zorai') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'c_neutral',
  CHANGE `cult` `cult` ENUM('c_neutral','c_kami','c_karavan') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'c_neutral';

ALTER TABLE `journal_entry`
  CHANGE `type` `type` ENUM('jet_credits','jet_notes') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'jet_notes';

ALTER TABLE `known_users`
  CHANGE `relation_type` `relation_type` ENUM('rt_friend','rt_banned','rt_friend_dm') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'rt_friend';

ALTER TABLE `mfs_mail`
  CHANGE `status` `status` ENUM('ms_new','ms_read','ms_erased') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'ms_new';

ALTER TABLE `ring_users`
  CHANGE `user_type` `user_type` ENUM('ut_character','ut_pioneer') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'ut_character',
  CHANGE `current_activity` `current_activity` ENUM('ca_none','ca_play','ca_edit','ca_anim') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'ca_none',
  CHANGE `current_status` `current_status` ENUM('cs_offline','cs_logged','cs_online') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'cs_offline',
  CHANGE `public_level` `public_level` ENUM('ul_none','ul_public') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'ul_none',
  CHANGE `account_type` `account_type` ENUM('at_normal','at_gold') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'at_normal',
  CHANGE `content_access_level` `content_access_level` VARCHAR(20) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  CHANGE `lang` `lang` ENUM('lang_en','lang_fr','lang_de') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'lang_en',
  CHANGE `cookie` `cookie` VARCHAR(30) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  CHANGE `add_privileges` `add_privileges` VARCHAR(64) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '';

ALTER TABLE `scenario`
  CHANGE `md5` `md5` VARCHAR(64) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  CHANGE `anim_mode` `anim_mode` ENUM('am_dm','am_autonomous') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'am_dm',
  CHANGE `language` `language` VARCHAR(11) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  CHANGE `orientation` `orientation` ENUM('so_newbie_training','so_story_telling','so_mistery','so_hack_slash','so_guild_training','so_other') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'so_other',
  CHANGE `level` `level` ENUM('sl_a','sl_b','sl_c','sl_d','sl_e','sl_f') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'sl_a';

ALTER TABLE `scenario_desc`
  CHANGE `relation_to_parent` `relation_to_parent` ENUM('rtp_same','rtp_variant','rtp_different') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'rtp_same',
  CHANGE `content_access_level` `content_access_level` VARCHAR(20) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '';

ALTER TABLE `session_participant`
  CHANGE `status` `status` ENUM('sps_play_subscribed','sps_play_invited','sps_edit_invited','sps_anim_invited','sps_playing','sps_editing','sps_animating') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'sps_play_subscribed';

ALTER TABLE `sessions`
  CHANGE `session_type` `session_type` ENUM('st_edit','st_anim','st_outland','st_mainland') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'st_edit',
  CHANGE `orientation` `orientation` ENUM('so_newbie_training','so_story_telling','so_mistery','so_hack_slash','so_guild_training','so_other') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'so_other',
  CHANGE `level` `level` ENUM('sl_a','sl_b','sl_c','sl_d','sl_e','sl_f') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'sl_a',
  CHANGE `rule_type` `rule_type` ENUM('rt_strict','rt_liberal') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'rt_strict',
  CHANGE `access_type` `access_type` ENUM('at_public','at_private') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'at_private',
  CHANGE `state` `state` ENUM('ss_planned','ss_open','ss_locked','ss_closed') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'ss_planned',
  CHANGE `estimated_duration` `estimated_duration` ENUM('et_short','et_medium','et_long') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'et_short',
  CHANGE `lang` `lang` VARCHAR(20) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  CHANGE `icone` `icone` VARCHAR(70) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  CHANGE `anim_mode` `anim_mode` ENUM('am_dm','am_autonomous') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'am_dm',
  CHANGE `race_filter` `race_filter` SET('rf_fyros','rf_matis','rf_tryker','rf_zorai') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  CHANGE `religion_filter` `religion_filter` SET('rf_kami','rf_karavan','rf_neutral') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT '',
  CHANGE `guild_filter` `guild_filter` ENUM('gf_only_my_guild','gf_any_player') CHARACTER SET ascii COLLATE ascii_general_ci NULL DEFAULT 'gf_only_my_guild',
  CHANGE `shard_filter` `shard_filter` SET('sf_shard00','sf_shard01','sf_shard02','sf_shard03','sf_shard04','sf_shard05','sf_shard06','sf_shard07','sf_shard08','sf_shard09','sf_shard10','sf_shard11','sf_shard12','sf_shard13','sf_shard14','sf_shard15','sf_shard16','sf_shard17','sf_shard18','sf_shard19','sf_shard20','sf_shard21','sf_shard22','sf_shard23','sf_shard24','sf_shard25','sf_shard26','sf_shard27','sf_shard28','sf_shard29','sf_shard30','sf_shard31') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'sf_shard00,sf_shard01,sf_shard02,sf_shard03,sf_shard04,sf_shard05,sf_shard06,sf_shard07,sf_shard08,sf_shard09,sf_shard10,sf_shard11,sf_shard12,sf_shard13,sf_shard14,sf_shard15,sf_shard16,sf_shard17,sf_shard18,sf_shard19,sf_shard20,sf_shard21,sf_shard22,sf_shard23,sf_shard24,sf_shard25,sf_shard26,sf_shard27,sf_shard28,sf_shard29,sf_shard30,sf_shard31',
  CHANGE `level_filter` `level_filter` SET('lf_a','lf_b','lf_c','lf_d','lf_e','lf_f') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'lf_a,lf_b,lf_c,lf_d,lf_e,lf_f';

ALTER TABLE `shard`
  CHANGE `OldState` `OldState` ENUM('ds_close','ds_dev','ds_restricted','ds_open') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'ds_restricted',
  CHANGE `RequiredState` `RequiredState` ENUM('ds_close','ds_dev','ds_restricted','ds_open') CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL DEFAULT 'ds_dev';

ALTER TABLE `mfs_guild_thread_message` CHANGE `date` `date` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00';
ALTER TABLE `session_log` CHANGE `launch_date` `launch_date` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00';
