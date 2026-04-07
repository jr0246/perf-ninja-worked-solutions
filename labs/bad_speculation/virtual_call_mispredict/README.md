# Lab: virtual_call_mispredict

## Background:

We have a container of polymorphic objects. We iterate through its entries and call the virtual
`handle` function on each one.

The issue is that the container is randomly populated with polymorphic objects of type `ClassA`, `ClassB`, and `ClassC`.
This random distribution within the container causes difficulty for the branch predictor, which attempts to predict
which virtual function will be called on subsequent iterations. This results in many branch misses, which incur heavy
penalties.

The original code contains 21.5% branch misses, which is extremely high and will have a negative performance impact.

```
-----------------------------------------------------------------
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
bench1/iterations:1000        589 us          589 us         1000

 Performance counter stats for './lab':

       601,822,974      task-clock                       #    0.996 CPUs utilized             
                 7      context-switches                 #   11.631 /sec                      
                 1      cpu-migrations                   #    1.662 /sec                      
             1,060      page-faults                      #    1.761 K/sec                     
       698,155,664      instructions                     #    0.33  insn per cycle            
     2,144,081,553      cycles                           #    3.563 GHz                       
       205,311,281      branches                         #  341.149 M/sec                     
        44,134,143      branch-misses                    #   21.50% of all branches           

       0.604055757 seconds time elapsed
```

Using `perf record ./lab` to inspect hot spots, we can see the main overhead is incurred in the `invoke` method;
specifically, the calls to the virtual function, `item->handle(data)`:

```c++
void invoke(InstanceArray& array, std::size_t& data) {
    for (const auto& item: array) {
        item->handle(data);
    }
}
```

However, the root of the problem really comes from the random organisation of the polymorphic objects in the input
`InstanceArray`, which we will address below.

### Solution:

A common approach to reduce the impact of runtime polymorphism is to use static polymorphism via templates (such as
through the CRTP idiom, or through concepts in C++ 20+), but this is not available here due to the fact we have our
objects in a container, which limits us to homogeneity.

In any case, we can assist the hardware by grouping the objects of the same derived type together in the container.

We create three temporary `InstanceArray`s in `generateObjects` to hold each of `ClassA`, `ClassB`, and `ClassC`:

```c++
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
```

There are branches here, but these have a smaller impact than the branch misses and extra virtual table lookups.

We then insert these data into the return array in a grouped fashion. Note that we need to use `std::make_move_iterator`
to wrap these iterators, as `InstanceArray` is a collection of `std::unique_ptr`, which can only be moved (not copied):

```c++
array.insert(array.end(), std::make_move_iterator(a.begin()), std::make_move_iterator(a.end()));
array.insert(array.end(), std::make_move_iterator(b.begin()), std::make_move_iterator(b.end()));
array.insert(array.end(), std::make_move_iterator(c.begin()), std::make_move_iterator(c.end()));
```

The complete `generateObjects` function with the changes:

```c++
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
```

After these changes, we see the code ends up around three times faster, and with only 0.12% branch misses:

```
-----------------------------------------------------------------
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
bench1/iterations:1000        186 us          185 us         1000

 Performance counter stats for './lab':

       214,844,475      task-clock                       #    0.989 CPUs utilized             
                11      context-switches                 #   51.200 /sec                      
                 0      cpu-migrations                   #    0.000 /sec                      
             1,320      page-faults                      #    6.144 K/sec                     
       697,751,364      instructions                     #    1.02  insn per cycle            
       682,564,050      cycles                           #    3.177 GHz                       
       205,439,992      branches                         #  956.227 M/sec                     
           252,241      branch-misses                    #    0.12% of all branches           

       0.217148628 seconds time elapsed
```