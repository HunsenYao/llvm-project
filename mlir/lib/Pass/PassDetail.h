//===- PassDetail.h - MLIR Pass details -------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef MLIR_PASS_PASSDETAIL_H_
#define MLIR_PASS_PASSDETAIL_H_

#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"

namespace mlir {
namespace detail {

//===----------------------------------------------------------------------===//
// Verifier Pass
//===----------------------------------------------------------------------===//

/// Pass to verify an operation and signal failure if necessary.
class VerifierPass : public PassWrapper<VerifierPass, OperationPass<>> {
  void runOnOperation() override;
};

//===----------------------------------------------------------------------===//
// OpToOpPassAdaptor
//===----------------------------------------------------------------------===//

/// An adaptor pass used to run operation passes over nested operations.
class OpToOpPassAdaptor
    : public PassWrapper<OpToOpPassAdaptor, OperationPass<>> {
public:
  OpToOpPassAdaptor(OpPassManager &&mgr);
  OpToOpPassAdaptor(const OpToOpPassAdaptor &rhs) = default;

  /// Run the held pipeline over all operations.
  void runOnOperation() override;

  /// Merge the current pass adaptor into given 'rhs'.
  void mergeInto(OpToOpPassAdaptor &rhs);

  /// Returns the pass managers held by this adaptor.
  MutableArrayRef<OpPassManager> getPassManagers() { return mgrs; }

  /// Populate the set of dependent dialects for the passes in the current
  /// adaptor.
  void getDependentDialects(DialectRegistry &dialects) const override;

  /// Return the async pass managers held by this parallel adaptor.
  MutableArrayRef<SmallVector<OpPassManager, 1>> getParallelPassManagers() {
    return asyncExecutors;
  }

  /// Returns the adaptor pass name.
  std::string getAdaptorName();

private:
  /// Run this pass adaptor synchronously.
  void runOnOperationImpl();

  /// Run this pass adaptor asynchronously.
  void runOnOperationAsyncImpl();

  /// Run the given operation and analysis manager on a single pass.
  static LogicalResult run(Pass *pass, Operation *op, AnalysisManager am);

  /// Run the given operation and analysis manager on a provided op pass
  /// manager.
  static LogicalResult
  runPipeline(iterator_range<OpPassManager::pass_iterator> passes,
              Operation *op, AnalysisManager am);

  /// A set of adaptors to run.
  SmallVector<OpPassManager, 1> mgrs;

  /// A set of executors, cloned from the main executor, that run asynchronously
  /// on different threads. This is used when threading is enabled.
  SmallVector<SmallVector<OpPassManager, 1>, 8> asyncExecutors;

  // For accessing "runPipeline".
  friend class mlir::PassManager;
};

} // end namespace detail
} // end namespace mlir
#endif // MLIR_PASS_PASSDETAIL_H_
