#include "core_api.h"
#include "exit_codes.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cctype>
#include <algorithm>
#include <map>

using namespace snes9x;

enum class OutputFormat { JSON, JSONL, PRETTY };

static OutputFormat parse_format(const std::string &s) {
    if (s == "json") return OutputFormat::JSON;
    if (s == "jsonl" || s == "ndjson") return OutputFormat::JSONL;
    if (s == "pretty") return OutputFormat::PRETTY;
    return OutputFormat::JSON;
}

static void print_pretty(std::ostream &o, bool success, const std::string &msg = std::string(), int frames = 0) {
    o << "== SNES9X CLI RESULT ==\n";
    o << "Status: " << (success?"OK":"FAIL") << "\n";
    if (frames) o << "Frames: " << frames << "\n";
    if (!msg.empty()) o << "Message: " << msg << "\n";
}

static void print_json_to_stream(std::ostream &o, bool success, const std::string &msg = std::string(), int frames = 0) {
    std::ostringstream os;
    os << "{\"success\":" << (success ? "true" : "false");
    if (!msg.empty()) os << ",\"message\":\"" << msg << "\"";
    if (frames) os << ",\"frames\":" << frames;
    os << "}";
    o << os.str() << std::endl;
}

static void print_jsonl_to_stream(std::ostream &o, bool success, const std::string &msg = std::string(), int frames = 0) {
    print_json_to_stream(o, success, msg, frames);
}

// Wrapper to route output to stdout or a file in the chosen format
static int emit_result(OutputFormat fmt, const std::string &out_path, bool success, const std::string &msg = std::string(), int frames = 0) {
    bool to_file = !out_path.empty();
    if (!to_file) {
        switch(fmt) {
            case OutputFormat::PRETTY: print_pretty(std::cout, success, msg, frames); break;
            case OutputFormat::JSONL: print_jsonl_to_stream(std::cout, success, msg, frames); break;
            case OutputFormat::JSON: default: print_json_to_stream(std::cout, success, msg, frames); break;
        }
        return success ? EXIT_SUCCESSFUL : EXIT_INTERNAL_ERROR;
    }
    std::ofstream fout(out_path, std::ios::app);
    if (!fout.is_open()) return EXIT_INTERNAL_ERROR;
    switch(fmt) {
        case OutputFormat::PRETTY: print_pretty(fout, success, msg, frames); break;
        case OutputFormat::JSONL: print_jsonl_to_stream(fout, success, msg, frames); break;
        case OutputFormat::JSON: default: print_json_to_stream(fout, success, msg, frames); break;
    }
    return success ? EXIT_SUCCESSFUL : EXIT_INTERNAL_ERROR;
}

// Extending script interpreter for variables and expect-json
static std::string substitute_vars(const std::string &s, const std::map<std::string,std::string> &vars) {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '$' && i+1 < s.size() && s[i+1] == '{') {
            size_t e = s.find('}', i+2);
            if (e != std::string::npos) {
                std::string key = s.substr(i+2, e - (i+2));
                auto it = vars.find(key);
                if (it != vars.end()) out += it->second;
                i = e; continue;
            }
        }
        out.push_back(s[i]);
    }
    return out;
}

static void print_usage() {
    std::cout << "snes9x-cli - headless Snes9x CI front-end\n";
    std::cout << "Usage:\n  --load <path>    Load a ROM\n  --run [frames]   Run (0 means brief run)\n  --step           Step one frame\n  --reset          Reset core to power-on state\n  --save-state <path>  Save state\n  --load-state <path>  Load state\n  --dump-memory <addr> <len>  Dump memory bytes (hex)\n  --run-script <path> Run a sequence of commands from a script file\n  --version        Print version\n  --json-output    Output machine-parseable JSON when applicable\"\n";
}

