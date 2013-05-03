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
    {0, 0, 0, 0}
};


const char *VERSION = "1.0";

void corvusVersion(void) {
    std::cout << "corvus PHP source analyzer " << VERSION << std::endl;
    std::cout << "author: Shannon Weyrick <weyrick@mozek.us>\n" << std::endl;
    std::cout << "USAGE: corvus [options] <input files/dirs>\n" \
                 "OPTIONS:\n" \
                 " --debug-model            - Debug the model builder\n" \
                 " --debug-parse            - Debug output from parser\n" \
                 " -h,--help                - Display available options (-help-hidden for more)\n" \
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

    std::string exts("php");
    std::vector<std::string> inputFiles;
    std::vector<std::string> includePaths;

    pSourceManager sm;

    while ((opt = getopt_long(argc, argv, "tai:hve:d:", longopts,
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
            includePaths.push_back(optarg);
            break;
        case 'e':
            exts = optarg;
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

    if (!includePaths.empty()) {
        for (unsigned i = 0; i != includePaths.size(); ++i) {
            sm.addIncludeDir(includePaths[i], exts);
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

    // try to read home directory config file
    char *home = getenv("HOME");
    if (home) {
        pConfig hc;
        if (hc.read(pStringRef(home)+"/.corvus")) {
            //std::cout << "read .corvus yay!!\n";
        }
    }

    for (unsigned i = 0; i != inputFiles.size(); ++i) {

        sys::fs::file_status stat;
        sys::fs::status(inputFiles[i], stat);
        if (sys::fs::is_directory(stat))
            sm.addSourceDir(inputFiles[i], exts);
        else if (sys::fs::is_regular_file(stat))
            sm.addSourceFile(inputFiles[i]);

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
