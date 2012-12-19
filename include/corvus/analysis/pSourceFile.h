/* ***** BEGIN LICENSE BLOCK *****
;; corvus analyzer
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
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

#ifndef COR_PSOURCEFILE_H_
#define COR_PSOURCEFILE_H_

#include <string>
#include "corvus/pTypes.h"
#include "corvus/pSourceTypes.h"

namespace corvus {

class pSourceFile {

private:
    pSourceFileDesc file_;
    pSourceString contents_;

public:

    pSourceFile(const pSourceFileDesc& file);

    const pFileNameString& fileName(void) const { return file_.fileName(); }
    const pSourceString& contents(void) const { return contents_; }

};

} // namespace

#endif /* COR_PSOURCEFILE_H_ */
