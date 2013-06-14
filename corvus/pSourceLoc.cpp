/* ***** BEGIN LICENSE BLOCK *****
 *
 * Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ***** END LICENSE BLOCK ***** */

#include "pSourceLoc.h"
#include "pSourceModule.h"
#include <sstream>

namespace corvus {

std::string pSourceLoc::path() const {
    if (module_)
        return module_->fileName();
    else
        return path_;
}

std::string pSourceLoc::toString() const {

    std::stringstream out;
    out << path() << ":" << range_.startLine << ":" << range_.startCol <<
                 range_.endLine << ":" << range_.endCol;
    return out.str();

}

}
