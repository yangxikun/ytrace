--TEST--
Test basic method call
--INI--
--SKIPIF--
<?php if (!extension_loaded("ytrace")) print "skip"; ?>
--FILE--
<?php 
$tf = sys_get_temp_dir() . '/'. uniqid('yt', TRUE);
ytrace_enable($tf);
class AAA {
	public $propAAA1;
	protected $propAAA2;
	private $propAAA3;

	public function fooAAA() {
		$this->fooAAA1();
	}
	protected function fooAAA1() {
		$this->fooAAA2();
	}
	private function fooAAA2() {}
}
class AA extends AAA {
	public $propAA1;
	protected $propAA2;
	private $propAA3;

	public function fooAA() {
		$this->fooAA1();
	}
	protected function fooAA1() {
		$this->fooAA2();
	}
	private function fooAA2() {
		$this->fooAAA();
	}
}
class A extends AA {
	public $prop1;
	protected $prop2;
	private $prop3;

	public function foo() {
		$this->foo1();
	}
	protected function foo1() {
		$this->foo2();
	}
	private function foo2() {
		$this->fooAA();
	}
}

$a = new A();
$a->foo();
ytrace_disable();
echo file_get_contents($tf);
unlink($tf);
?>
--EXPECTF--
cli
%sbasic_method_call.php	48	A	1	$a	class A {\n\tpublic $prop1 = NULL;\n\tprotected $prop2 = NULL;\n\tprivate $prop3 = NULL;\n\tpublic $propAA1 = NULL;\n\tprotected $propAA2 = NULL;\n\tprivate ${AA}:propAA3 = NULL;\n\tpublic $propAAA1 = NULL;\n\tprotected $propAAA2 = NULL;\n\tprivate ${AAA}:propAAA3 = NULL\n}
%sbasic_method_call.php	49	F	2	A->foo	0
%sbasic_method_call.php	38	F	3	A->foo1	0
%sbasic_method_call.php	41	F	4	A->foo2	0
%sbasic_method_call.php	44	F	5	AA->fooAA	0
%sbasic_method_call.php	23	F	6	AA->fooAA1	0
%sbasic_method_call.php	26	F	7	AA->fooAA2	0
%sbasic_method_call.php	29	F	8	AAA->fooAAA	0
%sbasic_method_call.php	10	F	9	AAA->fooAAA1	0
%sbasic_method_call.php	13	F	10	AAA->fooAAA2	0
%sbasic_method_call.php	50	F	2	ytrace_disable	0
