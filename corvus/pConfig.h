/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PCONFIG_H_
#define COR_PCONFIG_H_

#include "pTypes.h"
#include <string>
#include <llvm/ADT/Twine.h>

namespace corvus {

class pConfig {

public:

    bool read(const llvm::Twine &file);

};

} // namespace

#endif
