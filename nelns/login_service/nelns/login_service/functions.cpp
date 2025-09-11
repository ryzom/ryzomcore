#include <nelns/login_service/functions.h>

#include <nel/misc/command.h>
#include <nel/misc/debug.h>
#include <nelns/login_service/variables.h>

using std::string;
using NLMISC::ICommand;
using NLMISC::InfoLog;
using NLNET::TServiceId;

sint findShard (sint32 shardId)
{
	for (sint i = 0; i < (sint) Shards.size (); i++)
	{
		if (Shards[i].ShardId == shardId)
		{
			return i;
		}
	}
	// shard not found
	return -1;
}

sint findShardWithSId (TServiceId sid)
{
	for (sint i = 0; i < (sint) Shards.size (); i++)
	{
		if (Shards[i].SId == sid)
		{
			return i;
		}
	}
	// shard not found
	return -1;
}


// transform "192.168.1.1:80" into "192.168.1.1"
string removePort (const string &addr)
{
	return addr.substr (0, addr.find (":"));
}

void checkClients ()
{
	nldebug ("checkClients ()");
	/*for (uint i = 0; i < Users.size (); i++)
	{
		switch (Users[i].State)
		{
		case CUser::Offline:
			nlassert (Users[i].SockId == NULL);
			nlassert (!Users[i].Cookie.isValid());
			nlassert (Users[i].ShardId == NULL);
			break;
		case CUser::Authorized:
			nlassert (Users[i].SockId != NULL);
			nlassert (Users[i].Cookie.isValid());
			nlassert (Users[i].ShardId == NULL);
			break;
		case CUser::Awaiting:
			nlassert (Users[i].SockId == NULL);
			nlassert (!Users[i].Cookie.isValid());
			nlassert (Users[i].ShardId == NULL);
			break;
		case CUser::Online:
			nlassert (Users[i].SockId == NULL);
			nlassert (!Users[i].Cookie.isValid());
			nlassert (Users[i].ShardId != NULL);
			break;
		default:
			nlstop;
			break;
		}
	}*/
}

/*void disconnectClient (CUser &user, bool disconnectClient, bool disconnectShard)
{
	nlinfo ("User %d '%s' need to be disconnect (%d %d %d)", user.Id, user.Login.c_str(), user.State, disconnectClient, disconnectShard);

	switch (user.State)
	{
	case CUser::Offline:
		break;

	case CUser::Authorized:
		if (disconnectClient)
		{
			CNetManager::getNetBase("LS")->disconnect(user.SockId);
		}
		user.Cookie.clear ();
		break;

	case CUser::Awaiting:
		break;

	case CUser::Online:
		if (disconnectShard)
		{
			// ask the WS to disconnect the player from the shard
			CMessage msgout (CNetManager::getNetBase("WSLS")->getSIDA (), "DC");
			msgout.serial (user.Id);
			CNetManager::send ("WSLS", msgout, user.ShardId);
		}
		user.ShardId = NULL;
		break;

	default:
		nlstop;
		break;
	}

	user.SockId = NULL;
	user.State = CUser::Offline;
}
*/
sint findUser (uint32 Id)
{
/*	for (sint i = 0; i < (sint) Users.size (); i++)
	{
		if (Users[i].Id == Id)
		{
			return i;
		}
	}
	// user not found
*/	return -1;
}
/*
string CUser::Authorize (TSockId sender, CCallbackNetBase &netbase)
{
	string reason;

	switch (State)
	{
	case Offline:
		State = Authorized;
		SockId = sender;
		Cookie = CLoginCookie(netbase.hostAddress(sender).internalIPAddress(), Id);
		break;

	case Authorized:
		nlwarning ("user %d already authorized! disconnect him and the other one", Id);
		reason = "You are already authorized (another user uses your account?)";
		disconnectClient (*this, true, true);
		disconnectClient (Users[findUser(Id)], true, true);
		break;

	case Awaiting:
		nlwarning ("user %d already awaiting! disconnect the new user and the other one", Id);
		reason = "You are already awaiting (another user uses your account?)";
		disconnectClient (*this, true, true);
		disconnectClient (Users[findUser(Id)], true, true);
		break;

	case Online:
		nlwarning ("user %d already online! disconnect the new user and the other one", Id);
		reason = "You are already online (another user uses your account?)";
		disconnectClient (*this, true, true);
		disconnectClient (Users[findUser(Id)], true, true);
		break;

	default:
		reason = "default case should never occurs, there's a bug in the login_service.cpp";
		nlstop;
		break;
	}
	return reason;
}
*/
void displayShards ()
{
	ICommand::execute ("shards", *InfoLog);
}

void displayUsers ()
{
	ICommand::execute ("users", *InfoLog);
}

void beep (uint freq, uint nb, uint beepDuration, uint pauseDuration)
{
#ifdef NL_OS_WINDOWS
	try
	{
		if (IService::getInstance()->ConfigFile.getVar ("Beep").asInt() == 1)
		{
			for (uint i = 0; i < nb; i++)
			{
				Beep (freq, beepDuration);
				nlSleep (pauseDuration);
			}
		}
	}
	catch (Exception &)
	{
	}
#endif // NL_OS_WINDOWS
}
