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
template <typename T>
class dynamic_pod_array {
    T* data_;
    std::size_t size_;
    pod_allocator<T> alloc_;

public:
    explicit dynamic_pod_array(std::size_t n)
        : size_(n), data_(alloc_.allocate(n)) {
        std::memset(data_, 0, n * sizeof(T));
    }

    ~dynamic_pod_array() {
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
};

// ==============================
// 5. Пример использования

struct Point {
    int x, y;
    Point() = delete; // Конструктор удален
};

int main() {

    {
        // Используем pod_storage
        pod_storage<Point> PointStorage;

        // Работаем через operator->
        PointStorage->x = 42;
        std::cout << "PointStorage->x = " << PointStorage->x << '\n';

        // Работаем через operator*
        std::cout << "(*PointStorage).x = " << (*PointStorage).x << '\n';

        // Можно получить указатель напрямую
        Point* p = PointStorage.get();
        p->x = 100;
        std::cout << "p->x = " << p->x << '\n';
    }

    // Используем pod_array
    {
        pod_array<Point, 3> points;

        points[0] = { 1, 2 };
        points[1] = { 3, 4 };
        points[2] = { 5, 6 };

        for (size_t i = 0; i < points.size(); ++i) {
            std::cout << "pod_array[" << i << "] = {"
                << points[i].x << ", " << points[i].y << "}\n";
        }
    }

    // Используем dynamic_pod_array
    {
        dynamic_pod_array<Point> points(5);

        for (size_t i = 0; i < points.size(); ++i) {
            points[i] = Point{ static_cast<int>(i), static_cast<int>(i * 2) };
        }

        for (size_t i = 0; i < points.size(); ++i) {
            std::cout << "dynamic_pod_array[" << i << "] = {"
                << points[i].x << ", " << points[i].y << "}\n";
        }
    }

    return 0;
}