
#include "Macro.h"
#include "IntervalHeap.h"

template<typename T>
void IntervalHeap<T>::fixup(int ptr) {

    if (ptr <= 0) return;
    // ptr == 末尾 かつ 末尾を1要素しか持たない場合
    if (one(ptr)) {
        int parent = (ptr - 1) / 2;
        if (data_[ptr].first < data_[parent].first)
            swap(data_[ptr].first, data_[parent].first); // 左端
        else if (data_[ptr].first > data_[parent].second)
            swap(data_[ptr].first, data_[parent].second); // 右端
        ptr = parent;
    }
    while (ptr > 0) {
        int parent = (ptr - 1) / 2;
        if (data_[ptr].first < data_[parent].first)
            swap(data_[ptr].first, data_[parent].first); // 左端
        if (data_[ptr].second > data_[parent].second)
            swap(data_[ptr].second, data_[parent].second); // 右端
        ptr = parent;
    }
}

template<typename T>
void IntervalHeap<T>::fixdown(int ptr) {
    // ptr == 末尾 かつ 末尾を1要素しか持たない場合
    if (one(ptr)) return;

    // ptrの整合
    if (data_[ptr].first > data_[ptr].second)
        swap(data_[ptr].first, data_[ptr].second);

    int child = ptr * 2 + 1; // 左の子のid
    if (child >= (size_ + 1) / 2) // 左の子が居ない
        return;

    if (child + 1 >= (size_ + 1) / 2) { // 左の子しか居ない
        if (data_[child].first < data_[ptr].first)
            swap(data_[child].first, data_[ptr].first); // 左端
        if (!one(child) && data_[child].second > data_[ptr].second)
            swap(data_[child].second, data_[ptr].second); // 右端
        // childの整合
        if (!one(child) && data_[child].first > data_[child].second)
            swap(data_[child].first, data_[child].second);
        return;
    }
    bool leftfix = false, rightfix = false;
    if (data_[child].first < data_[child + 1].first) { // 左の子が右の子より小さい
        if (data_[child].first < data_[ptr].first)
            swap(data_[child].first, data_[ptr].first); // 左端
        leftfix = true;
    }
    else {// 右の子が左の子より小さい
        if (data_[child + 1].first < data_[ptr].first)
            swap(data_[child + 1].first, data_[ptr].first); // 左端
        rightfix = true;
    }
    if (one(child + 1) || data_[child].second > data_[child + 1].second) { // 左の子が右の子より大きい または 右の子は1要素しかもっていない
        if (data_[child].second > data_[ptr].second)
            swap(data_[child].second, data_[ptr].second); // 右端
        leftfix = true;
    }
    else {// 右の子が左の子より大きい
        if (data_[child + 1].second > data_[ptr].second)
            swap(data_[child + 1].second, data_[ptr].second); // 右端
        rightfix = true;
    }

    if (leftfix && rightfix)
        fixdown(child), fixdown(child + 1);
    else if (leftfix) fixdown(child);
    else if (rightfix) fixdown(child + 1);
}