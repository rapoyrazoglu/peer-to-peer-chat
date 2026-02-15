#include "peerchat/app.hpp"
#include "peerchat/types.hpp"
#include "peerchat/updater.hpp"
#include "peerchat/version.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct Args {
    uint16_t port{peerchat::kDefaultPort};
    std::string nickname{"peer"};
    bool show_version{false};
    bool update{false};
    bool update_beta{false};
    bool do_uninstall{false};
};

Args parse_args(int argc, char* argv[]) {
    Args args;
    std::vector<std::string> av(argv + 1, argv + argc);

    for (std::size_t i = 0; i < av.size(); ++i) {
        if (av[i] == "--port" && i + 1 < av.size()) {
            args.port = static_cast<uint16_t>(std::stoi(av[++i]));
        } else if (av[i] == "--nick" && i + 1 < av.size()) {
            args.nickname = av[++i];
        } else if (av[i] == "--version" || av[i] == "-v") {
            args.show_version = true;
        } else if (av[i] == "--update") {
            args.update = true;
        } else if (av[i] == "--update-beta") {
            args.update_beta = true;
        } else if (av[i] == "--uninstall") {
            args.do_uninstall = true;
        } else if (av[i] == "--help" || av[i] == "-h") {
            std::cout << "Usage: peerchat [OPTIONS]\n"
                      << "  --port PORT       Listen port (default: 9000)\n"
                      << "  --nick NICKNAME   Set nickname\n"
                      << "  --version, -v     Show version\n"
                      << "  --update          Update to latest release\n"
                      << "  --update-beta     Update to latest pre-release\n"
                      << "  --uninstall       Remove PeerChat completely\n"
                      << "  --help, -h        Show this help\n";
            std::exit(0);
        }
    }
    return args;
}

} // namespace

int main(int argc, char* argv[]) {
    auto args = parse_args(argc, argv);

    if (args.show_version) {
        std::cout << "PeerChat v" << peerchat::Version::full_string() << "\n";
        return 0;
    }

    if (args.do_uninstall) {
        return peerchat::Updater::uninstall() ? 0 : 1;
    }

    if (args.update || args.update_beta) {
        return peerchat::Updater::perform(args.update_beta) ? 0 : 1;
    }

    peerchat::App app(args.port, args.nickname);
    app.run();

    return 0;
}
