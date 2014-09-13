CREATE TABLE IF NOT EXISTS `settings` (
`idSettings` int(11) NOT NULL,
  `Setting` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `Value` varchar(32) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

INSERT INTO `settings` (`idSettings`, `Setting`, `Value`) VALUES
(1, 'userRegistration', '0');

ALTER TABLE `settings`
 ADD PRIMARY KEY (`idSettings`), ADD UNIQUE KEY `idSettings` (`idSettings`), ADD KEY `idSettings_2` (`idSettings`);

ALTER TABLE `settings`
MODIFY `idSettings` int(11) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=2;