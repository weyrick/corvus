/* ***** BEGIN LICENSE BLOCK *****
 *
 * Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef COR_PSOURCELOC_H_
#define COR_PSOURCELOC_H_

#include "pTypes.h"
#include <string>

namespace corvus {

class pSourceModule;

class pSourceLoc {
    std::string path_;
    const pSourceModule *module_;
    pSourceRange range_;

public:
    pSourceLoc(const pSourceModule *mod, pUInt sl, pUInt sc, pUInt el=0, pUInt ec=0):
        module_(mod) {
        range_.startLine = sl;
        range_.startCol = sc;
        range_.endLine = el;
        range_.endCol = ec;
    }

    pSourceLoc(pStringRef path, pUInt sl, pUInt sc, pUInt el=0, pUInt ec=0):
        path_(path), module_(0) {
        range_.startLine = sl;
        range_.startCol = sc;
        range_.endLine = el;
        range_.endCol = ec;
    }

    pSourceLoc(pStringRef path, const pSourceRange &r):
        path_(path), module_(0), range_(r) { }

    pSourceLoc(const pSourceModule *mod, const pSourceRange &r):
        module_(mod), range_(r) { }

    const pSourceRange& range() const { return range_; }

    std::string path() const;

    std::string toString() const;

    bool operator==(const pSourceLoc &other) const {
        return (path() == other.path() &&
                range_ == other.range_);
    }

    bool operator!=(const pSourceLoc &other) const {
        return !(*this == other);
    }

};

} /* namespace corvus */


#endif /* COR_PTYPES_H_ */
