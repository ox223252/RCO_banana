#ifndef __ACTIONVARS_H__
#define __ACTIONVARS_H__

#include <stdint.h>
#include "../lib/jsonParser/jsonParser.h"

extern json_el * _action_json; ///< json used to store all parsed file
extern uint32_t _action_jsonLength; ///< length of json array

extern json_el * _action_var; ///< json used to store env vars : aT(get_var)/aT(set_var)
extern uint32_t _action_varLength; ///< length of env json array

#endif