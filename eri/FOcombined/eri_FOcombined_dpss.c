#include <string.h>
#include <math.h>

#include "vectorization.h"
#include "constants.h"
#include "eri/shell.h"


int eri_FOcombined_dpss(struct multishell_pair const P,
                        struct multishell_pair const Q,
                        double * const restrict S_2_1_0_0)
{

    ASSUME_ALIGN(P.x);
    ASSUME_ALIGN(P.y);
    ASSUME_ALIGN(P.z);
    ASSUME_ALIGN(P.PA_x);
    ASSUME_ALIGN(P.PA_y);
    ASSUME_ALIGN(P.PA_z);
    ASSUME_ALIGN(P.bAB_x);
    ASSUME_ALIGN(P.bAB_y);
    ASSUME_ALIGN(P.bAB_z);
    ASSUME_ALIGN(P.alpha);
    ASSUME_ALIGN(P.prefac);

    ASSUME_ALIGN(Q.x);
    ASSUME_ALIGN(Q.y);
    ASSUME_ALIGN(Q.z);
    ASSUME_ALIGN(Q.PA_x);
    ASSUME_ALIGN(Q.PA_y);
    ASSUME_ALIGN(Q.PA_z);
    ASSUME_ALIGN(Q.bAB_x);
    ASSUME_ALIGN(Q.bAB_y);
    ASSUME_ALIGN(Q.bAB_z);
    ASSUME_ALIGN(Q.alpha);
    ASSUME_ALIGN(Q.prefac);
    ASSUME_ALIGN(integrals)

    const int nshell1234 = P.nshell12 * Q.nshell12;

    memset(S_2_1_0_0, 0, nshell1234*18*sizeof(double));

    // Holds AB_{xyz} and CD_{xyz} in a flattened fashion for later
    double AB_x[nshell1234];  double CD_x[nshell1234];
    double AB_y[nshell1234];  double CD_y[nshell1234];
    double AB_z[nshell1234];  double CD_z[nshell1234];

    int ab, cd, abcd;
    int i, j;

    // Workspace for contracted integrals
    double S_2_0_0_0[nshell1234 * 6];
    memset(S_2_0_0_0, 0, (nshell1234 * 6) * sizeof(double));

    double S_3_0_0_0[nshell1234 * 10];
    memset(S_3_0_0_0, 0, (nshell1234 * 10) * sizeof(double));



    ////////////////////////////////////////
    // Loop over shells and primitives
    ////////////////////////////////////////
    for(ab = 0, abcd = 0; ab < P.nshell12; ++ab)
    {
        const int abstart = P.primstart[ab];
        const int abend = P.primend[ab];

        // this should have been set/aligned in fill_multishell_pair or something else
        ASSUME(abstart%SIMD_ALIGN_DBL == 0);

        for(cd = 0; cd < Q.nshell12; ++cd, ++abcd)
        {
            const int cdstart = Q.primstart[cd];
            const int cdend = Q.primend[cd];

            // this should have been set/aligned in fill_multishell_pair or something else
            ASSUME(cdstart%SIMD_ALIGN_DBL == 0);

            // Store for later
            AB_x[abcd] = P.AB_x[ab];  CD_x[abcd] = Q.AB_x[cd];
            AB_y[abcd] = P.AB_y[ab];  CD_y[abcd] = Q.AB_y[cd];
            AB_z[abcd] = P.AB_z[ab];  CD_z[abcd] = Q.AB_z[cd];

            for(i = abstart; i < abend; ++i)
            {
                for(j = cdstart; j < cdend; ++j)
                {

                    // Holds the auxiliary integrals ( i 0 | 0 0 )^m in the primitive basis

                    // with m as the slowest index
                    // AM = 0: Needed from this AM: 1
                    double AUX_S_0_0_0_0[4 * 1];

                    // AM = 1: Needed from this AM: 3
                    double AUX_S_1_0_0_0[3 * 3];

                    // AM = 2: Needed from this AM: 4
                    double AUX_S_2_0_0_0[2 * 6];

                    // AM = 3: Needed from this AM: 10
                    double AUX_S_3_0_0_0[1 * 10];



                    // Holds temporary integrals for electron transfer


                    const double PQalpha_mul = P.alpha[i] * Q.alpha[j];
                    const double PQalpha_sum = P.alpha[i] + Q.alpha[j];

                    const double pfac = TWO_PI_52 / (PQalpha_mul * sqrt(PQalpha_sum));

                    /* construct R2 = (Px - Qx)**2 + (Py - Qy)**2 + (Pz -Qz)**2 */
                    const double PQ_x = P.x[i] - Q.x[j];
                    const double PQ_y = P.y[i] - Q.y[j];
                    const double PQ_z = P.z[i] - Q.z[j];
                    const double R2 = PQ_x*PQ_x + PQ_y*PQ_y + PQ_z*PQ_z;

                    // collected prefactors
                    const double allprefac =  pfac * P.prefac[i] * Q.prefac[j];

                    // various factors
                    const double alpha = PQalpha_mul/PQalpha_sum;   // alpha from MEST
                    // for VRR
                    const double one_over_p = 1.0 / P.alpha[i];
                    const double a_over_p =  alpha * one_over_p;     // a/p from MEST
                    const double one_over_2p = 0.5 * one_over_p;  // gets multiplied by i in VRR


                    //////////////////////////////////////////////
                    // Boys function section
                    // Maximum v value: 3
                    //////////////////////////////////////////////
                    // The paremeter to the boys function
                    const double F_x = R2 * alpha;


                    AUX_S_0_0_0_0[0] = allprefac
                             * pow(
                                     (
                                       (
                                                   1.
                                         + F_x * ( 0.414016243000866299
                                         + F_x * ( 0.130448682735044324
                                         + F_x * ( 0.0281490811816026161
                                         + F_x * ( 0.00462868463720416281
                                         + F_x * ( 0.00062025147610678493
                                         + F_x * ( 0.0000686770885390617984
                                         + F_x * ( 6.28488230669978749e-6
                                         + F_x * ( 5.01986197619830788e-7
                                         + F_x * ( 3.96915046153987083e-8
                                         + F_x * ( 1.14619057438675389e-9
                                         + F_x * ( 2.21422857239286206e-10
                                         + F_x * ( -3.47087628137958658e-12
                                         + F_x * ( 5.26907054399378694e-13
                                                 )))))))))))))
                                       )
                                       /
                                       (
                                                   1.0
                                         + F_x * ( 1.08068290966581548
                                         + F_x * ( 0.539792844780494916
                                         + F_x * ( 0.166084230769033217
                                         + F_x * ( 0.035790298646556066
                                         + F_x * ( 0.00589703243578234382
                                         + F_x * ( 0.000789592116312117277
                                         + F_x * ( 0.0000874456178357265776
                                         + F_x * ( 8.00211107603171383e-6
                                         + F_x * ( 6.39149165582055646e-7
                                         + F_x * ( 5.05367903369666356e-8
                                         + F_x * ( 1.45937517486452486e-9
                                         + F_x * ( 2.81924337930412797e-10
                                         + F_x * ( -4.4192569363292127e-12
                                         + F_x * ( 6.70878898061210528e-13
                                                 ))))))))))))))
                                       )
                                     ), 0.0+0.5);

                    AUX_S_0_0_0_0[1] = allprefac
                             * pow(
                                     (
                                       (
                                                   0.480749856769136127
                                         + F_x * ( 0.0757107453935371611
                                         + F_x * ( 0.0207544733443622468
                                         + F_x * ( 0.00296686757159093428
                                         + F_x * ( 0.000385086850988198076
                                         + F_x * ( 0.0000396245291118678106
                                         + F_x * ( 3.44653527568129186e-6
                                         + F_x * ( 2.60728584781887378e-7
                                         + F_x * ( 1.64651276793705422e-8
                                         + F_x * ( 1.24924114646889903e-9
                                         + F_x * ( -4.06609033385506782e-13
                                         + F_x * ( 6.69244577908522819e-12
                                         + F_x * ( -1.74969643368118421e-13
                                         + F_x * ( 9.69901476816967128e-15
                                                 )))))))))))))
                                       )
                                       /
                                       (
                                                   1.0
                                         + F_x * ( 0.557484696706444012
                                         + F_x * ( 0.16330778039059299
                                         + F_x * ( 0.0332854355906960737
                                         + F_x * ( 0.00522222434942978777
                                         + F_x * ( 0.000658578219691024581
                                         + F_x * ( 0.0000682687148667475058
                                         + F_x * ( 5.92820704830126442e-6
                                         + F_x * ( 4.486040686836368e-7
                                         + F_x * ( 2.83282406028920981e-8
                                         + F_x * ( 2.14933012694134515e-9
                                         + F_x * ( -6.99576077110757546e-13
                                         + F_x * ( 1.15144066901510615e-11
                                         + F_x * ( -3.01036676011995688e-13
                                         + F_x * ( 1.66872327689919498e-14
                                                 ))))))))))))))
                                       )
                                     ), 1.0+0.5);

                    AUX_S_0_0_0_0[2] = allprefac
                             * pow(
                                     (
                                       (
                                                   0.525305560880753447
                                         + F_x * ( 0.110492812543561698
                                         + F_x * ( 0.0191075521270522133
                                         + F_x * ( 0.00308864958825646759
                                         + F_x * ( 0.000365092211441395786
                                         + F_x * ( 0.0000386927386117543446
                                         + F_x * ( 3.43285176619925111e-6
                                         + F_x * ( 2.60432969408429629e-7
                                         + F_x * ( 1.81615413272499949e-8
                                         + F_x * ( 8.79574269616801187e-10
                                         + F_x * ( 8.17788745331821633e-11
                                         + F_x * ( 9.41377749237483758e-13
                                         + F_x * ( 1.10425964672642921e-13
                                         + F_x * ( 6.7330075025747763e-15
                                                 )))))))))))))
                                       )
                                       /
                                       (
                                                   1.0
                                         + F_x * ( 0.496054363546458276
                                         + F_x * ( 0.128217363190316964
                                         + F_x * ( 0.0237743767099492677
                                         + F_x * ( 0.00352539772392101481
                                         + F_x * ( 0.00043510113976810022
                                         + F_x * ( 0.0000454073941207125778
                                         + F_x * ( 4.04510802860155619e-6
                                         + F_x * ( 3.06619642129059298e-7
                                         + F_x * ( 2.13853590636569169e-8
                                         + F_x * ( 1.03568903196894899e-9
                                         + F_x * ( 9.62938758302946861e-11
                                         + F_x * ( 1.10846362277666573e-12
                                         + F_x * ( 1.3002555622660695e-13
                                         + F_x * ( 7.92805431387221855e-15
                                                 ))))))))))))))
                                       )
                                     ), 2.0+0.5);

                    AUX_S_0_0_0_0[3] = allprefac
                             * pow(
                                     (
                                       (
                                                   0.573513198744647626
                                         + F_x * ( 0.0884965100710721081
                                         + F_x * ( 0.0125945026937261292
                                         + F_x * ( 0.0018131346755066484
                                         + F_x * ( 0.000185501511589852242
                                         + F_x * ( 0.0000179665588981785096
                                         + F_x * ( 1.4663716751615509e-6
                                         + F_x * ( 1.07200007061958399e-7
                                         + F_x * ( 6.13886384827129358e-9
                                         + F_x * ( 5.0757503319816481e-10
                                         + F_x * ( -3.89402001604364884e-12
                                         + F_x * ( 3.54266040828850291e-12
                                         + F_x * ( -1.30207429069121938e-13
                                         + F_x * ( 6.46345036567436585e-15
                                                 )))))))))))))
                                       )
                                       /
                                       (
                                                   1.0
                                         + F_x * ( 0.376528191549264251
                                         + F_x * ( 0.0764525025412929857
                                         + F_x * ( 0.0117343580663816836
                                         + F_x * ( 0.00149763967922572319
                                         + F_x * ( 0.000163513443673380925
                                         + F_x * ( 0.0000154572094390025412
                                         + F_x * ( 1.26979191105616802e-6
                                         + F_x * ( 9.27046437188452642e-8
                                         + F_x * ( 5.30988729375523733e-9
                                         + F_x * ( 4.39022184757595294e-10
                                         + F_x * ( -3.36809850708008399e-12
                                         + F_x * ( 3.06419443797353572e-12
                                         + F_x * ( -1.12621824892382023e-13
                                         + F_x * ( 5.59050724287616386e-15
                                                 ))))))))))))))
                                       )
                                     ), 3.0+0.5);


                    //////////////////////////////////////////////
                    // Primitive integrals: Vertical recurrance
                    //////////////////////////////////////////////

                    // Forming AUX_S_1_0_0_0[3 * 3];
                    // Needed from this AM:
                    //    P_100
                    //    P_010
                    //    P_001
                    for(int m = 0; m < 3; m++)  // loop over orders of boys function
                    {
                        //P_100 : STEP: x
                        AUX_S_1_0_0_0[m * 3 + 0] = P.PA_x[i] * AUX_S_0_0_0_0[m * 1 + 0] - a_over_p * PQ_x * AUX_S_0_0_0_0[(m+1) * 1 + 0];

                        //P_010 : STEP: y
                        AUX_S_1_0_0_0[m * 3 + 1] = P.PA_y[i] * AUX_S_0_0_0_0[m * 1 + 0] - a_over_p * PQ_y * AUX_S_0_0_0_0[(m+1) * 1 + 0];

                        //P_001 : STEP: z
                        AUX_S_1_0_0_0[m * 3 + 2] = P.PA_z[i] * AUX_S_0_0_0_0[m * 1 + 0] - a_over_p * PQ_z * AUX_S_0_0_0_0[(m+1) * 1 + 0];

                    }


                    // Forming AUX_S_2_0_0_0[2 * 6];
                    // Needed from this AM:
                    //    D_200
                    //    D_110
                    //    D_020
                    //    D_002
                    for(int m = 0; m < 2; m++)  // loop over orders of boys function
                    {
                        //D_200 : STEP: x
                        AUX_S_2_0_0_0[m * 6 + 0] = P.PA_x[i] * AUX_S_1_0_0_0[m * 3 + 0] - a_over_p * PQ_x * AUX_S_1_0_0_0[(m+1) * 3 + 0]
                                      +1 * one_over_2p * ( AUX_S_0_0_0_0[m * 1 +  0] - a_over_p * AUX_S_0_0_0_0[(m+1) * 1 + 0] );

                        //D_110 : STEP: y
                        AUX_S_2_0_0_0[m * 6 + 1] = P.PA_y[i] * AUX_S_1_0_0_0[m * 3 + 0] - a_over_p * PQ_y * AUX_S_1_0_0_0[(m+1) * 3 + 0];

                        //D_020 : STEP: y
                        AUX_S_2_0_0_0[m * 6 + 3] = P.PA_y[i] * AUX_S_1_0_0_0[m * 3 + 1] - a_over_p * PQ_y * AUX_S_1_0_0_0[(m+1) * 3 + 1]
                                      +1 * one_over_2p * ( AUX_S_0_0_0_0[m * 1 +  0] - a_over_p * AUX_S_0_0_0_0[(m+1) * 1 + 0] );

                        //D_002 : STEP: z
                        AUX_S_2_0_0_0[m * 6 + 5] = P.PA_z[i] * AUX_S_1_0_0_0[m * 3 + 2] - a_over_p * PQ_z * AUX_S_1_0_0_0[(m+1) * 3 + 2]
                                      +1 * one_over_2p * ( AUX_S_0_0_0_0[m * 1 +  0] - a_over_p * AUX_S_0_0_0_0[(m+1) * 1 + 0] );

                    }

                    // Accumulating in contracted workspace
                    S_2_0_0_0[abcd * 6 + 0] += AUX_S_2_0_0_0[0];
                    S_2_0_0_0[abcd * 6 + 1] += AUX_S_2_0_0_0[1];
                    S_2_0_0_0[abcd * 6 + 3] += AUX_S_2_0_0_0[3];
                    S_2_0_0_0[abcd * 6 + 5] += AUX_S_2_0_0_0[5];


                    // Forming AUX_S_3_0_0_0[1 * 10];
                    // Needed from this AM:
                    //    F_300
                    //    F_210
                    //    F_201
                    //    F_120
                    //    F_111
                    //    F_102
                    //    F_030
                    //    F_021
                    //    F_012
                    //    F_003
                    for(int m = 0; m < 1; m++)  // loop over orders of boys function
                    {
                        //F_300 : STEP: x
                        AUX_S_3_0_0_0[m * 10 + 0] = P.PA_x[i] * AUX_S_2_0_0_0[m * 6 + 0] - a_over_p * PQ_x * AUX_S_2_0_0_0[(m+1) * 6 + 0]
                                      +2 * one_over_2p * ( AUX_S_1_0_0_0[m * 3 +  0] - a_over_p * AUX_S_1_0_0_0[(m+1) * 3 + 0] );

                        //F_210 : STEP: y
                        AUX_S_3_0_0_0[m * 10 + 1] = P.PA_y[i] * AUX_S_2_0_0_0[m * 6 + 0] - a_over_p * PQ_y * AUX_S_2_0_0_0[(m+1) * 6 + 0];

                        //F_201 : STEP: z
                        AUX_S_3_0_0_0[m * 10 + 2] = P.PA_z[i] * AUX_S_2_0_0_0[m * 6 + 0] - a_over_p * PQ_z * AUX_S_2_0_0_0[(m+1) * 6 + 0];

                        //F_120 : STEP: x
                        AUX_S_3_0_0_0[m * 10 + 3] = P.PA_x[i] * AUX_S_2_0_0_0[m * 6 + 3] - a_over_p * PQ_x * AUX_S_2_0_0_0[(m+1) * 6 + 3];

                        //F_111 : STEP: z
                        AUX_S_3_0_0_0[m * 10 + 4] = P.PA_z[i] * AUX_S_2_0_0_0[m * 6 + 1] - a_over_p * PQ_z * AUX_S_2_0_0_0[(m+1) * 6 + 1];

                        //F_102 : STEP: x
                        AUX_S_3_0_0_0[m * 10 + 5] = P.PA_x[i] * AUX_S_2_0_0_0[m * 6 + 5] - a_over_p * PQ_x * AUX_S_2_0_0_0[(m+1) * 6 + 5];

                        //F_030 : STEP: y
                        AUX_S_3_0_0_0[m * 10 + 6] = P.PA_y[i] * AUX_S_2_0_0_0[m * 6 + 3] - a_over_p * PQ_y * AUX_S_2_0_0_0[(m+1) * 6 + 3]
                                      +2 * one_over_2p * ( AUX_S_1_0_0_0[m * 3 +  1] - a_over_p * AUX_S_1_0_0_0[(m+1) * 3 + 1] );

                        //F_021 : STEP: z
                        AUX_S_3_0_0_0[m * 10 + 7] = P.PA_z[i] * AUX_S_2_0_0_0[m * 6 + 3] - a_over_p * PQ_z * AUX_S_2_0_0_0[(m+1) * 6 + 3];

                        //F_012 : STEP: y
                        AUX_S_3_0_0_0[m * 10 + 8] = P.PA_y[i] * AUX_S_2_0_0_0[m * 6 + 5] - a_over_p * PQ_y * AUX_S_2_0_0_0[(m+1) * 6 + 5];

                        //F_003 : STEP: z
                        AUX_S_3_0_0_0[m * 10 + 9] = P.PA_z[i] * AUX_S_2_0_0_0[m * 6 + 5] - a_over_p * PQ_z * AUX_S_2_0_0_0[(m+1) * 6 + 5]
                                      +2 * one_over_2p * ( AUX_S_1_0_0_0[m * 3 +  2] - a_over_p * AUX_S_1_0_0_0[(m+1) * 3 + 2] );

                    }

                    // Accumulating in contracted workspace
                    S_3_0_0_0[abcd * 10 + 0] += AUX_S_3_0_0_0[0];
                    S_3_0_0_0[abcd * 10 + 1] += AUX_S_3_0_0_0[1];
                    S_3_0_0_0[abcd * 10 + 2] += AUX_S_3_0_0_0[2];
                    S_3_0_0_0[abcd * 10 + 3] += AUX_S_3_0_0_0[3];
                    S_3_0_0_0[abcd * 10 + 4] += AUX_S_3_0_0_0[4];
                    S_3_0_0_0[abcd * 10 + 5] += AUX_S_3_0_0_0[5];
                    S_3_0_0_0[abcd * 10 + 6] += AUX_S_3_0_0_0[6];
                    S_3_0_0_0[abcd * 10 + 7] += AUX_S_3_0_0_0[7];
                    S_3_0_0_0[abcd * 10 + 8] += AUX_S_3_0_0_0[8];
                    S_3_0_0_0[abcd * 10 + 9] += AUX_S_3_0_0_0[9];




                    //////////////////////////////////////////////
                    // Primitive integrals: Electron transfer
                    //////////////////////////////////////////////

                    //...nothing to do...

                 }
            }
        }
    }


    //////////////////////////////////////////////
    // Contracted integrals: Horizontal recurrance
    // Bra part
    // Steps: 18
    //////////////////////////////////////////////

    for(abcd = 0; abcd < nshell1234; ++abcd)
    {
        // form S_2_1_0_0
        for(int iket = 0; iket < 1; ++iket)
        {
            // (D_200 P_100|_{i} = (F_300 S_000|_{t} + x_ab * (D_200 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 0 * 1 + iket] = S_3_0_0_0[abcd * 10 + 0 * 1 + iket] + ( AB_x[abcd] * S_2_0_0_0[abcd * 6 + 0 * 1 + iket] );

            // (D_200 P_010|_{i} = (F_210 S_000|_{t} + y_ab * (D_200 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 1 * 1 + iket] = S_3_0_0_0[abcd * 10 + 1 * 1 + iket] + ( AB_y[abcd] * S_2_0_0_0[abcd * 6 + 0 * 1 + iket] );

            // (D_200 P_001|_{i} = (F_201 S_000|_{t} + z_ab * (D_200 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 2 * 1 + iket] = S_3_0_0_0[abcd * 10 + 2 * 1 + iket] + ( AB_z[abcd] * S_2_0_0_0[abcd * 6 + 0 * 1 + iket] );

            // (D_110 P_100|_{i} = (F_210 S_000|_{t} + x_ab * (D_110 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 3 * 1 + iket] = S_3_0_0_0[abcd * 10 + 1 * 1 + iket] + ( AB_x[abcd] * S_2_0_0_0[abcd * 6 + 1 * 1 + iket] );

            // (D_110 P_010|_{i} = (F_120 S_000|_{t} + y_ab * (D_110 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 4 * 1 + iket] = S_3_0_0_0[abcd * 10 + 3 * 1 + iket] + ( AB_y[abcd] * S_2_0_0_0[abcd * 6 + 1 * 1 + iket] );

            // (D_110 P_001|_{i} = (F_111 S_000|_{t} + z_ab * (D_110 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 5 * 1 + iket] = S_3_0_0_0[abcd * 10 + 4 * 1 + iket] + ( AB_z[abcd] * S_2_0_0_0[abcd * 6 + 1 * 1 + iket] );

            // (D_101 P_100|_{i} = (F_201 S_000|_{t} + x_ab * (D_101 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 6 * 1 + iket] = S_3_0_0_0[abcd * 10 + 2 * 1 + iket] + ( AB_x[abcd] * S_2_0_0_0[abcd * 6 + 2 * 1 + iket] );

            // (D_101 P_010|_{i} = (F_111 S_000|_{t} + y_ab * (D_101 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 7 * 1 + iket] = S_3_0_0_0[abcd * 10 + 4 * 1 + iket] + ( AB_y[abcd] * S_2_0_0_0[abcd * 6 + 2 * 1 + iket] );

            // (D_101 P_001|_{i} = (F_102 S_000|_{t} + z_ab * (D_101 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 8 * 1 + iket] = S_3_0_0_0[abcd * 10 + 5 * 1 + iket] + ( AB_z[abcd] * S_2_0_0_0[abcd * 6 + 2 * 1 + iket] );

            // (D_020 P_100|_{i} = (F_120 S_000|_{t} + x_ab * (D_020 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 9 * 1 + iket] = S_3_0_0_0[abcd * 10 + 3 * 1 + iket] + ( AB_x[abcd] * S_2_0_0_0[abcd * 6 + 3 * 1 + iket] );

            // (D_020 P_010|_{i} = (F_030 S_000|_{t} + y_ab * (D_020 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 10 * 1 + iket] = S_3_0_0_0[abcd * 10 + 6 * 1 + iket] + ( AB_y[abcd] * S_2_0_0_0[abcd * 6 + 3 * 1 + iket] );

            // (D_020 P_001|_{i} = (F_021 S_000|_{t} + z_ab * (D_020 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 11 * 1 + iket] = S_3_0_0_0[abcd * 10 + 7 * 1 + iket] + ( AB_z[abcd] * S_2_0_0_0[abcd * 6 + 3 * 1 + iket] );

            // (D_011 P_100|_{i} = (F_111 S_000|_{t} + x_ab * (D_011 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 12 * 1 + iket] = S_3_0_0_0[abcd * 10 + 4 * 1 + iket] + ( AB_x[abcd] * S_2_0_0_0[abcd * 6 + 4 * 1 + iket] );

            // (D_011 P_010|_{i} = (F_021 S_000|_{t} + y_ab * (D_011 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 13 * 1 + iket] = S_3_0_0_0[abcd * 10 + 7 * 1 + iket] + ( AB_y[abcd] * S_2_0_0_0[abcd * 6 + 4 * 1 + iket] );

            // (D_011 P_001|_{i} = (F_012 S_000|_{t} + z_ab * (D_011 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 14 * 1 + iket] = S_3_0_0_0[abcd * 10 + 8 * 1 + iket] + ( AB_z[abcd] * S_2_0_0_0[abcd * 6 + 4 * 1 + iket] );

            // (D_002 P_100|_{i} = (F_102 S_000|_{t} + x_ab * (D_002 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 15 * 1 + iket] = S_3_0_0_0[abcd * 10 + 5 * 1 + iket] + ( AB_x[abcd] * S_2_0_0_0[abcd * 6 + 5 * 1 + iket] );

            // (D_002 P_010|_{i} = (F_012 S_000|_{t} + y_ab * (D_002 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 16 * 1 + iket] = S_3_0_0_0[abcd * 10 + 8 * 1 + iket] + ( AB_y[abcd] * S_2_0_0_0[abcd * 6 + 5 * 1 + iket] );

            // (D_002 P_001|_{i} = (F_003 S_000|_{t} + z_ab * (D_002 S_000|_{t}
            S_2_1_0_0[abcd * 18 + 17 * 1 + iket] = S_3_0_0_0[abcd * 10 + 9 * 1 + iket] + ( AB_z[abcd] * S_2_0_0_0[abcd * 6 + 5 * 1 + iket] );

        }


    }


    //////////////////////////////////////////////
    // Contracted integrals: Horizontal recurrance
    // Ket part
    // Steps: 0
    // Forming final integrals
    //////////////////////////////////////////////

    //Nothing to do.....


    return nshell1234;
}

