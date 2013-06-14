/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include <exception>
#include <iostream>
#include <string>

#include "corvus/pSourceManager.h"
#include "corvus/pConfig.h"
#include "corvus/pDiagnostic.h"
#include <llvm/Support/FileSystem.h>
#include <sstream>

using namespace llvm;
using namespace corvus;

void cassert(int a, int b, int line) {
    if (a != b) {
        std::cout << line << ": " << a << " != " << b << std::endl;
        exit(1);
    }
}

void cassert(pStringRef a, pStringRef b, int line) {
    if (!a.equals(b)) {
        std::cout << line << ": "<< "[" << a.str() << "] != [" << b.str() << "]" << std::endl;
        exit(1);
    }
}

void cassert(pSourceLoc a, pSourceLoc b, int line) {
    if (a != b) {
        std::cout << line << ": "<< "[" << a.toString() << "] != [" << b.toString() << "]" << std::endl;
        exit(1);
    }
}

void cassert(pSourceRange a, pSourceRange b, int line) {
    if (a != b) {
        std::stringstream outa, outb;
        outa << a.startLine << ":" << a.startCol << ":" <<
                     a.endLine << ":" << a.endCol;
        outb << b.startLine << ":" << b.startCol << ":" <<
                     b.endLine << ":" << b.endCol;
        std::cout << line << ": ""[" << outa.str() << "] != [" << outb.str() << "]" << std::endl;
        exit(1);
    }
}

int main( int argc, char* argv[] )
{

    pSourceManager sm;
    pConfig config;
    std::vector<std::string> inputFiles;

    config.exts = "php";

    // try to read home directory config file
    char *home = getenv("HOME");
    if (home) {
        // will ignore if not found
        pConfigMgr::read(pStringRef(home)+"/.corvus", config);
    }

    if (!config.includePaths.empty()) {
        for (unsigned i = 0; i != config.includePaths.size(); ++i) {
            sm.addIncludeDir(config.includePaths[i], config.exts);
        }
    }

    sm.setModelDBName("test.db");
    inputFiles.push_back("test1.php");

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

    try {
        sm.refreshModel();
        sm.runDiagnostics();
    }
    catch (std::exception& e) {
        std::cout << "exception: " << e.what() << "\n";
        exit(1);
    }

    pSourceManager::DiagModuleListType mList = sm.getDiagModules();

    cassert(mList.size(), 1, __LINE__);

    pSourceModule::DiagListType dList = mList[0]->getDiagnostics();

    // DIAG COUNT
    cassert(dList.size(), 5, __LINE__);

    // 1
    cassert(dList[0]->msg(), "function 'nonexist' not defined", __LINE__);
    cassert(dList[0]->location().range(), pSourceRange(56,1,56,0), __LINE__); // XXX should be 56,1 or 56,1,56,1

    // 2
    cassert(dList[1]->msg(), "wrong number of arguments: function 'bar' takes between minArity and maxArity arguments (0 specified)", __LINE__);
    cassert(dList[1]->location().range(), pSourceRange(59,1,59,0), __LINE__); // XXX

    // 3
    cassert(dList[2]->msg(), "wrong number of arguments: function 'bar' takes between minArity and maxArity arguments (4 specified)", __LINE__);
    cassert(dList[2]->location().range(), pSourceRange(63,1,63,0), __LINE__); //

    // 4
    cassert(dList[3]->msg(), "parameter should have default because previous parameter does", __LINE__);
    cassert(dList[3]->location().range(), pSourceRange(108,28,108,34), __LINE__); //

    // 5 (second part of 4)
    cassert(dList[4]->msg(), "first parameter with default defined here", __LINE__);
    cassert(dList[4]->location().range(), pSourceRange(108,20,108,24), __LINE__); //

    std::cout << "all tests passing" << std::endl;
    return 0;

}
