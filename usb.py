#! /usr/bin/python
#
#
# \Author  Hans Kramer
#
# \Date    Jan 2016
#


import os
import types


class USBus(object):

    DEV_PATH = "/sys/bus/usb/devices/"

    def __init__(self):
        self._device_paths = [x for x in self._get_devices_paths()]
        self._id2device    = {k:v for k,v in self._get_devices_ids()}

    def _get_devices_paths(self):
        for device in os.listdir(self.DEV_PATH):
            path = os.path.join(self.DEV_PATH, device, "idProduct")
            if os.path.exists(path):
                yield os.path.join(self.DEV_PATH, device)

    def _get_devices_ids(self):
        for device_path in self._device_paths:
            try:
                vendor  = int(open(os.path.join(device_path, "idVendor")).readline(),  16)
                product = int(open(os.path.join(device_path, "idProduct")).readline(), 16)
                yield (vendor, product), device_path
            except OSError:
                pass

    def get_device(self, id, refresh=False):
        if refresh:
            self.__init__()
        if isinstance(id, types.StringTypes):
            id = tuple([int(x, 16) for x in id.split(":")])
        try:
            return USBDevice(self._id2device[id])
        except KeyError:
            return USBDevice(None)

    def __call__(self):
        for k in self._id2device.iterkeys():
            yield k



class USBDevice(object):

    def __init__(self, usb_path):
       self._path = usb_path

    def _read_line(self, item):
        if self._path is None:
            return None
        try:
            return open(os.path.join(self._path, item)).readline().strip(' \n')
        except (OSError, IOError) as e:
            return None

    def get_version(self):
        return self._read_line("version")

    def get_serial(self):
        return self._read_line("serial")

    def active_duration(self):
        return self._read_line("power/active_duration")


if __name__ == "__main__":
    import unittest

    class TestUSB(unittest.TestCase):
      
        def __init__(self, *args):
            super(TestUSB, self).__init__(*args)
            self._bus = USBus()
 
        def test_open_bus(self):
            self.assertIsInstance(self._bus, USBus)
            device_list = [x for x in self._bus()]
            self.assertIsInstance(device_list, type(list()))

        def test_random_device(self):
            id = list(self._bus())[0]
            self.assertIsInstance(id, type(tuple()))
            device = self._bus.get_device(id)
            self.assertIsInstance(device, USBDevice)
            id_str = "{:04X}:{:04X}".format(*id)
            device = self._bus.get_device(id_str)
            self.assertIsInstance(device, USBDevice)
            version = device.get_version()
            self.assertIsInstance(version, type(""))
            self.assertIn(version, ["1.00", "2.00", "3.00"])
            

    unittest.main()

