/*
 * Description: Selection of time related utilities
 * Author: Michael Fletcher | EOS42
 * Date: 11/09/2018
 */
#include "ico.hpp"


namespace eosio {

    ACTION ico::init() {
        /* validates the configuration */
        require_auth(_self);

        eosio_assert(!ico_config.exists(), "ico has already been started");

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

        /* check the account configuration */
        eosio_assert(is_account(ico_settings.TOKEN_CONTRACT), "token contract account does not exist");

        /* initalise the contract */
        ico_config.set({now(), now() + ico_settings.CUT_OFF_TIME_SECONDS, get_balance(ico_settings.TOKEN_SYMBOL).amount}, _self);

    }

    asset ico::get_balance(symbol_type symbol) {
        /* gets the balance of the symbol */
        eosio::token t(ico_settings.TOKEN_CONTRACT);


        return t.get_balance(_self, symbol.name());
    }

    bool ico::is_active() {
        /* if not initalised or cutoff has expired ( unless disabled ) or there's no funds left */
        auto conf = ico_config.get();

        return (conf.init_time != 0 && (conf.cutoff_time > now() || conf.cutoff_time == 0) && get_balance(ico_settings.TOKEN_SYMBOL).amount > 0);
    }

    ACTION ico::on(eosio::currency::transfer const &transfer) {
        if (transfer.from == _self) return;

        eosio_assert(is_active(), "ico is not active");

        asset defaultasset(0);
        /* when eos is received, purchase the token */
        if (transfer.quantity.symbol == defaultasset.symbol) {

            /* only accept whole tokens */
            //Hmm, This condition always be true
            if(transfer.quantity.amount % 1 == 0){
              purchase(transfer.from, transfer.quantity);
            }

        }
    }

    void ico::purchase(account_name user, asset quantity) {
        asset ico_quantity = asset(0);
        asset refund_quantity = asset(0);

        /* get the current stats */
        auto conf = ico_config.get();

        /* if empty map 1:1 */
        if (ico_settings.sections.empty()) {
            ico_quantity = asset(MIN(quantity.amount, get_balance(ico_settings.TOKEN_SYMBOL).amount),
                                 ico_settings.TOKEN_SYMBOL);

            refund_quantity = asset(quantity.amount - ico_quantity.amount);

        } else {

            double current_percentile = 100.0 * (1.0 - ((double) get_balance(ico_settings.TOKEN_SYMBOL).amount / conf.quantity));
            double check_percentile = 0;

            for (auto it = ico_settings.sections.begin(); it != ico_settings.sections.end(); ++it) {
                check_percentile += it->SPLIT_SECTION_PERCENTILE;
                if (ceil(current_percentile) < check_percentile) {
                    ico_quantity = asset(MIN(get_balance(ico_settings.TOKEN_SYMBOL).amount,
                                             it->SPLIT_SECTION_PAYOUT_RATIO * quantity.amount),
                                         ico_settings.TOKEN_SYMBOL);
                    refund_quantity = asset(quantity.amount - (ico_quantity.amount / it->SPLIT_SECTION_PAYOUT_RATIO));
                    break;
                }
            }
        }

        /* send the token */
        send_funds(_self, user, ico_quantity, ico_settings.PURCHASE_MEMO);

        /* if we've ran out of tokens, return the change */
        if (refund_quantity > asset(0)) {
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
