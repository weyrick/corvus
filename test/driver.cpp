/* ***** BEGIN LICENSE BLOCK *****
;;
;; Copyright (c) 2013 Shannon Weyrick <weyrick@mozek.us>
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this
;; file, You can obtain one at http://mozilla.org/MPL/2.0/.
   ***** END LICENSE BLOCK *****
*/

#include <exception>
#include <iostream>
#include <string>

#include "corvus/pSourceManager.h"
#include "corvus/pConfig.h"
#include "corvus/pDiagnostic.h"
#include "corvus/pModel.h"
#include <llvm/Support/FileSystem.h>
#include <sstream>

using namespace llvm;
using namespace corvus;

#define ASSERT(A,B) cassert(A,B,__LINE__)
#define ASSERT_NOT(A,B) cassert(A,B,__LINE__,true)

void cassert(int a, int b, int line, bool negate=false) {
    bool comp = (negate) ? (a == b) : (a != b);
    if (comp) {
        std::cout << line << ": " << a << " != " << b << std::endl;
        exit(1);
    }
}

void cassert(pStringRef a, pStringRef b, int line, bool negate=false) {
    bool comp = (negate) ? (a.equals(b)) : (!a.equals(b));
    if (comp) {
        std::cout << line << ": "<< "[" << a.str() << "] != [" << b.str() << "]" << std::endl;
        exit(1);
    }
}

void cassert(pSourceLoc a, pSourceLoc b, int line, bool negate=false) {
    bool comp = (negate) ? (a == b) : (a != b);
    if (comp) {
        std::cout << line << ": "<< "[" << a.toString() << "] != [" << b.toString() << "]" << std::endl;
        exit(1);
    }
}

void cassert(pSourceRange a, pSourceRange b, int line, bool negate=false) {
    bool comp = (negate) ? (a == b) : (a != b);
    if (comp) {
        std::stringstream outa, outb;
        outa << a.startLine << ":" << a.startCol << ":" <<
                     a.endLine << ":" << a.endCol;
        outb << b.startLine << ":" << b.startCol << ":" <<
                     b.endLine << ":" << b.endCol;
        std::cout << line << ": ""[" << outa.str() << "] != [" << outb.str() << "]" << std::endl;
        exit(1);
    }
}

