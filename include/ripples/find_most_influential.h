//===------------------------------------------------------------*- C++ -*-===//
//
//             Ripples: A C++ Library for Influence Maximization
//                  Marco Minutoli <marco.minutoli@pnnl.gov>
//                   Pacific Northwest National Laboratory
//
//===----------------------------------------------------------------------===//
//
// Copyright (c) 2019, Battelle Memorial Institute
// 
// Battelle Memorial Institute (hereinafter Battelle) hereby grants permission
// to any person or entity lawfully obtaining a copy of this software and
// associated documentation files (hereinafter “the Software”) to redistribute
// and use the Software in source and binary forms, with or without
// modification.  Such person or entity may use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and may permit
// others to do so, subject to the following conditions:
// 
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimers.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// 3. Other than as used herein, neither the name Battelle Memorial Institute or
//    Battelle may be used in any form whatsoever without the express written
//    consent of Battelle.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL BATTELLE OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//===----------------------------------------------------------------------===//

#ifndef RIPPLES_FIND_MOST_INFLUENTIAL_H
#define RIPPLES_FIND_MOST_INFLUENTIAL_H

#include <algorithm>
#include <queue>
#include <unordered_set>
#include <vector>

#include <omp.h>
#include "ripples/utility.h"

namespace ripples {
//! Sequential swap ranges.
//!
//! \tparam ItrTy1 The iterator type of the first sequence.
//! \tparam ItrTy2 The iterator type of the second sequence.
//!
//! \param B The begin of the first sequence.
//! \param E The end of the second sequence.
//! \param O The begin of the second sequence.
//! \return The iterator to the one-past last element swapped.
template <typename ItrTy1, typename ItrTy2>
ItrTy2 swap_ranges(ItrTy1 B, ItrTy1 E, ItrTy2 O, sequential_tag) {
  return std::swap_ranges(B, E, O);
}

//! Parallel swap ranges.
//!
//! \tparam ItrTy1 The iterator type of the first sequence.
//! \tparam ItrTy2 The iterator type of the second sequence.
//!
//! \param B The begin of the first sequence.
//! \param E The end of the second sequence.
//! \param O The begin of the second sequence.
//! \return The iterator to the one-past last element swapped.
template <typename ItrTy1, typename ItrTy2>
ItrTy2 swap_ranges(ItrTy1 B, ItrTy1 E, ItrTy2 O, omp_parallel_tag) {
  size_t toBeSwaped = std::distance(B, E);
#pragma omp parallel for
  for (size_t i = 0; i < toBeSwaped; ++i) {
    std::iter_swap(B + i, O + i);
  }
  return O + toBeSwaped;
}

//! Reorder a sequence in such a way that all the element for which a predicate
//! is true preceed the one for which the predicate is false.
//!
//! \tparam ItrTy The type of the iterator of the input sequence.
//! \tparam UnaryPredicate The type of a unary predicate object.
//!
//! \param B The start of the sequence to be partitioned.
//! \param E The end of the sequence to be partitioned.
//! \param P A C++ collable object implementing the predicate.
//! \return An iterator to the first element for which the predicate is false.
template <typename ItrTy, typename UnaryPredicate>
ItrTy partition(ItrTy B, ItrTy E, UnaryPredicate P, sequential_tag) {
  return std::partition(B, E, P);
}

namespace {

template <typename ItrTy, typename ex_tag = omp_parallel_tag>
struct PartitionIndices {
  ItrTy begin;
  ItrTy end;
  ItrTy pivot;

  PartitionIndices(PartitionIndices &&O)
      : begin{std::move(O.begin)},
        end{std::move(O.end)},
        pivot{std::move(O.pivot)} {}

  PartitionIndices &operator=(PartitionIndices &&O) {
    this->begin = std::move(O.begin);
    this->end = std::move(O.end);
    this->pivot = std::move(O.pivot);
    return *this;
  }

  PartitionIndices(const PartitionIndices &O)
      : begin{O.begin}, end{O.end}, pivot{O.pivot} {}

  PartitionIndices &operator=(const PartitionIndices &O) {
    this->begin = O.begin;
    this->end = O.end;
    this->pivot = O.pivot;
    return *this;
  }

  PartitionIndices(ItrTy B, ItrTy E, ItrTy P) : begin{B}, end{E}, pivot{P} {}

