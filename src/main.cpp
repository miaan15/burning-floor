import std;
import spines;

int main() {
    SpinesContext cxt;
    auto err = cxt.parse(R"(
a {
    aa {
        aaa = 10
        aab = 1,
        0
        aac = "x"
    },
    101,
    "abc"
    ab = 5
    ac {1, 2, 3},
    ad {4, flag = 5},
    a = "dup"
    aaa = "dup"
}
    )");

    if (!err.type) {
        cxt.print_debug();
    }
    else {
        std::cout << "Error parse: " << err.type << " at "
                  << err.line << ":" << err.column << std::endl;
    }

    cxt.destroy();
}
