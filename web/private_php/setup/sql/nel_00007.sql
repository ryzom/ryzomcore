CREATE TABLE IF NOT EXISTS `setting` (
  `setting` VARCHAR(64) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL,
  `value` VARCHAR(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`setting`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO `setting` (`setting`, `value`) VALUES
  ('admin_privileges', ':DEV:SGM:GM:'),
  ('settings_privilege', ':DEV:'),
  ('default_privileges', ''),
  ('default_access_domains', 'ds_open'),
  ('registration_open', '0')
ON DUPLICATE KEY UPDATE `setting` = `setting`;
