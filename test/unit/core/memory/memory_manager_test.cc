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

#include "core/memory/memory_manager.h"
#include <gtest/gtest.h>

namespace bdm {

TEST(ListTest, PushPopEmpty) {
  List l;

  Node n1;
  Node n2;
  Node n3;
  Node n4;

  n1.next = &n2;
  n2.next = &n3;
  n3.next = &n4;

  EXPECT_TRUE(l.Empty());

  l.Push(&n1, &n3);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n1);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n2);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n3);
  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n4);
  EXPECT_TRUE(l.Empty());

  EXPECT_EQ(l.Pop(), nullptr);
}

// Pop less elements than are in the list
TEST(ListTest, PopNThreadSafe_Less) {
  List l;

  Node n1;
  Node n2;
  Node n3;
  Node n4;

  n1.next = &n2;
  n2.next = &n3;
  n3.next = &n4;

  l.Push(&n1, &n4);

  Node *head = nullptr;
  Node *tail = nullptr;
  l.PopNThreadSafe(3, &head, &tail);

  EXPECT_EQ(head, &n1);
  EXPECT_EQ(tail, &n3);

  EXPECT_FALSE(l.Empty());

  EXPECT_EQ(l.Pop(), &n4);
  EXPECT_TRUE(l.Empty());
}

// Pop more elements than are in the list
TEST(ListTest, PopNThreadSafe_More) {
  List l;

  Node n1;
  Node n2;
  Node n3;
  Node n4;

  n1.next = &n2;
  n2.next = &n3;
  n3.next = &n4;

  l.Push(&n1, &n4);

  Node *head = nullptr;
  Node *tail = nullptr;
  l.PopNThreadSafe(8, &head, &tail);

  EXPECT_EQ(head, &n1);
  EXPECT_EQ(tail, &n4);

  EXPECT_TRUE(l.Empty());
}

// Pop exeaclty the number of elements that are in the list
TEST(ListTest, PopNThreadSafe_Equal) {
  List l;

  Node n1;
  Node n2;
  Node n3;
  Node n4;

  n1.next = &n2;
  n2.next = &n3;
  n3.next = &n4;

  l.Push(&n1, &n4);

  Node *head = nullptr;
  Node *tail = nullptr;
  l.PopNThreadSafe(4, &head, &tail);

  EXPECT_EQ(head, &n1);
  EXPECT_EQ(tail, &n4);

  EXPECT_TRUE(l.Empty());
}

TEST(MemoryManagerTest, New) {
  MemoryManager memory;
  std::cout << memory.New(10) << std::endl;
}

}  // namespace bdm