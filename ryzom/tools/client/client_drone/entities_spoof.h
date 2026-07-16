#ifndef ENTITIES_SPOOF_H
#define ENTITIES_SPOOF_H

#include "nel/net/module.h"
#include "network_connection.h"
#include "cdb_synchronised.h"

/**
// Spoof of client CEntityCL class
 */
class CEntityCL
{
public:

	CEntityCL() : _Pos(NLMISC::CVectorD::Null) {}

	const NLMISC::CVectorD&	pos() const { return _Pos; }
	void					pos(const NLMISC::CVectorD &vect) { _Pos = vect; }
	const NLMISC::CVector&	front() const { return _Front; }
	void					front(const NLMISC::CVector &vect)	{ _Front = vect; }
	float					frontYaw() const {return (float)atan2(front().y, front().x);}
	const NLMISC::CVector&	dir() const { return _Dir; }
	void					dir(const NLMISC::CVector &vect) { _Dir = vect; }
	bool					isInitialized() const { return ! _Pos.isNull(); }

private:
	NLMISC::CVectorD	_Pos;
	NLMISC::CVector		_Front, _Dir;
};

/**
// Spoof of client CUserEntity class
 */
class CUserEntity : public CEntityCL
{
public:
	void					sendToServer(NLMISC::CBitMemStream &out);
};

#endif	// ENTITIES_SPOOF_H
