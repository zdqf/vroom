/*

This file is part of VROOM.

Copyright (c) 2015-2022, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/vroom/break.h"
#include "utils/helpers.h"

namespace vroom {

Break::Break(Id id,
             const std::vector<TimeWindow>& tws,
             UserDuration service,
             const std::string& description)
  : id(id),
    tws(tws),
    service(utils::scale_from_user_duration(service)),
    description(description) {
  utils::check_tws(tws);
}

bool Break::is_valid_start(Duration time) const {
  bool valid = false;

  for (const auto& tw : tws) {
    if (tw.contains(time)) {
      valid = true;
      break;
    }
  }

  return valid;
}

} // namespace vroom
