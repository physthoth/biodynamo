// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "core/execution_context/in_place_exec_ctxt.h"

#include "core/grid.h"
#include "core/resource_manager.h"
#include "core/sim_object/sim_object.h"

namespace bdm {

InPlaceExecutionContext::InPlaceExecutionContext()
    : tinfo_(ThreadInfo::GetInstance()) {}

InPlaceExecutionContext::~InPlaceExecutionContext() {
  for (auto& el : new_sim_objects_) {
    delete el.second;
  }
}

void InPlaceExecutionContext::SetupIterationAll(
    const std::vector<InPlaceExecutionContext*>& all_exec_ctxts) const {
  // first iteration might have uncommited changes
  TearDownIterationAll(all_exec_ctxts);
}

void InPlaceExecutionContext::TearDownIterationAll(
    const std::vector<InPlaceExecutionContext*>& all_exec_ctxts) const {
  // group execution contexts by numa domain
  std::vector<uint64_t> new_so_per_numa(tinfo_->GetNumaNodes());
  std::vector<uint64_t> thread_offsets(tinfo_->GetMaxThreads());

  for (int tid = 0; tid < tinfo_->GetMaxThreads(); ++tid) {
    auto* ctxt = all_exec_ctxts[tid];
    int nid = tinfo_->GetNumaNode(tid);
    thread_offsets[tid] = new_so_per_numa[nid];
    new_so_per_numa[nid] += ctxt->new_sim_objects_.size();
  }

  // reserve enough memory in ResourceManager
  std::vector<uint64_t> numa_offsets(tinfo_->GetNumaNodes());
  auto* rm = Simulation::GetActive()->GetResourceManager();
  for (unsigned n = 0; n < new_so_per_numa.size(); n++) {
    numa_offsets[n] = rm->GrowSoContainer(new_so_per_numa[n], n);
  }

// add new_sim_objects_ to the ResourceManager in parallel
#pragma omp parallel for schedule(static, 1)
  for (int i = 0; i < tinfo_->GetMaxThreads(); i++) {
    auto* ctxt = all_exec_ctxts[i];
    int nid = tinfo_->GetNumaNode(i);
    uint64_t offset = thread_offsets[i] + numa_offsets[nid];
    rm->AddNewSimObjects(nid, offset, ctxt->new_sim_objects_);
    ctxt->new_sim_objects_.clear();
  }

  // remove
  for (int i = 0; i < tinfo_->GetMaxThreads(); i++) {
    auto* ctxt = all_exec_ctxts[i];
    // removed sim objects
    // remove them after adding new ones (maybe one has been removed
    // that was in new_sim_objects_)
    for (auto& uid : ctxt->remove_) {
      rm->Remove(uid);
    }
    ctxt->remove_.clear();
  }
}

void InPlaceExecutionContext::Execute(
    SimObject* so, const std::vector<Operation>& operations) {
  auto* grid = Simulation::GetActive()->GetGrid();
  auto nb_mutex_builder = grid->GetNeighborMutexBuilder();
  if (nb_mutex_builder != nullptr) {
    auto mutex = nb_mutex_builder->GetMutex(so->GetBoxIdx());
    std::lock_guard<decltype(mutex)> guard(mutex);
    neighbor_cache_.clear();
    for (auto& op : operations) {
      op(so);
    }
  } else {
    neighbor_cache_.clear();
    for (auto& op : operations) {
      op(so);
    }
  }
}

void InPlaceExecutionContext::push_back(SimObject* new_so) {  // NOLINT
  new_sim_objects_[new_so->GetUid()] = new_so;
}

void InPlaceExecutionContext::ForEachNeighbor(
    const std::function<void(const SimObject*)>& lambda,
    const SimObject& query) {
  // use values in cache
  if (neighbor_cache_.size() != 0) {
    for (auto& pair : neighbor_cache_) {
      lambda(pair.first);
    }
    return;
  }

  auto* grid = Simulation::GetActive()->GetGrid();
  grid->ForEachNeighbor(lambda, query);
}

struct Fen : public FenFunc {
  const Param* param = Simulation::GetActive()->GetParam();
  std::vector<std::pair<const SimObject*, double>>& neighbor_cache_;
  FenFunc& function_;

