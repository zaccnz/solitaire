/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "util/unitbezier.h"

#include <math.h>

UnitBezier unit_bezier_new(float x1, float y1, float x2, float y2)
{
    UnitBezier bezier = {0};

    // Calculate the polynomial coefficients, implicit first and last control points are (0,0) and (1,1).
    bezier.cx = 3.0 * x1;
    bezier.bx = 3.0 * (x2 - x1) - bezier.cx;
    bezier.ax = 1.0 - bezier.cx - bezier.bx;

    bezier.cy = 3.0 * y1;
    bezier.by = 3.0 * (y2 - y1) - bezier.cy;
    bezier.ay = 1.0 - bezier.cy - bezier.by;

    return bezier;
}

float unit_bezier_sample_x(UnitBezier bezier, float t)
{
    // `ax t^3 + bx t^2 + cx t' expanded using Horner's rule.
    return ((bezier.ax * t + bezier.bx) * t + bezier.cx) * t;
}

float unit_bezier_sample_y(UnitBezier bezier, float t)
{
    return ((bezier.ay * t + bezier.by) * t + bezier.cy) * t;
}

float unit_bezier_sample_derivative_x(UnitBezier bezier, float t)
{
    return (3.0 * bezier.ax * t + 2.0 * bezier.bx) * t + bezier.cx;
}

// Given an x value, find a parametric value it came from.
float unit_bezier_solve_x(UnitBezier bezier, float t, float epsilon)
{
    float t0, t1, t2, x2, d2;

    // First try a few iterations of Newton's method -- normally very fast.
    for (int i = 0, t2 = t; i < 8; i++)
    {
        x2 = unit_bezier_sample_x(bezier, t2) - t;
        if (fabs(x2) < epsilon)
            return t2;
        d2 = unit_bezier_sample_derivative_x(bezier, t2);
        if (fabs(d2) < epsilon)
            break;
        t2 = t2 - x2 / d2;
    }

    // Fall back to the bisection method for reliability.
    t0 = 0.0;
    t1 = 1.0;
    t2 = t;

    if (t2 < t0)
    {
        return t0;
    }
    if (t2 > t1)
    {
        return t1;
    }

    while (t0 < t1)
    {
        x2 = unit_bezier_sample_x(bezier, t2);
        if (fabs(x2 - t) < epsilon)
            return t2;
        if (t > x2)
            t0 = t2;
        else
            t1 = t2;
        t2 = (t1 - t0) * 0.5 + t0;
    }

    // Failure.
    return t2;
}

float unit_bezier_solve(UnitBezier bezier, float t, float epsilon)
{
    return unit_bezier_sample_y(bezier, unit_bezier_solve_x(bezier, t, epsilon));
}