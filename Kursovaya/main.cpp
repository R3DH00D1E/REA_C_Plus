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
#include <locale.h>
#include <cstdlib>
#include <random>
#include <sstream>

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

class NetworkConnection; // Forward declaration

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

    // Объявляем метод, но определяем его после класса NetworkConnection
    virtual void displayInfo() const;

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
        
        // Если передан nullptr, возвращаем первое устройство
        if (!dev) {
            return dev1;
        }
        
        if (dev == dev1) return dev2;
        if (dev == dev2) return dev1;
        return nullptr;
    }

    // Добавим методы для безопасного получения устройств
    shared_ptr<NetworkDevice> getFirstDevice() const {
        return device1.lock();
    }
    
    shared_ptr<NetworkDevice> getSecondDevice() const {
        return device2.lock();
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
        std::cout << "Портов: " << portCount 
             << "\nИзучено MAC-адресов: " << macTable.size() << std::endl;
    }
};

class Phone : public NetworkDevice {
private:
    string phoneNumber;
    bool isConnected;
    vector<shared_ptr<DataPacket>> receivedPackets;

public:
    Phone(int id, const string& name, const string& mac, const string& number)
        : NetworkDevice(id, name, mac), phoneNumber(number), isConnected(true) {}

    void sendPacket(const string& content, shared_ptr<NetworkDevice> target) {
        if (!target) {
            cout << "Ошибка: Неверное целевое устройство" << endl;
            return;
        }

        auto packet = make_shared<DataPacket>(content, content.size(), macAddress, target->getMac());
        
        for (auto& conn : connections) {
            if (conn->connects(target)) {
                cout << name << " отправляет сообщение на " << target->getName() << endl;
                conn->transferPacket(packet, shared_from_this());
                return;
            }
        }
        cout << "Нет связи с " << target->getName() << endl;
    }

    void processPacket(shared_ptr<DataPacket> packet) override {
        cout << name << " получил сообщение: " << packet->getContent() << endl;
        receivedPackets.push_back(packet);
    }

    void displayInfo() const override {
        NetworkDevice::displayInfo();
        cout << "Номер: " << phoneNumber 
             << "\nСтатус: " << (isConnected ? "Подключен" : "Отключен")
             << "\nПолучено сообщений: " << receivedPackets.size() << endl;
    }

    string getPhoneNumber() const { return phoneNumber; }
};

class Router : public NetworkDevice {
private:
    string ipRange;
    map<string, shared_ptr<NetworkConnection>> routingTable;
    int maxConnections;

public:
    Router(int id, const string& name, const string& mac, const string& range, int maxConn)
        : NetworkDevice(id, name, mac), ipRange(range), maxConnections(maxConn) {}

    void processPacket(shared_ptr<DataPacket> packet) override {
        routingTable[packet->getSourceMac()] = connections[0];
        
        cout << name << " маршрутизирует пакет от " << packet->getSourceMac() 
             << " к " << packet->getDestinationMac() << endl;
             
        auto it = routingTable.find(packet->getDestinationMac());
        if (it != routingTable.end()) {
            it->second->transferPacket(packet, shared_from_this());
        } else {
            // Пересылаем на все порты кроме источника
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
        cout << "IP-диапазон: " << ipRange 
             << "\nМакс. подключений: " << maxConnections
             << "\nЗаписей в таблице маршрутизации: " << routingTable.size() << endl;
    }
};

class Printer : public NetworkDevice {
private:
    string printerModel;
    vector<string> printQueue;
    bool isOnline;

public:
    Printer(int id, const string& name, const string& mac, const string& model)
        : NetworkDevice(id, name, mac), printerModel(model), isOnline(true) {}

    void processPacket(shared_ptr<DataPacket> packet) override {
        if (isOnline) {
            cout << name << " получил задание на печать: " << packet->getContent() << endl;
            printQueue.push_back(packet->getContent());
            cout << name << " печатает документ..." << endl;
        } else {
            cout << name << " недоступен для печати" << endl;
        }
    }

    void displayInfo() const override {
        NetworkDevice::displayInfo();
        cout << "Модель: " << printerModel 
             << "\nСтатус: " << (isOnline ? "Онлайн" : "Офлайн")
             << "\nВ очереди печати: " << printQueue.size() << " документов" << endl;
    }

