/* ***** BEGIN LICENSE BLOCK *****
 * corvus analyzer Runtime Libraries
 *
 * Copyright (c) 2009 Shannon Weyrick <weyrick@mozek.us>
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

#ifndef COR_PRUNTIMETYPES_H_
#define COR_PRUNTIMETYPES_H_

#include "corvus/pTypes.h"
#include "corvus/pSourceTypes.h"
#include "corvus/runtime/CowPtr.h"

#include <boost/logic/tribool.hpp>
#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <vector>

namespace corvus {

/// a boost::tribool represents php true, false and null values
/// pTrue, pFalse and pNull are defined
typedef boost::logic::tribool pTriState;

// convenience accesors
BOOST_TRIBOOL_THIRD_STATE(pNull)
#define pTrue  pTriState(true)
#define pFalse pTriState(false)

// basic number types defined in pTypes.h
typedef CowPtr<pBigInt> pBigIntP;
typedef CowPtr<pFloat> pFloatP;

/// runtime "binary" string type
/// these are simple byte-wide character arrays
typedef std::string pBString;

class pHash;
/// copy-on-write auto shared hash table type
typedef CowPtr<pHash> pHashP;

class pObject;
/// auto shared pointer to an instantiated runtime object
typedef boost::shared_ptr<pObject> pObjectP;

class pResource;
/// auto shared pointer to an runtime resource
typedef boost::shared_ptr<pResource> pResourceP;

class pVar;
/// auto shared pointer to a pVar
typedef boost::shared_ptr<pVar> pVarP;

/// main pVar variant type
typedef boost::variant< pTriState,
                        pInt,
                        pBigIntP,
                        pFloatP,
                        pBString,
                        pHashP,
                        pObjectP,
                        pResourceP,
                        pVarP
                        > pVarDataType;

/// these should match the order of the actual types listed in the variant type
/// it is only used by the pVar type checker, which returns pVarType
/// this is only because pVarNullType and pVarBoolType are both handled
/// by pTriState
typedef enum {
    pVarTriStateType_ = 0,
    pVarIntType_      = 1,
    pVarBigIntType_   = 2,
    pVarFloatType_    = 3,
    pVarBStringType_  = 4,
    pVarUStringType_  = 5,
    pVarHashType_     = 6,
    pVarObjectType_   = 7,
    pVarResourceType_ = 8,
    pVarPtrType_      = 9
} pVarWhichType_;

/// an enum for determining the type of data stored in a pVar
typedef enum {
    pVarNullType = 0,
    pVarBoolType = 0,
    pVarIntType = 1,
    pVarBigIntType = 2,
    pVarFloatType = 3,
    pVarBStringType = 4,
    pVarUStringType = 5,
    pVarHashType = 6,
    pVarObjectType = 7,
    pVarResourceType = 8,
    pVarPtrType = 9
} pVarType;


class pRuntimeEngine;

#define COR_STDFUNC_ARGS   pVar* retVal, pRuntimeEngine* runtime
#define COR_STDMETHOD_ARGS pVar* retVal, pRuntimeEngine* runtime, pObjectP pThis

/// php function signature: no arguments
typedef void (*pFunPointer0)(COR_STDFUNC_ARGS);
/// php function signature: one argument
typedef void (*pFunPointer1)(COR_STDFUNC_ARGS, const pVar&);
/// php function signature: two arguments
typedef void (*pFunPointer2)(COR_STDFUNC_ARGS, const pVar&, const pVar&);
/// php function signature: three arguments
typedef void (*pFunPointer3)(COR_STDFUNC_ARGS, const pVar&, const pVar&, const pVar&);
/// php function signature: four arguments
typedef void (*pFunPointer4)(COR_STDFUNC_ARGS, const pVar&, const pVar&, const pVar&, const pVar&);
/// php function signature: five arguments
typedef void (*pFunPointer5)(COR_STDFUNC_ARGS, const pVar&, const pVar&, const pVar&, const pVar&, const pVar&);
/// php function signature: n arguments
typedef void (*pFunPointerN)(COR_STDFUNC_ARGS, std::vector<const pVar&>);

/// php method signature: no arguments
typedef void (*pMethodPointer0)(COR_STDMETHOD_ARGS);
/// php method signature: one argument
typedef void (*pMethodPointer1)(COR_STDMETHOD_ARGS, const pVar&);
/// php method signature: two arguments
typedef void (*pMethodPointer2)(COR_STDMETHOD_ARGS, const pVar&, const pVar&);
/// php method signature: three arguments
typedef void (*pMethodPointer3)(COR_STDMETHOD_ARGS, const pVar&, const pVar&, const pVar&);
/// php method signature: four arguments
typedef void (*pMethodPointer4)(COR_STDMETHOD_ARGS, const pVar&, const pVar&, const pVar&, const pVar&);
/// php method signature: five arguments
typedef void (*pMethodPointer5)(COR_STDMETHOD_ARGS, const pVar&, const pVar&, const pVar&, const pVar&, const pVar&);
/// php method signature: n arguments
typedef void (*pMethodPointerN)(COR_STDMETHOD_ARGS, std::vector<const pVar&>);

// runtime handlers
typedef void (*pIncludeHandlerFun)(pFileNameString file);
typedef void (*pEvalHandlerFun)(pSourceString code);

} /* namespace corvus */


#endif /* COR_PRUNTIMETYPES_H_ */
