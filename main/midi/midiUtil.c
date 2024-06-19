#include "midiUtil.h"

// Multiply a frequency by this value to bend it by a number of cents
// uint32_t bentPitch = (uint32_t)(((uint64_t)pitch * bendTable[bendCents + 100]) >> 24)
static const uq8_24 bendTable[] = {
    0x00f1a1bf, // -100 cents => 0.94387
    0x00f1c57d, // -99 cents => 0.94442
    0x00f1e940, // -98 cents => 0.94497
    0x00f20d08, // -97 cents => 0.94551
    0x00f230d5, // -96 cents => 0.94606
    0x00f254a8, // -95 cents => 0.94660
    0x00f27880, // -94 cents => 0.94715
    0x00f29c5e, // -93 cents => 0.94770
    0x00f2c040, // -92 cents => 0.94825
    0x00f2e428, // -91 cents => 0.94879
    0x00f30816, // -90 cents => 0.94934
    0x00f32c08, // -89 cents => 0.94989
    0x00f35000, // -88 cents => 0.95044
    0x00f373fe, // -87 cents => 0.95099
    0x00f39800, // -86 cents => 0.95154
    0x00f3bc08, // -85 cents => 0.95209
    0x00f3e015, // -84 cents => 0.95264
    0x00f40428, // -83 cents => 0.95319
    0x00f42840, // -82 cents => 0.95374
    0x00f44c5d, // -81 cents => 0.95429
    0x00f47080, // -80 cents => 0.95484
    0x00f494a8, // -79 cents => 0.95539
    0x00f4b8d5, // -78 cents => 0.95595
    0x00f4dd08, // -77 cents => 0.95650
    0x00f50140, // -76 cents => 0.95705
    0x00f5257d, // -75 cents => 0.95760
    0x00f549c0, // -74 cents => 0.95816
    0x00f56e08, // -73 cents => 0.95871
    0x00f59255, // -72 cents => 0.95926
    0x00f5b6a8, // -71 cents => 0.95982
    0x00f5db00, // -70 cents => 0.96037
    0x00f5ff5e, // -69 cents => 0.96093
    0x00f623c1, // -68 cents => 0.96148
    0x00f64829, // -67 cents => 0.96204
    0x00f66c97, // -66 cents => 0.96259
    0x00f6910a, // -65 cents => 0.96315
    0x00f6b582, // -64 cents => 0.96371
    0x00f6da00, // -63 cents => 0.96426
    0x00f6fe84, // -62 cents => 0.96482
    0x00f7230c, // -61 cents => 0.96538
    0x00f7479a, // -60 cents => 0.96594
    0x00f76c2e, // -59 cents => 0.96649
    0x00f790c7, // -58 cents => 0.96705
    0x00f7b565, // -57 cents => 0.96761
    0x00f7da09, // -56 cents => 0.96817
    0x00f7feb2, // -55 cents => 0.96873
    0x00f82361, // -54 cents => 0.96929
    0x00f84815, // -53 cents => 0.96985
    0x00f86cce, // -52 cents => 0.97041
    0x00f8918d, // -51 cents => 0.97097
    0x00f8b651, // -50 cents => 0.97153
    0x00f8db1b, // -49 cents => 0.97209
    0x00f8ffea, // -48 cents => 0.97265
    0x00f924bf, // -47 cents => 0.97322
    0x00f94999, // -46 cents => 0.97378
    0x00f96e78, // -45 cents => 0.97434
    0x00f9935d, // -44 cents => 0.97490
    0x00f9b848, // -43 cents => 0.97547
    0x00f9dd38, // -42 cents => 0.97603
    0x00fa022d, // -41 cents => 0.97660
    0x00fa2728, // -40 cents => 0.97716
    0x00fa4c28, // -39 cents => 0.97772
    0x00fa712e, // -38 cents => 0.97829
    0x00fa9639, // -37 cents => 0.97885
    0x00fabb4a, // -36 cents => 0.97942
    0x00fae060, // -35 cents => 0.97999
    0x00fb057c, // -34 cents => 0.98055
    0x00fb2a9d, // -33 cents => 0.98112
    0x00fb4fc4, // -32 cents => 0.98169
    0x00fb74f0, // -31 cents => 0.98225
    0x00fb9a21, // -30 cents => 0.98282
    0x00fbbf59, // -29 cents => 0.98339
    0x00fbe495, // -28 cents => 0.98396
    0x00fc09d7, // -27 cents => 0.98453
    0x00fc2f1f, // -26 cents => 0.98509
    0x00fc546c, // -25 cents => 0.98566
    0x00fc79bf, // -24 cents => 0.98623
    0x00fc9f17, // -23 cents => 0.98680
    0x00fcc475, // -22 cents => 0.98737
    0x00fce9d8, // -21 cents => 0.98794
    0x00fd0f41, // -20 cents => 0.98851
    0x00fd34b0, // -19 cents => 0.98909
    0x00fd5a23, // -18 cents => 0.98966
    0x00fd7f9d, // -17 cents => 0.99023
    0x00fda51c, // -16 cents => 0.99080
    0x00fdcaa0, // -15 cents => 0.99137
    0x00fdf02a, // -14 cents => 0.99195
    0x00fe15ba, // -13 cents => 0.99252
    0x00fe3b4f, // -12 cents => 0.99309
    0x00fe60ea, // -11 cents => 0.99367
    0x00fe868a, // -10 cents => 0.99424
    0x00feac30, // -9 cents => 0.99481
    0x00fed1dc, // -8 cents => 0.99539
    0x00fef78d, // -7 cents => 0.99596
    0x00ff1d43, // -6 cents => 0.99654
    0x00ff42ff, // -5 cents => 0.99712
    0x00ff68c1, // -4 cents => 0.99769
    0x00ff8e88, // -3 cents => 0.99827
    0x00ffb455, // -2 cents => 0.99885
    0x00ffda28, // -1 cents => 0.99942
    0x01000000, // +0 cents => 1.00000
    0x010025de, // +1 cents => 1.00058
    0x01004bc1, // +2 cents => 1.00116
    0x010071aa, // +3 cents => 1.00173
    0x01009798, // +4 cents => 1.00231
    0x0100bd8d, // +5 cents => 1.00289
    0x0100e386, // +6 cents => 1.00347
    0x01010986, // +7 cents => 1.00405
    0x01012f8b, // +8 cents => 1.00463
    0x01015595, // +9 cents => 1.00521
    0x01017ba5, // +10 cents => 1.00579
    0x0101a1bb, // +11 cents => 1.00637
    0x0101c7d7, // +12 cents => 1.00696
    0x0101edf8, // +13 cents => 1.00754
    0x0102141f, // +14 cents => 1.00812
    0x01023a4b, // +15 cents => 1.00870
    0x0102607d, // +16 cents => 1.00928
    0x010286b5, // +17 cents => 1.00987
    0x0102acf2, // +18 cents => 1.01045
    0x0102d335, // +19 cents => 1.01104
    0x0102f97e, // +20 cents => 1.01162
    0x01031fcc, // +21 cents => 1.01220
    0x01034620, // +22 cents => 1.01279
    0x01036c7a, // +23 cents => 1.01337
    0x010392d9, // +24 cents => 1.01396
    0x0103b93e, // +25 cents => 1.01455
    0x0103dfa9, // +26 cents => 1.01513
    0x01040619, // +27 cents => 1.01572
    0x01042c8f, // +28 cents => 1.01630
    0x0104530b, // +29 cents => 1.01689
    0x0104798d, // +30 cents => 1.01748
    0x0104a014, // +31 cents => 1.01807
    0x0104c6a1, // +32 cents => 1.01866
    0x0104ed33, // +33 cents => 1.01924
    0x010513cb, // +34 cents => 1.01983
    0x01053a69, // +35 cents => 1.02042
    0x0105610d, // +36 cents => 1.02101
    0x010587b6, // +37 cents => 1.02160
    0x0105ae65, // +38 cents => 1.02219
    0x0105d51a, // +39 cents => 1.02278
    0x0105fbd5, // +40 cents => 1.02337
    0x01062295, // +41 cents => 1.02397
    0x0106495b, // +42 cents => 1.02456
    0x01067027, // +43 cents => 1.02515
    0x010696f8, // +44 cents => 1.02574
    0x0106bdd0, // +45 cents => 1.02633
    0x0106e4ad, // +46 cents => 1.02693
    0x01070b8f, // +47 cents => 1.02752
    0x01073278, // +48 cents => 1.02811
    0x01075966, // +49 cents => 1.02871
    0x0107805a, // +50 cents => 1.02930
    0x0107a754, // +51 cents => 1.02990
    0x0107ce53, // +52 cents => 1.03049
    0x0107f558, // +53 cents => 1.03109
    0x01081c64, // +54 cents => 1.03168
    0x01084374, // +55 cents => 1.03228
    0x01086a8b, // +56 cents => 1.03288
    0x010891a7, // +57 cents => 1.03347
    0x0108b8ca, // +58 cents => 1.03407
    0x0108dff1, // +59 cents => 1.03467
    0x0109071f, // +60 cents => 1.03526
    0x01092e53, // +61 cents => 1.03586
    0x0109558c, // +62 cents => 1.03646
    0x01097ccb, // +63 cents => 1.03706
    0x0109a410, // +64 cents => 1.03766
    0x0109cb5b, // +65 cents => 1.03826
    0x0109f2ac, // +66 cents => 1.03886
    0x010a1a02, // +67 cents => 1.03946
    0x010a415e, // +68 cents => 1.04006
    0x010a68c0, // +69 cents => 1.04066
    0x010a9028, // +70 cents => 1.04126
    0x010ab796, // +71 cents => 1.04186
    0x010adf09, // +72 cents => 1.04247
    0x010b0683, // +73 cents => 1.04307
    0x010b2e02, // +74 cents => 1.04367
    0x010b5587, // +75 cents => 1.04427
    0x010b7d12, // +76 cents => 1.04488
    0x010ba4a2, // +77 cents => 1.04548
    0x010bcc39, // +78 cents => 1.04608
    0x010bf3d5, // +79 cents => 1.04669
    0x010c1b78, // +80 cents => 1.04729
    0x010c4320, // +81 cents => 1.04790
    0x010c6ace, // +82 cents => 1.04850
    0x010c9282, // +83 cents => 1.04911
    0x010cba3c, // +84 cents => 1.04972
    0x010ce1fb, // +85 cents => 1.05032
    0x010d09c1, // +86 cents => 1.05093
    0x010d318c, // +87 cents => 1.05154
    0x010d595d, // +88 cents => 1.05214
    0x010d8135, // +89 cents => 1.05275
    0x010da912, // +90 cents => 1.05336
    0x010dd0f5, // +91 cents => 1.05397
    0x010df8dd, // +92 cents => 1.05458
    0x010e20cc, // +93 cents => 1.05519
    0x010e48c1, // +94 cents => 1.05580
    0x010e70bb, // +95 cents => 1.05641
    0x010e98bc, // +96 cents => 1.05702
    0x010ec0c2, // +97 cents => 1.05763
    0x010ee8cf, // +98 cents => 1.05824
    0x010f10e1, // +99 cents => 1.05885
    0x010f38f9, // +100 cents => 1.05946
};


uq16_16 bendPitchFreq(uq16_16 freq, int32_t bendCents)
{
    // Grab the base frequency and store it in a 64-bit int
    uint64_t freq64 = freq;

    while (bendCents < -100)
    {
        freq64 *= bendTable[0];
        freq64 >>= 24;
        bendCents += 100;
    }

    while (bendCents > 100)
    {
        freq64 *= bendTable[200];
        freq64 >>= 24;
        bendCents -= 100;
    }

    // Multiply the base frequency by the appropriate UQ8.24 value in the pitch bend table
    freq64 *= bendTable[bendCents + 100];

    // Shift right 24 bits to account for the fractional component of the bend table
    // Mask the result back into a 32-bit value
    uq16_16 trimmedFreq = ((freq64 >> 24) & (0xFFFFFFFFu));

    // Not using an intermediate variable here makes weird stuff happen
    return trimmedFreq;
}
