<?php

namespace test_other {

class myexception extends \Exception { }

class myclass {
  const FOO = 2;
}

interface iface {
    public function foo();
    public function bar();
}

function foo($one, $two, $three) {
    echo $one, $two, $three;
}

}

namespace test_main {

//const MYSTATIC = 7;
define("MYFIRST", true);
define('MYSECOND', 6);
define('MYEXPR', 2+2); // define with exp in val

// futurediag: redefined constant
define('MYSECOND', 7);

const nsconst = 5;
const nsconst2 = 1, nsconst3 = 2;

$cc1 = nsconst;
$cc2 = \test_main\nsconst2;
// DIAG not defined
$cc3 = \test_main\nsconst4;

class myclass {
  const FOO = 1;

  static public $svar1 = 1;
  static protected $svar2 = 1;
  static private $svar3 = 1;

  public function __construct() {
  }

  public function bar() {
    $hey = 5;
    echo $hey;
    // nodiag on self
    $boo = self::FOO;
    echo $boo;
  }

  static public function bip() {
  }

  static protected function bip2() {
  }

  static private function bip3() {
  }

}

class myextendedclass extends myclass {

  public function yip() {
    // nodiag: from base class
    return self::FOO;
  }

}

interface realiface {
    public function foo();
    public function bar();
}

// DIAG: unknown class "noclass" and "noiface"
class myclass2 extends noclass implements noiface, realiface {
  public function myclass2() { }
  public function foo() { }
  // futurediag: unimplemented interface method bar()
}

// nodiag: extends/implements existing class/interface in another namespace
class myclass3 extends \test_other\myclass implements \test_other\iface { }

function foo($one, $two) {
    if (1) return 5;
    // DIAG: undefined variable $hello
    echo $hello;
    echo $one, $two;
}

function baz($foo1, $foo2=5, $foo3=10) {
  // DIAG: unused variables $foo1, $foo2, $foo3
}

//$two = 2;
// BUG: need to parse double quote so we can get uses
echo "$two";

// futurediag: quoted string
$x = foo(1, "$two");
echo $x;

// DIAG: function not found
nonexist();

// DIAG: bar requires at least one arg
bar();
// nodiag: bar called ok
bar(1); bar(1,2); bar(1,2,3);
// DIAG: bar has too many args
bar(1,2,3,4);

// futurediag: myclass has constructor which requires args
$foo = new myclass();
// futurediag: with type analysis we can say bar needs more args
$foo->bar();

// futurediag: static call to nonexistant
myclass::NONEXIST();

// nodiag: static call to public
myclass::bip();
// futurediag: static call to protected
myclass::bip2();
// futurediag: static call to private
myclass::bip3();

// nodiag: static access to public
$a1 = myclass::$svar1;
// futurediag: static access to protected
$a2 = myclass::$svar2;
// futurediag: static access to private
$a3 = myclass::$svar3;
echo $a1, $a2, $a3;

// nodiag: define()'d constant
$b1 = MYSECOND;
// DIAG undefine()'d constant
$b2 = MYTHIRD;
echo $b1, $b2;

// futurediag: myclass2 has constructor which requires args
$foo2 = new myclass2();
$foo2->bar();

// nodiag: class constant
$foo3 = myclass::FOO;
echo $foo3;

// DIAG nonexistant class constant (class)
$foo4 = myclassne::FOO;
echo $foo4;
// DIAG nonexistant class constant (constant)
$foo45 = myclass::FOO2;
echo $foo45;

// futurediag: using return value from function not returning anything
$foo5 = baz();
echo $foo5;

// DIAG: $three needs default
function bar($hey, $two=5, $three) {
    echo $hey, $two, $three;
}

// nodiag: arr defined, k and v used
$arr = array();
foreach ($arr as $k => $v) {
    echo $k, $v;
}

function returncheck() {
    // no diag: a used in return
    $a = "one";
    $a .= "two";
    return $a;
}

// XXX tmp, wrap in function since we don't do mains yet
function decluse() {
// DIAG: unused variable
$unused = 5;

// DIAG: multiple assignment
$dbl = 1;
$dbl = 5;
$dbl = 9;
echo $dbl;

$baz = 5;
$baz = 10;

// DIAG: $arr2 not declared (i.e. with $arr2 = array())
// nodiag: different keys in array are not multiple assign
$arr2['foo'] = 1;
$arr2['bar'] = 2;

// nodiag
$arr3 = array();
$arr3['foo'] = 1;

// nodiag: param used in array
function check1($param) {
    foo(1, array('key' => $param));
}

$vcheck1 = null;
// nodiag: defining as null will not flag a redeclare
$vcheck1 = 5;
echo $vcheck1;

}

} // namespace


?>
