/*
 * dt_int.c: Checked integers for Unit 5, Section A.
 *
 * In C, signed integer overflow has undefined behavior.
 * The compiler can assume that signed overflow does not occur.
 * An optimizer can remove a guarded check after the arithmetic.
 *
 *     long long sum = a + b
 *     if (b > 0 && sum < a) return DT_ERR_OVERFLOW // optimizer may remove this branch
 *
 * Check before the operation. Use comparison values that cannot overflow.
 * A positive b overflows when a > LLONG_MAX - b.
 * A negative b produces a result below LLONG_MIN when a < LLONG_MIN - b.
 * These comparison subtractions are safe.
 *
 * Multiplication has more cases. LLONG_MIN multiplied by -1 overflows.
 * LLONG_MIN divided by -1 also has undefined behavior.
 *
 * These stubs report overflow for every input. The normal cases fail until you implement them.
 */

#include "dt.h"

#include <limits.h>

/*
 * dt_int_add computes a + b.
 * It returns DT_ERR_OVERFLOW and does not change *out for an overflow.
 */
dt_status dt_int_add(long long a, long long b, long long *out)
{
//checker if within safe bounds
    if (b>0 && a > LLONG_MAX-b){
        return DT_ERR_OVERFLOW;
    }
    if (b<0 && a < LLONG_MIN -b){
        return DT_ERR_OVERFLOW;
    }
//only runs if within safe limits-- to avoid wrapping
    *out = a + b;
    return DT_OK;
}

/*
 * dt_int_sub computes a - b.
 * It returns DT_ERR_OVERFLOW and does not change *out for an overflow.
 */
dt_status dt_int_sub(long long a, long long b, long long *out)
{
    //boundary checker
    if (b > 0 && a < LLONG_MIN + b) {
        return DT_ERR_OVERFLOW;
    }
    if (b < 0 && a > LLONG_MAX + b) {
        return DT_ERR_OVERFLOW;
    }
    /*need to handle directly with addition based-reference bounds to 
    keep sub-expression within range*/
    *out = a - b;
    return DT_OK;  
}

/*
 * dt_int_mul computes a * b.
 * It returns DT_ERR_OVERFLOW and does not change *out for an overflow.
 */
dt_status dt_int_mul(long long a, long long b, long long *out)
{
    //zero multiplication property 
    if (a == 0 || b == 0) {
        *out = 0;
        return DT_OK;
    }
    /*LLONG_MIN wont have a + counterpart due to neg num having 1 reach further 
    mult by -1 will cause overflow*/
    if ((a == -1 && b == LLONG_MIN) || (b == -1 && a == LLONG_MIN)) {
        return DT_ERR_OVERFLOW;
    }

    if (a > 0) {
        if (b > 0) {
            //positive x positive-- only risk is exceeding ceiling
            if (a > LLONG_MAX / b) return DT_ERR_OVERFLOW; 
        } else {
            //negative product--- should not underflow on lowest val
            if (b < LLONG_MIN / a) return DT_ERR_OVERFLOW;
        }
    } else {
        if (b > 0) {
            //negative product-- risking underflow again same as before
            if (a < LLONG_MIN / b) return DT_ERR_OVERFLOW;
        } else {
            //neg * neg == positive ; risking overflow
            if (a < LLONG_MAX / b) return DT_ERR_OVERFLOW;
        }
    }

    *out = a * b;
    return DT_OK;
}
