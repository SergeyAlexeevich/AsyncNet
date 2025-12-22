#pragma once

#include<iostream>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <new>
#include <utility>
#include <tuple>

template <typename Type>
class RawMemory;

template <typename T>
class Vector {
public:

    using iterator = T*;
    using const_iterator = const T*;

    Vector() = default;

    explicit Vector(size_t size)
        : data_(size)
        , size_(size) {

        std::uninitialized_value_construct_n(data_.GetAddress(), size);

    }

    Vector(const Vector& other)
        : data_(other.size_)
        , size_(other.size_) {
        std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());
    }

    Vector(Vector&& other) noexcept 
    : data_()
    , size_(0){
        Swap(other);
    }

    template<typename... Types>
    T& EmplaceBack(Types&&... values) {
       return *Emplace(begin() + size_, std::forward<Types>(values)...);
    }

    template <typename... Types>
    iterator Emplace(const_iterator pos, Types&&... values) {

        assert(pos >= begin() && pos <= end());

        iterator result = end();

        size_t shift =  pos - begin();

        if (size_ == data_.Capacity()) {

            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
            result = new (new_data.GetAddress() + shift) T(std::forward<Types>(values)...);
            if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                std::uninitialized_move(begin(), begin() + shift, new_data.GetAddress());
                if (shift < size_) {
                    std::uninitialized_move(begin() + shift, end(), new_data.GetAddress() + shift + 1);
                }
            }
            else {
                std::uninitialized_copy(begin(), begin() + shift, new_data.GetAddress());
                if (shift < size_) {
                    std::uninitialized_copy(begin() + shift, end(), new_data.GetAddress() + shift + 1);
                }
            }

            std::destroy_n(data_.GetAddress(), size_);
            data_.Swap(new_data);
        }
        else {
            if (shift < size_) {
                RawMemory<T> temp(1);
                new(temp.GetAddress()) T(std::forward<Types>(values)...);
                new(end()) T(std::move(*(end() - 1)));
                std::move_backward(begin() + shift, end() - 1, end());
                std::destroy_at(begin() + shift);
                data_[shift] = std::move(temp[0]);
                result = begin() + shift;
            }
            else {
                result = new(begin() + shift) T(std::forward<Types>(values)...);
            }
        }
        ++size_;

        return result;
    }

    iterator Erase(const_iterator  pos) noexcept(std::is_nothrow_move_assignable_v<T>) {
        iterator result = end();
        auto shift = std::distance(cbegin(), pos);
        if (size_ > 0) {
            std::destroy_n(pos, 1);
            if (pos != end() - 1) {
                if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                    std::move(begin() + shift + 1, end(), begin() + shift);
                }
                else {
                    std::copy_n(begin() + shift + 1, end(), begin() + shift);
                }
            }
            result = begin() + shift;
            --size_;
        }
        return result;
    }

    iterator Insert(const_iterator pos, const T& value) {
        return Emplace(pos, std::forward<const T&>(value));
    }

    iterator Insert(const_iterator pos, T&& value) {
        return Emplace(pos, std::forward<T&&>(value));
    }

    iterator begin() noexcept {
        return data_.GetAddress();
    }

    iterator end() noexcept {
        return data_.GetAddress() + size_;
    }

    const_iterator begin() const noexcept {
        return const_iterator(data_.GetAddress());
    }

    const_iterator end() const noexcept {
        return const_iterator(data_.GetAddress() + size_);
    }

    const_iterator cbegin() const noexcept {
        return data_.GetAddress();
    }

    const_iterator cend() const noexcept {
        return end();
    }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            if (other.size_ > data_.Capacity()) {
                Vector other_copy(other);
                Swap(other_copy);
            }
            else {
                if (size_ > other.size_) {
                    std::copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());
                    std::destroy_n(data_.GetAddress() + other.size_, size_ - other.size_);
                }
                else if (size_ < other.size_) {
                    std::copy_n(other.data_.GetAddress(), size_, data_.GetAddress());
                    std::uninitialized_copy_n(other.data_.GetAddress() + size_, other.size_ - size_, data_.GetAddress() + size_);
                }
                size_ = other.size_;
            }
        }
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            if (size_ > 0) {
                std::destroy_n(data_.GetAddress(), size_);
                data_ = std::move(other.data_);
                size_ = 0;
            }
            else {
                Swap(other);
            }
        }
        return *this;
    }

    void Reserve(size_t new_capacity) {

        if (new_capacity <= data_.Capacity()) {
            return;
        }

        RawMemory<T> new_data(new_capacity);

        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
        }
        else {
            std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
        }

        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);

    }

    size_t Size() const noexcept {
        return size_;
    }

    size_t Capacity() const noexcept {
        return data_.Capacity();
    }

    const T& operator[](size_t index) const noexcept {
        return const_cast<Vector&>(*this)[index];
    }

    T& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }

    void PopBack() noexcept {
        Erase(end() - 1);
    }

    void PushBack(const T& value) {
        Emplace(begin() + size_, std::forward<const T&>(value));
    }

    void PushBack(T&& value) {
        Emplace(begin() + size_, std::forward<T&&>(value));
    }

    void Resize(size_t size) {

        if (size_ < size) {
            Reserve(size);
            std::uninitialized_value_construct_n(data_.GetAddress() + size_, size - size_);
            size_ = size;
        }
        else if (size_ > size) {
            std::destroy_n(data_.GetAddress() + size, size_ - size);
            size_ = size;
        }
    }

    void Swap(Vector& other) noexcept {
        data_.Swap(other.data_);
        std::swap(size_, other.size_);
    }

    ~Vector() {
        std::destroy_n(data_.GetAddress(), size_);
    }

