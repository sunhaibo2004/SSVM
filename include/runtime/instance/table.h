// SPDX-License-Identifier: Apache-2.0
//===-- ssvm/runtime/instance/table.h - Table Instance definition ---------===//
//
// Part of the SSVM Project.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the table instance definition in store manager.
///
//===----------------------------------------------------------------------===//
#pragma once

#include "common/ast/type.h"
#include "common/errcode.h"
#include "common/types.h"
#include "support/log.h"
#include "support/span.h"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace SSVM {
namespace Runtime {
namespace Instance {

class TableInstance {
public:
  TableInstance() = delete;
  TableInstance(const ElemType &Elem, const AST::Limit &Lim)
      : Type(Elem), HasMaxSize(Lim.hasMax()), MinSize(Lim.getMin()),
        MaxSize(Lim.getMax()), Elem(MinSize) {}
  virtual ~TableInstance() = default;

  /// Getter of element type.
  ElemType getElementType() const { return Type; }

  /// Get table size
  uint32_t getSize() const noexcept { return Elem.size(); }

  /// Getter of limit definition.
  bool getHasMax() const { return HasMaxSize; }

  /// Getter of limit definition.
  uint32_t getMin() const { return MinSize; }

  /// Getter of limit definition.
  uint32_t getMax() const { return MaxSize; }

  /// Set the function index initialization list.
  Expect<void> setInitList(const uint32_t Offset, Span<const uint32_t> Addrs) {
    /// Boundary checked during validation.
    std::transform(Addrs.begin(), Addrs.end(), Elem.begin() + Offset,
                   [](uint32_t Addr) -> std::pair<uint32_t, uint32_t> {
                     return {UINT32_C(1), Addr};
                   });
    return {};
  }

  /// Check is out of bound.
  bool checkAccessBound(uint32_t Offset, uint32_t Length) const noexcept {
    const uint64_t AccessLen =
        static_cast<uint64_t>(Offset) + static_cast<uint64_t>(Length);
    return AccessLen <= MinSize;
  }

  /// Get boundary index.
  uint32_t getBoundIdx() const noexcept {
    return ((MinSize > 0) ? (MinSize - 1) : 0);
  }

  /// Grow table
  bool growTable(const uint32_t Count) {
    uint32_t MaxTableCaped = 65536;
    if (getHasMax()) {
      MaxTableCaped = std::min(getMax(), MaxTableCaped);
    }
    if (Count + Elem.size() > MaxTableCaped) {
      return false;
    }
    Elem.resize(Count + Elem.size());
    if (Symbol) {
      *Symbol = Elem.data();
    }
    return true;
  }

  /// Get the elem address.
  Expect<uint32_t> getElemAddr(const uint32_t Idx) const {
    if (Idx >= Elem.size()) {
      LOG(ERROR) << ErrCode::UndefinedElement;
      LOG(ERROR) << ErrInfo::InfoBoundary(Idx, 1, getBoundIdx());
      return Unexpect(ErrCode::UndefinedElement);
    }
    if (Elem[Idx].first != 0) {
      return Elem[Idx].second;
    } else {
      LOG(ERROR) << ErrCode::UninitializedElement;
      return Unexpect(ErrCode::UninitializedElement);
    }
  }

  /// Getter of symbol
  const auto &getSymbol() const noexcept { return Symbol; }
  /// Setter of symbol
  void setSymbol(DLSymbol<std::pair<uint32_t, uint32_t> *> S) noexcept {
    Symbol = std::move(S);
    *Symbol = Elem.data();
  }

private:
  /// \name Data of table instance.
  /// @{
  const ElemType Type;
  const bool HasMaxSize;
  const uint32_t MinSize = 0;
  const uint32_t MaxSize = 0;
  std::vector<std::pair<uint32_t, uint32_t>> Elem;
  DLSymbol<std::pair<uint32_t, uint32_t> *> Symbol;
  /// @}
};

} // namespace Instance
} // namespace Runtime
} // namespace SSVM
