import time


def tarai(x, y, z):

    if x <= y:
        return y
    else:
        return tarai(tarai(x - 1, y, z), tarai(y - 1, z, x), tarai(z - 1, x, y))


class Int:

    def __init__(self, value):
        self._value = int(value)

    @property
    def value(self):
        return self._value

    def le(self, other):
        return self._value <= other._value

    def sub(self, other):
        return Int(self._value - other._value)


def Int_tarai(x, y, z):

    if x.le(y):
        return y
    else:
        return Int_tarai(Int_tarai(x.sub(Int(1)), y, z), Int_tarai(y.sub(Int(1)), z, x), Int_tarai(z.sub(Int(1)), x, y))


if __name__ == "__main__":

    now = time.perf_counter
    start = now()
    r = tarai(12, 6, 0)
    end = now()

    print("tarai(12, 6, 0) = {}".format(r))

    elapsed = end - start
    print("elapsed ms:", elapsed * 1000)

    start = now()
    r = Int_tarai(Int(12), Int(6), Int(0)).value
    end = now()

    print("Int_tarai(12, 6, 0) = {}".format(r))

    elapsed = end - start
    print("elapsed ms (wrapped):", elapsed * 1000)
