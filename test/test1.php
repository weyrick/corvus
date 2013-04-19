<?php

namespace myns;

//const MYSTATIC = 7;
define('MYSECOND', 6);

class myclass {
  const FOO = 1;
  public function bar($one) {
    $hey = 5;
    echo $baz;
  }
}

function foo($one, $two) {
    if (1) return 5;
    echo $hello;
}

// DIAG: $three needs default
function bar($hey, $two=5, $three) {
}

function baz($foo1, $foo2=5, $foo3=10) {
}

$two = 2;
$x = foo(1, "$two");
echo $x;

// DIAG: function not found
nonexist();

// DIAG: bar requires at least one arg
bar();


?>
