#ifndef INSERTION_SEARCH_H
#define INSERTION_SEARCH_H

#include "structures/typedefs.h"
#include "structures/vroom/solution_state.h"
#include "utils/helpers.h"

namespace vroom {
namespace ls {

struct RouteInsertion {
  Eval eval;
  Index single_rank;
  Index pickup_rank;
  Index delivery_rank;
};
constexpr RouteInsertion empty_insert = {NO_EVAL, 0, 0, 0};

template <class Route>
RouteInsertion
compute_best_insertion_single(const Input& input,
                              const utils::SolutionState& sol_state,
                              const Index j,
                              Index v,
                              const Route& route) {
  RouteInsertion result = empty_insert;
  const auto& current_job = input.jobs[j];
  const auto& v_target = input.vehicles[v];

  if (input.vehicle_ok_with_job(v, j)) {
    for (Index rank = sol_state.insertion_ranks_begin[v][j];
         rank < sol_state.insertion_ranks_end[v][j];
         ++rank) {
      Eval current_eval =
        utils::addition_cost(input, j, v_target, route.route, rank);
      if (current_eval.cost < result.eval.cost &&
          v_target.ok_for_travel_time(sol_state.route_evals[v].duration +
                                      current_eval.duration) &&
          route.is_valid_addition_for_capacity(input,
                                               current_job.pickup,
                                               current_job.delivery,
                                               rank) &&
          route.is_valid_addition_for_tw(input, j, rank)) {
        result = {current_eval, rank, 0, 0};
      }
    }
  }
  return result;
}

template <class Route, class Iter>
bool valid_for_capacity(const Input& input,
                        const Route& r,
                        Iter start,
                        Iter end,
                        Index pickup_r,
                        Index delivery_r) {
  Amount amount = input.zero_amount();

  for (auto it = start + 1; it != end - 1; ++it) {
    const auto& new_modified_job = input.jobs[*it];
    if (new_modified_job.type == JOB_TYPE::SINGLE) {
      amount += new_modified_job.delivery;
    }
  }

  return r.is_valid_addition_for_capacity_inclusion(input,
                                                    std::move(amount),
                                                    start,
                                                    end,
                                                    pickup_r,
                                                    delivery_r);
}

template <class Route>
RouteInsertion compute_best_insertion_pd(const Input& input,
                                         const utils::SolutionState& sol_state,
                                         const Index j,
                                         Index v,
                                         const Route& route,
                                         const Eval& cost_threshold) {
  RouteInsertion result = empty_insert;
  const auto& current_job = input.jobs[j];
  const auto& v_target = input.vehicles[v];
  const auto target_travel_time = sol_state.route_evals[v].duration;

  if (!input.vehicle_ok_with_job(v, j)) {
    return result;
  }

  result.eval = cost_threshold;

  // Pre-compute cost of addition for matching delivery.
  std::vector<Eval> d_adds(route.size() + 1);
  std::vector<unsigned char> valid_delivery_insertions(route.size() + 1, false);

  const auto begin_d_rank = sol_state.insertion_ranks_begin[v][j + 1];
  const auto end_d_rank = sol_state.insertion_ranks_end[v][j + 1];

  bool found_valid = false;
  for (unsigned d_rank = begin_d_rank; d_rank < end_d_rank; ++d_rank) {
    d_adds[d_rank] =
      utils::addition_cost(input, j + 1, v_target, route.route, d_rank);
    if (d_adds[d_rank] > result.eval) {
      valid_delivery_insertions[d_rank] = false;
    } else {
      valid_delivery_insertions[d_rank] =
        route.is_valid_addition_for_tw(input, j + 1, d_rank);
    }
    found_valid |= valid_delivery_insertions[d_rank];
  }

  if (!found_valid) {
    result.eval = NO_EVAL;
    return result;
  }

  for (Index pickup_r = sol_state.insertion_ranks_begin[v][j];
       pickup_r < sol_state.insertion_ranks_end[v][j];
       ++pickup_r) {
    Eval p_add =
      utils::addition_cost(input, j, v_target, route.route, pickup_r);
    if (p_add > result.eval) {
      // Even without delivery insertion more expensive than current best.
      continue;
    }

    if (!route.is_valid_addition_for_load(input,
                                          current_job.pickup,
                                          pickup_r) or
        !route.is_valid_addition_for_tw(input, j, pickup_r)) {
      continue;
    }

    // Build replacement sequence for current insertion.
    std::vector<Index> modified_with_pd({j});

    // No need to use begin_d_rank here thanks to
    // valid_delivery_insertions values.
    for (Index delivery_r = pickup_r; delivery_r < end_d_rank; ++delivery_r) {
      // Update state variables along the way before potential
      // early abort.
      if (pickup_r < delivery_r) {
        modified_with_pd.push_back(route.route[delivery_r - 1]);
      }

      if (!(bool)valid_delivery_insertions[delivery_r]) {
        continue;
      }

      Eval pd_eval;
      if (pickup_r == delivery_r) {
        pd_eval = utils::addition_cost(input,
                                       j,
                                       v_target,
                                       route.route,
                                       pickup_r,
                                       pickup_r + 1);
      } else {
        pd_eval = p_add + d_adds[delivery_r];
      }

      if (pd_eval < result.eval &&
          v_target.ok_for_travel_time(target_travel_time + pd_eval.duration)) {
        modified_with_pd.push_back(j + 1);

        // Update best cost depending on validity.
        bool is_valid = valid_for_capacity(input,
                                           route,
                                           modified_with_pd.begin(),
                                           modified_with_pd.end(),
                                           pickup_r,
                                           delivery_r);

        is_valid =
          is_valid && route.is_valid_addition_for_tw(input,
                                                     modified_with_pd.begin(),
                                                     modified_with_pd.end(),
                                                     pickup_r,
                                                     delivery_r);

        modified_with_pd.pop_back();

        if (is_valid) {
          result = {pd_eval, 0, pickup_r, delivery_r};
        }
      }
    }
  }

  assert(result.eval <= cost_threshold);
  if (result.eval == cost_threshold) {
    result.eval = NO_EVAL;
  }
  return result;
}

} // namespace ls
} // namespace vroom
#endif
