#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Класс пользователя системы
class User {
private:
    std::string name;
    int id;

public:
    User(const std::string& name, int id) : name(name), id(id) {}

    std::string getName() const {
        return name;
    }

    int getId() const {
        return id;
    }
};

// Базовый класс Документ
class Document {
protected:
    User author;
    std::string title;
    std::string text;

public:
    Document(const User& author, const std::string& title, const std::string& text)
        : author(author), title(title), text(text) {}

    virtual ~Document() {}

    // Геттеры
    User getAuthor() const {
        return author;
    }

    std::string getTitle() const {
        return title;
    }

    std::string getText() const {
        return text;
    }

    // Сеттеры
    void setTitle(const std::string& newTitle) {
        title = newTitle;
    }

    void setText(const std::string& newText) {
        text = newText;
    }

    // Виртуальный метод для вывода информации о документе
    virtual void displayInfo() const {
        std::cout << "Документ: " << title << std::endl;
        std::cout << "Автор: " << author.getName() << std::endl;
        std::cout << "Текст: " << text << std::endl;
    }
};

// Класс Рабочий документ
class WorkDocument : public Document {
private:
    std::string department;
    std::vector<User> workGroup;

public:
    WorkDocument(const User& author, const std::string& title, const std::string& text,
                 const std::string& department)
        : Document(author, title, text), department(department) {}

    // Добавление пользователя в рабочую группу
    void addToWorkGroup(const User& user) {
        workGroup.push_back(user);
    }

    // Проверка, имеет ли пользователь доступ к документу
    bool hasAccess(const User& user) const {
        for (const auto& member : workGroup) {
            if (member.getId() == user.getId()) {
                return true;
            }
        }
        return false;
    }

    std::string getDepartment() const {
        return department;
    }

    // Переопределение метода для вывода информации
    void displayInfo() const override {
        Document::displayInfo();
        std::cout << "Отдел: " << department << std::endl;
        std::cout << "Рабочая группа: " << std::endl;
        for (const auto& user : workGroup) {
            std::cout << "- " << user.getName() << std::endl;
        }
    }
};

// Класс Организационный документ
class OrganizationalDocument : public Document {
private:
    int number;
    bool isEndorsed;
    bool isSigned;
    User* endorser;
    User* signer;

public:
    OrganizationalDocument(const User& author, const std::string& title, const std::string& text,
                           int number)
        : Document(author, title, text), number(number), isEndorsed(false), isSigned(false),
          endorser(nullptr), signer(nullptr) {}

    // Методы для визирования и подписания
    void endorse(User* user) {
        endorser = user;
        isEndorsed = true;
    }

    void sign(User* user) {
        signer = user;
        isSigned = true;
    }

    // Геттеры
    int getNumber() const {
        return number;
    }

    bool getIsEndorsed() const {
        return isEndorsed;
    }

    bool getIsSigned() const {
        return isSigned;
    }

    User* getEndorser() const {
        return endorser;
    }

    User* getSigner() const {
        return signer;
    }

    // Переопределение метода для вывода информации
    void displayInfo() const override {
        Document::displayInfo();
        std::cout << "Номер: " << number << std::endl;
        std::cout << "Статус визирования: " << (isEndorsed ? "Визировано" : "Не визировано") << std::endl;
        if (isEndorsed && endorser != nullptr) {
            std::cout << "Визировал: " << endorser->getName() << std::endl;
        }
        std::cout << "Статус подписания: " << (isSigned ? "Подписано" : "Не подписано") << std::endl;
        if (isSigned && signer != nullptr) {
            std::cout << "Подписал: " << signer->getName() << std::endl;
        }
    }
};

int main() {
    // Создаем пользователей
    User user1("Иванов Иван", 1);
    User user2("Петров Петр", 2);
    User user3("Сидоров Сидор", 3);

    // Создаем рабочий документ
    WorkDocument workDoc(user1, "Технический отчет", "Содержимое технического отчета", "ИТ-отдел");
    workDoc.addToWorkGroup(user1);
    workDoc.addToWorkGroup(user2);

    // Создаем организационный документ
    OrganizationalDocument orgDoc(user1, "Приказ о премировании", "Премировать сотрудников...", 123);
    orgDoc.endorse(&user2);
    orgDoc.sign(&user3);

    // Выводим информацию о документах
    std::cout << "=== Информация о рабочем документе ===" << std::endl;
    workDoc.displayInfo();
    std::cout << std::endl;

    std::cout << "=== Информация об организационном документе ===" << std::endl;
    orgDoc.displayInfo();
    std::cout << std::endl;

    // Проверяем доступ к рабочему документу
    std::cout << "Пользователь " << user2.getName() << (workDoc.hasAccess(user2) ? " имеет " : " не имеет ")
              << "доступ к рабочему документу." << std::endl;
    std::cout << "Пользователь " << user3.getName() << (workDoc.hasAccess(user3) ? " имеет " : " не имеет ")
              << "доступ к рабочему документу." << std::endl;

    return 0;
}
