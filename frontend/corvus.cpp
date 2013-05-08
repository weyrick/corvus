/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2009-2010 Shannon Weyrick <weyrick@mozek.us>
;; Copyright (c) 2010 Cornelius Riemenschneider <c.r1@gmx.de>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include <exception>
#include <iostream>
#include <string>
#include <getopt.h>

#include "corvus/pSourceManager.h"
#include "corvus/pConfig.h"
#include <llvm/Support/FileSystem.h>

using namespace llvm;
using namespace corvus;

struct option longopts[] = {
    {"print-toks", 0, 0, 't'},
    {"print-ast", 0, 0, 'a'},
    {"debug-parse", 0, 0, 0},
    {"debug-model", 0, 0, 0},
    {"stubs", 1, 0, 's'},
    {"exts", 1, 0, 'e'},
    {"db", 1, 0, 'd'},
    {"help", 0, 0, 'h'},
    {"verbose", 0, 0, 'v'},
    {"config", 1, 0, 'c'},
    {0, 0, 0, 0}
};


const char *VERSION = "1.0";

void corvusVersion(void) {
    std::cout << "corvus PHP source analyzer " << VERSION << std::endl;
    std::cout << "author: Shannon Weyrick <weyrick@mozek.us>\n" << std::endl;
    std::cout << "USAGE: corvus [options] [-c <config> | <input files/dirs>]\n" \
                 "OPTIONS:\n" \
                 " --debug-model            - Debug the model builder\n" \
                 " --debug-parse            - Debug output from parser\n" \
                 " -c,--config=<file>       - Load corvus config file\n" \
                 " -h,--help                - Display available options\n" \
                 " -a,--print-ast           - Print AST in XML format\n" \
                 " -t,--print-toks          - Print tokens from lexer\n" \
                 " -e,--exts=<list>         - Source file extensions to parse when reading a directory (command separated, default: php)\n" \
                 " -i,--include=<directory> - Add a directory to build model from, but not generate diagnostics for\n" \
                 " -d,--db=<file>           - Name of model database. If not specified, no model data is stored.\n" \
                 " -v                       - Increase verbosity, may specify more than once\n" \
                 " --version                - Display the version of this program" << std::endl;
}


int main( int argc, char* argv[] )
{

    int opt, idx, verbosity;
    bool debugParse(false), debugModel(false);
    bool printToks(false), printAST(false);

    std::vector<std::string> inputFiles;

    pSourceManager sm;
    pConfig config;
    config.exts = "php";

    // try to read home directory config file
    char *home = getenv("HOME");
    if (home) {
        // will ignore if not found
        pConfigMgr::read(pStringRef(home)+"/.corvus", config);
    }

    while ((opt = getopt_long(argc, argv, "tai:hve:d:c:", longopts,
                              &idx
                              )
            ) != -1
           ) {
        switch (opt) {
        case 0:
            // long option tied to a flag variable
            if (strcmp(longopts[idx].name,"debug-parse") == 0) {
                debugParse = true;
                continue;
            }
            if (strcmp(longopts[idx].name,"debug-model") == 0) {
                debugModel = true;
                continue;
            }
            inputFiles.push_back(longopts[idx].name);
            continue;
        case 'a':
            printAST = true;
            break;
        case 't':
            printToks = true;
            break;
        case 'i':
            config.includePaths.push_back(optarg);
            break;
        case 'c':
            if (!pConfigMgr::read(optarg, config)) {
                std::cerr << "unable to load config file: " << optarg << std::endl;
            }
            break;
        case 'e':
            config.exts = optarg;
            break;
        case 'h':
            corvusVersion();
            exit(0);
        case 'd':
            sm.setModelDBName(optarg);
            break;
        case 'v':
            verbosity++;
            break;
        }
    }

    sm.setDebug(verbosity, debugParse, debugModel);

    // set values from config
    if (!config.dbName.empty()) {
        if (verbosity)
            std::cout << "[config] setting db name: " << config.dbName << std::endl;
        sm.setModelDBName(config.dbName);
    }
    if (!config.includePaths.empty()) {
        for (unsigned i = 0; i != config.includePaths.size(); ++i) {
            if (verbosity)
                std::cout << "[config] adding include path: " << config.includePaths[i] << std::endl;
            sm.addIncludeDir(config.includePaths[i], config.exts);
        }
    }
    if (!config.inputFiles.empty()) {
        for (unsigned i = 0; i != config.inputFiles.size(); ++i) {
            if (verbosity)
                std::cout << "[config] adding input file: " << config.inputFiles[i] << std::endl;
            inputFiles.push_back(config.inputFiles[i]);
        }
    }

    if (optind < argc) {
        while (optind < argc)
          inputFiles.push_back(argv[optind++]);
    }

    if (inputFiles.empty()) {
        corvusVersion();
        exit(1);
    }

    for (unsigned i = 0; i != inputFiles.size(); ++i) {

        sys::fs::file_status stat;
        sys::fs::status(inputFiles[i], stat);
        if (sys::fs::is_directory(stat))
            sm.addSourceDir(inputFiles[i], config.exts);
        else if (sys::fs::is_regular_file(stat))
            sm.addSourceFile(inputFiles[i]);
        else
            std::cerr << "skipping unknown path: " << inputFiles[i] << std::endl;

    }

    if (printToks) {
        sm.printToks();
        return 0;
    }
    else if (printAST) {
        sm.printAST();
        return 0;
    }

    sm.refreshModel();
    sm.runDiagnostics();

    // XXX code to render diagnostics

    if (verbosity)
        std::cerr << "analyzation complete" << std::endl;

    return 0;

}
