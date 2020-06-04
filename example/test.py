from pybloom import BloomFilter 


if __name__ == '__main__':
    total_items = 9585058
    error = 0.01
    bf = BloomFilter(capacity=total_items, error_rate=error)
    for i in range(total_items):
        bf.add(i)
    cf = 0
    ct = 0
    for i in range(total_items, 2*total_items):
        if i in bf:
            cf +=  1
        ct +=   1
    fpr = cf * 1.0 / ct * 100
    print("false positive rate: %.2f" % fpr)

