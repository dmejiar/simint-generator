#pragma once

#include "simint/shell/shell.h"

#ifdef __cplusplus
extern "C" {
#endif

//! A pointer to a function that calculates Fjt (ie, boys function)
typedef void (*Fjtfunc)(double * restrict, int, double const * restrict);

//! A pointer to a function that calculates TEI utilizing a shared workspace
typedef int (*simint_osteifunc_sharedwork)(struct simint_multi_shellpair const,
                                           struct simint_multi_shellpair const,
                                           Fjtfunc,
                                           double,
                                           double * restrict,
                                           double * restrict);


//! A pointer to a function that calculates TEI that allocates its own workspace
typedef int (*simint_osteifunc)(struct simint_multi_shellpair const,
                                struct simint_multi_shellpair const,
                                Fjtfunc, 
                                double,
                                double * restrict);




/*! \brief Compute an ostei given shell pair information
 *
 * \param [in] P The shell pairs for the bra side of the integral 
 * \param [in] Q The shell pairs for the ket side of the integral
 * \param [in] fjt Fjt function to use
 * \param [in] screen_tol Tolerance for screening (set to zero to disable)
 * \param [inout] integrals Storage for the final integrals. Since size information
 *                          is not passed, you are expected to ensure that this buffer
 *                          is large enough
 */
int simint_compute_ostei(struct simint_multi_shellpair const * P,
                         struct simint_multi_shellpair const * Q,
                         Fjtfunc fjt,
                         double screen_tol,
                         double * restrict integrals);


/*! \brief Compute an ostei given shell pair information
 *
 * \param [in] P The shell pairs for the bra side of the integral 
 * \param [in] Q The shell pairs for the ket side of the integral
 * \param [in] fjt Fjt function to use
 * \param [in] screen_tol Tolerance for screening (set to zero to disable)
 * \param [in] work Workspace to use in calculating the integrals
 * \param [inout] integrals Storage for the final integrals. Since size information
 *                          is not passed, you are expected to ensure that this buffer
 *                          is large enough
 */
int simint_compute_ostei_sharedwork(struct simint_multi_shellpair const * P,
                                    struct simint_multi_shellpair const * Q,
                                    Fjtfunc fjt,
                                    double screen_tol,
                                    double * restrict work,
                                    double * restrict integrals);



#ifdef __cplusplus
}
#endif