  PartitionIndices(ItrTy B, ItrTy E) : PartitionIndices(B, E, E) {}

  bool operator==(const PartitionIndices &O) const {
    return this->begin == O.begin && this->end == O.end &&
           this->pivot == O.pivot;
  }

  PartitionIndices operator+(const PartitionIndices &O) {
    PartitionIndices result(*this);

    if (this->pivot == this->begin && O.pivot == O.begin) {
      result.end = O.end;
      return result;
    } else if (this->pivot == this->end) {
      result.end = O.end;
      result.pivot = O.pivot;
      return result;
    }

    if (std::distance(this->pivot, this->end) <
        std::distance(O.begin, O.pivot)) {
      size_t toBeMoved = std::distance(this->pivot, this->end);
      swap_ranges(this->pivot, this->end, std::prev(O.pivot, toBeMoved),
                  ex_tag{});
      result.pivot = std::prev(O.pivot, toBeMoved);
    } else {
      result.pivot = swap_ranges(O.begin, O.pivot, this->pivot, ex_tag{});
    }
    result.end = O.end;

    return result;
  }
};

}  // namespace

//! Reorder a sequence in such a way that all the element for which a predicate
//! is true preceed the one for which the predicate is false.

//! \tparam ItrTy The type of the iterator of the input sequence.
//! \tparam UnaryPredicate The type of a unary predicate object.
//!
//! \param B The start of the sequence to be partitioned.
//! \param E The end of the sequence to be partitioned.
//! \param P A C++ collable object implementing the predicate.
//! \return An iterator to the first element for which the predicate is false.
template <typename ItrTy, typename UnaryPredicate>
ItrTy partition(ItrTy B, ItrTy E, UnaryPredicate P, omp_parallel_tag) {
  size_t num_threads(1);

#pragma omp single
  { num_threads = omp_get_max_threads(); }

  std::vector<PartitionIndices<ItrTy>> indices(num_threads,
                                               PartitionIndices<ItrTy>(B, E));

#pragma omp parallel
  {
    size_t num_elements = std::distance(B, E);
    size_t threadnum = omp_get_thread_num(), numthreads = omp_get_num_threads();
    size_t low = num_elements * threadnum / numthreads,
           high = num_elements * (threadnum + 1) / numthreads;

    indices[threadnum].begin = B + low;
    indices[threadnum].end = std::min(E, B + high);
    indices[threadnum].pivot =
        std::partition(indices[threadnum].begin, indices[threadnum].end, P);
  }

  for (size_t j = 1; j < num_threads; j <<= 1) {
#pragma omp parallel
    {
#pragma omp single nowait
      for (size_t i = 0; (i + j) < num_threads; i += j * 2) {
#pragma omp task firstprivate(i, j)
        { indices[i] = indices[i] + indices[i + j]; }
      }
    }
  }

  return indices[0].pivot;
}

//! \brief Count the occurrencies of vertices in the RRR sets.
//!
//! \tparam InItr The input sequence iterator type.
//! \tparam OutItr The output sequence iterator type.
//!
//! \param in_begin The begin of the sequence of RRR sets.
//! \param in_end The end of the sequence of RRR sets.
//! \param out_begin The begin of the sequence storing the counters for each
//! vertex.
//! \param out_end The end of the sequence storing the counters for each vertex.
template <typename InItr, typename OutItr>
void CountOccurrencies(InItr in_begin, InItr in_end, OutItr out_begin,
                       OutItr out_end, sequential_tag &&) {
  using rrr_set_type = typename std::iterator_traits<InItr>::value_type;
  using vertex_type = typename rrr_set_type::value_type;
  for (; in_begin != in_end; ++in_begin) {
    std::for_each(in_begin->begin(), in_begin->end(),
                  [&](const vertex_type v) { *(out_begin + v) += 1; });
  }
}

//! \brief Count the occurrencies of vertices in the RRR sets.
//!
//! \tparam InItr The input sequence iterator type.
//! \tparam OutItr The output sequence iterator type.
//!
//! \param in_begin The begin of the sequence of RRR sets.
//! \param in_end The end of the sequence of RRR sets.
//! \param out_begin The begin of the sequence storing the counters for each
//! vertex.
//! \param out_end The end of the sequence storing the counters for each vertex.
template <typename InItr, typename OutItr>
void CountOccurrencies(InItr in_begin, InItr in_end, OutItr out_begin,
                       OutItr out_end, omp_parallel_tag &&) {
  using rrr_set_type = typename std::iterator_traits<InItr>::value_type;
  using vertex_type = typename rrr_set_type::value_type;

#pragma omp parallel
  {
    size_t num_elements = std::distance(out_begin, out_end);
    size_t threadnum = omp_get_thread_num(), numthreads = omp_get_num_threads();
    vertex_type low = num_elements * threadnum / numthreads,
                high = num_elements * (threadnum + 1) / numthreads;

    for (auto itr = in_begin; itr != in_end; ++itr) {
      auto begin = std::lower_bound(itr->begin(), itr->end(), low);
      auto end = std::upper_bound(begin, itr->end(), high - 1);
      std::for_each(begin, end,
                    [&](const vertex_type v) { *(out_begin + v) += 1; });
    }
  }
}

//! \brief Initialize the Heap storage.
//!
//! \tparam InItr The input sequence iterator type.
//! \tparam OutItr The output sequence iterator type.
//!
//! \param in_begin The begin of the sequence of vertex counters.
//! \param in_end The end of the sequence of vertex counters.
//! \param out_begin The begin of the sequence used as storage in the Heap.
//! \param out_end The end of the sequence used as storage in the Heap.
template <typename InItr, typename OutItr>
void InitHeapStorage(InItr in_begin, InItr in_end, OutItr out_begin,
                     OutItr out_end, sequential_tag &&) {
  using value_type = typename std::iterator_traits<OutItr>::value_type;
  using vertex_type = typename value_type::first_type;

  for (vertex_type v = 0; in_begin != in_end; ++in_begin, ++v, ++out_begin) {
    *out_begin = {v, *in_begin};
  }
}

//! \brief Initialize the Heap storage.
//!
//! \tparam InItr The input sequence iterator type.
//! \tparam OutItr The output sequence iterator type.
//!
//! \param in_begin The begin of the sequence of vertex counters.
//! \param in_end The end of the sequence of vertex counters.
//! \param out_begin The begin of the sequence used as storage in the Heap.
//! \param out_end The end of the sequence used as storage in the Heap.
template <typename InItr, typename OutItr>
void InitHeapStorage(InItr in_begin, InItr in_end, OutItr out_begin,
                     OutItr out_end, omp_parallel_tag &&) {
  using value_type = typename std::iterator_traits<OutItr>::value_type;
  using vertex_type = typename value_type::first_type;

#pragma omp parallel for
  for (vertex_type v = 0; v < std::distance(in_begin, in_end); ++v) {
    *(out_begin + v) = {v, *(in_begin + v)};
  }
}

//! \brief Update the coverage counters.
//!
//! \tparam RRRsetsItrTy The iterator type of the sequence of RRR sets.
//! \tparam VertexCoverageVectorTy The type of the vector storing counters.
//!
//! \param B The start sequence of RRRsets covered by the just selected seed.
//! \param E The start sequence of RRRsets covered by the just selected seed.
//! \param vertexCoverage The vector storing the counters to be updated.
template <typename RRRsetsItrTy, typename VertexCoverageVectorTy>
void UpdateCounters(RRRsetsItrTy B, RRRsetsItrTy E,
                    VertexCoverageVectorTy &vertexCoverage, sequential_tag &&) {
  for (; B != E; ++B) {
    for (auto v : *B) {
      vertexCoverage[v] -= 1;
    }
  }
}

//! \brief Update the coverage counters.
//!
//! \tparam RRRsetsItrTy The iterator type of the sequence of RRR sets.
//! \tparam VertexCoverageVectorTy The type of the vector storing counters.
//!
//! \param B The start sequence of RRRsets covered by the just selected seed.
//! \param E The start sequence of RRRsets covered by the just selected seed.
//! \param vertexCoverage The vector storing the counters to be updated.
template <typename RRRsetsItrTy, typename VertexCoverageVectorTy>
void UpdateCounters(RRRsetsItrTy B, RRRsetsItrTy E,
                    VertexCoverageVectorTy &vertexCoverage,
                    omp_parallel_tag &&) {
  for (; B != E; ++B) {
#pragma omp parallel for
    for (size_t j = 0; j < (*B).size(); ++j) {
      vertexCoverage[(*B)[j]] -= 1;
    }
  }
}

//! \brief Select k seeds starting from the a list of Random Reverse
//! Reachability Sets.
//!
//! \tparam GraphTy The graph type.
//! \tparam RRRset The type storing Random Reverse Reachability Sets.
//! \tparam execution_tag The execution policy.
//!
//! \param G The input graph.
//! \param k The size of the seed set.
//! \param RRRsets A vector of Random Reverse Reachability sets.
//! \param ex_tag The execution policy tag.
//!
//! \return a pair where the size_t is the number of RRRset covered and
//! the set of vertices selected as seeds.
template <typename GraphTy, typename RRRset, typename execution_tag>
auto FindMostInfluentialSet(const GraphTy &G, size_t k,
                            std::vector<RRRset> &RRRsets,
                            execution_tag &&ex_tag) {
  using vertex_type = typename GraphTy::vertex_type;

  std::vector<uint32_t> vertexCoverage(G.num_nodes(), 0);

  auto cmp = [](std::pair<vertex_type, size_t> &a,
                std::pair<vertex_type, size_t> &b) {
    return a.second < b.second;
  };
  using priorityQueue =
      std::priority_queue<std::pair<vertex_type, size_t>,
                          std::vector<std::pair<vertex_type, size_t>>,
                          decltype(cmp)>;

  std::vector<std::pair<vertex_type, size_t>> queue_storage(G.num_nodes());

  CountOccurrencies(RRRsets.begin(), RRRsets.end(), vertexCoverage.begin(),
                    vertexCoverage.end(), std::forward<execution_tag>(ex_tag));

  InitHeapStorage(vertexCoverage.begin(), vertexCoverage.end(),
                  queue_storage.begin(), queue_storage.end(),
                  std::forward<execution_tag>(ex_tag));

  priorityQueue queue(cmp, std::move(queue_storage));

  std::vector<typename GraphTy::vertex_type> result;
  result.reserve(k);

  size_t uncovered = RRRsets.size();

  auto end = RRRsets.end();

  while (result.size() < k && uncovered != 0) {
    auto element = queue.top();
    queue.pop();

    if (element.second > vertexCoverage[element.first]) {
      element.second = vertexCoverage[element.first];
      queue.push(element);
      continue;
    }

    uncovered -= element.second;

    auto cmp = [=](const RRRset &a) -> auto {
      return !std::binary_search(a.begin(), a.end(), element.first);
    };

    auto itr = partition(RRRsets.begin(), end, cmp,
                         std::forward<execution_tag>(ex_tag));

    if (std::distance(itr, end) < std::distance(RRRsets.begin(), itr)) {
      UpdateCounters(itr, end, vertexCoverage,
                     std::forward<execution_tag>(ex_tag));
    } else {
      if (std::is_same<execution_tag, omp_parallel_tag>::value) {
#pragma omp parallel for simd
        for (size_t i = 0; i < vertexCoverage.size(); ++i)
          vertexCoverage[i] = 0;
      } else {
        std::fill(vertexCoverage.begin(), vertexCoverage.end(), 0);
      }
      CountOccurrencies(RRRsets.begin(), itr, vertexCoverage.begin(),
                        vertexCoverage.end(),
                        std::forward<execution_tag>(ex_tag));
    }

    end = itr;
    result.push_back(element.first);
  }

  double f = double(RRRsets.size() - uncovered) / RRRsets.size();

  return std::make_pair(f, result);
}

//! \brief Specialization of FindMostInfluentialSet() for CUDA execution.
//!
//! \tparam GraphTy The graph type.
//! \tparam RRRset The type storing Random Reverse Reachability Sets.
//! \tparam execution_tag The execution policy.
//!
//! \param G The input graph.
//! \param k The size of the seed set.
//! \param RRRsets A vector of Random Reverse Reachability sets.
//! \param ex_tag The execution policy tag.
//!
//! \return a pair where the size_t is the number of RRRset covered and
//! the set of vertices selected as seeds.
template <typename GraphTy, typename RRRset>
auto FindMostInfluentialSet(const GraphTy &G, size_t k,
                            std::vector<RRRset> &RRRsets,
                            cuda_parallel_tag &&ex_tag) {
  return FindMostInfluentialSet(G, k, RRRsets, omp_parallel_tag{});
}

}  // namespace ripples

#endif  // RIPPLES_FIND_MOST_INFLUENTIAL_H