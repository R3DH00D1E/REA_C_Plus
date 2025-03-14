#include <iostream>
#include <string>

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