#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <set>
#include <string>
#include <cassert>
#include <iostream>
#include <array>
#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace p4t {

auto constexpr MAX_WIDTH = 32 + 32 + 16 + 16 + 8;

using std::vector;
using std::set;
using std::pair;
using std::string;
using std::tuple;

using std::make_pair;
using std::make_tuple;

auto sink() -> std::shared_ptr<spdlog::sinks::sink> const&;
auto log() -> std::shared_ptr<spdlog::logger> const&;
auto python_log() -> std::shared_ptr<spdlog::logger> const&;

}

#endif // COMMON_H
