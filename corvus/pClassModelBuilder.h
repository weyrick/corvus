/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef PCLASSMODELBUILDER_H
#define PCLASSMODELBUILDER_H

namespace corvus {

namespace db { class pDB; }

class pClassModelBuilder
{
private:

    // we do not own
    db::pDB* db_;

    void build_graph();

public:
    pClassModelBuilder(db::pDB* db): db_(db) { }

    void refresh();


};

} // namespace

#endif // PCLASSMODELBUILDER_H
