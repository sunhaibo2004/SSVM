#include "executor/stackmgr.h"
#include <type_traits>

namespace SSVM {
namespace Executor {

/// Getter of top entry. See "include/executor/stackmgr.h".
template <typename T> TypeE<T, ErrCode> StackManager::getTop(T *&Entry) {
  /// Check the size of stack.
  if (Stack.size() == 0)
    return ErrCode::StackEmpty;
  /// Check is the top entry type matched and get pointer.
  try {
    Entry = std::get<T>(Stack.back()).get();
  } catch (std::bad_variant_access E) {
    return ErrCode::StackWrongEntry;
  }
  return ErrCode::Success;
}

/// Push a new frame entry to stack. See "include/executor/stackmgr.h".
template <typename T>
TypeE<T, ErrCode> StackManager::push(std::unique_ptr<T> &&NewEntry) {
  Stack.push_back(std::move(NewEntry));
  /// If is frame or label, record the index in stack.
  if (std::is_same<T, FrameEntry>::value)
    FrameIdx.push_back(Stack.size() - 1);
  if (std::is_same<T, LabelEntry>::value)
    LabelIdx.push_back(Stack.size() - 1);
  return ErrCode::Success;
}

template <typename T> TypeB<T, ErrCode> StackManager::pushValue(T Val) {
  if (!std::is_same<T, uint32_t>::value && !std::is_same<T, uint64_t>::value &&
      !std::is_same<T, float>::value && !std::is_same<T, double>::value) {
    return ErrCode::TypeNotMatch;
  }
  Stack.push_back(std::make_unique<ValueEntry>(Val));
  return ErrCode::Success;
}

/// Pop and return the frame entry. See "include/executor/stackmgr.h".
template <typename T>
TypeE<T, ErrCode> StackManager::pop(std::unique_ptr<T> &Entry) {
  /// Check the size of stack.
  if (Stack.size() == 0)
    return ErrCode::StackEmpty;

  /// Check is the top entry type matched and move pointer.
  try {
    Entry = std::move(std::get<T>(Stack.back()));
  } catch (std::bad_variant_access E) {
    return ErrCode::StackWrongEntry;
  }

  /// Delete the index of this popped entry.
  if (std::is_same<T, FrameEntry>::value)
    FrameIdx.pop_back();
  if (std::is_same<T, LabelEntry>::value)
    LabelIdx.pop_back();
  /// Drop the top entry.
  Stack.pop_back();
  return ErrCode::Success;
}

/// Drop the top entry of stack. See "include/executor/stackmgr.h".
ErrCode StackManager::pop() {
  /// Check the size of stack.
  if (Stack.size() == 0)
    return ErrCode::StackEmpty;
  /// Check is the top entry's type.
  switch (Stack.back().index()) {
  case 0: /// Frame entry
    FrameIdx.pop_back();
    break;
  case 1: /// Label entry
    LabelIdx.pop_back();
    break;
  default:
    break;
  }
  /// Drop the top entry.
  Stack.pop_back();
  return ErrCode::Success;
}

/// Frame related operation. See "include/executor/stackmgr.h".
ErrCode StackManager::getCurrentFrame(FrameEntry *&Frame) {
  /// Check is there current frame.
  if (FrameIdx.size() == 0)
    return ErrCode::WrongInstanceAddress;
  /// Get the current frame pointer.
  Frame = std::get<0>(Stack[FrameIdx.back()]).get();
  return ErrCode::Success;
}

/// Label related operation. See "include/executor/stackmgr.h".
ErrCode StackManager::getLabelWithCount(LabelEntry *&Label,
                                        unsigned int Count = 0) {
  /// Check is there at least count + 1 labels.
  if (LabelIdx.size() < Count + 1)
    return ErrCode::WrongInstanceAddress;
  /// Get the (count + 1)-th top of label.
  unsigned int Idx = LabelIdx.size() - Count - 1;
  Label = std::get<1>(Stack[LabelIdx[Idx]]).get();
  return ErrCode::Success;
}

} // namespace Executor
} // namespace SSVM
