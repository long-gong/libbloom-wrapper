#!/usr/bin/env python3
from pybf import BloomFilter


def test_bf_int(num_items=1000000, error=0.01):
    """Test the BloomFilter with parameters 'num_items' & `error`"""
    print(f"Bloom filter test for integer keys")
    bf = BloomFilter(num_items, error)
    sz = len(bf)
    print(f'size: {sz}')
    # checking
    for i in range(100):
        assert not (i in bf)

    # insertion
    for i in range(num_items):
        bf.add(i)

    # no false negative
    for i in range(num_items):
        assert i in bf

    # false positive
    fc = 0
    tc = 0
    for i in range(num_items, 2 * num_items):
        if i in bf:
            fc += 1
        tc += 1
    fpr = 1.0 * fc / tc
    print(f"False positive rate: {fpr}")
    print("Passed!")


def test_bf_str(num_items=1000000, error=0.01):
    """Test the BloomFilter with parameters 'num_items' & `error`"""
    print(f"Bloom filter test for str keys")
    bf = BloomFilter(num_items, error)
    sz = len(bf)
    print(f'size: {sz}')
    # checking
    for i in range(100):
        assert not (str(i) in bf)

    # insertion
    for i in range(num_items):
        bf.add(str(i))

    # no false negative
    for i in range(num_items):
        assert str(i) in bf

    # false positive
    fc = 0
    tc = 0
    for i in range(num_items, 2 * num_items):
        if str(i) in bf:
            fc += 1
        tc += 1
    fpr = 1.0 * fc / tc
    print(f"False positive rate: {fpr}")
    print("Passed!")

if __name__ == "__main__":
    test_bf_int()
    test_bf_str()
# import bloom


# if __name__ == '__main__':
#     total_items = 9585058
#     error = 0.01
#     bf = bloom.init(total_items, error)
#     print(f'{bf}')
#     sz = bloom.size(bf)
#     print(f"{sz}")
# for i in range(total_items):
#     bf.add(i)
# cf = 0
# ct = 0
# for i in range(total_items, 2*total_items):
#     if i in bf:
#         cf +=  1
#     ct +=   1
# fpr = cf * 1.0 / ct * 100
# print("false positive rate: %.2f" % fpr)

