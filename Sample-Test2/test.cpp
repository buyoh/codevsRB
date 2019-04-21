#include "pch.h"
#include "../codevsRB/Game.h"
#include "../codevsRB/GameExt.h"
//#include "../codevsRB/Exec.h"

TEST(PackTest, PackRotation) {
    Game::Pack pack = { 1,2,4,3 };
    EXPECT_TRUE((pack == vector<int8_t>{ 1, 2, 4, 3 }));
    pack.rotate(1);
    EXPECT_TRUE((pack == vector<int8_t>{ 2, 3, 1, 4 }));
    pack.rotate(1);
    EXPECT_TRUE((pack == vector<int8_t>{ 3, 4, 2, 1 }));
    pack.rotate(1);
    EXPECT_TRUE((pack == vector<int8_t>{ 4, 1, 3, 2 }));
    pack.rotate(1);
    EXPECT_TRUE((pack == vector<int8_t>{ 1, 2, 4, 3 }));
    EXPECT_FALSE((pack == vector<int8_t>{ 1, 2, 3, 4 }));
}

TEST(PackTest, PackRotated) {
    Game::Pack pack = { 1,2,4,3 };
    EXPECT_TRUE((pack == vector<int8_t>{ 1, 2, 4, 3 }));
    pack = pack.rotated(1);
    EXPECT_TRUE((pack == vector<int8_t>{ 2, 3, 1, 4 }));
    pack = pack.rotated(1);
    EXPECT_TRUE((pack == vector<int8_t>{ 3, 4, 2, 1 }));
    pack = pack.rotated(1);
    EXPECT_TRUE((pack == vector<int8_t>{ 4, 1, 3, 2 }));
    pack = pack.rotated(1);
    EXPECT_TRUE((pack == vector<int8_t>{ 1, 2, 4, 3 }));
    EXPECT_FALSE((pack == vector<int8_t>{ 1, 2, 3, 4 }));
}

TEST(FieldTest, FallenPack) {
    using namespace Game;
    Pack pack = { 1,2,4,3 };
    Field field;
    field.insert(pack, 0);
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field(0, 0), 2);
    EXPECT_EQ(field(1, 0), 1);
    EXPECT_EQ(field(0, 1), 3);
    EXPECT_EQ(field(1, 1), 4);
}

TEST(FieldTest, ChainHorizontal) {
    using namespace Game;
    Pack pack1 = { 9,9,9,6 };
    Pack pack2 = { 9,4,9,9 };
    Field field;
    field.insert(pack1, 0);
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);
    field.insert(pack2, 2);
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 2);
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);

    field.insert(pack1, W - 4);
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);
    field.insert(pack2, W - 2);
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 2);
    EXPECT_TRUE(field.fall());

    field.fill(Game::None);
    field.insert(pack1, 0);
    field.insert(pack2, 2);
    EXPECT_EQ(field.chain().first, 1);
    field.insert(pack1, W - 4);
    field.insert(pack2, W - 2);
    EXPECT_EQ(field.chain().first, 1);
}

TEST(FieldTest, ChainVertical) {
    using namespace Game;
    Pack pack1 = { 4,9,9,9 };
    Pack pack2 = { 9,6,9,9 };
    Field field;
    EXPECT_TRUE(field.insert(pack1, 0));
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);
    EXPECT_TRUE(field.insert(pack2, 0));
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 2);
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);

    EXPECT_TRUE(field.insert(pack1, W - 2));
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);
    EXPECT_TRUE(field.insert(pack2, W - 2));
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 2);
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);

    field.fill(Game::None);
    field.insert(pack1, 0);
    field.insert(pack2, 0);
    EXPECT_EQ(field.chain().first, 1);
    field.insert(pack1, W - 2);
    field.insert(pack2, W - 2);
    EXPECT_EQ(field.chain().first, 1);
}

TEST(FieldTest, ChainDiagonal1) {
    using namespace Game;
    Pack pack2 = { 9,4,9,9 }; // pack2が上
    Pack pack1 = { 9,9,6,9 }; // 先に落ちるのはpack1
    Field field;
    EXPECT_TRUE(field.insert(pack1, 0));
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);
    EXPECT_TRUE(field.insert(pack2, 0));
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 2);
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);

    EXPECT_TRUE(field.insert(pack1, W - 2));
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);
    EXPECT_TRUE(field.insert(pack2, W - 2));
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 2);
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);

    field.fill(Game::None);
    field.insert(pack1, 0);
    field.insert(pack2, 0);
    EXPECT_EQ(field.chain().first, 1);
    field.insert(pack1, W - 2);
    field.insert(pack2, W - 2);
    EXPECT_EQ(field.chain().first, 1);
}

