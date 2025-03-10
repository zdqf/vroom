#ifndef COST_WRAPPER_H
#define COST_WRAPPER_H

/*

This file is part of VROOM.

Copyright (c) 2015-2022, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/generic/matrix.h"
#include "structures/typedefs.h"

namespace vroom {

class CostWrapper {
private:
  const Duration discrete_duration_factor;
  std::size_t duration_matrix_size;
  const UserDuration* duration_data;

  Cost discrete_cost_factor;
  std::size_t cost_matrix_size;
  const UserCost* cost_data;

  bool _cost_is_duration;

public:
  CostWrapper(double speed_factor);

  void set_durations_matrix(const Matrix<UserDuration>* matrix);

  void set_costs_matrix(const Matrix<UserCost>* matrix,
                        bool reset_cost_factor = false);

  Duration get_discrete_duration_factor() const {
    return discrete_duration_factor;
  }

  bool cost_is_duration() const {
    return _cost_is_duration;
  }

  Duration duration(Index i, Index j) const {
    return discrete_duration_factor *
           static_cast<Duration>(duration_data[i * duration_matrix_size + j]);
  }

  Cost cost(Index i, Index j) const {
    return discrete_cost_factor *
           static_cast<Cost>(cost_data[i * cost_matrix_size + j]);
  }
};

} // namespace vroom

#endif
