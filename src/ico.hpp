//
// Created by michael on 10/09/18.
//

#ifndef EOSIO_ICO_HPP
#define EOSIO_ICO_HPP

#include <eosiolib/eosio.hpp>
#include <eosiolib/currency.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/singleton.hpp>
#include <eosio.token/eosio.token.hpp>

#include "ico_settings.hpp"




namespace eosio {

    //@abi table config
    struct config{
        uint64_t init_time = 0;
        uint64_t cutoff_time = 0;
        int64_t quantity = 0; /* contracts quantity, not total */
        int64_t remaining = 0; /* quantity remaining */

        uint64_t primary_key(){
            return init_time;
        }

        EOSLIB_SERIALIZE(config, (init_time)(cutoff_time)(quantity)(remaining))
    };

    typedef eosio::singleton<N(config), config> ico_config;


    class ico : public eosio::contract {

    private:
        ico_config ico_config;

        bool is_active();
        void purchase(account_name user, asset quantity);
        void send_funds(account_name from, account_name to, asset quantity, std::string memo);
        asset get_balance(symbol_name symbol);


    public:
        ico(account_name owner):contract(owner), ico_config(_self, N(config)){}

        ///@ abi action
        void init();

        void apply_onerror(const onerror &error);
        void on(eosio::currency::transfer const &t);

    };



}


#endif //EOSIO_ICO_HPP
