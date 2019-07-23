import time


def fibo(n):

    f_n0 = 0
    f_n1 = 1

    if n == 0:
        return f_n0

    if n == 1:
        return f_n1

    k = 1
    f_n2 = 0

    while k < n:
        f_n2 = f_n0 + f_n1
        f_n0 = f_n1
        f_n1 = f_n2
        k += 1

    return f_n2


if __name__ == "__main__":

    n = 1 << 17  # 1 << 21
    now = time.perf_counter
    start = now()
    r = fibo(n)
    end = now()

    print("n = {}    {:x}".format(n, r))

    elapsed = end - start
    print("elapsed ms:", elapsed * 1000)
