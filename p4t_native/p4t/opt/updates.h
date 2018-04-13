#ifndef UPDATES_H
#define UPDATES_H

#include <p4t/common.h>
#include <p4t/model/filter.h>
#include <p4t/model/support.h>

namespace p4t::opt {

auto incremental_oi_lpm(
        vector<model::Filter> const& new_filters,
        vector<pair<vector<model::Filter>, model::Support>> groups,
        vector<model::Filter> traditional,
        size_t max_traditional_size
    ) -> pair<size_t, size_t>;
}

#endif
