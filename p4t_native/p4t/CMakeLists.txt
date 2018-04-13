add_library(p4t_native SHARED 
    common.cpp

    model/support.cpp

    opt/chain_algos.cpp
    opt/oi_algos.cpp
    opt/boolean_minimization.cpp
    opt/expansion_algos.cpp
    opt/distribution_algos.cpp
    opt/intersections_opt.cpp
    opt/updates.cpp

    utils/python_utils.cpp

    p4t_native.cpp 
    p4t_native_ext.cpp
    )

set_target_properties(p4t_native PROPERTIES PREFIX "")
target_link_libraries(p4t_native ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
