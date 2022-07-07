#pragma once

#include <cstddef>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>

#include "array_ptr.h"

class ReserveProxyObj {
    size_t capacity_to_reserve_ = 0;
public:
    ReserveProxyObj(size_t capacity_to_reserve)
        : capacity_to_reserve_(capacity_to_reserve)
    {}
    size_t capacity() const {
        return capacity_to_reserve_;
    }
};

template <typename Type>
class SimpleVector {
    ArrayPtr<Type> array_;

public:

    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;
    ~SimpleVector() = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : array_(size)
    {}

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : array_(size)
    {
        for (Iterator it = begin(); it != end(); ++it) {
            *it = value;
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : array_(init.size())
    {
        assert(array_.GetSize() >= init.size());
        size_t i = 0;
        for (auto v : init) {
            array_[i++] = v;
        }
    }

    // Конструктор для резервирования
    SimpleVector(const ReserveProxyObj& reserveObj)
        : array_{0, reserveObj.capacity() }
    {}

    SimpleVector(const SimpleVector& other) = default;
    SimpleVector& operator=(const SimpleVector& rhs) = default;

    SimpleVector(SimpleVector&& other) = default;
    SimpleVector& operator=(SimpleVector&& rhs) = default;

    void swap(SimpleVector& other) noexcept {
        std::swap(this->array_, other.array_);
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        size_t old_size = array_.GetSize();
        GrowIfNeed();
        array_.SetSize(old_size+1);
        array_[old_size] = item;
    }

    // move версия
    void PushBack(Type&& item) {
        size_t old_size = array_.GetSize();
        GrowIfNeed();
        array_.SetSize(old_size+1);
        array_[old_size] = std::move(item);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t old_size = array_.GetSize();
        auto index = std::distance(cbegin(), pos);
        GrowIfNeed();
        array_.SetSize(old_size + 1);
        auto new_pos = Iterator{&array_[index]};
        std::copy(new_pos, end(), std::next(begin(), index + 1));
        array_[index] = value;
        return new_pos;
    }

    // move версия
    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t old_size = array_.GetSize();
        auto index = std::distance(cbegin(), pos);
        GrowIfNeed();
        array_.SetSize(old_size + 1);
        auto new_pos = Iterator{&array_[index]};
        std::copy(std::make_move_iterator(new_pos),
                  std::make_move_iterator(end()),
                  std::next(begin(), index + 1));
        array_[index] = std::move(value);
        return new_pos;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (IsEmpty() != true) {
            array_.SetSize(array_.GetSize() - 1);
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        auto index = std::distance(cbegin(), pos);
        Iterator it_start = Iterator{&array_[index+1]};
        std::copy(std::make_move_iterator(it_start),
                  std::make_move_iterator(end()),
                  std::next(begin(), index));
        array_.SetSize(array_.GetSize() - 1);
        return ((static_cast<size_t>(index) >= GetSize()) ? end() : Iterator{&array_[index]});
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return array_.GetSize();
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return array_.GetCapacity();
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return (array_.GetSize() == 0);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        return array_.At(index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        return array_.At(index);
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        array_.SetSize(0);
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        array_.Resize(new_size);
    }

    // Резервируем память под массив.
    void Reserve(size_t new_capacity) {
        array_.Reserve(new_capacity);
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return Iterator(IsEmpty() ? nullptr : &array_[0]);
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return Iterator(IsEmpty() ? nullptr : &array_[GetSize()]);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return ConstIterator(IsEmpty() ? nullptr : &array_[0]);
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return ConstIterator(IsEmpty() ? nullptr : &array_[GetSize()]);
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator(IsEmpty() ? nullptr : &array_[0]);
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ConstIterator(IsEmpty() ? nullptr : &array_[GetSize()]);
    }

private:
    void GrowIfNeed() {
        size_t current_size = GetSize();
        if (current_size == 0) {
            Resize(1);
        } else if (current_size >= GetCapacity()) {
            Resize(GetCapacity() * 2);
        }
    }
}; // class SimpleVector<T>

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return ((lhs.GetSize() == rhs.GetSize()) &&
            std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()));
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(
            lhs.begin(), lhs.end(),
            rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs < rhs || lhs == rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs <= rhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
