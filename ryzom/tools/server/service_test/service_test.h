/** \file service_test.h
 * <File description>
 *
 * $Id$
 */




#ifndef GD_SERVICE_TEST_H
#define GD_SERVICE_TEST_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/net/service.h"

#include "front_end_property_receiver.h"

// Callback called at service connexion
void cbServiceUp( const std::string& serviceName, uint16 serviceId, void * );

// Callback called at service down
void cbServiceDown( const std::string& serviceName, uint16 serviceId, void * );

/**
 * <Class description>
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CServiceTest : public NLNET::IService
{
public:
	// Initialisation of service
	void init (void) NL_OVERRIDE;

	// Update net processing 
	bool update (void) NL_OVERRIDE;

	// Update service processing
	void serviceUpdate(void);

	// Release the service
	void release (void) NL_OVERRIDE;
};

#endif // GD_SERVICE_TEST_H


/* End of service_test.h */
