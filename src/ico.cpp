//
// Created by michael on 10/09/18.
//

#include "ico.hpp"


namespace eosio {

    void ico::init() {
        require_auth(_self);

        eosio_assert(ico_config.get().init_time == 0, "ico has already been started");

        transaction trx;
        asset ico(ico_settings.TOKEN_SUPPLY, N(TOKEN_NAME));

        /* create the token */
        trx.actions.emplace_back(
                eosio::permission_level{_self, N(active)},
                N(eosio.token),
                N(create),
                std::make_tuple(_self, ico));

        /* issue the token */
        trx.actions.emplace_back(
                eosio::permission_level{_self, N(active)},
                N(eosio.token),
                N(issue),
                std::make_tuple(_self, asset(ico_settings.CONTRACT_TOKEN_SUPPLY, N(string(TOKEN_NAME))), "issuing token to self"));

        /* send the transaction */
        trx.send(_self, _self, false);

        /* validate the config is correct */
        uint32_t total_size = 0;
        if(!ico_settings.sections.empty()) {
            for (auto it = ico_settings.sections.begin(); it != ico_settings.sections.end(); ++it){
                total_size += it->SPLIT_SECTION_PERCENTILE;

                eosio_assert(it->SPLIT_SECTION_PAYOUT_RATIO != 0, "configuration is invalid, payout must be greater than 0" );
            }

            if(total_size != 100){
                eosio_assert(false, "configuration is invalid, percentile must amount to 100");
            }
        }

        /* initalise the contract */
        ico_config.set({now(), now() + ico_settings.CUT_OFF_TIME_SECONDS, ico.amount, ico.amount}, _self);

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
      //      purchase(transfer.from, transfer.quantity);
        }
    }

    void ico::purchase(account_name user, asset quantity) {

        asset ico_quantity;
        asset refund_quantity;

        /* get the current stats */
        auto conf = ico_config.get();

        /* if empty map 1:1 */
        if (ico_settings.sections.empty()) {
            ico_quantity = asset(MIN(quantity.amount, conf.remaining), N(string(TOKEN_NAME)));
            refund_quantity = quantity - ico_quantity;

        } else {

            /* else figure out what percentile we're in */
            uint64_t target_percentile = conf.remaining / conf.quantity;
            uint64_t current_percentile = 0;

            for (auto it = ico_settings.sections.begin(); it != ico_settings.sections.end(); ++it){
                current_percentile += it->SPLIT_SECTION_PERCENTILE;

                if(target_percentile < current_percentile){
                    ico_quantity = asset(MIN(conf.remaining, it->SPLIT_SECTION_PAYOUT_RATIO * quantity.amount), N(string(TOKEN_NAME))); // TODO: The TOKEN_NAME won't work with this N() macro
                    refund_quantity = quantity - (ico_quantity / it->SPLIT_SECTION_PAYOUT_RATIO);
                    break;
                }
            }
        }

        /* send the token */
        send_funds(_self, user, ico_quantity, ico_settings.PURCHASE_MEMO);

        /* if we've ran out of tokens, return the change */
        if(refund_quantity > asset(0)) {
            send_funds(_self, user, refund_quantity, ico_settings.PURCHASE_MEMO);
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
        /* resubmit any failed transactions */
        transaction trx = error.unpack_sent_trx();

        trx.send(_self, _self, false);
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

            default:
           EOSIO_API(eosio::ico, (init))
        }
    }
    }

}
