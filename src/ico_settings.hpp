//
// Created by michael on 10/09/18.
//

#ifndef EOSIO_ICO_SETTINGS_HPP
#define EOSIO_ICO_SETTINGS_HPP

#include <eosiolib/eosio.hpp>
#include <boost/assign/list_of.hpp>
#include <vector>

#include "declarations.hpp"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

namespace eosio {


    struct section {
        uint32_t SPLIT_SECTION_PERCENTILE; /* size of each token payout section */
        uint32_t SPLIT_SECTION_PAYOUT_RATIO; /* tokens paid out per eos */
    };


    ///SETTINGS///

    /* token name */
    const static char *TOKEN_NAME = "ICO";

    /* token quantity */
    const static int64_t TOKEN_SUPPLY = 10000000000000;

    /* tokens used by contract */
    const static int64_t CONTRACT_TOKEN_SUPPLY = 8000000000000;

    /* cut-off time in seconds after launch, set to 0 to disable */
    const static uint64_t CUT_OFF_TIME_SECONDS = catalogue::ONE_WEEK * 4;

    /* the memo sent after a purchase */
    const static char* PURCHASE_MEMO = "Thank you for purchasing ICO Token";

    /* set to empty for token to always be 1:1 */
    const static std::vector<section> sections = boost::assign::list_of
            (section{10, 10}) /* first 10% will receive 10 tokens */
            (section{20, 5}) /* next 20% will receive 5 */
            (section{20, 3})
            (section{50, 1}); /* bottom 50% will receive 1 tokens per eos */


    ///END OF SETTINGS///
}


#endif //EOSIO_ICO_SETTINGS_HPP
