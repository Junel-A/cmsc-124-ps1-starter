## 1. Three categories and what the "free" version costs:

**Integers, Python.** A thing in Python is that `a + b` does not and will never overflow. It's because integers just grow as big as they need to. Unlike in our `dt_int.c`, we have to check before every add, subtract, and multiply instead. What Python pays for that is a C `long long` is 8 bytes, while a Python int is a full object on the heap (reference count, type pointer, and digits). So even small values take around 28 bytes. Every `+` also passes through the interpreter as an operation on objects, not a single CPu instruction, and very big numbers take longer the more digits they have. We noticed that in a loop for example that adds millions of numbers. That is why libraries like NumPy use fixed-size machine integers and throws off the "never overflows" promise to get speed and memory. In C, we had to put the checking of input or variables before the operation because signed overflow is an undefined behavvior and an optimizing build is allowed to delete a check written after it. `dt_int_mul` was the longest because of the sign combinations and the `LLONG_MIN * -1` case.



## 2. Hand-written tag check



## 3. Dropping the insertion order from `dt_map`



## 4. Access after release vs. a leak at the final check