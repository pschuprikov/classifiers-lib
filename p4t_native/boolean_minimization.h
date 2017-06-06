#ifndef BOOLEAN_MINIMIZATION_H
#define BOOLEAN_MINIMIZATION_H

#include "filter.h"

namespace p4t {
    void perform_boolean_minimization(vector<Filter> * filters, vector<int> * actions);
}

#endif
