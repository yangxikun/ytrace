--TEST--
Test basic static
--INI--
--SKIPIF--
<?php if (!extension_loaded("ytrace")) print "skip"; ?>
--FILE--
<?php 
$tf = sys_get_temp_dir() . '/'. uniqid('yt', TRUE);
ytrace_enable($tf);
class AA {
	static public $propAA;
	static protected $prop1AA;
	static private $prop2AA;

	static public function fooAA() {
		self::$prop1AA = [];
		self::fooAA1();
	}
	static protected function fooAA1() {
		self::$prop2AA = 'AA';
		self::fooAA2();
	}
	static private function fooAA2() {

	}
}

class A extends AA {
	static public $prop;
	static protected $prop1;
	static private $prop2;

	static public function foo() {
		self::$prop1 = [];
		self::foo1();
	}
	static protected function foo1() {
		self::$prop2 = 'A';
		self::foo2();
	}
	static private function foo2() {
		self::$propAA = 123;
		self::fooAA();
	}
}

A::$prop = [];
A::$prop['foo'] = 'bar';
A::foo();
ytrace_disable();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
cli
%sbasic_static.php	41	A	1	A::\'prop\'	array ()
%sbasic_static.php	42	A	1	selfAAA::\'prop\'[\'foo\']	\'bar\'
%sbasic_static.php	43	F	2	A::foo	0
%sbasic_static.php	28	A	2	A::\'prop1\'	array ()
%sbasic_static.php	29	F	3	A::foo1	0
%sbasic_static.php	32	A	3	A::\'prop2\'	\'A\'
%sbasic_static.php	33	F	4	A::foo2	0
%sbasic_static.php	36	A	4	A::\'propAA\'	123
%sbasic_static.php	37	F	5	AA::fooAA	0
%sbasic_static.php	10	A	5	AA::\'prop1AA\'	array ()
%sbasic_static.php	11	F	6	AA::fooAA1	0
%sbasic_static.php	14	A	6	AA::\'prop2AA\'	\'AA\'
%sbasic_static.php	15	F	7	AA::fooAA2	0
%sbasic_static.php	44	F	2	ytrace_disable	0
