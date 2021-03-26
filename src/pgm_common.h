/* Copyright (c) 2020-2021, Zolisa Bleki
 *
 * SPDX-License-Identifier: BSD-3-Clause */
#ifndef PGM_COMMON_H
#define PGM_COMMON_H

#pragma once
#include <numpy/random/distributions.h>
#include <float.h>

#define PGM_PI2_8 1.233700550136169  // pi^2 / 8
#define PGM_LOGPI_2 0.4515827052894548  // log(pi / 2)
#define PGM_LS2PI 0.9189385332046727  // log(sqrt(2 * pi))
#define PGM_MAX_EXP 708.3964202663686  // maximum allowed exp() argument


/*
 * Compute the complementary error function.
 *
 * This implementation uses Rational Chebyshev Approximations for as described
 * in [1]. The polynomial coefficients are obtained from [2] and some from [3].
 * Maximum relative error compared to the standard library erfc function is
 * 1.077760e-09.
 *
 * References
 * ----------
 * [1] Cody, W. J. Rational Chebyshev approximations for the error function.
 *     Math. Comp. 23 (1969), 631–637.
 * [2] Temme, N. (1994). A Set of Algorithms for the Incomplete Gamma Functions.
 *     Probability in the Engineering and Informational Sciences, 8(2),
 *     291-307. doi:10.1017/S0269964800003417.
 * [3] https://www.netlib.org/specfun/erf
 */
NPY_INLINE double
pgm_erfc(double x)
{
#define PGM_1_SQRTPI 0.5641895835477563   // 1 / sqrt(pi)
#define PGM_BIG_VAL 26.615717509251258
#define PGM_SMALL_VAL -6.003636680306125

    if (x < PGM_SMALL_VAL) {
        return 2;
    }
    else if (x < -DBL_EPSILON) {
        return 2 - pgm_erfc(-x);
    }
    else if (x < DBL_EPSILON) {
       return 1;
    }
    else if (x < 0.5) {
        static const double p0 = 3.20937758913846947e+03;
        static const double p1 = 3.77485237685302021e+02;
        static const double p2 = 1.13864154151050156e+02;
        static const double p3 = 3.16112374387056560e+00;
        static const double p4 = 1.85777706184603153e-01;
        static const double q0 = 2.84423683343917062e+03;
        static const double q1 = 1.28261652607737228e+03;
        static const double q2 = 2.44024637934444173e+02;
        static const double q3 = 2.36012909523441209e+01;
        double z = x * x;
        return 1 - x * ((((p4 * z + p3) * z + p2) * z + p1) * z + p0) /
                        ((((z + q3) * z + q2) * z + q1) * z + q0);
    }
    else if (x < 4) {
        static const double p0 = 7.3738883116;
        static const double p1 = 6.8650184849;
        static const double p2 = 3.0317993362;
        static const double p3 = 5.6316961891e-01;
        static const double p4 = 4.3187787405e-05;
        static const double q0 = 7.3739608908;
        static const double q1 = 1.5184908190e+01;
        static const double q2 = 1.2795529509e+01;
        static const double q3 = 5.3542167949;
        return exp(-x * x) * ((((p4 * x + p3) * x + p2) * x + p1) * x + p0) /
                              ((((x + q3) * x + q2) * x + q1) * x + q0);
    }
    else if (x < PGM_BIG_VAL) {
        double z = x * x;
        double y = exp(-z);

        if (x * DBL_MIN > y * PGM_1_SQRTPI) {
            return 0;
        }
        static const double p0 = -4.25799643553e-02;
        static const double p1 = -1.96068973726e-01;
        static const double p2 = -5.16882262185e-02;
        static const double q0 = 1.50942070545e-01;
        static const double q1 = 9.21452411694e-01;
        z = 1 / z;
        z *= ((p2 * z + p1) * z + p0) / ((z + q1) * z + q0);
        return y * (PGM_1_SQRTPI + z) / x;
    }
    else {
        return 0;
    }
#undef PGM_1_SQRTPI
#undef PGM_BIG_VAL
#undef PGM_SMALL_VAL
}

