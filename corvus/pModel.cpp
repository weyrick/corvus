/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2008-2009 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include "pModel.h"
#include "pClassGraph.h"

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <algorithm>
#include <iterator>
#include <llvm/ADT/SmallVector.h>


namespace {
    // a generic joiner which takes a std::vector of anything that can be spit
    // out to a stringstream (mostly int, string) and joins them into a comma
    // delimited string
    template <typename ITEMTYPE>
    std::string join(const std::vector< ITEMTYPE > &list) {
            std::stringstream joinbuf;
            copy(list.begin(),list.end(), std::ostream_iterator< ITEMTYPE >(joinbuf,","));
            std::string result = joinbuf.str();
            return result.substr(0,result.size()-2);
    }
}

namespace corvus { 

void pModel::makeTables() {

    #define CORVUS_DBMODEL_VERSION "1.0"

    db_->begin();

    const char *META = "CREATE TABLE IF NOT EXISTS corvus (" \
            "key TEXT UNIQUE NOT NULL," \
            "val TEXT" \
            ")";
    db_->sql_execute(META);

    const char *SM = "CREATE TABLE IF NOT EXISTS sourceModule (" \
                         "id INTEGER PRIMARY KEY,"
                         "realpath TEXT UNIQUE NOT NULL," \
                         "hash TEXT" \
                         ");";
    db_->sql_execute(SM);

    // type:
    //   0 - const (outside class)
    //   1 - define()
    const char *SD = "CREATE TABLE IF NOT EXISTS constant (" \
                         "id INTEGER PRIMARY KEY,"
                         "sourceModule_id INTEGER NOT NULL," \
                         "type INTEGER NOT NULL," \
                         "name TEXT NOT NULL," \
                         "value TEXT NOT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "FOREIGN KEY(sourceModule_id) REFERENCES sourceModule(id) ON DELETE CASCADE"
                         ")";
    db_->sql_execute(SD);

    const char *SU = "CREATE TABLE IF NOT EXISTS constant_use (" \
                         "id INTEGER PRIMARY KEY,"
                         "constant_id INTEGER NOT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                             "FOREIGN KEY(constant_id) REFERENCES constant(id) ON DELETE CASCADE"
                         ")";
    db_->sql_execute(SU);

    const char *NS = "CREATE TABLE IF NOT EXISTS namespace (" \
                         "id INTEGER PRIMARY KEY,"
                         "namespace TEXT UNIQUE NOT NULL" \
                         ")";
    db_->sql_execute(NS);

    // type:
    //   0 - class
    //   1 - interface
    // flags:
    //   0x1 ABSTRACT
    const char *CL = "CREATE TABLE IF NOT EXISTS class (" \
                         "id INTEGER PRIMARY KEY,"
                         "sourceModule_id INTEGER NOT NULL," \
                         "namespace_id INTEGER NULL," \
                         "name TEXT NOT NULL,"
                         "type INTEGER NOT NULL," \
                         "flags INTEGER NOT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "end_line INTEGER NOT NULL," \
                         "end_col INTEGER NOT NULL," \
                         "extends_count INTEGER NOT NULL," \
                         "implements_count INTEGER NOT NULL," \
                         // text versions
                         "extends TEXT NULL," \
                         "implements TEXT NULL," \
                         "unresolved_extends TEXT NULL," \
                         "unresolved_implements TEXT NULL," \
                         //
                         "FOREIGN KEY(namespace_id) REFERENCES namespace(id) ON DELETE CASCADE," \
                         "FOREIGN KEY(sourceModule_id) REFERENCES sourceModule(id) ON DELETE CASCADE"
                         ")";
    db_->sql_execute(CL);

    // the relations go "lhs TYPE rhs"
    // type:
    //   0 - extends
    //   1 - implements
    const char *CR = "CREATE TABLE IF NOT EXISTS class_relations (" \
            "id INTEGER PRIMARY KEY,"
            "lhs_class_id INTEGER NOT NULL," \
            "type INTEGER NOT NULL," \
            "rhs_class_id INTEGER NOT NULL," \
            "FOREIGN KEY(lhs_class_id) REFERENCES class(id) ON DELETE CASCADE," \
            "FOREIGN KEY(rhs_class_id) REFERENCES class(id) ON DELETE CASCADE" \
                         ")";
    db_->sql_execute(CR);


    // type:
    //   0 - class property
    //   1 - class constant
    // flags:
    //   0x1 STATIC
    //   0x2 ABSTRACT
    // visibility:
    //   2 PRIVATE
    //   1 PROTECTED
    //   0 PUBLIC
    const char *CD = "CREATE TABLE IF NOT EXISTS class_decl (" \
                         "id INTEGER PRIMARY KEY,"
                         "class_id INTEGER NOT NULL," \
                         "name TEXT NULL," \
                         "type INTEGER NOT NULL," \
                         "flags INTEGER NOT NULL," \
                         "visibility INTEGER NOT NULL," \
                         "defaultVal TEXT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "FOREIGN KEY(class_id) REFERENCES class(id) ON DELETE CASCADE"
                         ")";
    db_->sql_execute(CD);

    const char *CU = "CREATE TABLE IF NOT EXISTS class_decl_use (" \
                         "id INTEGER PRIMARY KEY,"
                         "class_id INTEGER NOT NULL," \
                         // if using an undeclared, this will be NULL
                         "class_decl_id INTEGER NULL," \
                         // if using an undeclared, this will contain the name
                         "name TEXT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "FOREIGN KEY(class_id) REFERENCES class(id) ON DELETE CASCADE," \
                         "FOREIGN KEY(class_decl_id) REFERENCES class_decl(id) ON DELETE CASCADE"
                         ")";
    db_->sql_execute(CU);


    // class model version: considering the class heirarchy
    const char *CMD = "CREATE TABLE IF NOT EXISTS class_model_decl (" \
                         "id INTEGER PRIMARY KEY," \
                         // the origin class
                         "class_id INTEGER NOT NULL," \
                         // may be a decl from itself or any parent in the hierarchy
                         "class_decl_id INTEGER NOT NULL," \
                         // we only foreign cascade on class id, not decl, since it should suffice
                         "FOREIGN KEY(class_id) REFERENCES class(id) ON DELETE CASCADE"
                         ")";
    db_->sql_execute(CMD);

    // type:
    //   0 - top level main
    //   1 - function
    //   2 - method
    // flags:
    //   0x1 STATIC
    //   0x2 ABSTRACT
    // visibility:
    //   2 PRIVATE
    //   1 PROTECTED
    //   0 PUBLIC
    const char *FN = "CREATE TABLE IF NOT EXISTS function (" \
                         "id INTEGER PRIMARY KEY,"
                         "sourceModule_id INTEGER NOT NULL," \
                         "namespace_id INTEGER NOT NULL," \
                         "class_id INTEGER NULL," \
                         "name TEXT,"
                         "type INTEGER NOT NULL," \
                         "flags INTEGER NOT NULL," \
                         "visibility INTEGER NOT NULL," \
                         "minArity INTEGER NOT NULL," \
                         "maxArity INTEGER NOT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "end_line INTEGER NOT NULL," \
                         "end_col INTEGER NOT NULL," \
                         "FOREIGN KEY(namespace_id) REFERENCES namespace(id) ON DELETE CASCADE," \
                         "FOREIGN KEY(class_id) REFERENCES class(id) ON DELETE CASCADE," \
                         "FOREIGN KEY(sourceModule_id) REFERENCES sourceModule(id) ON DELETE CASCADE"
                         ")";
    db_->sql_execute(FN);

    // class model version: considering the class heirarchy
    const char *CMF = "CREATE TABLE IF NOT EXISTS class_model_function (" \
                         "id INTEGER PRIMARY KEY," \
                         // the origin class
                         "class_id INTEGER NOT NULL," \
                         // may be a function from itself or any parent in the hierarchy
                         "class_function_id INTEGER NOT NULL," \
                         // we only foreign cascade on class id, not decl, since it should suffice
                         "FOREIGN KEY(class_id) REFERENCES class(id) ON DELETE CASCADE"
                         ")";
    db_->sql_execute(CMF);

    // type:
    //   0 - parameter
    //   1 - free variable
    // datatype:
    //   0 - unknown
    //   1 - integer
    //   2 - string
    //   3 - array
    //   4 - bool
    //   5 - null
    //   6 - object
    // flags:
    //   0x1 STATIC
    const char *FV = "CREATE TABLE IF NOT EXISTS function_var (" \
                         "id INTEGER PRIMARY KEY,"
                         "function_id INTEGER NOT NULL," \
                         "name TEXT,"
                         "type INTEGER NOT NULL," \
                         "flags INTEGER NOT NULL," \
                         "datatype INTEGER NOT NULL," \
                         "datatype_obj TEXT NULL," \
                         "defaultVal TEXT NULL," \
                         // filled in after initial model build, set to
                         // 1 if this symbol is a redeclare so
                         // we can skip it in the use check (and only check
                         // the first declaration)
                         "is_redecl INTEGER NOT NULL,"
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "FOREIGN KEY(function_id) REFERENCES function(id) ON DELETE CASCADE"
                         ")";
    db_->sql_execute(FV);


    const char *FVU = "CREATE TABLE IF NOT EXISTS function_var_use (" \
                         "id INTEGER PRIMARY KEY,"
                         "function_id INTEGER NOT NULL," \
                         // if using an undeclared, this will be NULL
                         "function_var_id INTEGER NULL," \
                         // if using an undeclared, this will contain the name
                         "name TEXT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "FOREIGN KEY(function_id) REFERENCES function(id) ON DELETE CASCADE," \
                         "FOREIGN KEY(function_var_id) REFERENCES function_var(id) ON DELETE CASCADE"
                         ")";
    db_->sql_execute(FVU);


    const char *FU = "CREATE TABLE IF NOT EXISTS function_use (" \
                         "id INTEGER PRIMARY KEY,"
                         // if using an undeclared, this will be NULL
                         "function_id INTEGER NULL," \
                         // if using an undeclared, this will contain the name
                         "name TEXT NULL," \
                         "start_line INTEGER NOT NULL," \
                         "start_col INTEGER NOT NULL," \
                         "FOREIGN KEY(function_id) REFERENCES function(id) ON DELETE CASCADE" \
                         ")";
    db_->sql_execute(FU);

    db_->commit();

}

bool pModel::sourceModuleDirty(pStringRef realPath, pStringRef hash) const {

    std::stringstream sql;

    sql << "SELECT hash FROM sourceModule WHERE realPath='" << realPath.str() << "'";

    std::string existing_hash = db_->sql_select_single_string(sql.str());

    if (existing_hash.size()) {
        // not in our local cache but it's in the db model
        // we have it in the db. check hash to see if we should remodel it
        return (hash != existing_hash);
    }
    else {
        // not in model, it's dirty
        return true;
    }

}

pModel::oid pModel::getSourceModuleOID(pStringRef realPath, pStringRef hash, bool deleteFirst) {

    if (modules_.find(realPath) != modules_.end()) {
        return modules_[realPath];
    }    

    std::stringstream sql;

    if (!deleteFirst) {
        sql << "SELECT oid FROM sourceModule WHERE realPath='" << realPath.str() << "'";

        pModel::oid existing = db_->sql_select_single_id(sql.str());
        if (existing != pModel::NULLID) {
            modules_[realPath] = existing;
            return existing;
        }
    }
    else {
        sql << "DELETE FROM sourceModule WHERE realPath='" << realPath.str() << "'";
        db_->sql_execute(sql.str());
    }
    sql.str("");

    sql << "INSERT INTO sourceModule VALUES (NULL, '" << realPath.str() << "', '" << hash.str() << "')";
    oid result = db_->sql_insert(sql.str().c_str());
    modules_[realPath] = result;
    return result;

}

pModel::oid pModel::getNamespaceOID(pStringRef ns, bool create) const {

    if (namespaces_.find(ns) != namespaces_.end()) {
        return namespaces_[ns];
    }

    std::stringstream sql;

    sql << "SELECT oid FROM namespace WHERE namespace='" << ns.str() << "'";

    pModel::oid existing = db_->sql_select_single_id(sql.str());
    if (existing != pModel::NULLID) {
        namespaces_[ns] = existing;
        return existing;
    }

    if (!create)
        return pModel::NULLID;

    sql.str("");

    sql << "INSERT INTO namespace VALUES (NULL, '" << ns.str() << "')";
    oid result = db_->sql_insert(sql.str().c_str());
    namespaces_[ns] = result;
    return result;

}

std::string pModel::getNamespaceName(pModel::oid ns_id) const {

    // linear search the cache first
    for (IDMap::const_iterator i = namespaces_.begin();
         i != namespaces_.end();
         ++i) {
        if (i->second == ns_id)
            return i->first;
    }

    // try sql if we haven't found it
    std::stringstream sql;

    sql << "SELECT namespace FROM namespace WHERE id=" << ns_id;
    std::string result = db_->sql_select_single_string(sql.str().c_str());

    if (!result.empty()) {
        // cache it while we here
        namespaces_[result] = ns_id;
    }

    return result;

}


pModel::oid pModel::defineClass(pModel::oid ns_id, pModel::oid m_id, pStringRef name,
                                int type, int extends_count, int implements_count, pStringRef extends,
                                pStringRef implements, pSourceRange range) {

    int flags = pModel::NO_FLAGS;

    std::stringstream sql;
    sql << "INSERT INTO class VALUES (NULL,"
        << m_id << ','
        << ns_id
        << ",'" << name.str() << "',"
        << type << ','
        << flags << ','
        << range.startLine  << ',' << range.startCol  << ',' << range.endLine  << ',' << range.endCol << ','
        << extends_count << ',' << implements_count << ','
        << db_->sql_string(extends, true) << ','
        << db_->sql_string(implements, true) << ','
        << db_->sql_string(extends, true) << ',' // unresolved until resolveClassRelations is called
        << db_->sql_string(implements, true)     // unresolved until resolveClassRelations is called
        << ")";
    return db_->sql_insert(sql.str().c_str());

}


pModel::oid pModel::defineFunction(oid ns_id, oid m_id, oid c_id, pStringRef name,
                    int type, int flags, int vis, int minA, int maxA, pSourceRange range) {

    std::stringstream sql;
    sql << "INSERT INTO function VALUES (NULL,"
        << m_id << ','
        << ns_id << ','
        << db_->oidOrNull(c_id)
        << ",'" << name.str() << "',"
        << type << ','
        << flags << ','
        << vis << ','
        << minA << ','
        << maxA << ','
        << range.startLine  << ',' << range.startCol  << ',' << range.endLine  << ',' << range.endCol
        << ")";
    return db_->sql_insert(sql.str().c_str());

}

void pModel::defineClassDecl(oid c_id, pStringRef name, int type, int flags, int vis, pStringRef defaultVal, pSourceRange range) {

    std::stringstream sql;
    sql << "INSERT INTO class_decl VALUES (NULL,"
        << c_id
        << ",'" << name.str() << "',"
        << type << ','
        << flags << ','
        << vis << ','
        << db_->sql_string(defaultVal) << ","
        << range.startLine  << ',' << range.startCol
        << ")";

    db_->sql_insert(sql.str().c_str());

}

void pModel::defineClassRelation(oid lhs_c_id, int type, oid rhs_c_id) {

    std::stringstream sql;
    sql << "INSERT INTO class_relations VALUES (NULL,"
        << lhs_c_id << ','
        << type << ','
        << rhs_c_id
        << ")";

    db_->sql_insert(sql.str().c_str());

}


void pModel::defineFunctionVar(oid f_id, pStringRef name,
                    int type, int flags, int datatype, pStringRef datatype_obj,
                    pStringRef defaultVal,
                    pSourceRange range) {

    std::stringstream sql;

    sql << "INSERT INTO function_var VALUES (NULL,"
        << f_id << ','
        << "'" << name.str() << "'" << ','
        << type << ','
        << flags << ','
        << datatype << ','
        << db_->sql_string(datatype_obj) << ","
        << db_->sql_string(defaultVal) << ","
        << "0," // is_redecl
        << range.startLine  << ',' << range.startCol
        << ")";
    db_->sql_insert(sql.str().c_str());

}

void pModel::defineFunctionUse(oid f_id, pStringRef name, pSourceRange range) {

    std::stringstream sql;

    // first we see if there's an associated decl.
    sql << "SELECT id FROM function_var WHERE name=" << "'" << name.str() << "'" <<
           " AND function_id=" << f_id << " AND start_line <= " << range.startLine;

    oid f_v_id = db_->sql_select_single_id(sql.str());

    sql.str("");

    sql << "INSERT INTO function_var_use VALUES (NULL,"
        << f_id << ','
        << db_->oidOrNull(f_v_id) << ','
        << ((f_v_id != pModel::NULLID) ? "''" : db_->sql_string(name.str())) << ','
        << range.startLine  << ',' << range.startCol
        << ")";
    db_->sql_insert(sql.str().c_str());

}

void pModel::defineConstant(oid m_id, pStringRef name, int type, pStringRef val, pSourceRange range) {

    std::stringstream sql;

    sql << "INSERT INTO constant VALUES (NULL,"
        << m_id << ','
        << type << ','
        << "'" << name.str() << "'" << ','
        << db_->sql_string(val,false) << ','
        << range.startLine  << ',' << range.startCol
        << ")";
    db_->sql_insert(sql.str().c_str());

}

pModel::FunctionList pModel::queryFunctions(oid ns_id, oid c_id, pStringRef name) const {

    FunctionList result;
    std::stringstream query;

    if (ns_id == pModel::NULLID)
        ns_id = getRootNamespaceOID();

    query << "SELECT name, type, flags, visibility, minArity, maxArity, " \
             " start_line, start_col, sourceModule.realPath FROM " \
             " function, sourceModule WHERE sourceModule.id=sourceModule_id AND" \
             " " \
             " (namespace_id=" << ns_id << " OR namespace_id=" << getRootNamespaceOID() << ") AND class_id ";
    if (c_id) {
        query << " = " << c_id;
    }
    else {
        query << " IS NULL ";
    }

    query << " AND name='" << name.str() << "'";

    //db_->list_query<FunctionList>(query.str(), result);
    db_->list_query(query.str(), result);

    return result;

}

pModel::ClassList pModel::queryClasses(oid ns_id, pStringRef name, pModel::oid m_id) const {

    ClassList result;
    std::stringstream query;

    query << "SELECT class.id, name, type, flags, " \
             " start_line, start_col, sourceModule.realPath FROM " \
             " class, sourceModule WHERE sourceModule.id=sourceModule_id AND" \
             " (namespace_id=" << ns_id << " OR namespace_id=1)" \
             " AND name='" << name.str() << "'";

    if (m_id != pModel::NULLID) {
        query << " AND class.sourceModule_id=" << m_id;
    }

    //db_->list_query<ClassList>(query.str(), result);
    db_->list_query(query.str(), result);

    return result;

}

pModel::ClassDeclList pModel::queryClassDecls(std::vector<oid> c_id_list, pStringRef name) const {

    ClassDeclList result;
    std::stringstream query;

    std::string c_id_list_str = join(c_id_list);
    query << "SELECT class.id, class_decl.name, class.name AS className, class_decl.type, class_decl.flags, visibility, defaultVal, " \
             " class_decl.start_line, class_decl.start_col, sourceModule.realPath FROM " \
             " class_model_decl, class_decl, class, sourceModule WHERE sourceModule.id=sourceModule_id AND" \
             " class.id=class_decl.class_id AND class_decl.id=class_model_decl.class_decl_id AND" \
             " class_model_decl.class_id IN (" << c_id_list_str << ")" \
             " AND class_decl.name='" << name.str() << "'";

    //db_->list_query<ClassDeclList>(query.str(), result);
    db_->list_query(query.str(), result);

    return result;

}

pModel::ClassDeclList pModel::queryClassDecls(oid c_id, pStringRef name) const {

    ClassDeclList result;
    std::stringstream query;

    query << "SELECT class.id, class_decl.name, class.name AS className, class_decl.type, class_decl.flags, visibility, defaultVal, " \
             " class_decl.start_line, class_decl.start_col, sourceModule.realPath FROM " \
             " class_model_decl, class_decl, class, sourceModule WHERE sourceModule.id=sourceModule_id AND" \
             " class.id=class_decl.class_id AND class_decl.id=class_model_decl.class_decl_id AND" \
             " class_model_decl.class_id=" << c_id <<
             " AND class_decl.name='" << name.str() << "'";

    //db_->list_query<ClassDeclList>(query.str(), result);
    db_->list_query(query.str(), result);

    return result;

}


pModel::ConstantList pModel::queryConstants(pStringRef name) const {

    pModel::ConstantList result;
    std::stringstream query;

    query << "SELECT name, " \
             " start_line, start_col, sourceModule.realPath FROM " \
             " constant, sourceModule WHERE sourceModule.id=sourceModule_id AND" \
             " type=" << pModel::DEFINE << " AND name='" << name.str() << "'";

    //db_->list_query<pModel::ConstantList>(query.str(), result);
    db_->list_query(query.str(), result);

    return result;

}

std::pair<pModel::oid, std::string> pModel::resolveFQN(oid ns_id, pStringRef name) const {

    // the final resolved namespace to look in
    oid res_ns_id = ns_id;
    // the final resolved sym to look for in res_ns_id
    pStringRef res_name = name;

    // first we may need to resolve name into a namespace or list of namespaces
    // to find the sym in
    size_t p = name.count('\\');
    if (p) {
        // in the case that there is only one and it's in the first position,
        // this is the global namespace
        if (p == 1) {
            if (name.find_first_of('\\') == 0) {
                res_ns_id = getRootNamespaceOID();
                res_name = name.substr(1);
            }
        }
        else {
            // there are multiple namespace sep tokens
            // the sym will be the text from the last one
            res_name = name.substr(name.find_last_of('\\')+1);
            // the namespace specify part
            pStringRef ns_part = name.substr(0, name.find_last_of('\\'));
            if (ns_part.front() == '\\') {
                // if ns_part is absolute, we lookup that namespace
                res_ns_id = getNamespaceOID(ns_part);
                if (res_ns_id == pModel::NULLID) {
                    // namespace not found
                    return std::pair<pModel::oid, std::string>(pModel::NULLID, res_name.str());
                }
            }
            else {
                // otherwise we append it to the current namespace and
                // look that up
                std::string lookup_ns("\\");
                lookup_ns.append(getNamespaceName(ns_id));
                assert(lookup_ns.size() != 1 && "unable to locate ns");
                lookup_ns.push_back('\\');
                lookup_ns.append(ns_part);
                res_ns_id = getNamespaceOID(lookup_ns);
                if (res_ns_id == pModel::NULLID) {
                    // namespace not found
                    return std::pair<pModel::oid, std::string>(pModel::NULLID, res_name.str());
                }
            }
        }

    }

    return std::pair<pModel::oid, std::string>(res_ns_id, res_name.str());

}

pModel::oid pModel::lookupClass(oid ns_id, pStringRef name, pModel::oid m_id) const {

    std::pair<oid, std::string> resolved = resolveFQN(ns_id, name);

    pModel::ClassList cl = queryClasses(resolved.first, resolved.second, m_id);
    if (cl.size() == 0) {
        return pModel::NULLID;
    }
    else if (cl.size() == 1) {
        return cl[0].getID();
    }
    else {
        return pModel::MULTIPLE_IDS;
    }

}

pModel::oid pModel::lookupFunction(oid ns_id, oid c_id, pStringRef name) const {

    std::pair<oid, std::string> resolved = resolveFQN(ns_id, name);

    pModel::FunctionList cl = queryFunctions(resolved.first, c_id, resolved.second);
    if (cl.size() == 0) {
        return pModel::NULLID;
    }
    else if (cl.size() == 1) {
        return cl[0].getID();
    }
    else {
        return pModel::MULTIPLE_IDS;
    }

}

pModel::ClassList pModel::getUnresolvedClasses() const {

    ClassList result;

    // the method to retrieve this right now is to compare the count of
    // extends and implements as recorded by the class declaration against
    // the count of relations we have in the relations table. if there are fewer
    // in the relations table than in the class table, then we have unresolved
    const char *query = "SELECT class.id, namespace_id, name, type, flags, extends, implements, extends_count, implements_count, " \
             " unresolved_extends, unresolved_implements, " \
             " (select count(*) from class_relations where lhs_class_id=class.id and type=0) as resolved_extends_count, " \
             " (select count(*) from class_relations where lhs_class_id=class.id and type=1) as resolved_implements_count, " \
             " start_line, start_col, sourceModule.realPath FROM " \
             " class, sourceModule WHERE sourceModule.id=sourceModule_id AND" \
             " (extends_count > 0 or implements_count > 0) AND " \
             " ((extends_count > resolved_extends_count) or (implements_count > resolved_implements_count))";

    //db_->list_query<ClassList>(query.str(), result);
    db_->list_query(query, result);

    return result;

}

pModel::MultipleDeclList pModel::getMultipleDecls() const {

    MultipleDeclList result;
    db::pDB::RowList db_result;

    const char *query = "SELECT A.*, realPath FROM function_var A, function_var B, "\
            "function, sourceModule WHERE function.id=A.function_id AND "\
            "sourceModule.id=function.sourceModule_id AND "\
            "A.name=B.name and A.function_id=B.function_id AND "\
            "(A.start_line != B.start_line) GROUP BY A.function_id,A.name,A.start_line "\
            "ORDER BY A.start_line";

    db_->list_query(query, db_result);

    if (db_result.size() == 0)
        return result;

    // we put this in a convenient form for the caller
    // a vector of symbols and their duplicate locations, first def first
    model::mMultipleDecl entry;
    for (int i = 0; i < db_result.size(); ++i) {
        if (entry.symbol != db_result[i].get("name")) {
            // finish up the last one
            if (!entry.symbol.empty())
                result.push_back(entry);
            // new entry in result
            entry.symbol = db_result[i].get("name");
            entry.realPath = db_result[i].get("realpath");
            entry.redecl_locs.clear();
        }
        entry.redecl_locs.push_back(model::mMultipleDecl::locData(db_result[i].getAsOID("id"),
                                       pSourceRange(db_result[i].getAsInt("start_line"),
                                                    db_result[i].getAsInt("start_col"))));
    }
    result.push_back(entry);

    return result;

}

void pModel::resolveMultipleDecls() {

    pModel::MultipleDeclList redecl = getMultipleDecls();
    if (redecl.size() == 0)
        return;

    std::stringstream query;

    // update function_var to flag decls which were redeclared
    // so they won't be checked for use
    query << "UPDATE function_var SET is_redecl=1 WHERE id IN (";

    for (int i = 0; i < redecl.size(); ++i) {
        // note the j = 1, meaning only flag the duplicates, not the initial
        assert(redecl[i].redecl_locs.size() > 1 && "redecl had no redecls");
        for (int j = 1; j < redecl[i].redecl_locs.size(); ++j) {
            query << redecl[i].redecl_locs[j].first << ",";
        }
    }
    query << "0)"; // cheat with the 0 to avoid substr
    db_->sql_execute(query.str());

}

pModel::UndeclList pModel::getUndeclaredUses() const {

    UndeclList result;

    const char *query = "SELECT function_var_use.*, realPath FROM function_var_use, "\
            "function, sourceModule WHERE function_var_id IS NULL AND "\
            "function.id=function_id AND sourceModule.id=function.sourceModule_id";

    db_->list_query(query, result);

    return result;

}

pModel::UnusedList pModel::getUnusedDecls() const {

    UnusedList result;

    const char *query = "SELECT function_var.*, realPath FROM function_var LEFT OUTER JOIN "\
            "function_var_use ON function_var.id=function_var_id, " \
            "function, sourceModule WHERE function.id=function_var.function_id AND "\
            "sourceModule.id=function.sourceModule_id AND function_var_use.id IS NULL AND "\
            "is_redecl=0";

    db_->list_query(query, result);

    return result;

}

void pModel::resolveClassRelations() {

    ClassList unresolved = getUnresolvedClasses();

    std::vector<std::string> unresolved_extends, unresolved_implements;

    begin();

    for (int i = 0; i < unresolved.size(); ++i) {

        pModel::oid c_id = unresolved[i].getAsOID("id");

        if (unresolved[i].getAsInt("extends_count") > unresolved[i].getAsInt("resolved_extends_count")) {
            llvm::SmallVector<pStringRef, 32> e_list;
            pStringRef orig(unresolved[i].get("extends"));
            orig.split(e_list, ",", 32);
            for (int j = 0; j < e_list.size(); ++j) {
                pModel::oid resolved_id = lookupClass(unresolved[i].getAsOID("namespace_id"), e_list[j]);
                if (resolved_id != pModel::NULLID) {
                    defineClassRelation(c_id, pModel::EXTENDS, resolved_id);
                    // XXX i think we want to clear the class_model_decl and class_model_function here
                    //     for this class to be sure it's rebuilt properly
                }
                else {
                    unresolved_extends.push_back(e_list[j]);
                }
            }
        }

        if (unresolved[i].getAsInt("implements_count") > unresolved[i].getAsInt("resolved_implements_count")) {
            llvm::SmallVector<pStringRef, 32> i_list;
            pStringRef orig(unresolved[i].get("implements"));
            orig.split(i_list, ",", 32);
            for (int j = 0; j < i_list.size(); ++j) {
                pModel::oid resolved_id = lookupClass(unresolved[i].getAsOID("namespace_id"), i_list[j]);
                if (resolved_id != pModel::NULLID) {
                    // XXX i think we want to clear the class_model_decl and class_model_function here
                    //     for this class to be sure it's rebuilt properly
                    defineClassRelation(c_id, pModel::IMPLEMENTS, resolved_id);
                }
                else {
                    unresolved_implements.push_back(i_list[j]);
                }
            }
        }

        // we save the text version of unresolved classes for the benefit of
        // diagnostics
        if (unresolved_extends.size()) {
            std::stringstream query;
            query << "UPDATE class SET unresolved_extends='";
            if (unresolved_extends.size() > 1) {
                query << join(unresolved_extends);
            }
            else {
                query << unresolved_extends[0];
            }
            query << "' WHERE id=" << c_id;
            db_->sql_execute(query.str());
        }
        if (unresolved_implements.size()) {
            std::stringstream query;
            query << "UPDATE class SET unresolved_implements='";
            if (unresolved_implements.size() > 1) {
                query << join(unresolved_implements);
            }
            else {
                query << unresolved_implements[0];
            }
            query << "' WHERE id=" << c_id;
            db_->sql_execute(query.str());
        }

        unresolved_extends.clear();
        unresolved_implements.clear();

    }

    commit();

}

void pModel::refreshClassModel(pStringRef graphFileName) {

    pClassGraph cmb(db_);
    cmb.build();

    if (!graphFileName.empty()) {
        cmb.writeDot(graphFileName);
    }

}

} // namespace

