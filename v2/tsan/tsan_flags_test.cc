//===-- tsan_flags_test.cc --------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of ThreadSanitizer (TSan), a race detector.
//
//===----------------------------------------------------------------------===//
#include "tsan_flags.h"
#include "gtest/gtest.h"

namespace __tsan {

TEST(Flags, Basic) {
  // At least should not crash.
  Flags f = {};
  InitializeFlags(&f, 0);
  InitializeFlags(&f, "");
}

TEST(Flags, Parse) {
  Flags f = {};

  f.enable_annotations = false;
  InitializeFlags(&f, "enable_annotations");
  EXPECT_EQ(f.enable_annotations, true);

  f.enable_annotations = false;
  InitializeFlags(&f, "--enable_annotations");
  EXPECT_EQ(f.enable_annotations, true);

  f.enable_annotations = false;
  InitializeFlags(&f, "--enable_annotations=1");
  EXPECT_EQ(f.enable_annotations, true);

  f.enable_annotations = true;
  InitializeFlags(&f, "enable_annotations=0");
  EXPECT_EQ(f.enable_annotations, false);

  f.enable_annotations = true;
  InitializeFlags(&f, "--enable_annotations=0");
  EXPECT_EQ(f.enable_annotations, false);
}

}  // namespace __tsan
