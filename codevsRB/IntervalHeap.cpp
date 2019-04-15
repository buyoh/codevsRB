
#include "IntervalHeap.h"

template<typename T>
void IntervalHeap<T>::fixup(int ptr) {

    if (ptr <= 0) return;
    // ptr == ���� ���� ������1�v�f���������Ȃ��ꍇ
    if (one(ptr)) {
        int parent = (ptr - 1) / 2;
        if (data_[ptr].first < data_[parent].first)
            swap(data_[ptr].first, data_[parent].first); // ���[
        else if (data_[ptr].first > data_[parent].second)
            swap(data_[ptr].first, data_[parent].second); // �E�[
        ptr = parent;
    }
    while (ptr > 0) {
        int parent = (ptr - 1) / 2;
        if (data_[ptr].first < data_[parent].first)
            swap(data_[ptr].first, data_[parent].first); // ���[
        if (data_[ptr].second > data_[parent].second)
            swap(data_[ptr].second, data_[parent].second); // �E�[
        ptr = parent;
    }
}

template<typename T>
void IntervalHeap<T>::fixdown(int ptr) {
    // ptr == ���� ���� ������1�v�f���������Ȃ��ꍇ
    if (one(ptr)) return;

    // ptr�̐���
    if (data_[ptr].first > data_[ptr].second)
        swap(data_[ptr].first, data_[ptr].second);

    int child = ptr * 2 + 1; // ���̎q��id
    if (child >= (size_ + 1) / 2) // ���̎q�����Ȃ�
        return;

    if (child + 1 >= (size_ + 1) / 2) { // ���̎q�������Ȃ�
        if (data_[child].first < data_[ptr].first)
            swap(data_[child].first, data_[ptr].first); // ���[
        if (!one(child) && data_[child].second > data_[ptr].second)
            swap(data_[child].second, data_[ptr].second); // �E�[
        // child�̐���
        if (!one(child) && data_[child].first > data_[child].second)
            swap(data_[child].first, data_[child].second);
        return;
    }
    bool leftfix = false, rightfix = false;
    if (data_[child].first < data_[child + 1].first) { // ���̎q���E�̎q��菬����
        if (data_[child].first < data_[ptr].first)
            swap(data_[child].first, data_[ptr].first); // ���[
        leftfix = true;
    }
    else {// �E�̎q�����̎q��菬����
        if (data_[child + 1].first < data_[ptr].first)
            swap(data_[child + 1].first, data_[ptr].first); // ���[
        rightfix = true;
    }
    if (one(child + 1) || data_[child].second > data_[child + 1].second) { // ���̎q���E�̎q���傫�� �܂��� �E�̎q��1�v�f���������Ă��Ȃ�
        if (data_[child].second > data_[ptr].second)
            swap(data_[child].second, data_[ptr].second); // �E�[
        leftfix = true;
    }
    else {// �E�̎q�����̎q���傫��
        if (data_[child + 1].second > data_[ptr].second)
            swap(data_[child + 1].second, data_[ptr].second); // �E�[
        rightfix = true;
    }

    if (leftfix && rightfix)
        fixdown(child), fixdown(child + 1);
    else if (leftfix) fixdown(child);
    else if (rightfix) fixdown(child + 1);
}