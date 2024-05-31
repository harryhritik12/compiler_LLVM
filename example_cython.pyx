# example_cython.pyx

#cython: language_level=3 

def compute():
    result = 0
    for i in range(100):
        result += i
    return result
