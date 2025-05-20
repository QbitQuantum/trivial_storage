#include <iostream>
#include <memory>
#include <type_traits>
#include <cstddef> 
#include <cstring>
#include <vector>
#include "trivial_storage.hpp"
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