/*
 * Calculate logarithm of the gamma function of z.
 *
 * This implementation is based on an asymptotic expansion based on stirling's
 * approximation, rational approximations and a lookup table.
 *
 * - For interger values corresponding to logfactorial, we use a lookup table.
 * - Values greater than 12 use the asymptotic approximation based on stirling.
 * - For multiple intervals of values less than 12, rational approximations
 *   are used based on coefficients presented in [1].
 * - The implementation of rational approximations is based on the algorithms
 *   presented in [2].
 * - Tests show that the absolute relative error is 9.403053e-10 based on the
 *   values generated by standard library function `lgamma`.
 *
 * References
 * ----------
 *  [1] Cody, W., & Hillstrom, K. (1967). Chebyshev Approximations for the
 *      Natural Logarithm of the Gamma Function. Mathematics of Computation,
 *      21(98), 198-203. doi:10.2307/2004160.
 *  [2] Temme, N. (1994). A Set of Algorithms for the Incomplete Gamma Functions.
 *      Probability in the Engineering and Informational Sciences, 8(2),
 *      291-307. doi:10.1017/S0269964800003417.
 */
NPY_INLINE double
pgm_lgamma(double z)
{
    /* lookup table for integer values of log-gamma function where 1<=z<=200 */
    static const double logfactorial[200] = {
        0.000000000000000, 0.0000000000000000, 0.69314718055994529,
        1.791759469228055, 3.1780538303479458, 4.7874917427820458,
        6.5792512120101012, 8.5251613610654147, 10.604602902745251,
        12.801827480081469, 15.104412573075516, 17.502307845873887,
        19.987214495661885, 22.552163853123425, 25.19122118273868,
        27.89927138384089, 30.671860106080672, 33.505073450136891,
        36.395445208033053, 39.339884187199495, 42.335616460753485,
        45.380138898476908, 48.471181351835227, 51.606675567764377,
        54.784729398112319, 58.003605222980518, 61.261701761002001,
        64.557538627006338, 67.88974313718154, 71.257038967168015,
        74.658236348830158, 78.092223553315307, 81.557959456115043,
        85.054467017581516, 88.580827542197682, 92.136175603687093,
        95.719694542143202, 99.330612454787428, 102.96819861451381,
        106.63176026064346, 110.32063971475739, 114.03421178146171,
        117.77188139974507, 121.53308151543864, 125.3172711493569,
        129.12393363912722, 132.95257503561632, 136.80272263732635,
        140.67392364823425, 144.5657439463449, 148.47776695177302,
        152.40959258449735, 156.3608363030788, 160.3311282166309,
        164.32011226319517, 168.32744544842765, 172.35279713916279,
        176.39584840699735, 180.45629141754378, 184.53382886144948,
        188.6281734236716, 192.7390472878449, 196.86618167289001,
        201.00931639928152, 205.1681994826412, 209.34258675253685,
        213.53224149456327, 217.73693411395422, 221.95644181913033,
        226.1905483237276, 230.43904356577696, 234.70172344281826,
        238.97838956183432, 243.26884900298271, 247.57291409618688,
        251.89040220972319, 256.22113555000954, 260.56494097186322,
        264.92164979855278, 269.29109765101981, 273.67312428569369,
        278.06757344036612, 282.4742926876304, 286.89313329542699,
        291.32395009427029, 295.76660135076065, 300.22094864701415,
        304.68685676566872, 309.1641935801469, 313.65282994987905,
        318.1526396202093, 322.66349912672615, 327.1852877037752,
        331.71788719692847, 336.26118197919845, 340.81505887079902,
        345.37940706226686, 349.95411804077025, 354.53908551944079,
        359.1342053695754, 363.73937555556347, 368.35449607240474,
        372.97946888568902, 377.61419787391867, 382.25858877306001,
        386.91254912321756, 391.57598821732961, 396.24881705179155,
        400.93094827891576, 405.6222961611449, 410.32277652693733,
        415.03230672824964, 419.75080559954472, 424.47819341825709,
        429.21439186665157, 433.95932399501481, 438.71291418612117,
        443.47508812091894, 448.24577274538461, 453.02489623849613,
        457.81238798127816, 462.60817852687489, 467.4121995716082,
        472.22438392698058, 477.04466549258564, 481.87297922988796,
        486.70926113683936, 491.55344822329801, 496.40547848721764,
        501.26529089157924, 506.13282534203483, 511.00802266523596,
        515.89082458782241, 520.78117371604412, 525.67901351599517,
        530.58428829443358, 535.49694318016952, 540.41692410599762,
        545.34417779115483, 550.27865172428551, 555.22029414689484,
        560.16905403727310, 565.12488109487424, 570.08772572513419,
        575.05753902471020, 580.03427276713080, 585.01787938883899,
        590.00831197561786, 595.00552424938201, 600.00947055532743,
        605.02010584942377, 610.03738568623862, 615.06126620708494,
        620.09170412847732, 625.12865673089095, 630.17208184781020,
        635.22193785505965, 640.27818366040810, 645.34077869343503,
        650.40968289565524, 655.48485671088906, 660.56626107587351,
        665.65385741110595, 670.74760761191271, 675.84747403973688,
        680.95341951363753, 686.06540730199413, 691.18340111441080,
        696.30736509381404, 701.43726380873704, 706.57306224578736,
        711.71472580228999, 716.86222027910355, 722.01551187360133,
        727.17456717281584, 732.33935314673920, 737.50983714177733,
        742.68598687435122, 747.86777042464337, 753.05515623048404,
        758.24811308137441, 763.44661011264009, 768.65061679971711,
        773.86010295255835, 779.07503871016729, 784.29539453524569,
        789.52114120895885, 794.75224982581346, 799.98869178864345,
        805.23043880370301, 810.47746287586358, 815.72973630391016,
        820.98723167593789, 826.24992186484292, 831.51778002390620,
        836.79077958246978, 842.06889424170038, 847.35209797043842,
        852.64036500113298, 857.93366982585735,
    };

    if (z < 201 && z == (size_t)z) {
        return logfactorial[(size_t)z - 1];
    }
    else if (z > 12) {
        static const double a1 = 0.08333333333333333;  // 1 / 12
        static const double a2 = 0.002777777777777778;  // 1/360
        static const double a3 = 0.0007936507936507937;  // 1/1260
        double out, z2 = z * z;

        out = (z - 0.5) * log(z) - z + PGM_LS2PI;
        out += a1 / z - a2 / (z2 * z) + a3 / (z2 * z2 * z);
        return out;
    }
    else if (z >= 4) {
        static const double p0 = -2.12159572323e+05;
        static const double p1 = 2.30661510616e+05;
        static const double p2 = 2.74647644705e+04;
        static const double p3 = -4.02621119975e+04;
        static const double p4 = -2.29660729780e+03;
        static const double q0 = -1.16328495004e+05;
        static const double q1 = -1.46025937511e+05;
        static const double q2 = -2.42357409629e+04;
        static const double q3 = -5.70691009324e+02;

        return ((((p4 * z + p3) * z + p2) * z + p1) * z + p0) /
                ((((z + q3) * z + q2) * z + q1) * z + q0);
    }
    else if (z > 1.5) {
        static const double p0 = -7.83359299449e+01;
        static const double p1 = -1.42046296688e+02;
        static const double p2 = 1.37519416416e+02;
        static const double p3 = 7.86994924154e+01;
        static const double p4 = 4.16438922228;
        static const double q0 = 4.70668766060e+01;
        static const double q1 = 3.13399215894e+02;
        static const double q2 = 2.63505074721e+02;
        static const double q3 = 4.33400022514e+01;

        return (z - 2) * ((((p4 * z + p3) * z + p2) * z + p1) * z + p0) /
                          ((((z + q3) * z + q2) * z + q1) * z + q0);
    }
    else {
        static const double p0 = -2.66685511495;
        static const double p1 = -2.44387534237e+01;
        static const double p2 = -2.19698958928e+01;
        static const double p3 = 1.11667541262e+01;
        static const double p4 = 3.13060547623;
        static const double q0 = 6.07771387771e-01;
        static const double q1 = 1.19400905721e+01;
        static const double q2 = 3.14690115749e+01;
        static const double q3 = 1.52346874070e+01;

        if (z >= 0.5) {
            return (z - 1) * ((((p4 * z + p3) * z + p2) * z + p1) * z + p0) /
                              ((((z + q3) * z + q2) * z + q1) * z + q0);
        }
        else if (z > DBL_EPSILON) {
            double x = z + 1;
            return z * ((((p4 * x + p3) * x + p2) * x + p1) * x + p0) /
                        ((((x + q3) * x + q2) * x + q1) * x + q0) - log(z);
        }
        else if (z > DBL_MIN) {
            return -log(z);
        }
        else {
            return PGM_MAX_EXP;
        }
    }
}

