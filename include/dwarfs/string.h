/* vim:set ts=2 sw=2 sts=2 et: */
/**
 * \author     Marcus Holland-Moritz (github@mhxnet.de)
 * \copyright  Copyright (c) Marcus Holland-Moritz
 *
 * This file is part of dwarfs.
 *
 * dwarfs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dwarfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dwarfs.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <iterator>
#include <type_traits>
#include <utility>

#include <range/v3/algorithm/copy.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/split.hpp>

namespace dwarfs {

template <typename T, typename Input, typename Delim>
auto split_view(Input&& input, Delim&& delim) {
  return std::forward<Input>(input) |
         ranges::views::split(std::forward<Delim>(delim)) |
         ranges::views::transform([](auto&& rng) {
           return T(&*rng.begin(), ranges::distance(rng));
         });
}

template <typename R, typename Input, typename Delim>
R split_to(Input&& input, Delim&& delim) {
  return split_view<typename R::value_type>(std::forward<Input>(input),
                                            std::forward<Delim>(delim)) |
         ranges::to<R>;
}

template <typename Input, typename Delim, typename Container>
void split_to(Input&& input, Delim&& delim, Container& container) {
  ranges::copy(split_view<typename Container::value_type>(
                   std::forward<Input>(input), std::forward<Delim>(delim)),
               std::inserter(container, container.end()));
}

} // namespace dwarfs
