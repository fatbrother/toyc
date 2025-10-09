// struct 前向聲明測試
struct Node;

struct List {
    struct Node *head;
};

struct Node {
    int data;
    struct Node *next;
};

int main() {
    struct List list;
    return 0;
}
