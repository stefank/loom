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

#ifndef CPU_AARCH64_SMALLREGISTERMAP_AARCH64_INLINE_HPP
#define CPU_AARCH64_SMALLREGISTERMAP_AARCH64_INLINE_HPP

#include "runtime/frame.inline.hpp"
#include "runtime/registerMap.hpp"

// Java frames don't have callee saved registers (except for rfp), so we can use a smaller RegisterMap
class SmallRegisterMap {
public:
  static constexpr SmallRegisterMap* instance = nullptr;
private:
  static void assert_is_rfp(VMReg r) NOT_DEBUG_RETURN
                                     DEBUG_ONLY({ assert (r == rfp->as_VMReg() || r == rfp->as_VMReg()->next(), "Reg: %s", r->name()); })
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
      if (map->location(r, (intptr_t*)nullptr) != nullptr) assert_is_rfp(r);
    }
  #endif
  }

  inline address location(VMReg reg, intptr_t* sp) const {
    assert_is_rfp(reg);
    return (address)(sp - frame::sender_sp_offset);
  }

  inline void set_location(VMReg reg, address loc) { assert_is_rfp(reg); }

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
  VMReg find_register_spilled_here(void* p, intptr_t* sp) { return rfp->as_VMReg(); }
  void print() const { print_on(tty); }
  void print_on(outputStream* st) const { st->print_cr("Small register map"); }
#endif
};

#endif // CPU_AARCH64_SMALLREGISTERMAP_AARCH64_INLINE_HPP
