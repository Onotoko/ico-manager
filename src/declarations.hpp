/*
 * Description: Selection of time related utilities
 * Author: Michael Fletcher | EOS42
 * Date: 11/09/2018
 */

#include <eosiolib/types.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

namespace eosio{

    const static uint32_t ONE_SECOND = 1;
    const static uint32_t ONE_MINUTE = ONE_SECOND * 60;
    const static uint32_t ONE_HOUR = ONE_MINUTE * 60;
    const static uint32_t ONE_DAY = ONE_HOUR * 24;
    const static uint32_t ONE_WEEK = ONE_DAY * 7;


}