    void setOnline(bool status) { isOnline = status; }
};

class Server : public NetworkDevice {
private:
    string serverType;
    vector<string> services;
    int cpuLoad;

public:
    Server(int id, const string& name, const string& mac, const string& type)
        : NetworkDevice(id, name, mac), serverType(type), cpuLoad(0) {
        // Добавляем базовые сервисы
        services.push_back("HTTP");
        services.push_back("FTP");
        if (type == "Database") {
            services.push_back("MySQL");
        } else if (type == "Web") {
            services.push_back("Apache");
        }
    }

    void processPacket(shared_ptr<DataPacket> packet) override {
        cpuLoad = min(100, cpuLoad + 5);
        cout << name << " обрабатывает запрос: " << packet->getContent() 
             << " (Загрузка CPU: " << cpuLoad << "%)" << endl;
        
        // Имитируем ответ
        this_thread::sleep_for(chrono::milliseconds(50));
        cpuLoad = max(0, cpuLoad - 3);
    }

    void displayInfo() const override {
        NetworkDevice::displayInfo();
        cout << "Тип сервера: " << serverType 
             << "\nЗагрузка CPU: " << cpuLoad << "%"
             << "\nСервисы: ";
        for (size_t i = 0; i < services.size(); ++i) {
            cout << services[i];
            if (i < services.size() - 1) cout << ", ";
        }
        cout << endl;
    }
};

class NetworkManager {
private:
    vector<shared_ptr<NetworkDevice>> devices;
    vector<shared_ptr<NetworkConnection>> connections;
    mt19937 rng;

    int findDeviceById(int id) const {
        auto it = find_if(devices.begin(), devices.end(), 
            [id](const shared_ptr<NetworkDevice>& dev) { return dev->getId() == id; });
        return it != devices.end() ? distance(devices.begin(), it) : -1;
    }

    bool connectionExists(int id1, int id2) const {
        for (const auto& conn : connections) {
            // Получаем устройства из соединения более безопасным способом
            auto dev1_ptr = conn->getOtherDevice(nullptr);
            if (!dev1_ptr) continue;
            
            auto dev2_ptr = conn->getOtherDevice(dev1_ptr);
            if (!dev2_ptr) continue;
            
            int dev1_id = dev1_ptr->getId();
            int dev2_id = dev2_ptr->getId();
            
            if ((dev1_id == id1 && dev2_id == id2) || (dev1_id == id2 && dev2_id == id1)) {
                return true;
            }
        }
        return false;
    }

    string generateRandomMac() {
        uniform_int_distribution<int> dist(0, 255);
        stringstream ss;
        ss << hex << uppercase;
        for (int i = 0; i < 6; ++i) {
            if (i > 0) ss << ":";
            ss << setfill('0') << setw(2) << dist(rng);
        }
        return ss.str();
    }

    string generateRandomIP() {
        uniform_int_distribution<int> dist(1, 254);
        return "192.168.1." + to_string(dist(rng));
    }

public:
    NetworkManager() : rng(chrono::steady_clock::now().time_since_epoch().count()) {}

    shared_ptr<NetworkDevice> addDevice(const string& type, int id, const string& name, 
                                      const string& mac, const string& ip = "", int ports = 0) {
        cout << "Добавление устройства: " << name << " (ID: " << id << ")" << endl;
        
        if (findDeviceById(id) != -1) {
            throw runtime_error("Устройство с таким ID уже существует");
        }

        shared_ptr<NetworkDevice> newDevice;
        if (type == "Computer") {
            newDevice = make_shared<Computer>(id, name, mac, ip);
        } else if (type == "Switch") {
            newDevice = make_shared<Switch>(id, name, mac, ports);
        } else if (type == "Phone") {
            string phoneNumber = "+7-" + to_string(uniform_int_distribution<int>(1000000, 9999999)(rng));
            newDevice = make_shared<Phone>(id, name, mac, phoneNumber);
        } else if (type == "Router") {
            newDevice = make_shared<Router>(id, name, mac, "192.168.1.0/24", 24);
        } else if (type == "Printer") {
            vector<string> models = {"HP LaserJet", "Canon Pixma", "Epson WorkForce", "Brother HL"};
            string model = models[uniform_int_distribution<int>(0, models.size()-1)(rng)];
            newDevice = make_shared<Printer>(id, name, mac, model);
        } else if (type == "Server") {
            vector<string> serverTypes = {"Web", "Database", "File", "Mail"};
            string serverType = serverTypes[uniform_int_distribution<int>(0, serverTypes.size()-1)(rng)];
            newDevice = make_shared<Server>(id, name, mac, serverType);
        } else {
            throw runtime_error("Неизвестный тип устройства");
        }

        devices.push_back(newDevice);
        cout << "Устройство " << name << " успешно добавлено" << endl;
        return newDevice;
    }

