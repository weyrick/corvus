/* ***** BEGIN LICENSE BLOCK *****
 *
 * Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef COR_PTYPES_H_
#define COR_PTYPES_H_

#define BOOST_ALL_NO_LIB 1

#include <string>
#include <llvm/ADT/StringRef.h>
#include <boost/cstdint.hpp>
#include <boost/foreach.hpp>

#define foreach         BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

namespace corvus {

/// string ref, see llvm::StringRef
typedef llvm::StringRef pStringRef;

/// signed (fast) integer type
typedef signed long pInt;
#define COR_INT_MIN LONG_MIN
#define COR_INT_MAX LONG_MAX

/// float type (used in pVar)
typedef double pFloat;

/// unsigned integer type
typedef boost::uint_fast32_t pUInt;

/// source files. note that these are not safe to store. they must be copied
/// if they will live past the life of the buffer they came from (source file)
typedef pStringRef pSourceRef;
typedef pStringRef::iterator pSourceCharIterator;
typedef std::pair<pUInt,pUInt> pColRange;

struct pSourceRange {

    pSourceRange(void): startLine(0), endLine(0), startCol(0), endCol(0) { }

    pSourceRange(pUInt sl, pUInt sc): startLine(sl), endLine(0),
        startCol(sc), endCol(0) { }

    pSourceRange(pUInt sl, pUInt sc, pUInt el, pUInt ec): startLine(sl), endLine(el),
        startCol(sc), endCol(ec) { }

    pUInt startLine;
    pUInt endLine;
    pUInt startCol;
    pUInt endCol;

    bool operator==(const pSourceRange &other) const {
        return (startLine == other.startLine &&
                endLine == other.endLine &&
                startCol == other.startCol &&
                endCol == other.endCol
                );
    }

    bool operator!=(const pSourceRange &other) const {
        return !(*this == other);
    }
};

} /* namespace corvus */


#endif /* COR_PTYPES_H_ */
