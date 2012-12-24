/* ***** BEGIN LICENSE BLOCK *****
;; corvus analyzer
;;
;; Copyright (c) 2012 Shannon Weyrick <weyrick@mozek.us>
;;
;; This program is free software; you can redistribute it and/or
;; modify it under the terms of the GNU General Public License
;; as published by the Free Software Foundation; either version 2
;; of the License, or (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, write to the Free Software
;; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
   ***** END LICENSE BLOCK *****
*/

#include "corvus/analysis/passes/Trivial.h"

namespace corvus { namespace AST { namespace Pass {

/*
void Trivial::pre_run(void) {
}

void Trivial::post_run(void) {
}
*/

void Trivial::visit_pre_signature(signature* n) {

    // verify that if we see a parameter with a default, that
    // we don't find a parameter after it with no default
    int p = n->numParams();
    if (!p)
        return;
    bool reqDefault = false;
    formalParam* param(0);
    for (uint i = 0; i < p; i++) {
        param = n->getParam(i);
        if (param->hasDefault()) {
            reqDefault = true;
        }
        else if (reqDefault) {
            // here we have this situation:
            // function($foo, $bar='test', $baz)
            std::cout << "no no! on line " << n->startLineNum() << "\n";
        }
    }

}

} } } // namespace
