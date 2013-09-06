/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PFULLMODELCHECKER_H_
#define COR_PFULLMODELCHECKER_H_

#include "pTypes.h"

namespace corvus {

class pSourceManager;
class pModel;

class pFullModelChecker {

private:

    pSourceManager* sourceMgr_;
    const pModel* model_;

    void addDiagnostic(pStringRef realPath, int sl, int sc, pStringRef msg);

    void classRelations();
    void declUse();

public:

    pFullModelChecker(pSourceManager *s, const pModel* m):
        sourceMgr_(s), model_(m) { }

    void run();

};

} // namespace

#endif