    shared_ptr<NetworkConnection> connectDevices(int id1, int id2, float bw, int lat) {
        cout << "Создание соединения между устройствами " << id1 << " и " << id2 << endl;
        
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
        
        cout << "Соединение между " << devices[idx1]->getName() 
             << " и " << devices[idx2]->getName() << " создано" << endl;
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

    void generateRandomNetwork() {
        cout << "Генерация случайной сети..." << endl;
        
        // Очищаем существующую сеть
        devices.clear();
        connections.clear();
        
        vector<string> deviceTypes = {"Computer", "Phone", "Router", "Printer", "Server", "Switch"};
        vector<string> deviceNames = {
            "Офисный ПК", "Ноутбук", "Смартфон", "iPhone", "Принтер Canon", 
            "Роутер TP-Link", "Сервер базы данных", "Коммутатор D-Link",
            "Рабочая станция", "Планшет", "Сетевой принтер", "Файл-сервер"
        };
        
        uniform_int_distribution<int> deviceCountDist(5, 10);
        uniform_int_distribution<int> typeDist(0, deviceTypes.size() - 1);
        uniform_int_distribution<int> nameDist(0, deviceNames.size() - 1);
        uniform_int_distribution<int> bandwidthDist(10, 1000);
        uniform_int_distribution<int> latencyDist(1, 50);
        
        int deviceCount = deviceCountDist(rng);
        
        // Создаем устройства
        for (int i = 1; i <= deviceCount; ++i) {
            string type = deviceTypes[typeDist(rng)];
            string name = deviceNames[nameDist(rng)] + " " + to_string(i);
            string mac = generateRandomMac();
            string ip = generateRandomIP();
            
            try {
                if (type == "Switch" || type == "Router") {
                    addDevice(type, i, name, mac, "", uniform_int_distribution<int>(4, 24)(rng));
                } else {
                    addDevice(type, i, name, mac, ip);
                }
            } catch (const exception& e) {
                cout << "Ошибка создания устройства: " << e.what() << endl;
            }
        }
        
        // Создаем случайные соединения
        uniform_int_distribution<int> connectionCountDist(deviceCount - 1, deviceCount + 2);
        int connectionCount = connectionCountDist(rng);
        
        for (int i = 0; i < connectionCount; ++i) {
            uniform_int_distribution<int> deviceDist(1, deviceCount);
            int id1 = deviceDist(rng);
            int id2 = deviceDist(rng);
            
            if (id1 != id2) {
                try {
                    connectDevices(id1, id2, bandwidthDist(rng), latencyDist(rng));
                } catch (const exception& e) {
                    // Игнорируем ошибки дублирования соединений
                }
            }
        }
        
        cout << "Случайная сеть создана: " << devices.size() << " устройств, " 
             << connections.size() << " соединений" << endl;
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
        for (size_t i = 0; i < connections.size(); ++i) {
            const auto& conn = connections[i];
            try {
                // Проходим по всем устройствам и ищем те, которые связаны этим соединением
                shared_ptr<NetworkDevice> dev1 = nullptr;
                shared_ptr<NetworkDevice> dev2 = nullptr;
                
                for (const auto& device : devices) {
                    if (conn->connects(device)) {
                        if (!dev1) {
                            dev1 = device;
                        } else if (!dev2) {
                            dev2 = device;
                            break;
                        }
                    }
                }
                
                if (dev1 && dev2) {
                    cout << dev1->getName() << " (" << dev1->getId() << ") <---> " 
                         << dev2->getName() << " (" << dev2->getId() << ")"
                         << "\nПропускная способность: " << conn->getBandwidth() << "Мбит/с"
                         << ", Задержка: " << conn->getLatency() << "мс\n" << endl;
                } else {
                    cout << "Соединение " << i << ": Ошибка - не найдены оба устройства" << endl;
                }
            } catch (const exception& e) {
                cout << "Ошибка отображения соединения " << i << ": " << e.what() << endl;
            }
        }
    }
};

template<typename T>
T safeInput(const string& prompt = "") {
    T value;
    while (true) {
        if (!prompt.empty()) cout << prompt;
        if (!(cin >> value)) {
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
    cout << "5. Сгенерировать случайную сеть" << endl;
    cout << "6. Выход" << endl;
    cout << "Выберите действие: ";
}

int main() {
    // Установка UTF-8 кодировки для лучшей совместимости
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    
    cout << "Запуск симулятора сети..." << endl;
    
    NetworkManager nm;

    // Генерируем случайную тестовую сеть
    nm.generateRandomNetwork();

    cout << "Переход к главному меню..." << endl;

    while (true) {
        displayMenu();
        int choice = safeInput<int>();

        cout << "Выбрано: " << choice << endl;

        try {
            switch (choice) {
                case 1: {
                    cout << "\nДобавить новое устройство" << endl;
                    cout << "1. Компьютер\n2. Коммутатор\n3. Телефон\n4. Роутер\n5. Принтер\n6. Сервер" << endl;
                    int typeChoice = safeInput<int>("Выберите тип устройства: ");
                    
                    vector<string> types = {"", "Computer", "Switch", "Phone", "Router", "Printer", "Server"};
                    if (typeChoice < 1 || typeChoice > 6) {
                        cout << "Неверный выбор типа устройства!" << endl;
                        break;
                    }
                    
                    int id = safeInput<int>("Введите ID устройства: ");
                    
                    string name;
                    cout << "Введите имя устройства: "; 
                    cout.flush();
                    getline(cin, name);
                    
                    string mac;
                    cout << "Введите MAC-адрес: "; 
                    cout.flush();
                    getline(cin, mac);
                    
                    if (typeChoice == 1) { // Computer
                        string ip;
                        cout << "Введите IP-адрес: "; 
                        cout.flush();
                        getline(cin, ip);
                        nm.addDevice("Computer", id, name, mac, ip);
                    } else if (typeChoice == 2) { // Switch
                        int ports = safeInput<int>("Введите количество портов: ");
                        nm.addDevice("Switch", id, name, mac, "", ports);
                    } else {
                        nm.addDevice(types[typeChoice], id, name, mac);
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
                    cout.flush();
                    getline(cin, content);
                    
                    nm.sendPacket(srcId, destId, content);
                    break;
                }
                case 4:
                    cout << "Отображение сети..." << endl;
                    nm.displayNetwork();
                    break;
                case 5:
                    cout << "\nГенерация новой случайной сети..." << endl;
                    nm.generateRandomNetwork();
                    cout << "Новая сеть создана!" << endl;
                    break;
                case 6:
                    cout << "Завершение работы программы..." << endl;
                    return 0;
                default:
                    cout << "Неверный выбор. Пожалуйста, попробуйте снова." << endl;
            }
        } catch (const exception& e) {
            cerr << "Ошибка: " << e.what() << endl;
        }
        
        cout << "\nНажмите Enter для продолжения...";
        cin.get();
    }

    return 0;
}

// Теперь определяем метод displayInfo() после определения NetworkConnection
void NetworkDevice::displayInfo() const {
    cout << "[" << id << "] " << name 
         << "\nMAC: " << macAddress;
    
    if (connections.empty()) {
        cout << "\nПодключений: 0" << endl;
    } else {
        cout << "\nПодключений: " << connections.size() << " (ID: ";
        vector<int> connectedIds;
        
        for (const auto& conn : connections) {
            auto otherDevice = conn->getOtherDevice(const_pointer_cast<NetworkDevice>(shared_from_this()));
            if (otherDevice) {
                connectedIds.push_back(otherDevice->getId());
            }
        }
        
        for (size_t i = 0; i < connectedIds.size(); ++i) {
            cout << connectedIds[i];
            if (i < connectedIds.size() - 1) cout << ", ";
        }
        cout << ")" << endl;
    }
}