#include <iostream>
#include <memory>
#include <type_traits>
#include <cstddef> 
#include <cstring>
#include <vector>

// ==============================
// 1. pod_storage — хранение одного объекта без конструктора
template <typename T>
class pod_storage {
    alignas(T) std::byte data_[sizeof(T)];

public:
    pod_storage() noexcept {
        std::memset(data_, 0, sizeof(data_));
    }

    [[nodiscard]] T* get() noexcept {
        return std::launder(reinterpret_cast<T*>(data_));
    }

    [[nodiscard]] const T* get() const noexcept {
        return std::launder(reinterpret_cast<const T*>(data_));
    }

    T* operator->() noexcept { return get(); }
    const T* operator->() const noexcept { return get(); }

    T& operator*() noexcept { return *get(); }
    const T& operator*() const noexcept { return *get(); }
};

// ==============================
// 2. pod_array — статический массив без конструкторов
template <typename T, std::size_t N>
class pod_array {
    alignas(T) std::byte data_[N * sizeof(T)];

public:
    pod_array() noexcept {
        std::memset(data_, 0, sizeof(data_));
    }

    [[nodiscard]] T* data() noexcept {
        return std::launder(reinterpret_cast<T*>(data_));
    }

    [[nodiscard]] const T* data() const noexcept {
        return std::launder(reinterpret_cast<const T*>(data_));
    }

    T& operator[](std::size_t i) noexcept {
        return data()[i];
    }

    const T& operator[](std::size_t i) const noexcept {
        return data()[i];
    }

    static constexpr std::size_t size() noexcept {
        return N;
    }
};

// ==============================
// 3. Пользовательский аллокатор
template <typename T>
class pod_allocator {
public:
    using value_type = T;

    pod_allocator() = default;
    template <typename U>
    pod_allocator(const pod_allocator<U>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t n) {
        void* ptr = std::malloc(n * sizeof(T));
        if (!ptr) throw std::bad_alloc();
        return static_cast<T*>(ptr);
    }

    void deallocate(T* ptr, std::size_t /*n*/) noexcept {
        std::free(ptr);
    }
};

// Требуется для совместимости
template <typename T, typename U>
constexpr bool operator==(const pod_allocator<T>&, const pod_allocator<U>&) noexcept {
    return true;
}

template <typename T, typename U>
constexpr bool operator!=(const pod_allocator<T>& lhs, const pod_allocator<U>& rhs) noexcept {
    return !(lhs == rhs);
}

// ==============================
// 4. dynamic_pod_array — динамический массив с аллокатором
template <typename T, typename Allocator = std::allocator<T>>
class dynamic_pod_array {
private:
    T* data_;
    std::size_t size_;
    Allocator alloc_; // Используем пользовательский аллокатор

public:
    explicit dynamic_pod_array(std::size_t n)
        : size_(n), data_(alloc_.allocate(n)) {
        std::memset(data_, 0, n * sizeof(T));
    }

    ~dynamic_pod_array() {
        // Не вызываем деструкторы (POD)
        alloc_.deallocate(data_, size_);
    }

    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }

    T& operator[](std::size_t i) noexcept {
        return data()[i];
    }

    const T& operator[](std::size_t i) const noexcept {
        return data()[i];
    }

    std::size_t size() const noexcept {
        return size_;
    }

    const Allocator& get_allocator() const noexcept {
        return alloc_;
    }
};