private:
    RawMemory<T> data_;
    size_t size_ = 0;
};

template <typename Type>
class RawMemory {
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity)
        : buffer_(Allocate(capacity))
        , capacity_(capacity) {
    }

    RawMemory(const RawMemory&) = delete;
    RawMemory& operator=(const RawMemory& other) = delete;

    RawMemory(RawMemory&& other) noexcept 
        : buffer_(std::move(other.buffer_))
        , capacity_(std::move(other.capacity_)) {
        other.buffer_ = nullptr;
        other.capacity_ = 0;
    }

    RawMemory& operator=(RawMemory&& other) noexcept {
        if (this != &other) {
            if (capacity_ > 0) {
                Deallocate(buffer_);
                buffer_ = nullptr;
                capacity_ = 0;
            }
            Swap(other);
        }

        return *this;
    }

    ~RawMemory() {
        Deallocate(buffer_);
        capacity_ = 0;
        buffer_ = nullptr;
    }

    Type* operator+(size_t offset) noexcept {
        // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
        assert(offset <= capacity_);
        return buffer_ + offset;
    }

    const Type* operator+(size_t offset) const noexcept {
        return const_cast<RawMemory&>(*this) + offset;
    }

    const Type& operator[](size_t index) const noexcept {
        return const_cast<RawMemory&>(*this)[index];
    }

    Type& operator[](size_t index) noexcept {
        assert(index < capacity_);
        return buffer_[index];
    }

    void Swap(RawMemory& other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }

    const Type* GetAddress() const noexcept {
        return buffer_;
    }

    Type* GetAddress() noexcept {
        return buffer_;
    }

    size_t Capacity() const {
        return capacity_;
    }

    Type* operator++(int) {
        Type* temp = buffer_;
        ++buffer_;
        return temp;
    }

    Type& operator++() {
        ++buffer_;
        return buffer_;
    }

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static Type* Allocate(size_t n) {
        return n != 0 ? static_cast<Type*>(operator new(n * sizeof(Type))) : nullptr;
    }

    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(Type* buf) noexcept {
        operator delete(buf);
    }

    Type* buffer_ = nullptr;
    size_t capacity_ = 0;
};
