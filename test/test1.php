<?php

namespace myns;

//const MYSTATIC = 7;
define('MYSECOND', 6);
// futurediag: redefined constant
define('MYSECOND', 7);

class myclass {
  const FOO = 1;

  static public $svar1 = 1;
  static protected $svar2 = 1;
  static private $svar3 = 1;

  public function __construct($one) {
  }

  public function bar($one) {
    $hey = 5;
    echo $baz;
  }

  static public function bip($one) {
  }

  static protected function bip2($one) {
  }

  static private function bip3($one) {
  }

}

// futurediag: unknown class "noclass" and "noiface"
class myclass2 extends noclass implements noiface {
  public function myclass2($one) { }
}

function foo($one, $two) {
    if (1) return 5;
    // futurediag: undefined variable $hello
    echo $hello;
}

// DIAG: $three needs default
function bar($hey, $two=5, $three) {
}

function baz($foo1, $foo2=5, $foo3=10) {
  // futurediag: unused variables $foo1, $foo2, $foo3
}

$two = 2;
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

// nodiag: define()'d constant
$b1 = MYSECOND;
// futurediag: undefine()'d constant
$b2 = MYTHIRD;

// futurediag: myclass2 has constructor which requires args
$foo2 = new myclass2();

// nodiag: class constant
$foo3 = myclass::FOO;

// futurediag: nonexistant class constant
$foo4 = myclass::FOO2;

// futurediag: using return value from function not returning anything
$foo5 = baz();

// futurediag: unused variable $f
$unused = 5;

?>
