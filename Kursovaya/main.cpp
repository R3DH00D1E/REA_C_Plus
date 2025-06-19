#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <limits>
#include <windows.h>

using namespace std;

// Класс для представления сетевого пакета
class DataPacket {
private:
    string content;
    int size;
    string sourceMac;
    string destinationMac;

public:
    DataPacket(const string& content, int size, const string& srcMac, const string& destMac)
        : content(content), size(size), sourceMac(srcMac), destinationMac(destMac) {}

    string getContent() const { return content; }
    int getSize() const { return size; }
    string getSourceMac() const { return sourceMac; }
    string getDestinationMac() const { return destinationMac; }
};

class NetworkDevice : public enable_shared_from_this<NetworkDevice> {
protected:
    int id;
    string name;
    string macAddress;
    vector<shared_ptr<class NetworkConnection>> connections;

public:
    NetworkDevice(int id, const string& name, const string& mac)
        : id(id), name(name), macAddress(mac) {}

    virtual ~NetworkDevice() = default;

    void addConnection(shared_ptr<class NetworkConnection> conn) {
        connections.push_back(conn);
    }

    virtual void processPacket(shared_ptr<DataPacket> packet) = 0;

    virtual void displayInfo() const {
        cout << "[" << id << "] " << name 
             << "\nMAC: " << macAddress
             << "\nПодключений: " << connections.size() << endl;
    }

    int getId() const { return id; }
    string getName() const { return name; }
    string getMac() const { return macAddress; }
    vector<shared_ptr<class NetworkConnection>> getConnections() const { return connections; }
};

class NetworkConnection {
private:
    weak_ptr<NetworkDevice> device1;
    weak_ptr<NetworkDevice> device2;
    float bandwidth;
    int latency;
    vector<shared_ptr<DataPacket>> packetQueue;

public:
    NetworkConnection(shared_ptr<NetworkDevice> dev1, 
                     shared_ptr<NetworkDevice> dev2, 
                     float bw, int lat)
        : device1(dev1), device2(dev2), bandwidth(bw), latency(lat) {}

    void transferPacket(shared_ptr<DataPacket> packet, shared_ptr<NetworkDevice> sender) {
        packetQueue.push_back(packet);
        cout << "Пакет поставлен в очередь (" << bandwidth << "Мбит/с, " 
             << latency << "мс задержки): " << packet->getContent() << endl;
        
        this_thread::sleep_for(chrono::milliseconds(latency));
        
        auto dev1 = this->device1.lock();
        auto dev2 = this->device2.lock();
        
        if (sender == dev1 && dev2) {
            dev2->processPacket(packet);
        } else if (sender == dev2 && dev1) {
            dev1->processPacket(packet);
        }
    }

    bool connects(shared_ptr<NetworkDevice> dev) const {
        auto dev1 = this->device1.lock();
        auto dev2 = this->device2.lock();
        return (dev1 == dev || dev2 == dev);
    }

    shared_ptr<NetworkDevice> getOtherDevice(shared_ptr<NetworkDevice> dev) const {
        auto dev1 = this->device1.lock();
        auto dev2 = this->device2.lock();
        
        if (dev == dev1) return dev2;
        if (dev == dev2) return dev1;
        return nullptr;
    }

    float getBandwidth() const { return bandwidth; }
    int getLatency() const { return latency; }
};

class Computer : public NetworkDevice {
private:
    string ipAddress;
    vector<shared_ptr<DataPacket>> receivedPackets;

public:
    Computer(int id, const string& name, const string& mac, const string& ip)
        : NetworkDevice(id, name, mac), ipAddress(ip) {}

    void sendPacket(const string& content, shared_ptr<NetworkDevice> target) {
        if (!target) {
            cout << "Ошибка: Неверное целевое устройство" << endl;
            return;
        }

        auto packet = make_shared<DataPacket>(content, content.size(), macAddress, target->getMac());
        
        for (auto& conn : connections) {
            if (conn->connects(target)) {
                cout << name << " отправляет пакет на " << target->getName() << endl;
                conn->transferPacket(packet, shared_from_this());
                return;
            }
        }
        cout << "Нет маршрута к " << target->getName() << endl;
    }

    void processPacket(shared_ptr<DataPacket> packet) override {
        cout << name << " получил пакет: " << packet->getContent() << endl;
        receivedPackets.push_back(packet);
    }

    void displayInfo() const override {
        NetworkDevice::displayInfo();
        cout << "IP: " << ipAddress 
             << "\nПолучено пакетов: " << receivedPackets.size() << endl;
    }