static int run_script_file(const std::string &path, Core &core, bool json_output, OutputFormat fmt, const std::string &out_path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        if (json_output) emit_result(fmt, out_path, false, "failed to open script file");
        return EXIT_SCRIPT_FAILED;
    }
    std::string line;
    int last_code = EXIT_SUCCESSFUL;
    std::map<std::string,std::string> vars;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        std::istringstream iss(line);
        std::string cmd; iss >> cmd;
        // lower-case
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), [](unsigned char c){ return std::tolower(c); });
        if (cmd == "set") {
            std::string key, val; iss >> key; std::getline(iss, val); val = trim(val); vars[key] = val; continue;
        }
        if (cmd == "load") {
            std::string p; iss >> p; p = substitute_vars(p, vars); if (!core.loadRom(p)) { last_code = EXIT_LOAD_ROM_FAILED; break; }
        } else if (cmd == "run") {
            unsigned int frames = 0; if (iss >> frames) {
                auto r = core.run(frames); if (!r.success) { last_code = EXIT_RUN_FAILED; break; } 
                if (json_output) emit_result(fmt, out_path, r.success, r.message, r.frames_executed);
            } else { auto r = core.run(0); if (!r.success) { last_code = EXIT_RUN_FAILED; break; } if (json_output) emit_result(fmt, out_path, r.success, r.message, r.frames_executed); }
        } else if (cmd == "step") {
            if (!core.step()) { last_code = EXIT_STEP_FAILED; break; }
            else if (json_output) emit_result(fmt, out_path, true);
        } else if (cmd == "save-state") {
            std::string sp; iss >> sp; sp = substitute_vars(sp, vars); if (!core.saveState(sp)) { last_code = EXIT_SAVE_STATE_FAILED; break; }
        } else if (cmd == "load-state") {
            std::string lp; iss >> lp; lp = substitute_vars(lp, vars); if (!core.loadState(lp)) { last_code = EXIT_LOAD_STATE_FAILED; break; }
        } else if (cmd == "dump-memory") {
            uint64_t addr = 0; size_t len = 0; iss >> std::hex >> addr >> std::dec >> len; auto b = core.dumpMemory(addr, len); if (b.empty()) { last_code = EXIT_DUMP_MEMORY_FAILED; break; }
            if (json_output) {
                // Emit memory as JSON array
                std::ostringstream o; o << "{\"success\":true,\"memory\":";
                o << "[";
                for (size_t i = 0; i < b.size(); ++i) { if (i) o << ","; o << (int)b[i]; }
                o << "]}";
                if (out_path.empty()) std::cout << o.str() << std::endl; else { std::ofstream f(out_path, std::ios::app); f << o.str() << std::endl; }
            }
        } else if (cmd == "reset") {
            core.reset();
            if (json_output) emit_result(fmt, out_path, true, "reset");
        } else if (cmd == "sleep") {
            unsigned int ms = 0; iss >> ms; std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        } else if (cmd == "expect-json") {
            std::string key, val; iss >> key; std::getline(iss, val); val = trim(val);
            // For simple matching, run a short command to get last-run JSON if any. We'll assume last command emitted JSON to stdout or out_path.
            std::string source;
            if (!out_path.empty()) {
                std::ifstream f(out_path);
                if (f.is_open()) { std::string lastline; while (std::getline(f, lastline)) source = lastline; }
            } else {
                // Not reliable to fetch stdout; fail if no out_path
            }
            if (source.empty()) { last_code = EXIT_SCRIPT_FAILED; break; }
            try {
                auto pos = source.find("\"" + key + "\"");
                if (pos == std::string::npos) { last_code = EXIT_SCRIPT_FAILED; break; }
                auto colon = source.find(':', pos);
                if (colon == std::string::npos) { last_code = EXIT_SCRIPT_FAILED; break; }
                auto start = source.find_first_not_of(" \t\"", colon+1);
                if (start == std::string::npos) { last_code = EXIT_SCRIPT_FAILED; break; }
                auto end = source.find_first_of(",}\n", start);
                std::string actual = source.substr(start, end - start);
                // remove quotes
                if (!actual.empty() && actual.front() == '"' && actual.back() == '"') actual = actual.substr(1, actual.size()-2);
                if (actual != val) { last_code = EXIT_SCRIPT_FAILED; break; }
            } catch(...) { last_code = EXIT_SCRIPT_FAILED; break; }
        } else {
            last_code = EXIT_SCRIPT_FAILED; break;
        }
    }
    if (json_output) emit_result(fmt, out_path, last_code == EXIT_SUCCESSFUL, last_code == EXIT_SUCCESSFUL ? "ok" : "script failed");
    return last_code;
}

