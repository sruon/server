/*
===========================================================================

  Copyright (c) 2026 LandSandBoat Dev Teams

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/

===========================================================================
*/

#include <common/database.h>

#include <catch2/catch_test_macros.hpp>

namespace
{

constexpr auto kSellerId = 4000000001U;

void clearSales()
{
    db::preparedStmt("DELETE FROM delivery_box WHERE charid = ?", kSellerId);
    db::preparedStmt("DELETE FROM auction_house WHERE seller = ?", kSellerId);
}

auto sellAndReadQuantity(const bool stack) -> uint8
{
    db::preparedStmt("INSERT INTO auction_house (itemid, stack, seller, seller_name, date, price) VALUES (4096, ?, ?, 'Catch2', 0, 100)", stack, kSellerId);
    db::preparedStmt("UPDATE auction_house SET buyer = 1, buyer_name = 'Catch2Buyer', sale = 100, sell_date = 1 WHERE seller = ? AND buyer_name IS NULL", kSellerId);

    uint8      quantity = 0;
    const auto rset     = db::preparedStmt("SELECT ORD(extra) AS quantity FROM delivery_box WHERE charid = ?", kSellerId);
    FOR_DB_SINGLE_RESULT(rset)
    {
        quantity = rset->get<uint8>("quantity");
    }

    clearSales();
    return quantity;
}

} // namespace

TEST_CASE("auction_house_buy records the sold quantity of a stack in the seller delivery box", "[database][auction]")
{
    clearSales();

    SECTION("a stack sale records the quantity it was given")
    {
        REQUIRE(db::preparedStmt("SET @ah_sale_quantity = 12"));
        CHECK(sellAndReadQuantity(true) == 12);
    }

    SECTION("a stack sale with no quantity given records 1")
    {
        REQUIRE(db::preparedStmt("SET @ah_sale_quantity = NULL"));
        CHECK(sellAndReadQuantity(true) == 1);
    }

    SECTION("a single sale records 1 whatever quantity is set")
    {
        REQUIRE(db::preparedStmt("SET @ah_sale_quantity = 12"));
        CHECK(sellAndReadQuantity(false) == 1);
    }

    REQUIRE(db::preparedStmt("SET @ah_sale_quantity = NULL"));
}
