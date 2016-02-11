////////////////////////////////////////////////////////////////////////////////
/// \file usb.cpp
///
/// \dude Hans Kramer
///
/// \brief 
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>
#include <dirent.h>
#include <algorithm>
#include <map>

#include <stdio.h>


class ReadDir {
  
public:
    ReadDir(std::string path) {
        DIR *dir = opendir(path.c_str());
        if (dir == nullptr)
            return;
        struct dirent *ent;
        while ((ent = readdir(dir)) != nullptr) {
            this->dir_data.push_back(ent->d_name);
        }
        closedir(dir);
    };

    std::vector<std::string> get_files() const {
        return dir_data;
    };

    std::vector<std::string> get_visible_files() const {
        std::vector<std::string> output(this->dir_data.size());
        auto it = std::copy_if(this->dir_data.begin(), this->dir_data.end(), output.begin(), [](std::string v){return v[0] != '.';} );
        output.resize(std::distance(output.begin(),it));
        return output;
    };

private:
    std::vector<std::string> dir_data;

};


#include <unistd.h>
#include <fstream>
#include <iostream>


// In Python I had this as a singleton .... using a class decorator 
class USBDevice {

public:
    USBDevice(std::string path) {
        this->path = path; 
    }

    int get_speed() {
        int speed;
        this->read_value("speed", speed);
        return speed;
    }

    std::string get_version() {
        std::string version;
        this->read_value("version", version);
        return version;
    }

    std::string get_serial() {
        std::string serial;
        this->read_value("serial", serial);
        return serial;
    }

    std::string get_active_duration() {
        std::string duration;
        this->read_value("power/active_duration", duration);
        return duration;
    }

    std::string get_device_class() {
        std::string device_class;
        this->read_value("bDeviceClass", device_class);
        return device_class;
    }

private:
    std::string path;

    bool read_value(std::string item, std::string &value) {
        value = this->read_line(item);
        return true;
    }

    bool read_value(std::string item, int &value) {
        try {
            std::string line = this->read_line(item);
            value            = std::stoi(line);
            return true;
        } catch (...) {
            return false;
        }
        return true;
    }

    std::string read_line(std::string item) {
        std::string   line;

        std::string   path = this->path + "/" + item;
        std::ifstream file(path); 

        if (file.is_open()) {
            getline(file, line);
            file.close();
        }

        return line; 
    }
};


class USBus {

public:
    USBus() {
        this->get_device_paths();        
        this->get_device_ids();
    }

    unsigned int get_bus_count() {
        unsigned int count = 0;

        for (const auto device : ReadDir(this->usb_device_path).get_visible_files()) 
            if (device.compare(0, 3, "usb") == 0)
                count++;
 
        return count;
    }

    unsigned int get_device_count() {
        unsigned int count = 0;

        for (auto device : this->id2device) 
            if (USBDevice(device.second).get_device_class().compare("00") == 0)
                count++;
  
        return count;
    }

    USBDevice get_device(std::string device_id) {
        auto it = this->id2device.find(device_id);
        return USBDevice(it != this->id2device.end() ? it->second : "");
    }

    std::vector<USBDevice> get_all_devices(std::string device_id) {
        std::vector<USBDevice> output;

        auto range = this->id2device.equal_range(device_id);
        for (auto d=range.first; d!=range.second; d++)
            output.push_back(USBDevice(d->second));

        return output;
    }

void debug() {
   for (auto it : this->device_paths) {
       printf("%s\n", it.c_str());
   }
   for (std::multimap<std::string, std::string>::iterator it = this->id2device.begin();
        it != this->id2device.end(); ++it) {
        std::cout << "  [" << (*it).first << ", " << (*it).second << "]" << std::endl;
   }

}

protected:
    void get_device_paths() {
        for (const auto device : ReadDir(this->usb_device_path).get_visible_files()) {
            std::string idproduct = this->usb_device_path + device + "/" + "idProduct";
            if (access(idproduct.c_str(), F_OK) != -1)
                this->device_paths.push_back(this->usb_device_path + device);
        }
    }

    void get_device_ids() {
        for (const auto dev_path : this->device_paths) 
            this->id2device.insert(std::pair<std::string, std::string>(this->get_device_id(dev_path), dev_path));
    }

private:
    std::vector<std::string>                device_paths;
    std::multimap<std::string, std::string> id2device;

    const std::string usb_device_path = "/sys/bus/usb/devices/";

    std::string get_device_id(const std::string dev_path) {
        std::string vendor  = this->get_device_info(dev_path + "/" + "idVendor");
        std::string product = this->get_device_info(dev_path + "/" + "idProduct");

        return vendor + ":" + product;
    }

    std::string get_device_info(const std::string path) {
        std::ifstream file(path); 
        if (! file.is_open())
            return std::string();
        else {
            std::string info;
            getline(file, info);
            file.close();
            return info;
        }
    }
};


main()
{
    USBus usbus;
    std::cout << "Device count : " << usbus.get_device_count() << "\n";
    usbus.debug();
    auto usb1 = usbus.get_device("1d6b:0002");
    std::cout << usb1.get_speed() << "\n";
    std::cout << usb1.get_version() << "\n";
    auto usb2 = usbus.get_all_devices("0d3d:0001");
    for (auto usb : usb2) {
        std::cout << "found one " << usb.get_version() << "\n" ;
    }
    auto usb3 = usbus.get_all_devices("1d6b:0002");
}
