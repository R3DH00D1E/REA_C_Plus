#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <set>

using namespace std;

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

class Grades {
private:
    map<string, vector<int>> disciplineGrades; // Изменено на вектор баллов для накопления

public:
    // Добавление нового балла к дисциплине
    void addGrade(const string& discipline, int grade) {
        disciplineGrades[discipline].push_back(grade);
    }

    // Получение всех баллов по дисциплине
    vector<int> getAllGrades(const string& discipline) const {
        auto it = disciplineGrades.find(discipline);
        if (it != disciplineGrades.end()) {
            return it->second;
        }
        return {};
    }

    // Получение суммарного балла по дисциплине
    int getGrade(const string& discipline) const {
        auto it = disciplineGrades.find(discipline);
        if (it != disciplineGrades.end()) {
            int sum = 0;
            for (int grade : it->second) {
                sum += grade;
            }
            return sum;
        }
        return 0;
    }

    // Получение суммарного балла по всем дисциплинам
    int getTotalGrade() const {
        int total = 0;
        for (const auto& [discipline, grades] : disciplineGrades) {
            for (int grade : grades) {
                total += grade;
            }
        }
        return total;
    }

    // Получение среднего балла
    double getAverageGrade() const {
        int totalGrades = 0;
        int totalSum = 0;
        
        for (const auto& [discipline, grades] : disciplineGrades) {
            for (int grade : grades) {
                totalSum += grade;
                totalGrades++;
            }
        }
        
        if (totalGrades == 0) {
            return 0.0;
        }
        return static_cast<double>(totalSum) / totalGrades;
    }

    // Получение списка дисциплин
    vector<string> getDisciplines() const {
        vector<string> disciplines;
        for (const auto& [discipline, _] : disciplineGrades) {
            disciplines.push_back(discipline);
        }
        return disciplines;
    }
};

class student : public IStudent
{
private:
    string name;
    int age;
    string group;
    Grades grades;

public:
    student() : name(""), age(0), group("") {}

    int getAge() const override {return age;}
    void setAge(int age) override;
    string getName() const override {return name;}
    void setName(const string& name) override {this->name = name;}
    string getGroup() const override {return group;}
    void setGroup(const string& group) override {this->group = group;}

    // Добавление нового балла (накопительно)
    void addGrade(const string& discipline, int grade) {
        grades.addGrade(discipline, grade);
    }

    // Получение всех баллов по дисциплине
    vector<int> getAllGrades(const string& discipline) const {
        return grades.getAllGrades(discipline);
    }

    // Совместимость с предыдущим интерфейсом
    void setGrade(const string& discipline, int grade) {
        grades.addGrade(discipline, grade);
    }

    int getGrade(const string& discipline) const {
        return grades.getGrade(discipline);
    }

    int getTotalGrade() const {
        return grades.getTotalGrade();
    }

    double getAverageGrade() const {
        return grades.getAverageGrade();
    }

    Grades& getGrades() {
        return grades;
    }
};

class Group {
private:
    string groupName;
    vector<student*> students;

public:
    Group(const string& name) : groupName(name) {}

    ~Group() {
        // Не удаляем студентов, так как они могут использоваться в других местах
    }

    void addStudent(student* s) {
        students.push_back(s);
        s->setGroup(groupName);
    }

    string getName() const {
        return groupName;
    }

    size_t getStudentCount() const {
        return students.size();
    }

    student* getStudent(size_t index) {
        if (index < students.size()) {
            return students[index];
        }
        return nullptr;
    }

    const vector<student*>& getStudents() const {
        return students;
    }

    double getAverageGradeForDiscipline(const string& discipline) {
        if (students.empty()) {
            return 0.0;
        }

        int totalGrade = 0;
        for (const auto& student : students) {
            totalGrade += student->getGrade(discipline);
        }
        return static_cast<double>(totalGrade) / students.size();
    }

    // Вывод детальной информации о баллах группы
    void printDetailedGrades() const {
        cout << "\n=== ДЕТАЛЬНАЯ ИНФОРМАЦИЯ О БАЛЛАХ ГРУППЫ " << groupName << " ===" << endl;
        
        // Собираем все дисциплины
        set<string> allDisciplines;
        for (const auto& student : students) {
            for (const auto& discipline : student->getGrades().getDisciplines()) {
                allDisciplines.insert(discipline);
            }
        }
        
        // Для каждой дисциплины выводим баллы всех студентов
        for (const auto& discipline : allDisciplines) {
            cout << "\nДисциплина: " << discipline << endl;
            cout << "-------------------------------------------" << endl;
            
            for (const auto& student : students) {
                vector<int> grades = student->getAllGrades(discipline);
                if (!grades.empty()) {
                    cout << student->getName() << ": ";
                    
                    // Выводим отдельные баллы
                    for (size_t i = 0; i < grades.size(); ++i) {
                        cout << grades[i];
                        if (i < grades.size() - 1) {
                            cout << " + ";
                        }
                    }
                    
                    // Выводим сумму
                    int sum = student->getGrade(discipline);
                    cout << " = " << sum << endl;
                }
            }
        }
    }
};

