//===-- tsan_interface_ann.cc -----------------------------------*- C++ -*-===//
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
#include "tsan_interface_ann.h"
#include "tsan_mutex.h"
#include "tsan_placement_new.h"
#include "tsan_report.h"
#include "tsan_rtl.h"
#include "tsan_slab.h"

namespace __tsan {

static const int kMaxDescLen = 128;

struct ExpectRace {
  ExpectRace *next;
  ExpectRace *prev;
  int hitcount;
  uptr addr;
  char *file;
  int line;
  char desc[kMaxDescLen];
};

struct DynamicAnnContext {
  Mutex mtx;
  SlabAlloc expect_alloc;
  SlabCache expect_slab;
  ExpectRace expect;

  DynamicAnnContext()
    : expect_alloc(sizeof(ExpectRace))
    , expect_slab(&expect_alloc) {
  }
};

static DynamicAnnContext *dyn_ann_ctx;
static char dyn_ann_ctx_placeholder[sizeof(DynamicAnnContext)] ALIGN(64);

void InitializeDynamicAnnotations() {
  dyn_ann_ctx = new(dyn_ann_ctx_placeholder) DynamicAnnContext;
  dyn_ann_ctx->expect.next = &dyn_ann_ctx->expect;
  dyn_ann_ctx->expect.prev = &dyn_ann_ctx->expect;
}

bool IsExceptReport(uptr addr) {
  Lock lock(&dyn_ann_ctx->mtx);
  for (ExpectRace *race = dyn_ann_ctx->expect.next;
      race != &dyn_ann_ctx->expect; race = race->next) {
    if (race->addr == addr) {
      DPrintf("Hit expected race: %s addr=%p %s:%d\n",
          race->desc, race->addr, race->file, race->line);
      race->hitcount++;
      return true;
    }
  }
  return false;
}

}  // namespace __tsan

using namespace __tsan;  // NOLINT

extern "C" {
void AnnotateHappensBefore(char *f, int l, uptr *addr) {
}

void AnnotateHappensAfter(char *f, int l, uptr cv) {
}

void AnnotateCondVarSignal(char *f, int l, uptr cv) {
}

void AnnotateCondVarSignalAll(char *f, int l, uptr cv) {
}

void AnnotateMutexIsNotPHB(char *f, int l, uptr mu) {
}

void AnnotateCondVarWait(char *f, int l, uptr cv, uptr lock) {
}

void AnnotateRWLockCreate(char *f, int l, uptr lock) {
}

void AnnotateRWLockDestroy(char *f, int l, uptr lock) {
}

void AnnotateRWLockAcquired(char *f, int l, uptr lock, uptr is_w) {
}

void AnnotateRWLockReleased(char *f, int l, uptr lock, uptr is_w) {
}

void AnnotateTraceMemory(char *f, int l, uptr mem) {
}

void AnnotateFlushState(char *f, int l) {
}

void AnnotateNewMemory(char *f, int l, uptr mem, uptr size) {
}

void AnnotateNoOp(char *f, int l, uptr mem) {
}

static void ReportMissedExpectedRace(ExpectRace *race) {
  Printf("==================\n");
  Printf("WARNING: ThreadSanitizer: missed expected data race\n");
  Printf("  %s addr=%p %s:%d\n",
      race->desc, race->addr, race->file, race->line);
  Printf("==================\n");
}

void AnnotateFlushExpectedRaces(char *f, int l) {
  Lock lock(&dyn_ann_ctx->mtx);
  while (dyn_ann_ctx->expect.next != &dyn_ann_ctx->expect) {
    ExpectRace *race = dyn_ann_ctx->expect.next;
    if (race->hitcount == 0)
      ReportMissedExpectedRace(race);
    race->prev->next = race->next;
    race->next->prev = race->prev;
    dyn_ann_ctx->expect_slab.Free(race);
  }
}

void AnnotateEnableRaceDetection(char *f, int l, int enable) {
}

void AnnotateMutexIsUsedAsCondVar(char *f, int l, uptr mu) {
}

void AnnotatePCQGet(char *f, int l, uptr pcq) {
}

void AnnotatePCQPut(char *f, int l, uptr pcq) {
}

void AnnotatePCQDestroy(char *f, int l, uptr pcq) {
}

void AnnotatePCQCreate(char *f, int l, uptr pcq) {
}

void AnnotateExpectRace(char *f, int l, uptr mem, char *desc) {
  Lock lock(&dyn_ann_ctx->mtx);
  ExpectRace *race = (ExpectRace*)dyn_ann_ctx->expect_slab.Alloc();
  race->hitcount = 0;
  race->addr = mem;
  race->file = f;
  race->line = l;
  race->desc[0] = 0;
  if (desc) {
    int i = 0;
    for (; i < kMaxDescLen - 1 && desc[i]; i++)
      race->desc[i] = desc[i];
    race->desc[i] = 0;
  }
  race->prev = &dyn_ann_ctx->expect;
  race->next = dyn_ann_ctx->expect.next;
  race->next->prev = race;
  dyn_ann_ctx->expect.next = race;
  DPrintf("Add expected race: %s addr=%p %s:%d\n",
      race->desc, race->addr, race->file, race->line);
}

void AnnotateBenignRace(char *f, int l, uptr mem, char *desc) {
}

void AnnotateBenignRaceSized(char *f, int l, uptr mem, uptr size, char *desc) {
}

void AnnotateIgnoreReadsBegin(char *f, int l) {
}

void AnnotateIgnoreReadsEnd(char *f, int l) {
}

void AnnotateIgnoreWritesBegin(char *f, int l) {
}

void AnnotateIgnoreWritesEnd(char *f, int l) {
}

void AnnotatePublishMemoryRange(char *f, int l, uptr addr, uptr size) {
}

void AnnotateUnpublishMemoryRange(char *f, int l, uptr addr, uptr size) {
}

void AnnotateThreadName(char *f, int l, char *name) {
}

void WTFAnnotateHappensBefore(char *f, int l, uptr addr) {
}

void WTFAnnotateHappensAfter(char *f, int l, uptr addr) {
}

void WTFAnnotateBenignRaceSized(char *f, int l, uptr mem, uptr sz, char *desc) {
}

int RunningOnValgrind() {
  return 1;
}
}  // extern "C"