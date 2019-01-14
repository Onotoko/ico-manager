/*
 * Description: Selection of time related utilities
 * Author: Michael Fletcher | EOS42
 * Date: 11/09/2018
 */

#ifndef EOSIO_ICO_HPP
#define EOSIO_ICO_HPP

#include <eosiolib/eosio.hpp>
#include <eosiolib/currency.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/singleton.hpp>
#include <eosio.token/eosio.token.hpp>

#include "ico_settings.hpp"
#include <cmath>



namespace eosio {

    TABLE config{
        uint64_t init_time = 0;
        uint64_t cutoff_time = 0;
        int64_t quantity = 0; /* contracts quantity */

        uint64_t primary_key() const{return init_time; }

        EOSLIB_SERIALIZE(config, (init_time)(cutoff_time)(quantity))
    };

    typedef eosio::singleton<N(config), config> ico_config;


    CONTRACT ico : public eosio::contract {

    private:
        ico_config ico_config;
        settings ico_settings;

        bool is_active();
        void purchase(account_name user, asset quantity);
        void send_funds(account_name from, account_name to, asset quantity, std::string memo);
        asset get_balance(symbol_type symbol);


    public:
          ico(account_name owner):contract(owner), ico_config(_self, N(config)), ico_settings(){}


          ACTION init();
          ACTION on(eosio::currency::transfer const &t);

          void apply_onerror(const onerror &error);

    };

    EOSIO_DISPATCH(ico, (init)(on))

}


#endif //EOSIO_ICO_HPP
