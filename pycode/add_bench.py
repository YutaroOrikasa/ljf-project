import time


def add_bench(n):
    ONE = 1
    a = 0
    while a <= n:
        a = a + ONE

    return a


class Int:

    def __init__(self, value):
        self._value = int(value)

    @property
    def value(self):
        return self._value

    def le(self, other):
        return self._value <= other._value

    def add(self, other):
        return Int(self._value + other._value)


def Int_add_bench(n):
    ONE = Int(1)
    a = Int(0)
    while a.le(n):
        a = a.add(ONE)

    return a


if __name__ == "__main__":

    n = 1 << 17
    now = time.perf_counter
    start = now()
    r = add_bench(n)
    end = now()

    print("add_bench({}) = {}".format(n, r))

    elapsed = end - start
    print("elapsed ms:", elapsed * 1000)

    start = now()
    r = Int_add_bench(Int(n)).value
    end = now()

    print("Int_add_bench({}) = {}".format(n, r))

    elapsed = end - start
    print("elapsed ms (wrapped):", elapsed * 1000)
