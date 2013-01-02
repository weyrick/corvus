/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/pPassManager.h"
#include "corvus/analysis/pPass.h"
#include "corvus/analysis/pSourceModule.h"

namespace corvus {

pPassManager::~pPassManager(void) {

    // free passes
    for (queueType::iterator i = passQueue_.begin();
         i != passQueue_.end();
         ++i) {
        delete *i;
    }

}

void pPassManager::run(pSourceModule* mod) {

    for (queueType::iterator i = passQueue_.begin();
         i != passQueue_.end();
         ++i) {
        (*i)->do_pre_run(mod);
        (*i)->do_run(mod);
        (*i)->do_post_run(mod);
    }
}

void pPassManager::addPass(AST::pPass* p) {
    passQueue_.push_back(p);
}


} // namespace