/*
 * sample from X ~ Gamma(a, rate=b) truncated on the interval {x | x > t}.
 *
 * For a > 1 we use the algorithm described in Dagpunar (1978)
 * For a == 1, we truncate an Exponential of rate=b.
 * For a < 1, we use algorithm [A4] described in Philippe (1997)
 *
 * TODO: There is a more efficient algorithm for a > 1 in Philippe (1997), which
 * should replace this one in the future.
 */
NPY_INLINE double
random_left_bounded_gamma(bitgen_t* bitgen_state, double a, double b, double t)
{
    if (a > 1) {
        b = t * b;
        double x, threshold;
        const double amin1 = a - 1;
        const double bmina = b - a;
        const double c0 = 0.5 * (bmina + sqrt((bmina * bmina) + 4 * b)) / b;
        const double one_minus_c0 = 1 - c0;
        const double log_m = amin1 * (log(amin1 / one_minus_c0) - 1);

        do {
            x = b + random_standard_exponential(bitgen_state) / c0;
            threshold = amin1 * log(x) - x * one_minus_c0 - log_m;
        } while (log1p(-random_standard_uniform(bitgen_state)) > threshold);
        return t * (x / b);
    }
    else if (a == 1) {
        return t + random_standard_exponential(bitgen_state) / b;
    }
    else {
        double x;
        const double amin1 = a - 1;
        const double tb = t * b;
        do {
            x = 1 + random_standard_exponential(bitgen_state) / tb;
        } while (log1p(-random_standard_uniform(bitgen_state)) > amin1 * log(x));
        return t * x;
    }
}


