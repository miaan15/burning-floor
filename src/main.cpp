import std;
import spines;
import common;

int main() {
    SpinesContext cxt;

    stdf::path filepath = _src_dir / "test_spines.txt";
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Could not open file " << filepath << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string config_content = buffer.str();
    file.close();

    auto char_count = config_content.size();
    auto line_count = std::count(config_content.begin(), config_content.end(), '\n');

    auto start_time = std::chrono::high_resolution_clock::now();
    auto err = cxt.parse(config_content);
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_ms = end_time - start_time;

    if (!err.type) {
        std::cout << "Lines: " << line_count << " : " << "Chars: " << char_count << "\n";
        std::cout << "Identifiers: " << cxt.identifiers_cap << "\n";
        std::cout << "Fields: " << cxt.fields_cap << "\n";
        std::cout << "Time: " << elapsed_ms.count() << " ms.\n";
        std::cout << "Mem: " << (float)cxt.arena.cap / 1024.0f << " kB.\n\n";
        // cxt.print_debug();
    }
    else {
        std::cout << "Error parse: " << err.type << " at "
                  << err.line << ":" << err.column << std::endl;
    }

    cxt.destroy();
    return 0;
}
