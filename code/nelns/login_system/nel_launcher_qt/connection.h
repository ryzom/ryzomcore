#ifndef NL_LAUNCHER_CONNECTION_H
#define NL_LAUNCHER_CONNECTION_H

#include <string>
#include <vector>

#include <nel/net/tcp_sock.h>

#include "shard.h"

typedef std::vector<CShard> TShardList;

class CNelLauncherConnection
{
public:
	bool connect();
	bool send(const std::string &url);
	bool receive(std::string &res);
	std::string checkLogin(const std::string &login, const std::string &password, const std::string &clientApp);
	std::string selectShard(uint32 shardId, std::string &cookie, std::string &addr);

	TShardList getShards() { return m_Shards; }

protected:
	std::string		m_Login;
	std::string		m_Password;
	std::string		m_ClientApp;

	TShardList		m_Shards;

	NLNET::CTcpSock		m_Sock;
};

#endif // NL_LAUNCHER_CONNECTION_H