  Fen(FenFunc& function, std::vector<std::pair<const SimObject*, double>>& neigbor_cache)
   : function_(function), neighbor_cache_(neigbor_cache)
  {}

  void operator()(const SimObject* so, double squared_distance) override {
    if (param->cache_neighbors_) {
      neighbor_cache_.push_back(make_pair(so, squared_distance));
    }
    function_(so, squared_distance);
  }
};

void InPlaceExecutionContext::ForEachNeighbor(
    FenFunc& lambda,
    const SimObject& query) {
  // use values in cache
  if (neighbor_cache_.size() != 0) {
    for (auto& pair : neighbor_cache_) {
      lambda(pair.first, pair.second);
    }
    return;
  }

  // forward call to grid and populate cache
  auto* grid = Simulation::GetActive()->GetGrid();
  Fen for_each(lambda, neighbor_cache_);
  grid->ForEachNeighbor(for_each, query);
}

struct FenWithinRadius : public FenFunc {
  const Param* param = Simulation::GetActive()->GetParam();
  std::vector<std::pair<const SimObject*, double>>& neighbor_cache_;
  double squared_radius_ = 0;
  FenFunc& function_;

  FenWithinRadius(FenFunc& function, std::vector<std::pair<const SimObject*, double>>& neigbor_cache, double squared_radius)
   : function_(function), neighbor_cache_(neigbor_cache), squared_radius_(squared_radius)
  {}

  void operator()(const SimObject* so, double squared_distance) override {
    if (param->cache_neighbors_) {
      neighbor_cache_.push_back(make_pair(so, squared_distance));
    }
    if (squared_distance < squared_radius_) {
      function_(so, 0);
    }
  }
};

void InPlaceExecutionContext::ForEachNeighborWithinRadius(
    FenFunc& lambda, const SimObject& query,
    double squared_radius) {
  // use values in cache
  if (neighbor_cache_.size() != 0) {
    for (auto& pair : neighbor_cache_) {
      if (pair.second < squared_radius) {
        lambda(pair.first, 0);
      }
    }
    return;
  }

  // forward call to grid and populate cache
  auto* grid = Simulation::GetActive()->GetGrid();
  // struct {
  //   const Param* param = Simulation::GetActive()->GetParam();
  //   InPlaceExecutionContext* ctxt;
  //   double sradius;
  //   const std::function<void(const SimObject*)>* lambda;
  // } data;
  // data.ctxt = this;
  // data.sradius = squared_radius;
  // data.lambda = &lambda;
  //
  // auto for_each = [&data](const SimObject* so, double squared_distance) {
  //   if (data.param->cache_neighbors_) {
  //     data.ctxt->neighbor_cache_.push_back(make_pair(so, squared_distance));
  //   }
  //   if (squared_distance < data.sradius) {
  //     (*data.lambda)(so);
  //   }
  // };
  FenWithinRadius for_each(lambda, neighbor_cache_, squared_radius);
  grid->ForEachNeighbor(for_each, query);
}

SimObject* InPlaceExecutionContext::GetSimObject(SoUid uid) {
  auto* so = GetCachedSimObject(uid);
  if (so != nullptr) {
    return so;
  }

  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  so = rm->GetSimObject(uid);
  if (so != nullptr) {
    return so;
  }

  // sim object must be cached in another InPlaceExecutionContext
  for (auto* ctxt : sim->GetAllExecCtxts()) {
    so = ctxt->GetCachedSimObject(uid);
    if (so != nullptr) {
      return so;
    }
  }
  return nullptr;
}

const SimObject* InPlaceExecutionContext::GetConstSimObject(SoUid uid) {
  return GetSimObject(uid);
}

void InPlaceExecutionContext::RemoveFromSimulation(SoUid uid) {
  remove_.push_back(uid);
}

void InPlaceExecutionContext::DisableNeighborGuard() {
  Simulation::GetActive()->GetGrid()->DisableNeighborMutexes();
}

SimObject* InPlaceExecutionContext::GetCachedSimObject(SoUid uid) {
  SimObject* ret_val = nullptr;
  auto search_it = new_sim_objects_.find(uid);
  if (search_it != new_sim_objects_.end()) {
    ret_val = search_it->second;
  }
  return ret_val;
}

// TODO(lukas) Add tests for caching mechanism in ForEachNeighbor*

}  // namespace bdm
