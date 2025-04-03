#include "utils/json.hpp"
#include "utils/register.hpp"
#include "utils/supervise.hpp"

#include <CLI/CLI.hpp>

#include <algorithm>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char** argv)
{
    CLI::App app{"mfg-tool: temporary utilities for manufacturing support"};
    app.require_subcommand(1);
    mfgtool::init_commands(app);

    int timeout = 0;
    int retries = 0;
    std::string configFN = std::format("{}/.mfgtool.json", getenv("HOME"));

    if (std::filesystem::exists(configFN))
    {
        std::ifstream ifs(configFN);
        auto config = mfgtool::js::parse(ifs);

        if (auto iter = config.find("timeout"); iter != config.end())
        {
            timeout = config["timeout"];
        }
        if (auto iter = config.find("retries"); iter != config.end())
        {
            retries = config["retries"];
        }
    }
    auto _ = [&]() {
        CLI11_PARSE(app, argc, argv);
        return 0;
    };
    return mfg_tool::supervise(_, timeout, retries);
}
