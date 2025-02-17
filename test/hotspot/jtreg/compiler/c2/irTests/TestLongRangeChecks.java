/*
 * Copyright (c) 2021, Red Hat, Inc. All rights reserved.
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
 */

package compiler.c2.irTests;

import compiler.lib.ir_framework.*;
import java.util.Objects;

/*
 * @test
 * @bug 8259609 8276116
 * @summary C2: optimize long range checks in long counted loops
 * @library /test/lib /
 * @run driver compiler.c2.irTests.TestLongRangeChecks
 */

public class TestLongRangeChecks {
    public static void main(String[] args) {
        TestFramework.run();
    }


    @Test
    @IR(counts = { IRNode.LOOP, "1" })
    @IR(failOn = { IRNode.COUNTEDLOOP })
    public static void testStridePosScalePos(long start, long stop, long length, long offset) {
        final long scale = 1;
        final long stride = 1;

        // Loop is first transformed into a loop nest, long range
        // check into an int range check, the range check is hoisted
        // and the inner counted loop becomes empty so is optimized
        // out.
        for (long i = start; i < stop; i += stride) {
            Objects.checkIndex(scale * i + offset, length);
        }
    }

    @Run(test = "testStridePosScalePos")
    private void testStridePosScalePos_runner() {
        testStridePosScalePos(0, 100, 100, 0);
    }

    @Test
    @IR(counts = { IRNode.LOOP, "1" })
    @IR(failOn = { IRNode.COUNTEDLOOP })
    public static void testStridePosScalePosInIntLoop1(int start, int stop, long length, long offset) {
        final long scale = 2;
        final int stride = 1;

        // Same but with int loop
        for (int i = start; i < stop; i += stride) {
            Objects.checkIndex(scale * i + offset, length);
        }
    }

    @Run(test = "testStridePosScalePosInIntLoop1")
    private void testStridePosScalePosInIntLoop1_runner() {
        testStridePosScalePosInIntLoop1(0, 100, 200, 0);
    }

    @Test
    @IR(counts = { IRNode.LOOP, "1" })
    @IR(failOn = { IRNode.COUNTEDLOOP })
    public static void testStridePosScalePosInIntLoop2(int start, int stop, long length, long offset) {
        final int scale = 2;
        final int stride = 1;

        // Same but with int loop
        for (int i = start; i < stop; i += stride) {
            Objects.checkIndex(scale * i + offset, length);
        }
    }

    @Run(test = "testStridePosScalePosInIntLoop2")
    private void testStridePosScalePosInIntLoop2_runner() {
        testStridePosScalePosInIntLoop2(0, 100, 200, 0);
    }

    @Test
    @IR(counts = { IRNode.LOOP, "1"})
    @IR(failOn = { IRNode.COUNTEDLOOP})
    public static void testStrideNegScaleNeg(long start, long stop, long length, long offset) {
        final long scale = -1;
        final long stride = 1;
        for (long i = stop; i > start; i -= stride) {
            Objects.checkIndex(scale * i + offset, length);
        }
    }

    @Run(test = "testStrideNegScaleNeg")
    private void testStrideNegScaleNeg_runner() {
        testStrideNegScaleNeg(0, 100, 100, 100);
    }

    @Test
    @IR(counts = { IRNode.LOOP, "1" })
    @IR(failOn = { IRNode.COUNTEDLOOP })
    public static void testStrideNegScaleNegInIntLoop1(int start, int stop, long length, long offset) {
        final long scale = -2;
        final int stride = 1;

        for (int i = stop; i > start; i -= stride) {
            Objects.checkIndex(scale * i + offset, length);
        }
    }

    @Run(test = "testStrideNegScaleNegInIntLoop1")
    private void testStrideNegScaleNegInIntLoop1_runner() {
        testStrideNegScaleNegInIntLoop1(0, 100, 200, 200);
    }

    @Test
    @IR(counts = { IRNode.LOOP, "1" })
    @IR(failOn = { IRNode.COUNTEDLOOP })
    public static void testStrideNegScaleNegInIntLoop2(int start, int stop, long length, long offset) {
        final int scale = -2;
        final int stride = 1;

        for (int i = stop; i > start; i -= stride) {
            Objects.checkIndex(scale * i + offset, length);
        }
    }

