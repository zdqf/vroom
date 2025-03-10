#ifndef BREAK_H
#define BREAK_H

/*

This file is part of VROOM.

Copyright (c) 2015-2022, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <string>

#include "structures/typedefs.h"
#include "structures/vroom/time_window.h"

namespace vroom {

struct Break {
  Id id;
  std::vector<TimeWindow> tws;
  Duration service;
  std::string description;

  Break(Id id,
        const std::vector<TimeWindow>& tws,
        UserDuration service = 0,
        const std::string& description = "");

  bool is_valid_start(Duration time) const;
};

} // namespace vroom

#endif