TEST(FieldTest, ChainDiagonal2) {
    using namespace Game;
    Pack pack2 = { 9,9,9,4 }; // pack2が上
    Pack pack1 = { 6,9,9,9 };
    Field field;
    EXPECT_TRUE(field.insert(pack1, 0));
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);
    EXPECT_TRUE(field.insert(pack2, 0));
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 2);
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);

    EXPECT_TRUE(field.insert(pack1, W - 2));
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);
    EXPECT_TRUE(field.insert(pack2, W - 2));
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 2);
    EXPECT_TRUE(field.fall());
    EXPECT_EQ(field.eliminate(), 0);

    field.fill(Game::None);
    field.insert(pack1, 0);
    field.insert(pack2, 0);
    EXPECT_EQ(field.chain().first, 1);
    field.insert(pack1, W - 2);
    field.insert(pack2, W - 2);
    EXPECT_EQ(field.chain().first, 1);
}

TEST(FieldTest, OverflowField) {
    using namespace Game;
    Pack pack = { 0,9,9,0 };
    Field field;
    int height = 0;
    for (int lop = 0; lop < H; ++lop) {

        EXPECT_TRUE(field.insert(pack, 0));
        bool ok = field.fall();
		bool ok2 = !field.isOverFlow();
		EXPECT_EQ(ok, ok2);
        height += 1;
        if (height > HLimit) {
            EXPECT_FALSE(ok) << "fall will fail H=" << height;
            break;
        }
        EXPECT_TRUE(ok) << "fall will success H=" << height;
    }
}

TEST(FieldTest, Eliminate) {
    using namespace Game;
    ifstream ifs("../../eliminatetest.txt");
    Field field;
    while (true) {
        string testcasename;
        ifs >> testcasename;
        if (testcasename == "EOF") break;

        field.fill(0);
        int h, w;
        ifs >> h >> w;

        rrepeat(y, h) {
            repeat(x, w) {
                int a; ifs >> a;
                field(y, x) = a;
            }
        }

        while (true) {
            string cmd;
            ifs >> cmd;
            int ret = 0;
            if (cmd == "OK") break;
            if (cmd == "EL") ret = field.eliminate();
            if (cmd == "EX") ret = field.explode();
            field.fall();

            rrepeat(y, h) {
                repeat(x, w) {
                    int a; ifs >> a;
                    ASSERT_EQ((int)field(y, x), (int)a)
                        << "fail: " << testcasename << "(" << y << "," << x << ") ret:" << ret;
                }
            }
        }

    }
}


TEST(DataStructTest, PField1) {
    using namespace Game;
    Pack pack = { 6,7,8,9 };
    Field field1;
    auto pf1 = PField::create(field1);
    Field field2 = field1;
    field2.insert(pack, 0);
    auto pf2 = PField::append(pf1, field2);
    Field field3 = field2;
    field3.insert(pack, 1);
    auto pf3 = PField::append(pf2, field3);
    Field field3a = field2;
    field3a.insert(pack, 2);
    auto pf3a = PField::append(pf2, field3a);
    {
        auto f1 = pf1->generate();
        auto f2 = pf2->generate();
        auto f3 = pf3->generate();
        auto f3a = pf3a->generate();
        ASSERT_EQ(field1, f1);
        ASSERT_EQ(field2, f2);
        ASSERT_EQ(field3, f3);
        ASSERT_EQ(field3a, f3a);
    }
}


TEST(DataStructTest, PField2) {
    using namespace Game;
    Pack pack = { 6,7,8,9 };
    Field field1;
    auto pf1 = PField::create(field1);
    Field field2 = field1;
    field2.insert(pack, 0);
    auto pf2 = PField::append_hint(pf1, field2, field1);
    Field field3 = field2;
    field3.insert(pack, 1);
    auto pf3 = PField::append_hint(pf2, field3, field2);
    Field field3a = field2;
    field3a.insert(pack, 2);
    auto pf3a = PField::append_hint(pf2, field3a, field2);
    {
        auto f1 = pf1->generate();
        auto f2 = pf2->generate();
        auto f3 = pf3->generate();
        auto f3a = pf3a->generate();
        ASSERT_EQ(field1, f1);
        ASSERT_EQ(field2, f2);
        ASSERT_EQ(field3, f3);
        ASSERT_EQ(field3a, f3a);
    }
}
