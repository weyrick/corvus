/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/pPassManager.h"
#include "corvus/pPass.h"
#include "corvus/pSourceModule.h"

namespace corvus {

pPassManager::~pPassManager(void) {

    // free passes
    for (queueType::iterator i = passQueue_.begin();
         i != passQueue_.end();
         ++i) {
        delete *i;
    }

}

void pPassManager::run(pSourceModule* mod, int verbosity) {

    for (queueType::iterator i = passQueue_.begin();
         i != passQueue_.end();
         ++i) {
        if (verbosity > 1) {
            std::cerr << "running pass [" << (*i)->name().str() << "] on " << mod->fileName() << std::endl;
        }
        (*i)->do_pre_run(mod);
        if ((*i)->aborted())
                return;
        (*i)->do_run(mod);
        if ((*i)->aborted())
                return;
        (*i)->do_post_run(mod);
    }
}

void pPassManager::addPass(AST::pPass* p) {
    p->setModel(model_);
    passQueue_.push_back(p);
}


} // namespace
