#! /usr/bin/python
#
# \Author  Hans Kramer
#
# \Date    Jan 2016
#


import collections


class Singleton(object):

    def __init__(self, klass):
        self.klass = klass
        self.cache = {}

    def __call__(self, *args):
        if not isinstance(args, collections.Hashable):
            return self.klass(*args)
        hash_key = (self.klass, args)
        if hash_key not in self.cache:
            self.cache[hash_key] = self.klass(*args)
        return self.cache[hash_key] 


def singleton(klass):
    instances = {}
    def getinstance(*args, **kwargs):
        if klass not in instances:
            instances[klass] = klass(*args, **kwargs)
        return instances[klass]
    return getinstance


if __name__ == "__main__":
    import unittest

    class TestSingleton(unittest.TestCase):
      
        def __init__(self, *args):
            pass

    # test it yourself!
    # okay okay, after the sprint
 
    unittest.main()
