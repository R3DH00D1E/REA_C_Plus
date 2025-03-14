#include <iostream>
#include <string>

using namespace std;

// Интерфейс для студента
class IStudent {
public:
    virtual int getAge() const = 0;
    virtual void setAge(int age) = 0;
    virtual string getName() const = 0;
    virtual void setName(const string& name) = 0;
    virtual string getGroup() const = 0;
    virtual void setGroup(const string& group) = 0;
    virtual ~IStudent() {}
};

// Класс, представляющий студента, реализующий интерфейс IStudent
class student : public IStudent
{
private:
    string name; // Имя студента
    int age;     // Возраст студента
    string group; // Группа студента
public:
    // Конструктор по умолчанию
    student() : name(""), age(0), group("") {}
    // Геттеры и сеттеры
    int getAge() const override {return age;}
    void setAge(int age) override;
    string getName() const override {return name;}
    void setName(const string& name) override {this->name = name;}
    string getGroup() const override {return group;}
    void setGroup(const string& group) override {this->group = group;}
};

int main() {
    student s;       // Создание объекта класса student
    s.setName("Иван");
    s.setAge(20);
    s.setGroup("ИС-201");

    // Вывод информации о студенте
    cout << "Имя: " << s.getName() << endl;
    cout << "Возраст: " << s.getAge() << endl;
    cout << "Группа: " << s.getGroup() << endl;

    cout << "\nИзменение через интерфейс:" << endl;

    // Использование через указатель на интерфейс
    IStudent* iStudent = &s;
    iStudent->setName("Мария");
    iStudent->setAge(21);
    iStudent->setGroup("ИС-202");

    // Вывод обновленной информации
    cout << "Имя: " << iStudent->getName() << endl;
    cout << "Возраст: " << iStudent->getAge() << endl;
    cout << "Группа: " << iStudent->getGroup() << endl;

    return 0;
}

// Реализация метода setAge для установки возраста
void student::setAge(int age)
{
    this->age = age; // this указывает на текущий объект
}