    string getIp() const { return ipAddress; }
};

class Switch : public NetworkDevice {
private:
    int portCount;
    map<string, shared_ptr<NetworkConnection>> macTable;

public:
    Switch(int id, const string& name, const string& mac, int ports)
        : NetworkDevice(id, name, mac), portCount(ports) {}

    void processPacket(shared_ptr<DataPacket> packet) override {
        macTable[packet->getSourceMac()] = connections[0];
        
        auto it = macTable.find(packet->getDestinationMac());
        if (it != macTable.end()) {
            cout << name << " пересылает пакет на известный MAC: " << packet->getDestinationMac() << endl;
            it->second->transferPacket(packet, shared_from_this());
        } else {
            cout << name << " выполняет flooding (MAC " << packet->getDestinationMac() << " неизвестен)" << endl;
            for (auto& conn : connections) {
                auto otherDev = conn->getOtherDevice(shared_from_this());
                if (otherDev && otherDev->getMac() != packet->getSourceMac()) {
                    conn->transferPacket(packet, shared_from_this());
                }
            }
        }
    }

    void displayInfo() const override {
        NetworkDevice::displayInfo();
        cout << "Портов: " << portCount 
             << "\nИзучено MAC-адресов: " << macTable.size() << endl;
    }
};

class NetworkManager {
private:
    vector<shared_ptr<NetworkDevice>> devices;
    vector<shared_ptr<NetworkConnection>> connections;

    int findDeviceById(int id) const {
        auto it = find_if(devices.begin(), devices.end(), 
            [id](const shared_ptr<NetworkDevice>& dev) { return dev->getId() == id; });
        return it != devices.end() ? distance(devices.begin(), it) : -1;
    }

    bool connectionExists(int id1, int id2) const {
        return any_of(connections.begin(), connections.end(), 
            [id1, id2](const shared_ptr<NetworkConnection>& conn) {
                auto dev1 = conn->getOtherDevice(nullptr);
                auto dev2 = conn->getOtherDevice(dev1);
                return (dev1->getId() == id1 && dev2->getId() == id2) ||
                       (dev1->getId() == id2 && dev2->getId() == id1);
            });
    }

public:
    shared_ptr<NetworkDevice> addDevice(const string& type, int id, const string& name, 
                                      const string& mac, const string& ip = "", int ports = 0) {
        if (findDeviceById(id) != -1) {
            throw runtime_error("Устройство с таким ID уже существует");
        }

        shared_ptr<NetworkDevice> newDevice;
        if (type == "Computer") {
            newDevice = make_shared<Computer>(id, name, mac, ip);
        } else if (type == "Switch") {
            newDevice = make_shared<Switch>(id, name, mac, ports);
        } else {
            throw runtime_error("Неизвестный тип устройства");
        }

        devices.push_back(newDevice);
        return newDevice;
    }

    shared_ptr<NetworkConnection> connectDevices(int id1, int id2, float bw, int lat) {
        int idx1 = findDeviceById(id1);
        int idx2 = findDeviceById(id2);
        
        if (idx1 == -1 || idx2 == -1) {
            throw runtime_error("Одно или оба устройства не найдены");
        }

        if (connectionExists(id1, id2)) {
            throw runtime_error("Соединение уже существует");
        }

        auto conn = make_shared<NetworkConnection>(devices[idx1], devices[idx2], bw, lat);
        connections.push_back(conn);
        devices[idx1]->addConnection(conn);
        devices[idx2]->addConnection(conn);
        return conn;
    }

    void sendPacket(int sourceId, int destId, const string& content) {
        int srcIdx = findDeviceById(sourceId);
        int dstIdx = findDeviceById(destId);
        
        if (srcIdx == -1 || dstIdx == -1) {
            throw runtime_error("Устройство-источник или устройство-назначение не найдены");
        }

        auto computer = dynamic_pointer_cast<Computer>(devices[srcIdx]);
        if (!computer) {
            throw runtime_error("Только компьютеры могут отправлять пакеты");
        }

        computer->sendPacket(content, devices[dstIdx]);
    }

