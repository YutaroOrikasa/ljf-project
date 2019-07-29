import time


class Object:
    pass


def object_new(n):

    for _ in range(n):
        Object()


if __name__ == "__main__":

    n = 1 << 23
    now = time.perf_counter
    start = now()
    r = object_new(n)
    end = now()

    print("object_new({}) = {}".format(n, r))

    elapsed = end - start
    print("elapsed ms:", elapsed * 1000)
