#pragma once

#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <type_traits> // std::is_arithmetic_v
#include <algorithm>

template <typename Type>
class ArrayPtr {
    Type* memory_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;

public:

    ArrayPtr() noexcept
        : memory_(nullptr)
        , size_(0)
        , capacity_(0)
    {}

    explicit ArrayPtr(size_t size)
        : memory_(nullptr)
        , size_(0)
        , capacity_(0)
    {
        if (size > 0) {
            Resize(size);
        }
    }

    ArrayPtr(size_t size, size_t capacity)
        : memory_(nullptr)
        , size_(0)
        , capacity_(0)
    {
        Resize(capacity);
        SetSize(size);
    }

    ArrayPtr(const ArrayPtr & other) {
        ArrayPtr tmp;
        tmp.size_ = other.size_;
        tmp.capacity_ = other.capacity_;
        tmp.memory_ = new Type[tmp.GetCapacity()];
        std::copy(&other.memory_[0], &other.memory_[other.size_], tmp.memory_);
        swap(tmp);
    }

    ArrayPtr(ArrayPtr&& other) {
        swap(other);
    }

    ArrayPtr& operator=(const ArrayPtr & rhs) {
        if (&rhs != this) {
            ArrayPtr tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    ArrayPtr& operator=(ArrayPtr&& rhs) {
        if (&rhs != this) {
            if (memory_ != nullptr) {
                delete[] memory_;
                memory_ = nullptr;
            }
            size_ = 0;
            capacity_ = 0;
            swap(rhs);
        }
        return *this;
    }

    void swap(ArrayPtr& other) noexcept {
        std::swap(this->memory_,   other.memory_);
        std::swap(this->size_,     other.size_);
        std::swap(this->capacity_, other.capacity_);
    }

    ~ArrayPtr() {
        delete[] memory_;
    }

    size_t GetSize() const {
        return size_;
    }
    void SetSize(size_t value) {
        size_ = value;
    }

    size_t GetCapacity() const {
        return capacity_;
    }
    void SetCapacity(size_t value) {
        capacity_ = value;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return memory_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return memory_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index is invalid");
        }
        return memory_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index is invalid");
        }
        return memory_[index];
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        size_t old_size = GetSize();
        Reserve(new_size);
        if (std::is_arithmetic<Type>::value) {
            for (size_t i = old_size; i < new_size; ++i) {
                memory_[i] = 0;
            }
        }
        SetSize(new_size);
    }

    // Резервируем память под массив.
    void Reserve(size_t new_capacity) {
        if (new_capacity <= GetCapacity())
            return;
        Type * new_memory = nullptr;
        try {
            new_memory = new Type[new_capacity]();
            std::copy(std::make_move_iterator(memory_ + 0),
                      std::make_move_iterator(memory_ + size_),
                      new_memory);
            delete[] memory_;
            capacity_ = new_capacity;
            memory_ = new_memory;
            new_memory = nullptr;
        } catch(...) {
            if (new_memory != nullptr) {
                delete[] new_memory;
                new_memory = nullptr;
            }
        }
    }
}; // class ArrayPtr<T>