    void displayNetwork() const {
        cout << "\n=== Обзор сети ===" << endl;
        cout << "Устройств: " << devices.size() 
             << "\nСоединений: " << connections.size() << "\n" << endl;
        
        cout << "=== Устройства ===" << endl;
        for (const auto& dev : devices) {
            dev->displayInfo();
            cout << "----------------" << endl;
        }

        cout << "\n=== Соединения ===" << endl;
        for (const auto& conn : connections) {
            auto dev1 = conn->getOtherDevice(nullptr);
            auto dev2 = conn->getOtherDevice(dev1);
            cout << dev1->getName() << " (" << dev1->getId() << ") <---> " 
                 << dev2->getName() << " (" << dev2->getId() << ")"
                 << "\nПропускная способность: " << conn->getBandwidth() << "Мбит/с"
                 << ", Задержка: " << conn->getLatency() << "мс\n" << endl;
        }
    }
};

template<typename T>
T safeInput(const string& prompt = "") {
    T value;
    while (true) {
        if (!prompt.empty()) cout << prompt;
        cin >> value;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Неверный ввод. Пожалуйста, попробуйте снова." << endl;
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}

void displayMenu() {
    cout << "\n=== Меню симулятора сети ===" << endl;
    cout << "1. Добавить устройство" << endl;
    cout << "2. Создать соединение" << endl;
    cout << "3. Отправить пакет" << endl;
    cout << "4. Показать сеть" << endl;
    cout << "5. Выход" << endl;
    cout << "Выберите действие: ";
}

int main() {
    // Установка русской локали для Windows
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    
    NetworkManager nm;

    // Создаем тестовую сеть
    try {
        auto pc1 = nm.addDevice("Computer", 1, "Офисный ПК", "00:1A:1B:1C:1D:11", "192.168.1.1");
        auto pc2 = nm.addDevice("Computer", 2, "Ноутбук", "00:1A:1B:1C:1D:22", "192.168.1.2");
        auto sw1 = nm.addDevice("Switch", 3, "Главный коммутатор", "00:1A:1B:1C:1D:33", "", 8);

        nm.connectDevices(1, 3, 100.0, 1);
        nm.connectDevices(2, 3, 100.0, 1);
        
        cout << "Тестовая сеть успешно создана!" << endl;
    } catch (const exception& e) {
        cerr << "Ошибка инициализации: " << e.what() << endl;
    }

    while (true) {
        displayMenu();
        int choice = safeInput<int>();

        try {
            switch (choice) {
                case 1: {
                    cout << "\nДобавить новое устройство" << endl;
                    cout << "1. Компьютер\n2. Коммутатор" << endl;
                    int typeChoice = safeInput<int>("Выберите тип устройства: ");
                    
                    int id = safeInput<int>("Введите ID устройства: ");
                    string name, mac;
                    cout << "Введите имя устройства: "; 
                    getline(cin, name);
                    if (name.empty()) getline(cin, name);
                    cout << "Введите MAC-адрес: "; 
                    getline(cin, mac);
                    
                    if (typeChoice == 1) {
                        string ip;
                        cout << "Введите IP-адрес: "; 
                        getline(cin, ip);
                        nm.addDevice("Computer", id, name, mac, ip);
                    } else {
                        int ports = safeInput<int>("Введите количество портов: ");
                        nm.addDevice("Switch", id, name, mac, "", ports);
                    }
                    cout << "Устройство успешно добавлено!" << endl;
                    break;
                }
                case 2: {
                    cout << "\nСоздать соединение" << endl;
                    nm.displayNetwork();
                    
                    int id1 = safeInput<int>("Введите ID первого устройства: ");
                    int id2 = safeInput<int>("Введите ID второго устройства: ");
                    float bw = safeInput<float>("Введите пропускную способность (Мбит/с): ");
                    int lat = safeInput<int>("Введите задержку (мс): ");
                    
                    nm.connectDevices(id1, id2, bw, lat);
                    cout << "Соединение успешно создано!" << endl;
                    break;
                }
                case 3: {
                    cout << "\nОтправить пакет" << endl;
                    nm.displayNetwork();
                    
                    int srcId = safeInput<int>("Введите ID устройства-источника: ");
                    int destId = safeInput<int>("Введите ID устройства-назначения: ");
                    string content;
                    cout << "Введите содержимое пакета: "; 
                    getline(cin, content);
                    if (content.empty()) getline(cin, content);
                    
                    nm.sendPacket(srcId, destId, content);
                    break;
                }
                case 4:
                    nm.displayNetwork();
                    break;
                case 5:
                    cout << "Завершение работы программы..." << endl;
                    return 0;
                default:
                    cout << "Неверный выбор. Пожалуйста, попробуйте снова." << endl;
            }
        } catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << endl;
        }
    }

    return 0;
}