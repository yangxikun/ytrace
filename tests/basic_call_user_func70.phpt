--TEST--
Test basic call_user_func
--INI--
--SKIPIF--
<?php
if (!extension_loaded("ytrace")) print "skip";
if (!version_compare(phpversion(), "7.0", '>=')) print "skip < PHP 7.0";
?>
--FILE--
<?php 
$tf = sys_get_temp_dir() . '/'. uniqid('yt', TRUE);
ytrace_enable($tf);
function foo($a, $b) {
	$c = $a;
	$d = $b;
}
class A
{
	public function __call($name, $arguments) {
		call_user_func_array($name, $arguments);
	}
}
$a = new A();
call_user_func([$a, 'foo'], 1, 2);
ytrace_disable();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
cli
%sbasic_call_user_func70.php	14	A	1	$a	class A {}
%sbasic_call_user_func70.php	15	F	2	A->foo	2	$...	1	$...	2
%sbasic_call_user_func70.php	15	F	3	A->__call	2	$name	\'foo\'	$arguments	array (\n\t0 => 1,\n\t1 => 2\n)
%sbasic_call_user_func70.php	11	F	4	foo	2	$a	1	$b	2
%sbasic_call_user_func70.php	5	A	4	$c	1
%sbasic_call_user_func70.php	6	A	4	$d	2
%sbasic_call_user_func70.php	16	F	2	ytrace_disable	0
