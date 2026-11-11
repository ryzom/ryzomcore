#ifndef NELNS_LOGIN_SERVICE_FUNCTIONS_H
#define NELNS_LOGIN_SERVICE_FUNCTIONS_H

#include <string>

#include <nel/net/unified_network.h>

//
// Functions
//

sint findShardWithSId (NLNET::TServiceId sid);
sint findShard (sint32 shardId);

void displayShards ();
void displayUsers ();
sint findUser (uint32 Id);
void beep (uint freq = 400, uint nb = 2, uint beepDuration = 100, uint pauseDuration = 100);

// transform "192.168.1.1:80" into "192.168.1.1"
std::string removePort (const std::string &addr);

// True when login may be handed out as a new account name: 3 to 64 ASCII
// letters, digits or underscores. The same rule as the web login scripts and
// the account tool (web/public_php/tools/account_name.php). Only for creating
// an account, an existing login is never checked against it.
bool isValidAccountName(const std::string &login);


#endif // NELNS_LOGIN_SERVICE_FUNCTIONS_H
