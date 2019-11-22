#ifndef __API_CONFIG_H__
#define __API_CONFIG_H__

#include <stdint.h>

struct api_config
{
    uint32_t num_trustees;
    uint32_t threshold;
    uint32_t subgroup_order;
    char *  election_meta;
};

#endif /* __API_CONFIG_H__ */
