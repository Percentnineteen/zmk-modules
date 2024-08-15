// Copyright 2022 Manna Harbour
// https://github.com/manna-harbour/miryoku

#define XXX &none

#define MIRYOKU_MAPPING( \
K00, K01, K02, K03, K04,                K05, K06, K07, K08, K09, \
K10, K11, K12, K13, K14,                K15, K16, K17, K18, K19, \
K20, K21, K22, K23, K24,                K25, K26, K27, K28, K29, \
N30, N31, K32, K33, K34,                K35, K36, K37, N38, N39 \
) \
K00  K01  K02  K03  K04                 K05  K06  K07  K08  K09 \
K10  K11  K12  K13  K14                 K15  K16  K17  K18  K19 \
K20  K21  K22  K23  K24  XXX       XXX  K25  K26  K27  K28  K29 \
          K32  K33  K34                 K35  K36  K37

#define MIRYOKU_KLUDGE_BOTTOMROWCOMBOS_LEFTPINKIE 21 22
#define MIRYOKU_KLUDGE_BOTTOMROWCOMBOS_LEFTINNERINDEX 22 23
#define MIRYOKU_KLUDGE_BOTTOMROWCOMBOS_RIGHTINNERINDEX 28 29
#define MIRYOKU_KLUDGE_BOTTOMROWCOMBOS_RIGHTPINKIE 29 30

#define MIRYOKU_KLUDGE_THUMBCOMBOS_LEFT 33 34
#define MIRYOKU_KLUDGE_THUMBCOMBOS_RIGHT 35 36
