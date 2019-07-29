import time


def fun(a, b, c):
    return a


def call_bench(n):

    for _ in range(n):
        fun(None, True, False)


if __name__ == "__main__":

    n = 1 << 23
    now = time.perf_counter
    start = now()
    r = call_bench(n)
    end = now()

    print("call_bench({}) = {}".format(n, r))

    elapsed = end - start
    print("elapsed ms:", elapsed * 1000)
