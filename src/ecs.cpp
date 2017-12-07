#include "ecs.h"

namespace ecs {
	std::map<entity_id, component_mask> mask_list;

	entity_id maxEntityId = 0;
}

namespace components {
	ecs::component_mask maxMask = 1;
}