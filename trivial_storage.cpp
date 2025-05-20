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

    // Используем std::allocator через dynamic_pod_array
    {
        dynamic_pod_array<int> arr(4);
        for (size_t i = 0; i < arr.size(); ++i) {
            arr[i] = static_cast<int>(i * 10);
        }

        std::cout << "Using std::allocator:\n";
        for (size_t i = 0; i < arr.size(); ++i) {
            std::cout << "arr[" << i << "] = " << arr[i] << '\n';
        }
    }

    // Используем pod_allocator
    {
        dynamic_pod_array<Point, pod_allocator<Point>> arr(3);

        arr[0] = { 1, 2 };
        arr[1] = { 3, 4 };
        arr[2] = { 5, 6 };

        std::cout << "Using pod_allocator:\n";
        for (size_t i = 0; i < arr.size(); ++i) {
            std::cout << "Point[" << i << "] = {"
                << arr[i].x << ", " << arr[i].y << "}\n";
        }
    }

    return 0;
}