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
    // function($foo, $bar='test', $baz)
    int p = n->numParams();
    if (!p)
        return;
    formalParam* param(0);
    formalParam* firstParam(0);
    // note, the params are in reverse order
    for (int i = p-1; i >= 0; i--) {
        param = n->getParam(i);
        if (param->hasDefault()) {
            if (!firstParam)
                firstParam = param;
        }
        else if (firstParam) {
            addDiagnostic(param, "parameter should have default because previous parameter does");
            addDiagnostic(firstParam, "first parameter with default defined here");
            // return so that we only show the diag once
            return;
        }
    }

}

} } } // namespace

