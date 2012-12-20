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

#include <boost/function.hpp>
#include <boost/cstdint.hpp>
#include <boost/tuple/tuple.hpp>

#include <string>

#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>

#include <boost/foreach.hpp>

#define foreach         BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

namespace corvus {

/// string ref, see llvm::StringRef
typedef llvm::StringRef pStringRef;

/// Twine for fast string concats, see llvm::Twine
typedef llvm::Twine pTwine;

/// signed (fast) integer type (used in pVar)
typedef signed long pInt;
#define COR_INT_MIN LONG_MIN
#define COR_INT_MAX LONG_MAX

/// float type (used in pVar)
typedef double pFloat;

/// unsigned integer type (not used in pVar)
typedef boost::uint_fast32_t pUInt;

/// string type used for identifiers (classes, functions, variable names)
typedef std::string pIdentString;

/// string type used for filenames
typedef std::string pFileNameString;

} /* namespace corvus */


#endif /* COR_PTYPES_H_ */
