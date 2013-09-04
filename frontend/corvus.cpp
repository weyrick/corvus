/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012-2013 Shannon Weyrick <weyrick@mozek.us>
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
#include <algorithm>

#include "corvus/pSourceManager.h"
#include "corvus/pConfig.h"
#include "corvus/pDiagnostic.h"
#include "corvus/pModel.h"
#include <llvm/Support/FileSystem.h>

using namespace llvm;
using namespace corvus;

struct option longopts[] = {
    {"print-toks", 0, 0, 't'},
    {"print-ast", 0, 0, 'a'},
    {"class-graph", 1, 0, 'g'},
    {"debug-parse", 0, 0, 0},
    {"debug-model", 0, 0, 0},
    {"debug-diags", 0, 0, 0},
    {"include", 1, 0, 'i'},
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
    std::cout << "USAGE: \n" \
                 "corvus [options] <source files/dirs>\n" \
                 "corvus [options] -c <config> [diagnostic files]\n\n" \
                 "in the first invocation format, specify a list of source files " \
                 "or directories to both parse and produce diagnostics on.\n\n" \
                 "in the second invocation format, specify a config file which includes " \
                 "the source directories and files, then optionally list which of those source files\n" \
                 "to produce diagnostics on.\n\n" \
                 "OPTIONS:\n" \
                 " --debug-model            - Debug the model builder\n" \
                 " --debug-diags            - Debug the diagnostics\n" \
                 " --debug-parse            - Debug output from parser\n" \
                 " --class-graph            - Generate a DOT graph of the class heirarchy\n" \
                 " -c,--config=<file>       - Load corvus config file\n" \
                 " -h,--help                - Display available options\n" \
                 " -a,--print-ast           - Print AST in XML format\n" \
                 " -t,--print-toks          - Print tokens from lexer\n" \
                 " -e,--exts=<list>         - Source file extensions to parse when reading a directory (command separated, default: php)\n" \
                 " -i,--include=<directory> - Add a directory to build model from, but not generate diagnostics for\n" \
                 " -d,--db=<file>           - Name of model database. If not specified, no model data is stored.\n" \
                 " -v                       - Increase verbosity, may specify more than once\n" \
                 " --version                - Display the version of this program\n" << std::endl;
}

void renderDiagnostic(pStringRef cwd, const pDiagnostic *d) {
    /*
    const pParseContext &C_ = module_->context();
    std::cout << C_.getOwner()->fileName() << ":" << s->startLineNum() << ":" << s->startCol() << ": " << msg.data() << std::endl;
    if (!s->startLineNum() || !s->startCol())
        return;
    // diag source line
    pSourceCharIterator i = C_.getOwner()->source()->contents()->getBufferStart();
    pSourceCharIterator end = C_.getOwner()->source()->contents()->getBufferEnd();
    pUInt curLine(1), diagLine(s->startLineNum());
    while (curLine != diagLine) {
        if (*i == '\n')
            curLine++;
        i++;
    }
    pSourceCharIterator e = i;
    while (*e != '\n' && e != end)
        e++;
    pStringRef line(i, e-i);
    std::cout << line.str() << std::endl;
    // arrow to problem column
    std::cout << std::string(s->startCol()-1, ' ') << "^" << std::endl;
    */
    // strip leading cwd
    pStringRef fname = d->location().path();
    if (fname.startswith(cwd)) {
        std::cout << fname.substr(cwd.size()+1).str() << ":";
    }
    else {
        std::cout << fname.str() << ":";
    }
    std::cout << d->location().range().startLine << ":" << d->location().range().startCol << ": " << d->msg().str() << std::endl;
}

bool willRenderFor(pStringRef cwd, const pConfig &config, pStringRef fname) {

    if (config.diagFiles.empty())
        return true;

    pConfig::StringListType::const_iterator i = find(config.diagFiles.begin(),
                                                     config.diagFiles.end(),
                                                     fname.str());
    if (i != config.diagFiles.end())
        // found it, render
        return true;

    // otherwise, try stripping cwd from file and check again
    // in this case, they specified a diagFile relative to cwd
    if (!fname.startswith(cwd))
        return false;

    i = find(config.diagFiles.begin(),
             config.diagFiles.end(),
             fname.substr(cwd.size()+1).str());
    return (i != config.diagFiles.end());

}

int main( int argc, char* argv[] )
{

    int opt, idx, verbosity(0);
    bool debugParse(false), debugModel(false), debugDiags(false);
    bool printToks(false), printAST(false);
    std::string graphFileName;

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
            if (strcmp(longopts[idx].name,"debug-diags") == 0) {
                debugDiags = true;
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
        case 'g':
            graphFileName = optarg;
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

    sm.setDebug(verbosity, debugParse, debugModel, debugDiags);

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

    bool haveSourceFromConfig = false;
    if (!config.inputFiles.empty()) {
        haveSourceFromConfig = true;
        for (unsigned i = 0; i != config.inputFiles.size(); ++i) {
            if (verbosity)
                std::cout << "[config] adding input file: " << config.inputFiles[i] << std::endl;
            inputFiles.push_back(config.inputFiles[i]);
        }
    }

    if (optind < argc) {
        // if we have files on the command line, but there were already files
        // added via a config file, then instead of adding these files to
        // the list we will instead limit diagnostic output to just these
        // files
        if (haveSourceFromConfig) {
            while (optind < argc)
                config.diagFiles.push_back(argv[optind++]);
        }
        else {
            while (optind < argc)
                inputFiles.push_back(argv[optind++]);
        }
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

    sm.refreshModel(graphFileName);
    sm.runDiagnostics();

    // render diagnostics
    llvm::SmallString<128> cwd;
    llvm::sys::fs::current_path(cwd);
    pSourceManager::DiagModuleListType mList = sm.getDiagModules();
    for (int i = 0; i < mList.size(); ++i) {
        // if we have diagfiles, only render for the ones in that list
        if (!willRenderFor(cwd, config, mList[i]->fileName()))
            continue;
        pSourceModule::DiagListType dList = mList[i]->getDiagnostics();
        for (int j = 0; j < dList.size(); ++j) {
            renderDiagnostic(cwd, dList[j]);
        }
    }

    if (verbosity)
        std::cerr << "analyzation complete" << std::endl;

    return 0;

}