    @Run(test = "testStrideNegScaleNegInIntLoop2")
    private void testStrideNegScaleNegInIntLoop2_runner() {
        testStrideNegScaleNegInIntLoop2(0, 100, 200, 200);
    }

    @Test
    @IR(counts = { IRNode.LOOP, "1"})
    @IR(failOn = { IRNode.COUNTEDLOOP})
    public static void testStrideNegScalePos(long start, long stop, long length, long offset) {
        final long scale = 1;
        final long stride = 1;
        for (long i = stop-1; i >= start; i -= stride) {
            Objects.checkIndex(scale * i + offset, length);
        }
    }

    @Run(test = "testStrideNegScalePos")
    private void testStrideNegScalePos_runner() {
        testStrideNegScalePos(0, 100, 100, 0);
    }

    @Test
    @IR(counts = { IRNode.LOOP, "1" })
    @IR(failOn = { IRNode.COUNTEDLOOP })
    public static void testStrideNegScalePosInIntLoop1(int start, int stop, long length, long offset) {
        final long scale = 2;
        final int stride = 1;
        for (int i = stop-1; i >= start; i -= stride) {
            Objects.checkIndex(scale * i + offset, length);
        }
    }

    @Run(test = "testStrideNegScalePosInIntLoop1")
    private void testStrideNegScalePosInIntLoop1_runner() {
        testStrideNegScalePosInIntLoop1(0, 100, 200, 0);
    }

    @Test
    @IR(counts = { IRNode.LOOP, "1" })
    @IR(failOn = { IRNode.COUNTEDLOOP })
    public static void testStrideNegScalePosInIntLoop2(int start, int stop, long length, long offset) {
        final int scale = 2;
        final int stride = 1;
        for (int i = stop-1; i >= start; i -= stride) {
            Objects.checkIndex(scale * i + offset, length);
        }
    }

    @Run(test = "testStrideNegScalePosInIntLoop2")
    private void testStrideNegScalePosInIntLoop2_runner() {
        testStrideNegScalePosInIntLoop1(0, 100, 200, 0);
    }

    @Test
    @IR(counts = { IRNode.LOOP, "1"})
    @IR(failOn = { IRNode.COUNTEDLOOP})
    public static void testStridePosScaleNeg(long start, long stop, long length, long offset) {
        final long scale = -1;
        final long stride = 1;
        for (long i = start; i < stop; i += stride) {
            Objects.checkIndex(scale * i + offset, length);
        }
    }

    @Run(test = "testStridePosScaleNeg")
    private void testStridePosScaleNeg_runner() {
        testStridePosScaleNeg(0, 100, 100, 99);
    }

    @Test
    @IR(counts = { IRNode.LOOP, "1"})
    @IR(failOn = { IRNode.COUNTEDLOOP})
    public static void testStridePosScaleNegInIntLoop1(int start, int stop, long length, long offset) {
        final long scale = -2;
        final int stride = 1;
        for (int i = start; i < stop; i += stride) {
            Objects.checkIndex(scale * i + offset, length);
        }
    }

    @Run(test = "testStridePosScaleNegInIntLoop1")
    private void testStridePosScaleNegInIntLoop1_runner() {
        testStridePosScaleNegInIntLoop1(0, 100, 200, 198);
    }

    @Test
    @IR(counts = { IRNode.LOOP, "1"})
    @IR(failOn = { IRNode.COUNTEDLOOP})
    public static void testStridePosScaleNegInIntLoop2(int start, int stop, long length, long offset) {
        final int scale = -2;
        final int stride = 1;
        for (int i = start; i < stop; i += stride) {
            Objects.checkIndex(scale * i + offset, length);
        }
    }

    @Run(test = "testStridePosScaleNegInIntLoop2")
    private void testStridePosScaleNegInIntLoop2_runner() {
        testStridePosScaleNegInIntLoop1(0, 100, 200, 198);
    }
}
