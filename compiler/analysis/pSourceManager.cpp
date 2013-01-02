/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/pSourceManager.h"

#include "corvus/analysis/pLexer.h"
#include "corvus/analysis/pSourceModule.h"
#include "corvus/analysis/pPassManager.h"

#include "corvus/analysis/passes/PrintAST.h"
#include "corvus/analysis/passes/DumpStats.h"
#include "corvus/analysis/passes/Trivial.h"

namespace corvus { 

pSourceManager::~pSourceManager() {
    for (unsigned i = 0; i != moduleList_.size(); ++i) {
        delete moduleList_[i];
    }
}

void pSourceManager::addSourceFile(pStringRef name) {
    pSourceModule* unit(new pSourceModule(name));
    moduleList_.push_back(unit);
}

void pSourceManager::addSourceDir(pStringRef name, pStringRef glob) {

}

void pSourceManager::runPasses(pPassManager *pm) {

    for (unsigned i = 0; i != moduleList_.size(); ++i) {

        try {
            // catch parse errors
            // this is idempotent
            moduleList_[i]->parse(debug_);
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            continue;
        }

        try {
            // run selected passes
            pm->run(moduleList_[i]);
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }

    } // input file loop

}

void pSourceManager::printAST() {

    pPassManager passManager;

    passManager.addPass<AST::Pass::PrintAST>();
    if (debug_)
        passManager.addPass<AST::Pass::DumpStats>();

    runPasses(&passManager);

}

void pSourceManager::printToks() {

    for (unsigned i = 0; i != moduleList_.size(); ++i) {

        lexer::pLexer l(moduleList_[i]->source());
        l.dumpTokens();

    }

}


void pSourceManager::refreshModel() {

    pPassManager passManager;

    // standard passes
    passManager.addPass<AST::Pass::Trivial>();

    runPasses(&passManager);

}

} // namespace

