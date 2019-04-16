#pragma once

#include "./Macro.h"

template<typename T>
class IntervalHeap {
    // �A���S���Y���T�v
    // Heap�̊e�m�[�h��1~2�̗v�f�����D����2�̗v�f�͏����֌W�ɂ���ċ�ԂƂ݂Ȃ����Ƃ��ł���D
    // ���𖞂����悤��Heap��ۂF�q���e

    using TT = pair<T, T>; // first <= second

    vector<TT> data_; // data_�̔z��v�f��2�̗v�f�����Ddata_.size() �� (size_ + 1) / 2 �Ɠ���
    int size_; // �f�[�^�\�������v�f�̐�

    // data_[ptr]��1�̗v�f���������Ă��Ȃ��H
    inline bool one(int ptr) const noexcept {
        // ptr == ���� ���� ������1�v�f���������Ȃ�
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
        // 1. �Ƃ肠���������ɒǉ�
        if ((size_ & 1) == 0) {
            data_.emplace_back(elem, T());
        }
        else {
            data_.back().second = elem;
            if (data_.back().first > data_.back().second)
                swap(data_.back().first, data_.back().second);
        }
        ++size_;
        // 2. ����
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
            // ���݋�������C��Ɍ���
            // �����ƌ���
            swap(data_.front().first, data_.back().second);
            fixdown(0);
        }
        else {
            // ���݊����C�����Ɍ���
            // �����ƌ���
            swap(data_.front().first, data_.back().first);
            data_.pop_back();
            // ����
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
            // ���݋�������C��Ɍ���
            // �����ƌ���
            swap(data_.front().second, data_.back().second);
            // ����
            fixdown(0);
        }
        else {
            // ���݊����C�����Ɍ���
            // �����ƌ���
            swap(data_.front().second, data_.back().first);
            data_.pop_back();
            // ����
            fixdown(0);
        }
    }
};
