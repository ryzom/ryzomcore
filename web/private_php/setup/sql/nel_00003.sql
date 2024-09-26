ALTER TABLE `shard` DROP `prim` ;
ALTER TABLE `shard` ADD PRIMARY KEY ( `ShardId` ) ;
ALTER TABLE `shard` DROP `PatchURL` ;
ALTER TABLE `shard` DROP `DynPatchURL` ;
ALTER TABLE `shard` DROP `ClientApplication` ;

