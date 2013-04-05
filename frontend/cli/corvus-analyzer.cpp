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
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/PathV2.h>
#include <llvm/Support/FileSystem.h>

#include "corvus/analysis/pSourceManager.h"

using namespace llvm;
using namespace corvus;

void corvusVersion(void) {
    std::cout << "corvus PHP source analyzer" << std::endl;
}


int main( int argc, char* argv[] )
{

    // command line options
    cl::list<std::string> inputFiles(cl::Positional,
                                     cl::desc("<input files/dirs>"),
                                     cl::OneOrMore,
                                     cl::ValueRequired
                                    );

    cl::opt<bool> printToks ("print-toks", cl::desc("Print tokens from lexer"));
    cl::opt<bool> printAST ("print-ast", cl::desc("Print AST in XML format"));
    cl::opt<bool> debugParse ("debug-parse", cl::desc("Debug output from parser"));
    cl::opt<bool> debugModel ("debug-model", cl::desc("Debug the model builder"));

    cl::SetVersionPrinter(&corvusVersion);
    cl::ParseCommandLineOptions(argc, argv, "corvus analyzer");

    pSourceManager sm;
    sm.setDebug(debugParse, debugModel);

    for (unsigned i = 0; i != inputFiles.size(); ++i) {

        sys::fs::file_status stat;
        sys::fs::status(inputFiles[i], stat);
        if (sys::fs::is_directory(stat))
            sm.addSourceDir(inputFiles[i], ".php");
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

    return 0;

}
