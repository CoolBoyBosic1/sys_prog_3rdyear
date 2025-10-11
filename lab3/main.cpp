#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "ast.hpp"
#include "eval.hpp"
#include "ast_dot.hpp"
#include "gen_c.hpp"

// з bison
int yyparse();
extern std::unique_ptr<Program> g_program;
extern FILE* yyin;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./mini_cpp <source.mc++> [--run] [--emit-c]\n";
        return 1;
    }

    // аргументи
    const char* src = argv[1];
    bool do_run = false;
    bool do_emit_c = false;
    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--run") do_run = true;
        if (std::string(argv[i]) == "--emit-c") do_emit_c = true;
    }

    // відкрити вхід
    yyin = std::fopen(src, "r");
    if (!yyin) {
        perror("fopen");
        return 1;
    }

    // парсинг
    if (yyparse() != 0) {
        std::cerr << "Parse failed\n";
        std::fclose(yyin);
        return 2;
    }
    std::fclose(yyin);

    // Візуалізація AST -> ast.dot
    {
        std::ofstream out("ast.dot");
        out << ast_to_dot(g_program.get());
    }
    std::cerr << "AST written to ast.dot (use: dot -Tpng ast.dot -o ast.png)\n";

    // Підготовка світу (функції, середовище)
    World w;
    collect_functions(w, g_program.get());

    // Генерація C-коду (за потреби)
    if (do_emit_c) {
        std::string code = gen_c_code(g_program.get());
        FILE* f = std::fopen("out.c", "wb");
        if (f) {
            std::fwrite(code.data(), 1, code.size(), f);
            std::fclose(f);
            std::printf("C code written to out.c\n");
        } else {
            std::fprintf(stderr, "Failed to write out.c\n");
        }
    }

    // Виконання main() (за потреби)
    if (do_run) {
        try {
            // виклик користувацької функції main без аргументів
            Value ret = call_func(w, "main", {});
            std::cout << "Program returned: " << ret.as_num() << "\n";
        } catch (const std::exception& ex) {
            std::cerr << "Runtime error: " << ex.what() << "\n";
            return 3;
        }
    }

    return 0;
}