#define PGM_CONFLUENT_EPSILON 1e-07

/*
 * Compute function G(p, x) (A confluent hypergeometric function ratio).
 * This function is defined in equation 14 of [1] and this implementation
 * uses a continued fraction (eq. 15) defined for x <= p. The continued
 * fraction is evaluated using the Modified Lentz method.
 *
 * G(p, x) = a_1/b_1+ a_2/b_2+ a_3/b_3+ ..., such that a_1 = 1 and for n >= 1:
 * a_2n = -(p - 1 + n)*x, a_(2n+1) = n*x, b_n = p - 1 + n.
 *
 * Note that b_n can be reduced to b_1 = p, b_n = b_(n-1) + 1 for n >= 2. Also
 * for odd n, the argument of a_n is "k=(n-1)/2" and for even n "k=n/2". This
 * means we can pre-compute constant terms s = 0.5 * x and r = -(p - 1) * x.
 * This simplifies a_n into: a_n = s * (n - 1) for odd n and a_n = r - s * n
 * for even n >= 2. The terms for the first iteration are pre-calculated as
 * explained in [1].
 *
 * References
 * ----------
 *  [1] Algorithm 1006: Fast and accurate evaluation of a generalized
 *      incomplete gamma function, Rémy Abergel and Lionel Moisan, ACM
 *      Transactions on Mathematical Software (TOMS), 2020. DOI: 10.1145/3365983
 */
static NPY_INLINE double
confluent_x_smaller(double p, double x)
{
    size_t n;
    double f, c, d, delta;
    double a = 1, b = p;
    double r = -(p - 1) * x;
    double s = 0.5 * x;
    for (n = 2, f = a / b, c = a / DBL_MIN, d = 1 / b; n < 100; n++) {
        a =  n & 1 ? s * (n - 1) : r - s * n;
        b++;

        c = b + a / c;
        if (c < DBL_MIN) {
            c = DBL_MIN;
        }

        d = a * d + b;
        if (d < DBL_MIN) {
            d = DBL_MIN;
        }

        d = 1 / d;
        delta = c * d;
        f *= delta;
        if (fabs(delta - 1) < PGM_CONFLUENT_EPSILON) {
            break;
        }
    }
    return f;
}

/*
 * Compute function G(p, x) (A confluent hypergeometric function ratio).
 * This function is defined in equation 14 of [1] and this implementation
 * uses a continued fraction (eq. 16) defined for x > p. The continued
 * fraction is evaluated using the Modified Lentz method.
 *
 * G(p, x) = a_1/b_1+ a_2/b_2+ a_3/b_3+ ..., such that a_1 = 1 and for n > 1:
 * a_n = -(n - 1) * (n - p - 1), and for n >= 1: b_n = x + 2n - 1 - p.
 *
 * Note that b_n can be re-written as b_1 = x - p + 1 and
 * b_n = (((x - p + 1) + 2) + 2) + 2 ...) for n >= 2. Thus b_n = b_(n-1) + 2
 * for n >= 2. Also a_n can be re-written as a_n = (n - 1) * ((p - (n - 1)).
 * So if we can initialize the series with a_1 = 1 and instead of computing
 * (n - 1) at every iteration we can instead start the counter at n = 1 and
 * just compute a_(n+1) = n * (p - n). This doesnt affect b_n terms since all
 * we need is to keep incrementing b_n by 2 every iteration after initializing
 * the series with b_1 = x - p + 1.
 *
 * References
 * ----------
 *  [1] Algorithm 1006: Fast and accurate evaluation of a generalized
 *      incomplete gamma function, Rémy Abergel and Lionel Moisan, ACM
 *      Transactions on Mathematical Software (TOMS), 2020. DOI: 10.1145/3365983
 */
