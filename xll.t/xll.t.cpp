// xll.t.cpp - Console testing program for xll12
#include "../xll/xll.h"
#include <ctime>
#include <random>

//[module(name="xll12")];
using namespace xll;

static std::default_random_engine dre;

void test_sref()
{
	REF12 r1, r2;
	ensure (r1 == r2);
	REF12 r3(1,2,3,4);
	REF12 r4(r3);
	ensure (r3 == r4);
	r2 = r3;
	ensure (r2 == r3);

	OPER12 o1(r2);
	ensure (o1.type() == xltypeSRef);
	ensure (r2 == o1.val.sref.ref);

	OPER12 o2(REF12(1,2,3,4));
	ensure (o1 == o2);
	ensure (o2.val.sref.ref == r2);

	{
		REF12 r(1,2,3,4);
		r.move(1,1);
		ensure (r == REF12(2,3,3,4));
		r.up();
		ensure (r == REF12(1,3,3,4));
		r.right();
		ensure (r == REF12(1,4,3,4));
		r.down();
		ensure (r == REF12(2,4,3,4));
		r.left();
		ensure (r == REF12(2,3,3,4));

		ensure (move(r, 2, 3) == REF12(4, 6, 3, 4));
	}
	{
		// strict-weak ordering
		// !(x < y) && !(y < x) is an equivalence relation
		// x R x, x R y => y R x, !(x R y) and y R z => x R z
		std::uniform_int_distribution<RW> u(1, 10);
		auto R = [](const REF12& x, const REF12& y) {
			return !(x < y) && !(y < x);
		};
		auto ref = [&u]() { return REF12(u(dre),u(dre),u(dre),u(dre)); };
		for (int i = 0; i < 100; ++i) {
			auto x = ref(), y = ref(), z = ref();
			ensure (R(x, x));
			ensure (R(y, y));
			ensure (R(z, z));
			ensure (R(x, y) ? R(y, x) : true);
			ensure ((R(x, y) && R(y, z)) ? R(x, z) : true);
		}
	}
}

void test_swap()
{
	OPER12 str(L"foo");
	OPER12 multi(2,3);
	swap(str, multi);
	ensure (str.xltype == xltypeMulti);
	ensure (str.rows() == 2);
	ensure (str.columns() == 3);
	ensure (multi.xltype == xltypeStr);
	ensure (multi == L"foo");
}

void test_oper()
{
	OPER12 o;
	ensure (o.size() == 1);
	ensure (o.xltype == xltypeMissing);
	OPER12 o2(o);
	ensure (o2.xltype == xltypeMissing);
	ensure (o == o2);
	o = o2;
	ensure (o.xltype == xltypeMissing);
	ensure (o == o2);
	ensure (!(o != o2));
}

void test_num()
{
	OPER12 num(1.23);
	ensure (num.xltype == xltypeNum);
	ensure (num.val.num == 1.23);
	ensure (num == 1.23); // operator double
	ensure (1.23 == num);
	ensure (num);
	num = 0.; // calls operator=(double)
	ensure (num.xltype == xltypeNum);
	ensure (num.val.num == 0);
	ensure (num == 0);
	ensure (!num);

	OPER12 num2(num);
	ensure (num == num2);
	num = num2;
	ensure (num == num2);
}

