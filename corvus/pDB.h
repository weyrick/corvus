/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#ifndef COR_PDB_H_
#define COR_PDB_H_

#include "corvus/pTypes.h"

#include <sqlite3.h>
#include <map>
#include <vector>

#include <iostream>

namespace corvus {

// these are for caching data retrieved from sqlite
namespace db {

// a simple wrapper for a row in the model db
// which saves us from having to make structs for each row
// here we can just access it by field name
class dbRow {
public:
    typedef std::map<const std::string, std::string> StringMap;
    typedef std::map<const std::string, long> IntMap;
protected:
    StringMap fields_;
    mutable IntMap intFields_; // lazyily built cache
public:
    void dump() const {
        for (StringMap::const_iterator i = fields_.begin();
             i != fields_.end();
             ++i) {
            std::cerr << i->first << ": " << i->second << "\n";
        }
    }
    const std::string& get(pStringRef key) const {
        //std::cout << "key: " << key.str() << std::endl;
        StringMap::const_iterator i = fields_.find(key);
        assert(i != fields_.end() && "key not found");
        return i->second;
    }
    int getAsInt(pStringRef key) const;
    sqlite3_int64 getAsOID(pStringRef key) const;
    sqlite3_int64 getID() const {
        return getAsOID("id");
    }
    void set(pStringRef key, pStringRef val) {
        fields_[key] = val;
    }
};

class pDB {
public:

    typedef sqlite3_int64 oid;
    typedef std::vector<dbRow> RowList;

    enum {
        NULLID   = 0
    };

private:

    sqlite3 *db_;
    bool trace_;

public:

    pDB(sqlite3 *db, bool trace=false): db_(db), trace_(trace) {
        sql_setup();
    }

    void sql_execute(pStringRef query) const;
    oid sql_insert(pStringRef query) const;
    oid sql_select_single_id(pStringRef query) const;
    std::string sql_select_single_string(pStringRef query) const;
    void sql_setup();
    void sql_done();

    std::string oidOrNull(oid val);
    std::string sql_string(pStringRef val, bool allowNull=true);

    //template <typename LTYPE>
    void list_query(pStringRef query, RowList &result) const;

    void setTrace(bool trace) { trace_ = trace; }
    bool trace(void) const { return trace_; }

    void begin();
    void commit();

};

} } // namespace

#endif
