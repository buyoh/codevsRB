#pragma once

#include "./Macro.h"

template<typename T>
class IntervalHeap {
    // アルゴリズム概要
    // Heapの各ノードは1~2つの要素を持つ．この2つの要素は順序関係によって区間とみなすことができる．
    // 次を満たすようにHeapを保つ：子⊆親

    using TT = pair<T, T>; // first <= second

    vector<TT> data_; // data_の配列要素は2つの要素を持つ．data_.size() は (size_ + 1) / 2 と等価
    int size_; // データ構造が持つ要素の数

    // data_[ptr]は1つの要素しか持っていない？
    inline bool one(int ptr) const noexcept {
        // ptr == 末尾 かつ 末尾を1要素しか持たない
        return ptr == (size_ - 1) / 2 && (size_ & 1) == 1;
    }

    void fixup(int ptr);
    void fixdown(int ptr);

public:

    inline IntervalHeap() :size_(0) { }

    inline void reserve(size_t s) { data_.reserve(s); }
    inline size_t size() const noexcept { return size_; }
    inline void clear() {
        size_ = 0;
        data_.clear();
    }

    // TODO forward
    inline void push(const T & elem) {
        // 1. とりあえず末尾に追加
        if ((size_ & 1) == 0) {
            data_.emplace_back(elem, T());
        }
        else {
            data_.back().second = elem;
            if (data_.back().first > data_.back().second)
                swap(data_.back().first, data_.back().second);
        }
        ++size_;
        // 2. 整合
        fixup((size_ - 1) / 2);
    }

    inline const T & min() const {
        return data_.front().first;
    }

    inline const T& max() const {
        return size_ <= 1 ? data_.front().first : data_.front().second;
    }

    void popmin() {
        if (size_ <= 0)
            return;
        --size_;
        if (size_ == 1) {
            data_.pop_back();
        }
        else if (size_ == 2) {
            swap(data_.front().first, data_.front().second);
        }
        else if ((size_ & 1) == 0) {
            // 現在偶数個あり，奇数個に減る
            // 末尾と交換
            swap(data_.front().first, data_.back().second);
            fixdown(0);
        }
        else {
            // 現在奇数個あり，偶数個に減る
            // 末尾と交換
            swap(data_.front().first, data_.back().first);
            data_.pop_back();
            // 整合
            fixdown(0);
        }
    }

    void popmax() {
        if (size_ <= 0)
            return;
        --size_;
        if (size_ == 1) {
            data_.pop_back();
        }
        else if (size_ == 2) {
        }
        else if ((size_ & 1) == 0) {
            // 現在偶数個あり，奇数個に減る
            // 末尾と交換
            swap(data_.front().second, data_.back().second);
            // 整合
            fixdown(0);
        }
        else {
            // 現在奇数個あり，偶数個に減る
            // 末尾と交換
            swap(data_.front().second, data_.back().first);
            data_.pop_back();
            // 整合
            fixdown(0);
        }
    }
};
