import time


def object_new(n):

    for _ in range(n):
        object()


def int_new(n):
    for _ in range(n):
        int()


def str_new(n):
    for _ in range(n):
        str()


def dict_new(n):
    for _ in range(n):
        dict()


def dict_new2(n):
    for _ in range(n):
        dict(_=_)


def dict_new3(n):
    for _ in range(n):
        {"_": _}


def list_new(n):
    for _ in range(n):
        list()


# def list_new2(n):
#     for _ in range(n):
#         list(_)


def list_new3(n):
    for _ in range(n):
        [_]


def tuple_new(n):
    for _ in range(n):
        tuple()


# def tuple_new2(n):
#     for _ in range(n):
#         tuple(_)


def tuple_new3(n):
    for _ in range(n):
        (_,)


class Object:
    pass


def Object_new(n):

    for _ in range(n):
        Object()


class ObjectEmptySlot:
    __slots__ = ()


def ObjectEmptySlot_new(n):

    for _ in range(n):
        ObjectEmptySlot()


class LJFObjectEmu:

    def __init__(self):
        self._mutex = 0
        self._type_object = None
        self._table = {}
        self._hidden_table = {}
        self._array_table = []
        self._array = []
        self._function_id_table = {}
        self._native_data = 0


def LJFObjectEmu_new(n):

    for _ in range(n):
        LJFObjectEmu()


class LJFObjectEmuSlots:

    __slots__ = ("_mutex", "_type_object", "_table", "_hidden_table",
                 "_array_table", "_array", "_function_id_table", "_native_data")

    def __init__(self):
        self._mutex = 0
        self._type_object = None
        self._table = {}
        self._hidden_table = {}
        self._array_table = []
        self._array = []
        self._function_id_table = {}
        self._native_data = 0


def LJFObjectEmuSlots_new(n):

    for _ in range(n):
        LJFObjectEmuSlots()


def LJFObject_emu_tuple_new(n):

    for _ in range(n):
        (0, None, {}, {}, [], [], {}, 0)


def LJFObject_emu_mutex_init_emu_tuple_new(n):

    for _ in range(n):
        ([0] * 7, None, {}, {}, [], [], {}, 0)


if __name__ == "__main__":

    n = 1 << 23
    now = time.perf_counter

    for f in [object_new, int_new, str_new,
              dict_new, dict_new2, dict_new3,
              list_new, list_new3,
              tuple_new, tuple_new3,
              Object_new, ObjectEmptySlot_new,
              LJFObjectEmu_new, LJFObjectEmuSlots_new, LJFObject_emu_tuple_new, LJFObject_emu_mutex_init_emu_tuple_new]:

        name = f.__name__
        start = now()
        r = f(n)
        end = now()
        elapsed = end - start

        print("{}({}) = {}".format(name, n, r))

        print("elapsed ms:", elapsed * 1000)
        print()