void test_str()
{
//	[cpp_quote(__FUNCDNAME__)];
//	[idl_quote(__FUNCTION__)];
	OPER12 str(L"str");
	ensure (str.xltype == xltypeStr);
	ensure (str.val.str[0] == 3);
	ensure (0 == wcsncmp(str.val.str + 1, L"str", 3));
	ensure (str == L"str");
	OPER12 str2(str);
	ensure (str2.xltype == xltypeStr);
	ensure (str2.val.str[0] == 3);
	ensure (0 == wcsncmp(str2.val.str + 1, L"str", 3));
	ensure (str == str2);
	str = str2;
	ensure (str == str2);

	str = L"foobar";
	ensure (str.xltype == xltypeStr);
	ensure (str == L"foobar");

	OPER12 o;
	o = L"foo";
	ensure (o == L"foo");
	ensure (o != L"bar");
	o &= L"bar";
	ensure (o.val.str[0] == 6);
	ensure (o == L"foobar");

	const XCHAR* null = 0;
	OPER12 Null(null);
	ensure (Null.xltype == xltypeMissing);
	OPER12 Empty(L"");
	ensure (Empty.xltype == xltypeStr);
	ensure (Empty.val.str[0] == 0);
	Empty &= L"a";
	ensure (Empty.val.str[0] == 1);
	ensure (Empty.val.str[1] == L'a');

	OPER12 o2;
	o2 = L"foo";
	ensure (o2 == L"foo");
	o2 &= L"bar";
	ensure (o2 == L"foobar")
	{
		auto u = std::uniform_int_distribution<int>{1, std::numeric_limits<XCHAR>::max()/2};
		for (int i = 0; i < 100; ++i) {
			size_t len = u(dre);
			std::wstring s3;
			s3.resize(len);
			std::generate_n(begin(s3), len, [&]() { return static_cast<XCHAR>(u(dre)); });
			OPER12 o3(s3);
			o3 &= s3;
			ensure (o3.val.str[0] == 2*len);
		}
	}
}
void test_bool()
{
	OPER12 b(true);
	ensure (b.xltype == xltypeBool);
	ensure (b.val.xbool);
	ensure (b.val.xbool == 1);
	ensure (b.val.xbool == TRUE);
	ensure (b);
}
void test_multi()
{
	OPER12 multi(2,3);
	ensure (multi.xltype == xltypeMulti);
	ensure (multi.rows() == multi.val.array.rows);
	ensure (multi.columns() == multi.val.array.columns);
	ensure (multi[0] == OPER12());

	multi[0] = 1.23;
	multi[1] = L"str";

	int i = 0;
	for (auto& m : multi)
		ensure (m == multi[i++]);

	multi.push_back(multi);
	ensure (multi.xltype == xltypeMulti);
	ensure (multi.rows() == 4);
	ensure (multi.columns() == 3);
/*
	bool thrown = false;
	try {
		multi.push_back(OPER12(1.23));
	}
	catch (...) {
		thrown = true;
	}
	ensure (thrown);
*/
	OPER12 m0;
	m0.push_back(OPER12(L"foo"));
	ensure (m0.xltype == xltypeMulti);
	ensure (m0.rows() == 1);
	ensure (m0.columns() == 2);
	ensure (m0[0].xltype == xltypeMissing);
	ensure (m0[1] == L"foo");

	std::uniform_int_distribution<RW> u(1, 100);
	for (int j = 0; j < 100; ++j) {
		OPER12 a(u(dre), u(dre)), b(u(dre), u(dre));
		if (a.columns() == b.columns()) {
			if (a.rows() == 1 && b.rows() != 1)
				b.resize(1, b.size());
			a.push_back(b);
		}
	}

	{
		OPER12 o{OPER12(1.23), OPER12(L"foo")};
		ensure (o.xltype == xltypeMulti);
		ensure (o.size() == 2);
		ensure (o[0] == 1.23);
		ensure (o[1] == L"foo");
	}
	{
		OPER12 o{{OPER12(1.23)},{OPER12(L"foo"), OPER12(true)}};
		ensure (o.xltype == xltypeMulti);
		ensure (o.rows() == 2);
		ensure (o.columns() == 2);
		ensure (o(0,0) == 1.23);
		ensure (o(0,1) == OPER12());
		ensure (o(1,0) == L"foo");
		ensure (o(1,1) == true);
	}
}
void test_handle()
{
	std::unique_ptr<int> pi{new int()};
	HANDLEX h = p2h<int>(pi.get());
	ensure (pi.get() == h2p<int>(h));
}

void test_arity()
{
	Args args(XLL_DOUBLE, L"?proc", L"Proc");
	ensure (args.Arity() == 0);
	args.Num(L"x", L"is an x");
	ensure (args.Arity() == 1);
	args.Arg(XLL_CSTRING, L"s", L"is a string");
	ensure (args.Arity() == 2);
	args.Uncalced();
	ensure (args.Arity() == 2);
	args.Volatile();
	ensure (args.Arity() == 2);
}

void test_fp()
{
	xll::FP12 f0;
	ensure (f0.is_empty());
	f0.resize(2,3);
	ensure (f0.size() == 6);
	ensure (f0.rows() == 2);
	ensure (f0.columns() == 3);
	f0(1,2) = 3;
	ensure (f0[5] == 3);
}

int main()
{
	dre.seed(static_cast<unsigned>(::time(0)));

	test_sref();
	test_swap();
	test_oper();
	test_num();
	test_str();
	test_bool();
	test_multi();
	test_handle();
	test_arity();
	test_fp();

	return 0;
}