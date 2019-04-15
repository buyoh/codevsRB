#include "pch.h"
#include "../codevsRB/IntervalHeap.h"
//#include "../codevsRB/Exec.h"

TEST(IntervalHeapTest, Easy) { // TODO:
    IntervalHeap<int> ih;
    ih.push(3);
    ih.push(1);
    ih.push(4);
    EXPECT_TRUE((ih.max() == 4));
    EXPECT_TRUE((ih.min() == 1));
}
