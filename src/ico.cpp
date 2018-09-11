//
// Created by michael on 10/09/18.
//

#include "ico.hpp"


namespace eosio {

    void ico::init() {
        require_auth(_self);

        eosio_assert(ico_config.get().init_time == 0, "ico has already been started");

        transaction trx;
        asset ico(TOKEN_SUPPLY, N(TOKEN_NAME));

        /* create the token */
        trx.actions.emplace_back(
                eosio::permission_level{_self, N(active)},
                N(eosio.token),
                N(create),
                std::make_tuple(_self, ico));

        /* send the transaction */
        trx.send(_self, _self, false);

        /* validate the config is correct */
        uint32_t total_size = 0;
        if(!sections.empty()) {
            for (auto it = sections.begin(); it != sections.end(); ++it){
                total_size += it->SPLIT_SECTION_PERCENTILE;

                eosio_assert(it->SPLIT_SECTION_PAYOUT_RATIO != 0, "configuration is invalid, payout must be greater than 0" );
            }

            if(total_size != 100){
                eosio_assert(false, "configuration is invalid, percentile must amount to 100");
            }
        }

        /* initalise the contract */
        ico_config.set({now(), now() + CUT_OFF_TIME_SECONDS, ico.amount, ico.amount}, _self);

    }

    asset ico::get_balance(symbol_name symbol) {
        /* gets the balance of the symbol */

        eosio::token t(N(eosio.token));
        const auto sym_name = eosio::symbol_type(symbol);

        return t.get_balance(_self, sym_name);
    }

    bool ico::is_active() {
        /* if not initalised or cutoff has expired ( unless disabled ) or there's no funds left */
        auto conf = ico_config.get();

        return (conf.init_time != 0 && (conf.cutoff_time > now() || conf.cutoff_time == 0) && conf.remaining > 0);
    }

    void ico::on(eosio::currency::transfer const &transfer) {
        if (transfer.from == _self) return;

        eosio_assert(is_active(), "ico is not active");

        asset defaultasset(0);
        /* when eos is received, purchase the token */
        if (transfer.quantity.symbol == defaultasset.symbol) {
            purchase(transfer.from, transfer.quantity);
        }
    }

    void ico::purchase(account_name user, asset quantity) {

        asset ico_quantity;
        asset refund_quantity;

        /* get the current stats */
        auto conf = ico_config.get();

        /* if empty map 1:1 */
        if (sections.empty()) {
            ico_quantity = asset(MIN(quantity.amount, conf.remaining) * 10000, N(TOKEN_NAME));
            refund_quantity = quantity - ico_quantity;

        } else {

            /* else figure out what percentile we're in */
            uint64_t target_percentile = conf.remaining / conf.quantity;
            uint64_t current_percentile = 0;

            for (auto it = sections.begin(); it != sections.end(); ++it){
                current_percentile += it->SPLIT_SECTION_PERCENTILE;

                if(target_percentile < current_percentile){
                    ico_quantity = asset(MIN(conf.remaining, it->SPLIT_SECTION_PAYOUT_RATIO * quantity.amount) * 10000, N(TOKEN_NAME)); // TODO: The TOKEN_NAME won't work with this N() macro
                    refund_quantity = quantity - (ico_quantity / it->SPLIT_SECTION_PAYOUT_RATIO);
                    break;
                }
            }
        }

        /* send the token */
        send_funds(_self, user, ico_quantity, PURCHASE_MEMO);

        /* if we've ran out of tokens, return the change */
        if(refund_quantity > asset(0)) {
            send_funds(_self, user, refund_quantity, PURCHASE_MEMO);
        }

    }


    void ico::send_funds(account_name from, account_name to, asset quantity, std::string memo) {
        transaction trx;

        /* add the transfer action to the transaction */
        trx.actions.emplace_back(
                eosio::permission_level{_self, N(active)},
                N(eosio.token),
                N(transfer),
                std::make_tuple(from, to, quantity, memo));

        /* send the transaction */
        trx.send(from + to + quantity.amount + current_time(), _self, false);

        /* update our available token supply */
        auto it = ico_config.get();
        ico_config.set({it.init_time, it.cutoff_time, (it.quantity - quantity.amount), it.remaining } ,_self);

    }


    void ico::apply_onerror(const onerror &error) {

    }

    extern "C"
    {
    void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        auto self = receiver;
        eosio::ico thiscontract(self);

        if (action == N(onerror)) {
            /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */
            eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account");
            thiscontract.apply_onerror(onerror::from_current_action());
        }

        switch (action) {
            case N(transfer):
                eosio_assert(code == N(eosio.token), "only eosio.token supported");
                thiscontract.on(unpack_action_data<eosio::currency::transfer>());
                break;

        //    default:
         //   EOSIO_API(eosio::ico, (init))
        }
    }
    }

}
