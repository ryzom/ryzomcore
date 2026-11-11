ALTER TABLE `permission`
  DROP INDEX `UserShard`,
  ADD UNIQUE KEY `UserDomainShard` (`UId`,`DomainId`,`ShardId`) USING BTREE;
