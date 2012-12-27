/* ***** BEGIN LICENSE BLOCK *****
 * corvus analyzer Runtime Libraries
 *
 * Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
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
typedef pStringRef pSourceRange;
typedef pStringRef::iterator pSourceCharIterator;
typedef std::pair<pUInt,pUInt> pColRange;

} /* namespace corvus */


#endif /* COR_PTYPES_H_ */
