# ICO Manager

This code will assist in managing the distribution of your ICO. Whenever EOS is received, the token will be distributed proportionally
inline with the setting configured. This is an initial version of the code and many improvements can be made, please feel free to contribute
any improvements.

# Setup
The only file you will need to amend is ico_settings.hpp, the following configurations can be made:

- TOKEN_CONTRACT - The name of the contract running the ICO ( Only standard eosio.token contract supported and tested )
- TOKEN_SYMBOL - The name of the token you plan to distribute
- CUT_OFF_TIME_SECONDS - The total running period of the contract. Set to 0 to run forever.
- PURCHASE_MEMO - The Memo to send the user when they have purchased tokens.
- sections - Each section represents a percentile and it's payout ratio. See code for example.

Once configured and launched, any users who send EOS to the contract will be rewarded as configured in the tokens.

# Notes
- The token contract must be launched, your token created and issued to the account this contract will exist on
- The contract will use all available tokens in the account, anything excess you will need to keep seperate
- Only whole EOS are allowed
- If a user purchases EOS which takes them into another distribution bracket, they will receive the rewards of the bracket they started in
- You will pay for the RAM required for users to purchase your token
- Please test before deployment to ensure the behaviour you're expecting
