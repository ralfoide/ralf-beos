/*
---------------------------------
From: Pierre Raynaud-Richard.
Be, Inc.
800 El Camino Real, Suite 300
Menlo Park CA 94025
work: 415-462-4101
fax:  415-462-4129
pierre@be.com
http://www.be.com
---------------------------------
*/

/***************************************************************************
/
/          FastMath, inline version 0.9, Copyright 1997 Be,Inc.
/
/**************************************************************************/

/* Subject to the terms and conditions of this Agreement, Be hereby grants
   licensee a nonexclusive, worldwide, perpetual right and license, to use,
   copy, modify, and prepare derivative works of the Software with the
   Modifications, if any, in source and object code form solely as
   incorporated into or bundled with other Software.

   Subject to the rights and licenses granted to you, the licensee, hereunder,
   Be shall retain all rights, title and interest in and to Fastmath, and all
   patents, copyrights, trade secrets and other intellectual property rights
   therein. The above shall in no way be construed to grant the licensee rights
   or licenses to Modifications or to any software bundled with the BeOS. */

#ifndef _FAST_MATH_H
#define _FAST_MATH_H

/* Those tables are used by the function inl_b_atan_1. You need to include a
   copy of them in each application/library which wants to use b_atan_1 as an
   inline function. You can do that by adding "#define _B_ATAN_1" explicitely
   before your "#include <fastmath.h>", in each source file where you want
   to use it. Or you can declare one copy as a global of your application. */

#ifdef _B_ATAN_1
#undef _B_ATAN_1

/*static*/ float b_ty[40] = {
0.012499349, 0.037482437, 0.062418811, 0.087277710, 0.112028956,
0.136643171, 0.161091909, 0.185347959, 0.209385410, 0.233179823,
0.256708354, 0.279949844, 0.302884907, 0.325495899, 0.347767025,
0.369684339, 0.391235620, 0.412410498, 0.433200210, 0.453597754,
0.473597556, 0.493195683, 0.512389481, 0.531177580, 0.549559951,
0.567537546, 0.585112393, 0.602287293, 0.619065940, 0.635452867,
0.651453078, 0.667072237, 0.682316423, 0.697192133, 0.711706162,
0.725865781, 0.739678204, 0.753151119, 0.766292036, 0.779108763 };

/*static*/ float b_ttany2[40] = {
0.999843774, 0.998595725, 0.996108949, 0.992401923, 0.987501930,
0.981444563, 0.974273098, 0.966037729, 0.956794729, 0.946605520,
0.935535729, 0.923654195, 0.911032010, 0.897741601, 0.883855800,
0.869447056, 0.854586691, 0.839344220, 0.823786799, 0.807978742,
0.791981158, 0.775851603, 0.759643917, 0.743408077, 0.727190122,
0.711032151, 0.694972366, 0.679045162, 0.663281262, 0.647707813,
0.632348707, 0.617224541, 0.602353071, 0.587749252, 0.573425474,
0.559391830, 0.545656239, 0.532224718, 0.519101511, 0.506289259 };

/*static*/ float b_ttany[40] = {
0.0125, 0.0375, 0.0625, 0.0875, 0.1125, 0.1375, 0.1625, 0.1875,
0.2125, 0.2375, 0.2625, 0.2875, 0.3125, 0.3375, 0.3625, 0.3875,
0.4125, 0.4375, 0.4625, 0.4875, 0.5125, 0.5375, 0.5625, 0.5875,
0.6125, 0.6375, 0.6625, 0.6875, 0.7125, 0.7375, 0.7625, 0.7875,
0.8125, 0.8375, 0.8625, 0.8875, 0.9125, 0.9375, 0.9625, 0.9875 };

#else
extern float b_ty[40];
extern float b_ttany2[40];
extern float b_ttany[40];

#endif

/* inline version of b_cos_90 : cosinus for 0 <= x <= Pi/2 */
inline float inl_b_cos_90(float x) {
        float   t, x4, x6;

        x = x*x;
        x4 = x*x;
        t = (float)(1.414213538)-x*(float)(1.414213538/8.0);
        x6 = x4*x;
        x = (float)(1.414213538/46080.0)-x*(float)(1.414213538/10321920.0);
        t += (float)(1.414213538/384.0)*x4;
        t -= x6*x;
        return t*t-(float)1.0;
}

/* inline version of b_sqrt : square root for x > 0 */
inline float inl_b_sqrt(float x) {
        unsigned long    val;
        float            y, z, t;
        float            flottant, tampon;

        flottant = x;
        val = *((unsigned long*)&flottant);
        val -= 0x3f800000L;
        val >>= 1;
        val += 0x3f800000L;
        val &= 0x7fffffffL;
        *((unsigned long*)&tampon) = val;
        y = tampon;
        z = y*y+x;
        t = y*y-x;
        y *= (float)4.0;
        x = z*z;
        t = t*t;
        y = z*y;
        t = (float)2.0*x-t;
        return  t/y;
}

/* inline version of b_sqrt_inv : inverse square root for x > 0 */
inline float inl_b_sqrt_inv(float x) {
        unsigned long    val;
        float            y, z, t;
        float            flottant, tampon;

        flottant = x;
        val = *((unsigned long*)&flottant);
        val -= 0x3f800000L;
        val >>= 1;
        val += 0x3f800000L;
        val &= 0x7fffffffL;
        *((unsigned long*)&tampon) = val;
        y = tampon;
        z = y*y+x;
        t = y*y-x;
        y *= (float)4.0;
        x = z*z;
        t = t*t;
        y = z*y;
        t = (float)2.0*x-t;
        return  y/t;
}

/* inline version of b_tan_45 : tangent for 0 <= x <= Pi/4 */
inline float inl_b_tan_45(float x) {
        float   t, x4, x6;
        float   y, v, y4, y6;
        float   c0, c1;

        y = (float)1.570793826-x;
        x = x*x;
        y = y*y;
        x4 = x*x;
        y4 = y*y;
        c0 = (float)(1.414213538);
        c1 = (float)(1.414213538/8.0);
        t = c0-x*c1;
        v = c0-y*c1;
        x6 = x4*x;
        y6 = y4*y;
        c0 = (float)(1.414213538/46080.0);
        c1 = (float)(1.414213538/10321920.0);
        x = c0-x*c1;
        y = c0-y*c1;
        c0 = (float)(1.414213538/384.0);
        t += c0*x4;
        v += c0*y4;
        t -= x6*x;
        v -= y6*y;
        c0 = (float)1.0;
        t = t*t-c0;
        v = v*v-c0;
        return v/t;
}

/* inline version of b_atan_1 : arctanget for 0 <= x <= 1 */
inline float inl_b_atan_1(float x) {
        long     index;
        float    y, z, e, t, y2, t2;

        index = (long)(x*(float)40.0);
        y = b_ttany[index];
        z = b_ttany2[index];
        e = x-y;
        y2 = y*y;
        t = e*z;
        t2 = t*t;
        return b_ty[index] + t-t2*(y+t*y2);
}

#endif