int main( int argc, char* argv[] )
{

    pSourceManager sm;
    pConfig config;
    std::vector<std::string> inputFiles;

    //sm.setDebug(0,false,true);
    config.exts = "php";

    // try to read home directory config file
    char *home = getenv("HOME");
    if (home) {
        // will ignore if not found
        pConfigMgr::read(pStringRef(home)+"/.corvus", config);
    }

    if (!config.includePaths.empty()) {
        for (unsigned i = 0; i != config.includePaths.size(); ++i) {
            sm.addIncludeDir(config.includePaths[i], config.exts);
        }
    }

    sm.setModelDBName("test.db");
    inputFiles.push_back("test1.php");

    for (unsigned i = 0; i != inputFiles.size(); ++i) {

        sys::fs::file_status stat;
        sys::fs::status(inputFiles[i], stat);
        if (sys::fs::is_directory(stat))
            sm.addSourceDir(inputFiles[i], config.exts);
        else if (sys::fs::is_regular_file(stat))
            sm.addSourceFile(inputFiles[i]);
        else
            std::cerr << "skipping unknown path: " << inputFiles[i] << std::endl;

    }

    try {
        sm.refreshModel();
        sm.runDiagnostics();
    }
    catch (std::exception& e) {
        std::cout << "exception: " << e.what() << "\n";
        exit(1);
    }

    pSourceManager::DiagModuleListType mList = sm.getDiagModules();

    // 1 source module
    cassert(mList.size(), 1, __LINE__);

    pSourceModule::DiagListType dList = mList[0]->getDiagnostics();

    // DIAG COUNT
    cassert(dList.size(), 22, __LINE__);

    // DIAGS

    int i = 0;


    ASSERT(dList[i]->msg(), "class myclass2 extends noclass which is unresolved");

    i++;
    ASSERT(dList[i]->msg(), "class myclass2 implements noiface which is unresolved");

    i++;
    ASSERT(dList[i]->msg(), "$hello used but not defined");

    i++;
    ASSERT(dList[i]->msg(), "$foo1 unused");
    i++;
    ASSERT(dList[i]->msg(), "$foo2 unused");
    i++;
    ASSERT(dList[i]->msg(), "$foo3 unused");

    i++;
    ASSERT(dList[i]->msg(), "function 'nonexist' not defined");
    //ASSERT(dList[i]->location().range(), pSourceRange(57,1,57,0)); // XXX

    i++;
    ASSERT(dList[i]->msg(), "wrong number of arguments: function 'bar' takes between 1 and 3 arguments (0 specified)");
    //ASSERT(dList[i]->location().range(), pSourceRange(60,1,60,0)); // XXX

    i++;
    ASSERT(dList[i]->msg(), "wrong number of arguments: function 'bar' takes between 1 and 3 arguments (4 specified)");
    //ASSERT(dList[i]->location().range(), pSourceRange(64,1,64,0)); // XXX

    i++;
    ASSERT(dList[i]->msg(), "undefined constant: MYTHIRD");
    //ASSERT(dList[i]->location().range(), pSourceRange(91,0,91,0)); // XXX

    i++;
    ASSERT(dList[i]->msg(), "class constant from undefined class: myclassne");

    i++;
    ASSERT(dList[i]->msg(), "undefined class constant: myclass::FOO2");

    i++;
    ASSERT(dList[i]->msg(), "$unused unused");

    i++;
    ASSERT(dList[i]->msg(), "$two should have explicit default value (currently implicit NULL)");
    //ASSERT(dList[i]->location().range(), pSourceRange(109,28,109,34));

    i++;
    ASSERT(dList[i]->msg(), "$dbl first declared here");

    i++;
    ASSERT(dList[i]->msg(), "$dbl redeclared");

    i++;
    ASSERT(dList[i]->msg(), "$dbl redeclared");

    i++;
    ASSERT(dList[i]->msg(), "$baz first declared here");

    i++;
    ASSERT(dList[i]->msg(), "$baz unused");

    i++;
    ASSERT(dList[i]->msg(), "$baz redeclared");

    i++;
    ASSERT(dList[i]->msg(), "$arr2 used but not defined");
    i++;
    ASSERT(dList[i]->msg(), "$arr2 used but not defined");


    // MODEL QUERIES
    const pModel *m = sm.model();

    // namespaces
    pModel::oid main_ns = m->getNamespaceOID("\\test_main");
    pModel::oid other_ns = m->getNamespaceOID("\\test_other");
    ASSERT_NOT(main_ns, pModel::NULLID);
    ASSERT_NOT(other_ns, pModel::NULLID);

    // functions
    pModel::FunctionList f;
    f = m->queryFunctions(main_ns, pModel::NULLID, "foo");
    ASSERT(f.size(), 1);
    f = m->queryFunctions(other_ns, pModel::NULLID, "foo");
    ASSERT(f.size(), 1);

    // classes
    pModel::ClassList c;
    // \test_main\myclass
    c = m->queryClasses(main_ns, "myclass");
    ASSERT(c.size(), 1);

    // constants
    pModel::ConstantList cn;
    // define('MYFIRST' .. )
    cn = m->queryConstants("MYFIRST");
    ASSERT(cn.size(), 1);

    // class constants
    pModel::ClassDeclList cdl;
    // \test_main\myclass::FOO
    cdl = m->queryClassDecls(c[0].getID(), "FOO");
    ASSERT(cdl.size(), 1);

    std::cout << "all tests passing" << std::endl;
    return 0;

}
