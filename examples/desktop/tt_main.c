
#include "tinytest.h"

TINYTEST_DECLARE_MODULE("tt_main.cpp");

void testDiag() {
	ttDiagnostic("Int: %d, %d, %08x", 123, -456, 0xff);
}
void testAssertOk() {
	TT_ASSERT(true);
}
void testAssertFail() {
	TT_ASSERT(false);
}
void testIgnore() {
	TT_IGNORE();
	TT_ASSERT(false);
}

void testFailExplicit() {
	TT_FAIL("Explicit failure.");
}

void dumper() {
	ttDiagnostic("Dump function.");
}
void testDump() {
	TT_ASSERT(false);
}

void setup() { ttDiagnostic("In setup()."); }
void teardown() { ttDiagnostic("In teardown()."); }
void testFixtureOk() { TT_ASSERT(true); }
void testFixtureFail() { TT_ASSERT(false); }
void testFixtureIgnore() { TT_IGNORE(); TT_ASSERT(false); }	

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
	ttSetDump(dumper);
	TT_TEST_SIMPLE(testDump);
	ttSetDump(NULL);
	
	ttRegisterFixture(setup, teardown);
		TT_TEST_SIMPLE(testFixtureOk);
		TT_TEST_SIMPLE(testFixtureFail);
		TT_TEST_SIMPLE(testFixtureIgnore);
	ttRegisterFixture(NULL, NULL);

	TT_TEST_SIMPLE(testAssertEqOk);
	TT_TEST_SIMPLE(testAssertIntFail1);
	TT_TEST_SIMPLE(testAssertIntFail2);
	TT_TEST_SIMPLE(testAssertHexFail);
	TT_TEST_SIMPLE(testAssertStrFail);
}

int main(int argc, char* argv[]) {	
	return ttMain(argc, argv);
}

