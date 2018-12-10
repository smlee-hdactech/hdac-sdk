#ifndef RAWMETADATA_H
#define RAWMETADATA_H

#include <json_spirit/json_spirit.h>
#include "script/script.h"
class mc_EntityDetails;

CScript ParseRawMetadata(json_spirit::Value param,uint32_t allowed_objects,mc_EntityDetails *given_entity,mc_EntityDetails *found_entity);

#endif // RAWMETADATA_H
