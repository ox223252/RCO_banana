#ifndef __ACTIONEXTRACT_H__
#define __ACTIONEXTRACT_H__

#include <stdbool.h>
#include <stdint.h>

#include "../lib/jsonParser/jsonParser.h"

////////////////////////////////////////////////////////////////////////////////
/// \fn int getStepId ( const json_el * const data, uint32_t * const stepId );
/// \prama[in] data : json element where you search
/// \param[in/out] stepId : step number in the strategie main array
/// \brief this function will convert the index in the strategie array into an 
///     id in the data array, this id should be used to feed getActionId func.
/// \note stepId is used as input and output
/// \return 0 : ok, -1 : no data remaining, else error see errno
////////////////////////////////////////////////////////////////////////////////
int getStepId ( const json_el * const data, uint32_t * const stepId );

////////////////////////////////////////////////////////////////////////////////
/// \fn int getActionId ( const json_el * const data, const uint32_t stepId, 
///     uint32_t * const actionId );
/// \prama[in] data : json element where you search
/// \param[in] stepId : step index in the main data array
/// \param[in/out] actionId : action number in the step main array
/// \brief this function will convert the index in the step array into an id in
///     the data array, this id should be used to feed getActionId func.
/// \note actionId is used as input and output
/// \return 0 : ok, -1 : no data remaining, else error see errno
////////////////////////////////////////////////////////////////////////////////
int getActionId ( const json_el * const data, const uint32_t stepId, 
	uint32_t * const actionId );

////////////////////////////////////////////////////////////////////////////////
/// \fn int getNextActions ( const json_el * const data, 
///     const uint32_t actionId, uint32_t ** const out, uint32_t * const length, 
///     bool timeout );
/// \param[in] data : json element where you search
/// \param[in] actionId : action index in the data main array
/// \param[out] out : table of id of next els
/// \param[out] length : size of table in elements
/// \param[in] timeout : search for timeout or not
/// \brief return a table of id  of the nexts actions
/// \note don't forget to free out table after use
/// \return 0 : ok, -1 : no data remaining, else error see errno
////////////////////////////////////////////////////////////////////////////////
int getNextActions ( const json_el * const data, const uint32_t actionId, 
	uint32_t ** const out, uint32_t * const length, bool timeout );

////////////////////////////////////////////////////////////////////////////////
/// \fn int getActionParams ( const json_el * const data, const uint32_t action,
///     json_el ** out, uint32_t * outLength );
/// \prama[in] data : json element where you search
/// \param[in] actionId : action index in the data main array
/// \param[out] out : json object containing parameters of action
/// \param[out] outLength : nb of element in ac
/// \brief this function will extract params form action associated to actionId
///     and return JSON
/// \note *out should be null, else you will probably have memory leak
/// \return 0 : ok, -1 : no data remaining, else error see errno
////////////////////////////////////////////////////////////////////////////////
int getActionParams ( const json_el * const data, const uint32_t actionId, 
	json_el ** out, uint32_t * outLength );

#endif