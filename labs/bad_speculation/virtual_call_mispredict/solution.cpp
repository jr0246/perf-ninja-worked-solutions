#include "solution.h"

#include <random>

void generateObjects(InstanceArray& array) {
    std::default_random_engine generator(0);
    std::uniform_int_distribution<std::uint32_t> distribution(0, 2);

    InstanceArray a;
    InstanceArray b;
    InstanceArray c;

    for (std::size_t i = 0; i < N; i++) {
        int value = distribution(generator);
        if (value == 0) {
            a.push_back(std::make_unique<ClassA>());
        } else if (value == 1) {
            b.push_back(std::make_unique<ClassB>());
        } else {
            c.push_back(std::make_unique<ClassC>());
        }
    }
    array.insert(array.end(), std::make_move_iterator(a.begin()), std::make_move_iterator(a.end()));
    array.insert(array.end(), std::make_move_iterator(b.begin()), std::make_move_iterator(b.end()));
    array.insert(array.end(), std::make_move_iterator(c.begin()), std::make_move_iterator(c.end()));
}

// Invoke the `handle` method on all instances in `output`
void invoke(InstanceArray& array, std::size_t& data) {
    for (const auto& item: array) {
        item->handle(data);
    }
}