int main(int argc, char **argv) {
    bool json_output = false;
    std::string output_file;
    OutputFormat out_format = OutputFormat::JSON;
    std::string rom_path;
    std::string save_path;
    std::string load_state_path;
    bool do_run = false;
    unsigned int run_frames = 0;
    bool do_step = false;
    bool do_reset = false;
    bool do_dump = false;
    uint64_t dump_addr = 0;
    size_t dump_len = 0;
    std::string script_path;

    std::vector<std::string> args(argv + 1, argv + argc);
    for (size_t i = 0; i < args.size(); ++i) {
        const auto &a = args[i];
        if (a == "--help" || a == "-h") { print_usage(); return 0; }
        if (a == "--version") { std::cout << Core::version() << "\n"; return 0; }
        if (a == "--json-output") { json_output = true; continue; }
        if (a == "--format" && i + 1 < args.size()) { out_format = parse_format(args[++i]); continue; }
        if (a == "--output-file" && i + 1 < args.size()) { output_file = args[++i]; continue; }
        if (a == "--load" && i + 1 < args.size()) { rom_path = args[++i]; continue; }
        if (a == "--run") { do_run = true; if (i + 1 < args.size()) {
            // optional frames
            try { run_frames = std::stoul(args[i + 1]); i++; }
            catch(...) { run_frames = 0; }
            }
            continue;
        }
        if (a == "--step") { do_step = true; continue; }
        if (a == "--reset") { do_reset = true; continue; }
        if (a == "--save-state" && i + 1 < args.size()) { save_path = args[++i]; continue; }
        if (a == "--load-state" && i + 1 < args.size()) { load_state_path = args[++i]; continue; }
        if (a == "--dump-memory" && i + 2 < args.size()) {
            do_dump = true; dump_addr = std::stoull(args[++i], nullptr, 0); dump_len = std::stoul(args[++i]); continue; }
        if (a == "--run-script" && i + 1 < args.size()) { script_path = args[++i]; continue; }
        std::cerr << "Unknown option: " << a << "\n";
        print_usage();
        return 2;
    }

    Core core;
    // If a run-script was specified, execute it using the same Core instance and exit
    if (!script_path.empty()) {
        int rc = run_script_file(script_path, core, json_output, out_format, output_file);
        return rc;
    }
    if (!rom_path.empty()) {
        if (!core.loadRom(rom_path)) {
            if (json_output) emit_result(out_format, output_file, false, "failed to load rom");
            return EXIT_LOAD_ROM_FAILED;
        }
    }

    if (!load_state_path.empty()) {
        if (!core.loadState(load_state_path)) {
            if (json_output) emit_result(out_format, output_file, false, "failed to load state");
            return EXIT_LOAD_STATE_FAILED;
        }
    }

    if (do_reset) {
        core.reset();
        if (json_output) emit_result(out_format, output_file, true, "reset");
        return EXIT_SUCCESSFUL;
    }

    if (do_run) {
        auto r = core.run(run_frames);
        if (json_output) {
            emit_result(out_format, output_file, r.success, r.message, r.frames_executed);
            return r.success ? EXIT_SUCCESSFUL : EXIT_RUN_FAILED;
        }
        return r.success ? EXIT_SUCCESSFUL : EXIT_RUN_FAILED;
    }

    if (do_step) {
        bool ok = core.step();
        if (json_output) emit_result(out_format, output_file, ok);
        return ok ? EXIT_SUCCESSFUL : EXIT_STEP_FAILED;
    }

    if (!save_path.empty()) {
        if (!core.saveState(save_path)) { if (json_output) std::cout << "{\"success\":false}\n"; return 7; }
        if (json_output) emit_result(out_format, output_file, true);
        return EXIT_SUCCESSFUL;
    }

    if (do_dump) {
        auto buf = core.dumpMemory(dump_addr, dump_len);
        if (json_output) {
            std::ostringstream o;
            o << "{\"success\":true,\"memory\":";
            o << "[";
            for (size_t i = 0; i < buf.size(); ++i) {
                if (i) o << ",";
                o << (int)buf[i];
            }
            o << "]}";
            if (output_file.empty()) std::cout << o.str() << std::endl; else { std::ofstream f(output_file, std::ios::app); f << o.str() << std::endl; }
        } else {
            for (auto b : buf) printf("%02X ", b);
            printf("\n");
        }
        return buf.empty() ? EXIT_DUMP_MEMORY_FAILED : EXIT_SUCCESSFUL;
    }

    print_usage();
    return 0;
}
