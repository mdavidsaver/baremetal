#include "common.h"

static
int con3(int a)
{
    printk(0, "In cons3(%d)\n", a);
    return a*2;
}

static int val3 = con3(-41);

namespace {
struct klass2 {
    int val;
    klass2(int a) :val(a*2) {
        printk(0, "In klass2::klass2(%d)\n", a);
    }
    ~klass2() {
        printk(0, "In klass2::~klass2()\n");
    }
};

klass2 val4(3);
} // namespace

void part2(void)
{
    printk(0, "part2\n");
    printk(0, "val3 %d\n", val3);
    printk(0, "val4.val %d\n", val4.val);
}
