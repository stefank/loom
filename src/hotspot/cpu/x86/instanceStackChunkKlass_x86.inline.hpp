/*
 * Copyright (c) 2019, 2022, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef CPU_X86_INSTANCESTACKCHUNKKLASS_X86_INLINE_HPP
#define CPU_X86_INSTANCESTACKCHUNKKLASS_X86_INLINE_HPP

#include "interpreter/oopMapCache.hpp"
#include "runtime/frame.inline.hpp"
#include "runtime/registerMap.hpp"

int InstanceStackChunkKlass::metadata_words() { return frame::sender_sp_offset; }
int InstanceStackChunkKlass::align_wiggle()   { return 1; }

#ifdef ASSERT
template <chunk_frames frame_kind>
inline bool StackChunkFrameStream<frame_kind>::is_in_frame(void* p0) const {
  assert (!is_done(), "");
  intptr_t* p = (intptr_t*)p0;
  int argsize = is_compiled() ? (_cb->as_compiled_method()->method()->num_stack_arg_slots() * VMRegImpl::stack_slot_size) >> LogBytesPerWord : 0;
  int frame_size = _cb->frame_size() + argsize;
  return p == sp() - frame::sender_sp_offset || ((p - unextended_sp()) >= 0 && (p - unextended_sp()) < frame_size);
}
#endif

template <chunk_frames frame_kind>
inline frame StackChunkFrameStream<frame_kind>::to_frame() const {
  if (is_done()) return frame(_sp, _sp, nullptr, nullptr, nullptr, nullptr, true);
  return frame_kind == chunk_frames::MIXED && !is_interpreted()
    ? frame(sp(), unextended_sp(), fp(), pc(), cb(), _oopmap) // we might freeze deoptimized frame in slow mode
    : frame(sp(), unextended_sp(), fp(), pc(), cb(), _oopmap, true);
}

template <chunk_frames frame_kind>
inline address StackChunkFrameStream<frame_kind>::get_pc() const {
  assert (!is_done(), "");
  return *(address*)(_sp - 1);
}

template <chunk_frames frame_kind>
inline intptr_t* StackChunkFrameStream<frame_kind>::fp() const {
  intptr_t* fp_addr = _sp - frame::sender_sp_offset;
  return (frame_kind == chunk_frames::MIXED && is_interpreted())
    ? fp_addr + *fp_addr // derelativize
    : *(intptr_t**)fp_addr;
}

template <chunk_frames frame_kind>
inline intptr_t* StackChunkFrameStream<frame_kind>::derelativize(int offset) const {
  intptr_t* fp = this->fp();
  assert (fp != nullptr, "");
  return fp + fp[offset];
}

template <chunk_frames frame_kind>
inline intptr_t* StackChunkFrameStream<frame_kind>::unextended_sp_for_interpreter_frame() const {
  assert (frame_kind == chunk_frames::MIXED && is_interpreted(), "");
  return derelativize(frame::interpreter_frame_last_sp_offset);
}

// template <chunk_frames frame_kind>
// inline intptr_t* StackChunkFrameStream<frame_kind>::unextended_sp_for_interpreter_frame_caller() const {
//   assert (frame_kind == chunk_frames::MIXED, "");
//   intptr_t* callee_fp = sp() - frame::sender_sp_offset;
//   intptr_t* unextended_sp = callee_fp + callee_fp[frame::interpreter_frame_sender_sp_offset];
//   assert (unextended_sp > callee_fp && unextended_sp >= sp(), "callee_fp: %p (%d) offset: %ld", callee_fp, _chunk->to_offset(callee_fp), callee_fp[frame::interpreter_frame_sender_sp_offset]);
//   return unextended_sp;
// }

template <chunk_frames frame_kind>
intptr_t* StackChunkFrameStream<frame_kind>::next_sp_for_interpreter_frame() const {
  assert (frame_kind == chunk_frames::MIXED && is_interpreted(), "");
  return (derelativize(frame::interpreter_frame_locals_offset) + 1 >= _end) ? _end : fp() + frame::sender_sp_offset;
}

template <chunk_frames frame_kind>
inline void StackChunkFrameStream<frame_kind>::next_for_interpreter_frame() {
  assert (frame_kind == chunk_frames::MIXED && is_interpreted(), "");
  if (derelativize(frame::interpreter_frame_locals_offset) + 1 >= _end) {
    _unextended_sp = _end;
    _sp = _end;
  } else {
    intptr_t* fp = this->fp();
    _unextended_sp = fp + fp[frame::interpreter_frame_sender_sp_offset];
    _sp = fp + frame::sender_sp_offset;
  }
}

template <chunk_frames frame_kind>
inline int StackChunkFrameStream<frame_kind>::interpreter_frame_size() const {
  assert (frame_kind == chunk_frames::MIXED && is_interpreted(), "");
  // InterpreterOopMap mask;
  // to_frame().interpreted_frame_oop_map(&mask);
  // intptr_t* top = derelativize(frame::interpreter_frame_initial_sp_offset) - mask.expression_stack_size();

  intptr_t* top = unextended_sp(); // later subtract argsize if callee is interpreted
  intptr_t* bottom = derelativize(frame::interpreter_frame_locals_offset) + 1; // the sender's unextended sp: derelativize(frame::interpreter_frame_sender_sp_offset);
  return (int)(bottom - top);
}

template <chunk_frames frame_kind>
inline int StackChunkFrameStream<frame_kind>::interpreter_frame_stack_argsize() const {
  assert (frame_kind == chunk_frames::MIXED && is_interpreted(), "");
  int diff = (int)(derelativize(frame::interpreter_frame_locals_offset) - derelativize(frame::interpreter_frame_sender_sp_offset) + 1);
  return diff;
}

template <chunk_frames frame_kind>
inline int StackChunkFrameStream<frame_kind>::interpreter_frame_num_oops() const {
  assert (frame_kind == chunk_frames::MIXED && is_interpreted(), "");
  ResourceMark rm;
  InterpreterOopMap mask;
  frame f = to_frame();
  f.interpreted_frame_oop_map(&mask);
  return  mask.num_oops()
        + 1 // for the mirror oop
        + ((intptr_t*)f.interpreter_frame_monitor_begin()
            - (intptr_t*)f.interpreter_frame_monitor_end())/BasicObjectLock::size();
}

inline void stackChunkOopDesc::relativize_frame_pd(frame& fr) const {
  if (fr.is_interpreted_frame()) fr.set_offset_fp(relativize_address(fr.fp()));
}

inline void stackChunkOopDesc::derelativize_frame_pd(frame& fr) const {
  if (fr.is_interpreted_frame()) fr.set_fp(derelativize_address(fr.offset_fp()));
}

template<>
template<>
inline void StackChunkFrameStream<chunk_frames::MIXED>::update_reg_map_pd(RegisterMap* map) {
  if (map->update_map()) {
    frame::update_map_with_saved_link(map, map->in_cont() ? (intptr_t**)(intptr_t)frame::sender_sp_offset
                                                          : (intptr_t**)(_sp - frame::sender_sp_offset));
  }
}

template<>
template<>
inline void StackChunkFrameStream<chunk_frames::COMPILED_ONLY>::update_reg_map_pd(RegisterMap* map) {
  if (map->update_map()) {
    frame::update_map_with_saved_link(map, map->in_cont() ? (intptr_t**)(intptr_t)frame::sender_sp_offset
                                                          : (intptr_t**)(_sp - frame::sender_sp_offset));
  }
}

template <chunk_frames frame_kind>
template <typename RegisterMapT>
inline void StackChunkFrameStream<frame_kind>::update_reg_map_pd(RegisterMapT* map) {}

// Java frames don't have callee saved registers (except for rbp), so we can use a smaller RegisterMap
class SmallRegisterMap {
public:
  static constexpr SmallRegisterMap* instance = nullptr;
private:
  static void assert_is_rbp(VMReg r) NOT_DEBUG_RETURN
                                     DEBUG_ONLY({ assert (r == rbp->as_VMReg() || r == rbp->as_VMReg()->next(), "Reg: %s", r->name()); })
public:
  // as_RegisterMap is used when we didn't want to templatize and abstract over RegisterMap type to support SmallRegisterMap
  // Consider enhancing SmallRegisterMap to support those cases
  const RegisterMap* as_RegisterMap() const { return nullptr; }
  RegisterMap* as_RegisterMap() { return nullptr; }

  RegisterMap* copy_to_RegisterMap(RegisterMap* map, intptr_t* sp) const {
    map->clear();
    map->set_include_argument_oops(this->include_argument_oops());
    frame::update_map_with_saved_link(map, (intptr_t**)sp - frame::sender_sp_offset);
    return map;
  }

  SmallRegisterMap() {}

  SmallRegisterMap(const RegisterMap* map) {
  #ifdef ASSERT
    for(int i = 0; i < RegisterMap::reg_count; i++) {
      VMReg r = VMRegImpl::as_VMReg(i);
      if (map->location(r, (intptr_t*)nullptr) != nullptr) assert_is_rbp(r);
    }
  #endif
  }

  inline address location(VMReg reg, intptr_t* sp) const {
    assert_is_rbp(reg);
    return (address)(sp - frame::sender_sp_offset);
  }

  inline void set_location(VMReg reg, address loc) { assert_is_rbp(reg); }

  JavaThread* thread() const {
  #ifndef ASSERT
    guarantee (false, "unreachable");
  #endif
    return nullptr;
  }

  bool update_map()    const { return false; }
  bool walk_cont()     const { return false; }
  bool include_argument_oops() const { return false; }
  void set_include_argument_oops(bool f)  {}
  bool in_cont()       const { return false; }
  stackChunkHandle stack_chunk() const { return stackChunkHandle(); }

#ifdef ASSERT
  bool should_skip_missing() const  { return false; }
  VMReg find_register_spilled_here(void* p, intptr_t* sp) { return rbp->as_VMReg(); }
  void print() const { print_on(tty); }
  void print_on(outputStream* st) const { st->print_cr("Small register map"); }
#endif
};

#endif // CPU_X86_INSTANCESTACKCHUNKKLASS_X86_INLINE_HPP
