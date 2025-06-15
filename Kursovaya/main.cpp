#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(void) {
    // 1. Создание устройств
auto pc1 = make_shared<Computer>(1, "PC-1", "00:1A:1B:1C:1D:11", "192.168.1.1");
auto pc2 = make_shared<Computer>(2, "PC-2", "00:1A:1B:1C:1D:22", "192.168.1.2");
auto sw1 = make_shared<Switch>(3, "Switch-1", "00:1A:1B:1C:1D:33", 8);

// 2. Создание соединений
auto conn1 = make_shared<NetworkConnection>(pc1, sw1, 100, 1);
auto conn2 = make_shared<NetworkConnection>(pc2, sw1, 100, 1);

// 3. Добавление соединений
pc1->addConnection(conn1);
pc2->addConnection(conn2);
sw1->addConnection(conn1);
sw1->addConnection(conn2);

// 4. Тестирование передачи данных
pc1->sendPacket("Ping to PC-2", pc2);  // Успешная передача
pc1->sendPacket("Broadcast", nullptr); // Широковещательный пакет

// 5. Отображение состояния
pc1->displayInfo();
pc2->displayInfo();
sw1->displayInfo();
}