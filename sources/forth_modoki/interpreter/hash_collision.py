import types
from random import choice

hash = lambda s: sum(map(ord, s)) % 1024

char_range = [('0', '9'), ('A', 'Z'), ('a', 'z')]
chars = lambda r: [chr(c) for c in range(ord(r[0]), ord(r[1]))]
possible_chars = sum([chars(r) for r in char_range], [])

## 深さ優先探索するやつ
def _search_preimage(orig_hash, m):
    if hash(m) == orig_hash:
        return m

    for c in possible_chars:
        return lambda: _search_preimage(orig_hash, m + c)

def search_another_preimage(key):
    original_hash = hash(key)
    message = ''

    return _search_preimage(original_hash, '')

def trampoline(fn):
    def proc(*args):
        v = fn(*args)
        while type(v) is types.FunctionType:
            v = v()
        return v

    return proc

## 幅優先探索するやつ
def search_another_preimage_width(key):
    original_hash = hash(key)
    message = ''
    queue = []

    while True:
        for c in possible_chars:
            queue.append(message + c)

        m = queue[0]
        queue = queue[1:]
        if hash(m) == original_hash:
            return m

key = 'key'

# 深さ優先探索 (みつからない)
#print(trampoline(search_another_preimage)(key))

# 幅優先探索 (みつからない)
#print(search_another_preimage_width(key))
