#include "pch.h"
#include "../codevsRB/Game.h"
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
    Pack pack2 = { 9,4,9,9 }; // pack2����
    Pack pack1 = { 9,9,6,9 }; // ��ɗ�����̂�pack1
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
    Pack pack2 = { 9,9,9,4 }; // pack2����
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
    Pack pack = { 9,9,9,9 };
    Field field;
    int height = 0;
    for (int lop = 0; lop < H; ++lop) {

        EXPECT_TRUE(field.insert(pack, 0));
        bool ok = field.fall();
        height += 2;
        if (height >= HLimit) {
            EXPECT_FALSE(ok) << "fall will fail";
            break;
        }
        EXPECT_TRUE(ok) << "fall will success";
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