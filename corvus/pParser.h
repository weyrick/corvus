/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PPARSER_H_
#define COR_PPARSER_H_

#include "corvus/pSourceFile.h"

namespace corvus {

class pSourceModule;

namespace parser {

void parseSourceFile(pSourceModule* pMod, bool debug);

} } // namespace

#endif /* COR_PPARSER_H_ */
