#include "peerchat/app.hpp"
#include "peerchat/types.hpp"
#include "peerchat/version.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Args {
    uint16_t port{peerchat::kDefaultPort};
    std::string nickname{"peer"};
};

Args parse_args(int argc, char* argv[]) {
    Args args;
    std::vector<std::string> av(argv + 1, argv + argc);

    for (std::size_t i = 0; i < av.size(); ++i) {
        if (av[i] == "--port" && i + 1 < av.size()) {
            args.port = static_cast<uint16_t>(std::stoi(av[++i]));
        } else if (av[i] == "--nick" && i + 1 < av.size()) {
            args.nickname = av[++i];
        } else if (av[i] == "--help" || av[i] == "-h") {
            std::cout << "Usage: peerchat [--port PORT] [--nick NICKNAME]\n";
            std::exit(0);
        }
    }
    return args;
}

} // namespace

int main(int argc, char* argv[]) {
    auto args = parse_args(argc, argv);

    peerchat::App app(args.port, args.nickname);
    app.run();

    return 0;
}
