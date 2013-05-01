/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PPASSMANAGER_H_
#define COR_PPASSMANAGER_H_

#include <vector>

namespace corvus {

class pSourceModule;
class pModel;

namespace AST {
class pPass;
}

class pPassManager {
public:
    typedef std::vector<AST::pPass*> queueType;

private:

    queueType passQueue_;
    pModel *model_;

    // no copy constructor
    pPassManager(const pPassManager&);

public:

    pPassManager(pModel *m): model_(m), passQueue_() { }
    ~pPassManager(void);

    /// add a pass. takes ownership.
    void addPass(AST::pPass* p);

    template <typename PassType>
    void addPass(void) {
        PassType* P = new PassType();
        addPass(P);
    }

    void run(pSourceModule *mod, int verbosity);

};

} // namespace

#endif /* COR_PPASSMANAGER_H_ */
