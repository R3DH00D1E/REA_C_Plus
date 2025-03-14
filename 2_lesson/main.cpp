#include <iostream>
#include <string>
#include <vector>
#include <memory>

using namespace std;

class Student {
private:
    string name;
    int age;
    string group;
public:
    Student(const string& name, int age) : name(name), age(age), group("") {}

    int getAge() const { return age; }
    string getName() const { return name; }
    string getGroup() const { return group; }
    void setGroup(const string& group) { this->group = group; }
};

class Group {
private:
    string name;
    vector<shared_ptr<Student>> students;

public:
    Group(const string& name) : name(name) {}

    string getName() const { return name; }

    void addStudent(shared_ptr<Student> student) {
        student->setGroup(name);
        students.push_back(student);
    }

    void removeStudent(const string& studentName) {
        for (auto it = students.begin(); it != students.end(); ++it) {
            if ((*it)->getName() == studentName) {
                students.erase(it);
                return;
            }
        }
    }

    void displayStudents() const {
        cout << "Группа: " << name << endl;
        if (students.empty()) {
            cout << "Пусто\n" << endl;
            return;
        }

        for (const auto& student : students) {
            cout << "- " << student->getName() << ", " << student->getAge() << " лет" << endl;
        }
        cout << endl;
    }
};

int main() {
    vector<Group> groups;
    vector<shared_ptr<Student>> students;

    cout << "Введите количество групп: ";
    int n;
    cin >> n;
    cin.ignore();

    for (int i = 0; i < n; i++) {
        cout << "Название группы " << (i + 1) << ": ";
        string name;
        getline(cin, name);
        groups.emplace_back(name);
    }

    int choice;
    do {
        cout << "\n1. Добавить студента\n2. Показать группы\n3. Перевести студента\n0. Выход\nВыбор: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1: {
                string name;
                int age, groupNum;

                cout << "Имя студента: ";
                getline(cin, name);
                cout << "Возраст: ";
                cin >> age;

                auto student = make_shared<Student>(name, age);
                students.push_back(student);

                cout << "Группы:\n";
                for (size_t i = 0; i < groups.size(); i++) {
                    cout << (i + 1) << ". " << groups[i].getName() << endl;
                }

                cout << "Номер группы: ";
                cin >> groupNum;
                cin.ignore();

                if (groupNum >= 1 && groupNum <= static_cast<int>(groups.size())) {
                    groups[groupNum - 1].addStudent(student);
                    cout << "Студент добавлен" << endl;
                }
                break;
            }
            case 2:
                for (const auto& group : groups) {
                    group.displayStudents();
                }
                break;
            case 3: {
                if (groups.size() < 2) {
                    cout << "Нужно минимум 2 группы" << endl;
                    break;
                }

                cout << "Группы:\n";
                for (size_t i = 0; i < groups.size(); i++) {
                    cout << (i + 1) << ". " << groups[i].getName() << endl;
                }

                int fromGroup;
                cout << "Из группы (номер): ";
                cin >> fromGroup;
                cin.ignore();

                if (fromGroup < 1 || fromGroup > static_cast<int>(groups.size())) break;

                groups[fromGroup - 1].displayStudents();

                string studentName;
                cout << "Имя студента: ";
                getline(cin, studentName);

                int toGroup;
                cout << "В группу (номер): ";
                cin >> toGroup;
                cin.ignore();

                if (toGroup < 1 || toGroup > static_cast<int>(groups.size())) break;

                shared_ptr<Student> studentToMove = nullptr;
                for (auto& student : students) {
                    if (student->getName() == studentName && 
                        student->getGroup() == groups[fromGroup - 1].getName()) {
                        studentToMove = student;
                        break;
                    }
                }

                if (studentToMove) {
                    groups[fromGroup - 1].removeStudent(studentName);
                    groups[toGroup - 1].addStudent(studentToMove);
                    cout << "Студент переведен" << endl;
                }
                break;
            }
        }
    } while (choice != 0);

    return 0;
}
