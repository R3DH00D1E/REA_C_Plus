#include <iostream>
#include <string>
#include <vector>
#include <memory>

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

class Document {
protected:
    User author;
    std::string title;
    std::string content;

public:
    Document(const User& author, const std::string& title, const std::string& content)
        : author(author), title(title), content(content) {}

    virtual ~Document() {}

    User getAuthor() const {
        return author;
    }

    std::string getTitle() const {
        return title;
    }

    std::string getContent() const {
        return content;
    }

    void setTitle(const std::string& newTitle) {
        title = newTitle;
    }

    void setContent(const std::string& newContent) {
        content = newContent;
    }

    virtual void display() const {
        std::cout << "Document: " << title << std::endl;
        std::cout << "Author: " << author.getName() << std::endl;
        std::cout << "Content: " << content << std::endl;
    }
};

class WorkDocument : public Document {
private:
    std::string department;
    std::vector<User> accessGroup;

public:
    WorkDocument(const User& author, const std::string& title, const std::string& content,
                 const std::string& department)
        : Document(author, title, content), department(department) {}

    void addUserToAccessGroup(const User& user) {
        accessGroup.push_back(user);
    }

    bool checkAccess(const User& user) const {
        for (const auto& member : accessGroup) {
            if (member.getId() == user.getId()) {
                return true;
            }
        }
        return false;
    }

    std::string getDepartment() const {
        return department;
    }

    void display() const override {
        Document::display();
        std::cout << "Department: " << department << std::endl;
        std::cout << "Access Group: " << std::endl;
        for (const auto& user : accessGroup) {
            std::cout << "- " << user.getName() << std::endl;
        }
    }
};

class OrganizationalDocument : public Document {
private:
    int docNumber;
    bool endorsed;
    bool sign;
    User* endorser;
    User* signer;

public:
    OrganizationalDocument(const User& author, const std::string& title, const std::string& content,
                           int docNumber)
        : Document(author, title, content), docNumber(docNumber), endorsed(false), sign(false),
          endorser(nullptr), signer(nullptr) {}

    void setEndorsement(User* user) {
        endorser = user;
        endorsed = true;
    }

    void setSignature(User* user) {
        signer = user;
     sign = true;
    }

    int getDocNumber() const {
        return docNumber;
    }

    bool isEndorsed() const {
        return endorsed;
    }

    bool isSigned() const {
        return sign;
    }

    User* getEndorser() const {
        return endorser;
    }

    User* getSigner() const {
        return signer;
    }

    void display() const override {
        Document::display();
        std::cout << "Document Number: " << docNumber << std::endl;
        std::cout << "Endorsement Status: " << (endorsed ? "Endorsed" : "Not Endorsed") << std::endl;
        if (endorsed && endorser != nullptr) {
            std::cout << "Endorsed by: " << endorser->getName() << std::endl;
        }
        std::cout << "Signature Status: " <<  (sign ? "Signed" : "Not Signed") << std::endl;
        if  (sign && signer != nullptr) {
            std::cout << "Signed by: " << signer->getName() << std::endl;
        }
    }
};

class DocumentManager {
private:
    std::vector<std::shared_ptr<Document>> documents;
    std::vector<std::shared_ptr<OrganizationalDocument>> orgDocuments;

public:
    void addDocument(std::shared_ptr<Document> doc) {
        documents.push_back(doc);
        
        auto orgDoc = std::dynamic_pointer_cast<OrganizationalDocument>(doc);
        if (orgDoc) {
            orgDocuments.push_back(orgDoc);
        }
    }
    
    void displaySummary() const {
        int totalOrgDocs = orgDocuments.size();
        int signedDocs = 0;
        int endorsedButNotSigned = 0;
        int notEndorsed = 0;
        
        for (const auto& doc : orgDocuments) {
            if (doc->isSigned()) {
                signedDocs++;
            } else if (doc->isEndorsed()) {
                endorsedButNotSigned++;
            } else {
                notEndorsed++;
            }
        }
        
        std::cout << "\n=== Document Summary ===" << std::endl;
        std::cout << "Total organizational documents: " << totalOrgDocs << std::endl;
        std::cout << "Signed documents: " << signedDocs << std::endl;
        std::cout << "Endorsed but not signed: " << endorsedButNotSigned << std::endl;
        std::cout << "Waiting for endorsement: " << notEndorsed << std::endl;
    }
    
    int getTotalDocuments() const {
        return documents.size();
    }
    
    int getTotalOrgDocuments() const {
        return orgDocuments.size();
    }
};

int main() {
    User user1("John Smith", 1);
    User user2("Robert Johnson", 2);
    User user3("Emma Davis", 3);

    auto workDoc = std::make_shared<WorkDocument>(user1, "Technical Report", "Technical report content...", "IT Department");
    workDoc->addUserToAccessGroup(user1);
    workDoc->addUserToAccessGroup(user2);

    auto orgDoc1 = std::make_shared<OrganizationalDocument>(user1, "Bonus Order", "All employees will receive a bonus...", 123);
    orgDoc1->setEndorsement(&user2);
    orgDoc1->setSignature(&user3);
    
    auto orgDoc2 = std::make_shared<OrganizationalDocument>(user2, "Vacation Policy", "New vacation policy...", 124);
    orgDoc2->setEndorsement(&user3);
    
    auto orgDoc3 = std::make_shared<OrganizationalDocument>(user3, "Office Closure", "Office will be closed...", 125);
    
    DocumentManager docManager;
    docManager.addDocument(workDoc);
    docManager.addDocument(orgDoc1);
    docManager.addDocument(orgDoc2);
    docManager.addDocument(orgDoc3);

    std::cout << "=== Work Document Information ===" << std::endl;
    workDoc->display();
    std::cout << std::endl;

    std::cout << "=== Organizational Document Information ===" << std::endl;
    orgDoc1->display();
    std::cout << std::endl;

    std::cout << "User " << user2.getName() << (workDoc->checkAccess(user2) ? " has " : " does not have ")
              << "access to the work document." << std::endl;
    std::cout << "User " << user3.getName() << (workDoc->checkAccess(user3) ? " has " : " does not have ")
              << "access to the work document." << std::endl;
              
    docManager.displaySummary();

    return 0;
}
