
#include "tinytest.h"

TT_DECLARE_MODULE("tt_main.cpp");

void testDiag() {
	ttDiagnostic("Int: %d, %d, %08x", 123, -456, 0xff);
}
void testAssertOk() {
	TT_ASSERT(1);
}
void testAssertFail() {
	TT_ASSERT(0);
}
void testIgnore() {
	TT_IGNORE();
	TT_ASSERT(0);
}

void testFailExplicit() {
	TT_FAIL("Explicit failure.");
}

void dumper() {
	ttDiagnostic("Dump function.");
}
void testDump() {
	TT_ASSERT(0);
}

void setup() { ttDiagnostic("In setup()."); }
void teardown() { ttDiagnostic("In teardown()."); }
void testFixtureOk() { TT_ASSERT(1); }
void testFixtureFail() { TT_ASSERT(0); }
void testFixtureIgnore() { TT_IGNORE(); TT_ASSERT(0); }	

void testAssertEqOk() { 
	TT_ASSERT_INT(2147483647, 2147483647); 
	TT_ASSERT_INT(-2147483648, -2147483648); 
	TT_ASSERT_INT_HEX(0x82345678, 0x82345678); 
	TT_ASSERT_STR("zzz", "zzz");
}
void testAssertIntFail1() { 
	TT_ASSERT_INT(2147483647, -2147483648); 
}
void testAssertIntFail2() { 
	TT_ASSERT_INT(-2147483648, 2147483647); 
}
void testAssertHexFail() { 
	TT_ASSERT_INT_HEX(0x12345678, 0x87654321); 
}
void testAssertStrFail() { 
	TT_ASSERT_STR("zzz", "aaa"); 
}

void ttRunTests(void) {
	TT_TEST_SIMPLE(testDiag);
	TT_TEST_SIMPLE(testAssertOk);
	TT_TEST_SIMPLE(testAssertFail);
	TT_TEST_SIMPLE(testIgnore);
	TT_TEST_SIMPLE(testFailExplicit);
	ttRegisterFixture(NULL, dumper, NULL);
	TT_TEST_SIMPLE(testDump);
	ttUnregisterFixture();
	
	ttRegisterFixture(setup, NULL, teardown);
		TT_TEST_SIMPLE(testFixtureOk);
		TT_TEST_SIMPLE(testFixtureFail);
		TT_TEST_SIMPLE(testFixtureIgnore);
	ttUnregisterFixture();

	TT_TEST_SIMPLE(testAssertEqOk);
	TT_TEST_SIMPLE(testAssertIntFail1);
	TT_TEST_SIMPLE(testAssertIntFail2);
	TT_TEST_SIMPLE(testAssertHexFail);
	TT_TEST_SIMPLE(testAssertStrFail);
}

int main(int argc, char* argv[]) {	
	return ttMain(argc, argv);
}

