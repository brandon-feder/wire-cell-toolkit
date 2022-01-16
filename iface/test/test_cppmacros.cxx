#include <iostream>

#define FOO(TypeName, varname)                                                                      \
    namespace FooBar {                                                                              \
        void the##TypeName##func(TypeName var##varname) { std::cout << var##varname << std::endl; } \
    }

FOO(int, INT)

int main() { FooBar::theintfunc(42); }
