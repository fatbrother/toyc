// 複雜 struct 測試 - 包含多種類型成員
struct Person {
    char name;
    int age;
    float height;
    double weight;
    struct Person *spouse;
};

struct Company {
    char *name;
    struct Person *employees;
    int employee_count;
    struct {
        char *street;
        char *city;
        int zipcode;
    } address;
};

int main() {
    struct Person john;
    struct Company company;
    
    john.age = 30;
    john.height = 5.9;
    john.weight = 70.5;
    
    company.employee_count = 1;
    company.address.zipcode = 12345;
    
    return 0;
}
