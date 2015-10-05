#include "common.h"

static
int con1(int a)
{
    printk(0, "In cons1(%d)\n", a);
    return a*2;
}

static int val1 = con1(-42);

namespace {
struct klass {
    int val;
    klass(int a) :val(a*2) {
        printk(0, "In klass::klass(%d)\n", a);
    }
    ~klass() {
        printk(0, "In klass::~klass()\n");
    }
};

klass val2(3);
} // namespace

void part2(void);

extern "C"
void Init(void)
{
    printk(0, "Testing c++ global constructors and destructors\n");
    printk(0, "val1 %d\n", val1);
    printk(0, "val2.val %d\n", val2.val);
    part2();
    printk(0, "Repeat\n");
    part2();
}
