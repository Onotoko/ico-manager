/*
 * Description: Selection of time related utilities
 * Author: Michael Fletcher | EOS42
 * Date: 11/09/2018
 */
#ifndef EOSIO_ICO_SETTINGS_HPP
#define EOSIO_ICO_SETTINGS_HPP

#include <eosiolib/eosio.hpp>
#include <eosiolib/symbol.hpp>
#include <boost/assign/list_of.hpp>
#include <vector>

#include "declarations.hpp"


namespace eosio {

    struct section {
        uint32_t SPLIT_SECTION_PERCENTILE; /* size of each token payout section */
        uint32_t SPLIT_SECTION_PAYOUT_RATIO; /* tokens paid out per eos */
    };


    class settings {
        ///SETTINGS///

    public:

        /* account name of token contract */
        account_name TOKEN_CONTRACT = N(eosio.token);

        /* token name(Precision, name) */
        const symbol_type TOKEN_SYMBOL = S(4,DEF);

        /* cut-off time in seconds after launch, set to 0 to disable */
        const uint64_t CUT_OFF_TIME_SECONDS = ONE_WEEK * 4;

        /* the memo sent after a purchase */
        const char *PURCHASE_MEMO = "Thank you for purchasing DEF Token";

    /* set to empty for token to always be 1:1 */
        const std::vector<section> sections = boost::assign::list_of
            (section{10, 10}) /* first 10% will receive 10 tokens */
            (section{20, 5}) /* next 20% will receive 5 */
            (section{20, 3})
            (section{50, 1}); /* bottom 50% will receive 1 tokens per eos */


        ///END OF SETTINGS///
    };
}


#endif //EOSIO_ICO_SETTINGS_HPP
