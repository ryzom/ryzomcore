# --------------------------------------------------------
# Host:                         94.23.202.75
# Database:                     nel
# Server version:               5.1.37-1ubuntu5.1
# Server OS:                    debian-linux-gnu
# HeidiSQL version:             5.0.0.3272
# Date/time:                    2010-05-08 15:31:21
# --------------------------------------------------------
USE `nel`;

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
# Dumping data for table nel.domain: 8 rows
/*!40000 ALTER TABLE `domain` DISABLE KEYS */;
INSERT IGNORE INTO `domain` (`domain_id`, `domain_name`, `status`, `patch_version`, `backup_patch_url`, `patch_urls`, `login_address`, `session_manager_address`, `ring_db_name`, `web_host`, `web_host_php`, `description`) VALUES (12, 'ryzom_open', 'ds_open', 610, 'http://open.ryzom.com:23001', NULL, 'open.ryzom.com:49998', 'open.ryzom.com:49999', 'ring_open', 'open.ryzom.com:30000', 'open.ryzom.com:40916', 'Open Domain');
/*!40000 ALTER TABLE `domain` ENABLE KEYS */;

# Dumping data for table nel.shard: 17 rows
/*!40000 ALTER TABLE `shard` DISABLE KEYS */;
INSERT IGNORE INTO `shard` (`ShardId`, `domain_id`, `WsAddr`, `NbPlayers`, `Name`, `Online`, `ClientApplication`, `Version`, `PatchURL`, `DynPatchURL`, `FixedSessionId`, `State`, `MOTD`, `prim`) VALUES (302, 12, 'open.ryzom.com', 0, 'Open Shard', 0, 'ryzom_open', '', '', '', 0, 'ds_dev', '', 30);
/*!40000 ALTER TABLE `shard` ENABLE KEYS */;
# --------------------------------------------------------
# Host:                         94.23.202.75
# Database:                     ring_open
# Server version:               5.1.37-1ubuntu5.1
# Server OS:                    debian-linux-gnu
# HeidiSQL version:             5.0.0.3272
# Date/time:                    2010-05-08 15:31:22
# --------------------------------------------------------
USE `ring_open`;

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
# Dumping data for table ring_open.sessions: 1 rows
/*!40000 ALTER TABLE `sessions` DISABLE KEYS */;
INSERT IGNORE INTO `sessions` (`session_id`, `session_type`, `title`, `owner`, `plan_date`, `start_date`, `description`, `orientation`, `level`, `rule_type`, `access_type`, `state`, `host_shard_id`, `subscription_slots`, `reserved_slots`, `free_slots`, `estimated_duration`, `final_duration`, `folder_id`, `lang`, `icone`, `anim_mode`, `race_filter`, `religion_filter`, `guild_filter`, `shard_filter`, `level_filter`, `subscription_closed`, `newcomer`) VALUES (302, 'st_mainland', 'open shard mainland', 0, '2005-09-21 12:41:33', '2005-08-31 00:00:00', '', 'so_other', 'sl_a', 'rt_strict', 'at_public', 'ss_planned', 0, 0, 0, 0, 'et_short', 0, 0, 'lang_en', '', 'am_dm', 'rf_fyros,rf_matis,rf_tryker,rf_zorai', 'rf_kami,rf_karavan,rf_neutral', 'gf_any_player', '', 'lf_a,lf_b,lf_c,lf_d,lf_e,lf_f', 0, 0);
/*!40000 ALTER TABLE `sessions` ENABLE KEYS */;

# Dumping data for table ring_open.shard: 1 rows
/*!40000 ALTER TABLE `shard` DISABLE KEYS */;
INSERT IGNORE INTO `shard` (`shard_id`, `WSOnline`, `MOTD`, `OldState`, `RequiredState`) VALUES (302, 1, 'Shard up', 'ds_restricted', 'ds_open');
/*!40000 ALTER TABLE `shard` ENABLE KEYS */;
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
