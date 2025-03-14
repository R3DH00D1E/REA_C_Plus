#include <iostream>
#include <string>
#include <vector>
#include <map>

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
    map<string, int> disciplineGrades;

public:
    void setGrade(const string& discipline, int grade) {
        disciplineGrades[discipline] = grade;
    }

    int getGrade(const string& discipline) const {
        auto it = disciplineGrades.find(discipline);
        if (it != disciplineGrades.end()) {
            return it->second;
        }
        return 0;
    }

    int getTotalGrade() const {
        int total = 0;
        for (const auto& [discipline, grade] : disciplineGrades) {
            total += grade;
        }
        return total;
    }

    double getAverageGrade() const {
        if (disciplineGrades.empty()) {
            return 0.0;
        }
        return static_cast<double>(getTotalGrade()) / disciplineGrades.size();
    }

    vector<string> getDisciplines() const {
        vector<string> disciplines;
        for (const auto& [discipline, grade] : disciplineGrades) {
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

    void setGrade(const string& discipline, int grade) {
        grades.setGrade(discipline, grade);
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

    s1.setGrade("Математика", 85);
    s1.setGrade("Программирование", 92);
    s1.setGrade("Физика", 78);

    s2.setGrade("Математика", 92);
    s2.setGrade("Программирование", 88);
    s2.setGrade("Физика", 90);

    s3.setGrade("Математика", 75);
    s3.setGrade("Программирование", 95);
    s3.setGrade("Физика", 82);

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
        cout << "Баллы:" << endl;
        cout << "  Математика: " << s->getGrade("Математика") << endl;
        cout << "  Программирование: " << s->getGrade("Программирование") << endl;
        cout << "  Физика: " << s->getGrade("Физика") << endl;
        cout << "Общий балл: " << s->getTotalGrade() << endl;
        cout << "Средний балл: " << s->getAverageGrade() << endl;
    }

    return 0;
}