void student::setAge(int age)
{
    this->age = age;
}

int main() {
    student s1, s2, s3;

    s1.setName("Иван");
    s1.setAge(20);

    s2.setName("Мария");
    s2.setAge(21);

    s3.setName("Алексей");
    s3.setAge(19);

    // Накопительное добавление баллов
    s1.addGrade("Математика", 45);
    s1.addGrade("Математика", 40); // Дополнительные баллы
    s1.addGrade("Программирование", 50);
    s1.addGrade("Программирование", 42);
    s1.addGrade("Физика", 38);
    s1.addGrade("Физика", 40);

    s2.addGrade("Математика", 47);
    s2.addGrade("Математика", 45);
    s2.addGrade("Программирование", 48);
    s2.addGrade("Программирование", 40);
    s2.addGrade("Физика", 44);
    s2.addGrade("Физика", 46);

    s3.addGrade("Математика", 35);
    s3.addGrade("Математика", 40);
    s3.addGrade("Программирование", 50);
    s3.addGrade("Программирование", 45);
    s3.addGrade("Физика", 42);
    s3.addGrade("Физика", 40);

    Group group("ИС-201");
    group.addStudent(&s1);
    group.addStudent(&s2);
    group.addStudent(&s3);

    cout << "Группа: " << group.getName() << endl;
    cout << "Количество студентов: " << group.getStudentCount() << endl;
    cout << "\nСредний балл группы по дисциплинам:" << endl;
    cout << "Математика: " << group.getAverageGradeForDiscipline("Математика") << endl;
    cout << "Программирование: " << group.getAverageGradeForDiscipline("Программирование") << endl;
    cout << "Физика: " << group.getAverageGradeForDiscipline("Физика") << endl;

    cout << "\nИнформация о студентах:" << endl;
    for (size_t i = 0; i < group.getStudentCount(); i++) {
        student* s = group.getStudent(i);
        cout << "\nСтудент: " << s->getName() << endl;
        cout << "Возраст: " << s->getAge() << endl;
        cout << "Группа: " << s->getGroup() << endl;
        cout << "Суммарные баллы:" << endl;
        
        vector<string> disciplines = {"Математика", "Программирование", "Физика"};
        for (const auto& discipline : disciplines) {
            vector<int> grades = s->getAllGrades(discipline);
            cout << "  " << discipline << ": ";
            
            // Выводим отдельные баллы
            for (size_t j = 0; j < grades.size(); ++j) {
                cout << grades[j];
                if (j < grades.size() - 1) {
                    cout << " + ";
                }
            }
            
            // Выводим сумму
            cout << " = " << s->getGrade(discipline) << endl;
        }
        
        cout << "Общий балл: " << s->getTotalGrade() << endl;
        cout << "Средний балл: " << fixed << setprecision(2) << s->getAverageGrade() << endl;
    }
    
    // Выводим детальную информацию о баллах
    group.printDetailedGrades();
    
    // Интерактивный ввод баллов
    cout << "\n=== ДОБАВЛЕНИЕ НОВЫХ БАЛЛОВ ===" << endl;
    cout << "Выберите студента (1-" << group.getStudentCount() << "): ";
    int studentIndex;
    cin >> studentIndex;
    
    if (studentIndex >= 1 && studentIndex <= static_cast<int>(group.getStudentCount())) {
        student* s = group.getStudent(studentIndex - 1);
        
        cout << "Выберите дисциплину:" << endl;
        cout << "1. Математика" << endl;
        cout << "2. Программирование" << endl;
        cout << "3. Физика" << endl;
        cout << "Ваш выбор: ";
        
        int disciplineChoice;
        cin >> disciplineChoice;
        
        string discipline;
        switch (disciplineChoice) {
            case 1: discipline = "Математика"; break;
            case 2: discipline = "Программирование"; break;
            case 3: discipline = "Физика"; break;
        }
    }
}