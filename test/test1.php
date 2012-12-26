<?php

function foo($one, $two) {
    if (1) return 5;
    echo $hello;
}

function bar($hey, $two=5, $three) {
}

function baz($foo1, $foo2=5, $foo3=10) {
}

$two = 2;
$x = foo(1, "$two");
echo $x;

?>
