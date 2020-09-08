#include "actionVars.h"

json_el * _action_json = NULL; ///< json used to store all parsed file
uint32_t _action_jsonLength = 0; ///< length of json array

json_el * _action_var = NULL; ///< json used to store env vars : aT(get_var)/aT(set_var)
uint32_t _action_varLength = 0; ///< length of env json array
