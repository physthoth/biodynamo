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

#include "core/util/spinlock.h"
#include <gtest/gtest.h>

namespace bdm {

TEST(SpinlockTest, All) {
  Spinlock lock;

  EXPECT_TRUE(lock.try_lock());
  EXPECT_FALSE(lock.try_lock());

  lock.unlock();
  EXPECT_TRUE(lock.try_lock());

  lock.unlock();
  lock.lock();
  EXPECT_FALSE(lock.try_lock());
  lock.unlock();
}

}  // namespace bdm