static NPY_INLINE double
confluent_p_smaller(double p, double x)
{
    size_t n;
    double f, c, d, delta;
    double a = 1, b = x - p + 1;
    for (n = 1, f = a / b, c = a / DBL_MIN, d = 1 / b; n < 100; n++) {
        a = n * (p - n);
        b += 2;

        c = b + a / c;
        if (c < DBL_MIN) {
            c = DBL_MIN;
        }

        d = a * d + b;
        if (d < DBL_MIN) {
            d = DBL_MIN;
        }

        d = 1 / d;
        delta = c * d;
        f *= delta;
        if (fabs(delta - 1) < PGM_CONFLUENT_EPSILON) {
            break;
        }
    }
    return f;
}

/*
 * Compute the (normalized) upper incomplete gamma function for the pair (p, x).
 *
 * We use the algorithm described in [1]. We use two continued fractions to
 * evaluate the function in the regions {0 < x <= p} and {0 <= p < x}
 * (algorithm 3 of [1]).
 *
 * We also use a terminating series to evaluate the normalized version for
 * integer and half-integer values of p <= 30 as described in [2]. This is
 * faster than the algorithm of [1] when p is small since not more than p terms
 * are required to evaluate the function.
 *
 * Parameters
 * ----------
 *  normalized : if true, the normalized upper incomplete gamma is returned,
 *      else the non-normalized version is returned for the arguments (p, x).
 *
 * References
 * ----------
 *  [1] Algorithm 1006: Fast and accurate evaluation of a generalized
 *      incomplete gamma function, Rémy Abergel and Lionel Moisan, ACM
 *      Transactions on Mathematical Software (TOMS), 2020. DOI: 10.1145/3365983
 *  [2] https://www.boost.org/doc/libs/1_71_0/libs/math/doc/html/math_toolkit/sf_gamma/igamma.html
 */
static NPY_INLINE double
pgm_gammaq(double p, double x, bool normalized)
{
    if (normalized) {
        size_t p_int = (size_t)p;
        if (p == p_int && p < 30) {
            size_t k;
            double sum, r;
            for (k = sum = r = 1; k < p_int; k++) {
                sum += (r *= x / k);
            }
            return exp(-x) * sum;
        }
        else if (p == (p_int + 0.5) && p < 30) {
            size_t k;
            double sum, r;
            static const double one_sqrtpi = 0.5641895835477563;
            double sqrt_x = sqrt(x);
            for (k = r = 1, sum = 0; k < p_int + 1; k++) {
                sum += (r *= x / (k - 0.5));
            }
            return pgm_erfc(sqrt_x) + exp(-x) * one_sqrtpi * sum / sqrt_x;
        }
    }

    bool x_smaller = p >= x;
    double f = x_smaller ? confluent_x_smaller(p, x) : confluent_p_smaller(p, x);

    if (normalized) {
        double out = f * exp(-x + p * log(x) - pgm_lgamma(p));
        return x_smaller ? 1 - out : out;
    }
    else if (x_smaller) {
        double lgam = pgm_lgamma(p);
        double exp_lgam = lgam >= PGM_MAX_EXP ? exp(PGM_MAX_EXP) : exp(lgam);
        double arg = -x + p * log(x) - lgam;

        if (arg >= PGM_MAX_EXP) {
            arg = PGM_MAX_EXP;
        }
        else if (arg <= -PGM_MAX_EXP) {
            arg = -PGM_MAX_EXP;
        }
        return (1 - f * exp(arg)) * exp_lgam;
    }
    else {
        double arg = -x + p * log(x);
        return f * (arg >= PGM_MAX_EXP ? exp(PGM_MAX_EXP) : exp(arg));
    }
}

#endif
