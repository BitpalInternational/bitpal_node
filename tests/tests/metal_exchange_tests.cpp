/*
 * Copyright (c) 2018 Abit More, and other contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <boost/test/unit_test.hpp>

#include <graphene/chain/hardfork.hpp>

#include <graphene/chain/market_object.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE(metal_exchange_tests, database_fixture)

/***
 * This test case tests the mismatch quantity from metal-exchange expectations.
 * Bitpal
 */
BOOST_AUTO_TEST_CASE( limit_order_metal_exchange )
{
   try {
      generate_blocks( HARDFORK_555_TIME );
      generate_block();
      set_expiration( db, trx );

      ACTORS( (seller)(buyer) );

      const asset_object& test = create_user_issued_asset( "UIATEST" );
      const asset_id_type test_id = test.id;
      const asset_object& core = get_asset( GRAPHENE_SYMBOL );
      const asset_id_type core_id = core.id;

      transfer( committee_account(db), seller, asset( 100000000 ) );

      issue_uia( buyer, asset( 10000000, test_id ) );

      BOOST_CHECK_EQUAL(get_balance(buyer, core), 0);
      BOOST_CHECK_EQUAL(get_balance(buyer, test), 10000000);
      BOOST_CHECK_EQUAL(get_balance(seller, test), 0);
      BOOST_CHECK_EQUAL(get_balance(seller, core), 100000000);

      // buyer buys 17 core with 3 test, price 3/17 = 0.176 test per core
      limit_order_id_type tmp_buy_id = create_sell_order( buyer, test.amount(400), core.amount(100) )->id;
      // seller sells 33 core for 5 test, price 5/33 = 0.1515 test per core
      limit_order_id_type sell_id = create_sell_order( seller, core.amount(100), test.amount(300) )->id;

      BOOST_CHECK( !db.find_object( tmp_buy_id ) ); // buy order is filled
      BOOST_CHECK_EQUAL( sell_id(db).for_sale.value, 0 ); // 17 core sold, 16 remaining
      BOOST_CHECK_EQUAL( tmp_buy_id(db).for_sale.value, 100);

      BOOST_CHECK_EQUAL(get_balance(seller, core), 99999900);
      BOOST_CHECK_EQUAL(get_balance(seller, test), 300); // seller got 3 test
      BOOST_CHECK_EQUAL(get_balance(buyer, core), 100); // buyer got 17 core
      BOOST_CHECK_EQUAL(get_balance(buyer, test), 99999700); // buyer paid 3 test

      /*generate_block();
      set_expiration( db, trx );

      // buyer buys 15 core with 3 test, price 3/15 = 0.2 test per core
      // even 15 < 16, since it's taker, we'll check with maker's price, then turns out the buy order is bigger
      limit_order_id_type buy_id = create_sell_order( buyer_id, asset(3,test_id), asset(15,core_id) )->id;

      generate_block();

      BOOST_CHECK( !db.find_object( sell_id ) ); // sell order is filled
      BOOST_CHECK_EQUAL( buy_id(db).for_sale.value, 1 ); // 2 test sold, 1 remaining

      BOOST_CHECK_EQUAL(get_balance(seller_id, core_id), 99999967); // seller paid the 16 core which was remaining in the order
      BOOST_CHECK_EQUAL(get_balance(seller_id, test_id), 5); // seller got 2 more test
                                                             // effective price 16/2 which is much higher than 33/5
      BOOST_CHECK_EQUAL(get_balance(buyer_id, core_id), 33); // buyer got 16 more core
      BOOST_CHECK_EQUAL(get_balance(buyer_id, test_id), 9999994);*/

   } catch( const fc::exception& e) {
      edump((e.to_detail_string()));
      throw;
   }
}

BOOST_AUTO_TEST_SUITE_END()
