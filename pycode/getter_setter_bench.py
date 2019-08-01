import time


class GS:

    def __init__(self, v):
        self._value = v

    def get(self):
        return self._value

    def set(self, v):
        self._value = v


def getter_setter_bench(n):
    k = 1
    G = GS(0)
    S = GS(0)
    for _ in range(n):
        r = k + G.get()
        S.set(r)

        G, S = S, G

    return G.get()


if __name__ == "__main__":

    # n = 1 << 23
    n = 1 << 20
    now = time.perf_counter
    start = now()
    r = getter_setter_bench(n)
    end = now()

    print("getter_setter_bench({}) = {}".format(n, r))

    elapsed = end - start
    print("elapsed ms:", elapsed * 1000)
