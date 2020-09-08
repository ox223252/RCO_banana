#include <stdint.h>
#include "lib/jsonParser/jsonParser.h"
#include "utilActions/actionVars.h"

////////////////////////////////////////////////////////////////////////////////
/// \return the number of cup
////////////////////////////////////////////////////////////////////////////////
uint16_t getNbCups ( void )
{
	double *data = NULL;
	JSON_TYPE type;
	
	jsonGet ( _action_var, 0, "nbVerre", (void**)&data, &type );
	if ( type != jT(double) )
	{
		return ( 0 );
	}
	return ( (uint16_t)*data );
}