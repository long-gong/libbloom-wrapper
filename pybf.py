"""Python wrapper for libbloom."""

import typing
import bloom
import numpy as np

class BloomFilter:
    def __init__(self, entries, error, seed=None):
        """constructor

        Arguments:
            entries {[type]} -- [description]
            error {[type]} -- [description]

        Keyword Arguments:
            seed {bool} -- [description] (default: {True})
        """
        self.bloom_ptr = bloom.init(entries, error)
        if not (seed is None):
            bloom.set_seed(self.bloom_ptr, seed)

    def __del__(self):
        """deconstructor"""
        bloom.free(self.bloom_ptr)

    def __contains__(self, key):
        """membership test"""
        if isinstance(key, int):
            return bloom.check(self.bloom_ptr, key) == 1
        elif isinstance(key, str):
            return bloom.check_str(self.bloom_ptr, key, len(key)) == 1
        elif isinstance(key, (np.uint32,np.int32)):
            return bloom.check(self.bloom_ptr, int(key)) == 1
        else:
            raise NotImplementedError("Not implemented for key type: %s" % type(key))


    def add(self, key):
        """insert an object"""
        if isinstance(key, int):
            bloom.add(self.bloom_ptr, key)
        elif isinstance(key, str):
            bloom.add_str(self.bloom_ptr, key, len(key))
        elif isinstance(key, (np.uint32,np.int32)):
            bloom.add(self.bloom_ptr, int(key))
        else:
            raise NotImplementedError("Not implemented for key type: %s" % type(key))

    def __len__(self):
        """size of the underlying bit array"""
        return bloom.size(self.bloom_ptr)

    def num_hashes(self):
        """number of hash functions """
        return bloom.num_hash(self.bloom_ptr)

    def error(self):
        """expected false positive rate"""
        return bloom.error(self.bloom_ptr)

    def print(self):
        """print this bloom filter"""
        bloom.print(self.bloom_ptr)

    def fpr(self):
        """estimated false positive rate"""
        ret = bloom.fpr(self.bloom_ptr)
        if ret is None:
            raise NotImplementedError(
                "fpr() is not implmented by default, if you want to use it, please read README.md carefully."
            )
        return ret
