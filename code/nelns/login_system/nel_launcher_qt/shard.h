#ifndef NL_LAUNCHER_SHARD_H
#define NL_LAUNCHER_SHARD_H

struct CShard
{
	CShard(const std::string &version, bool online, uint32 shardId, const std::string &name, uint32 nbPlayers, const std::string &wsAddr, const std::string &patchURL)
	{
		Version = version;
		Online = online;
		ShardId = shardId;
		Name = name;
		NbPlayers = nbPlayers;
		WsAddr = wsAddr;
		PatchURL = patchURL;
	}

	std::string Version;
	bool Online;
	uint32 ShardId;
	std::string Name;
	uint32 NbPlayers;
	std::string WsAddr;
	std::string PatchURL;
};


#endif // NL_LAUNCHER_SHARD_H
