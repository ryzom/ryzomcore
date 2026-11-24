// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <gtest/gtest.h>

#include <algorithm>
#include <string>

#include <nel/3d/glsl_block_struct_reads.h>

namespace {

// The declarations the tests share: a struct, and a uniform block with an array of it
const char *const s_LightBlock = "struct Light { vec3 pos; int mode; vec4 color; };\n"
                                 "layout(std140) uniform Lights { Light lights[16]; };\n";
const char *const s_LightAccessor = " Light nlBlockRead_lights(int nlBlockIndex0) { return Light(lights[nlBlockIndex0].pos, lights[nlBlockIndex0].mode, lights[nlBlockIndex0].color); }";

std::string withAccessor(const std::string &body)
{
	return std::string("struct Light { vec3 pos; int mode; vec4 color; };\n"
	                   "layout(std140) uniform Lights { Light lights[16]; };")
	    + s_LightAccessor + "\n" + body;
}

size_t lineCount(const std::string &text)
{
	return (size_t)std::count(text.begin(), text.end(), '\n');
}

} // namespace

class CUT3DGlslBlockStructReads : public testing::Test
{
protected:
	// the rewrite of a source that is rewritten without a message
	std::string rewritten(const std::string &source)
	{
		std::string result, message;
		EXPECT_TRUE(NL3D::rewriteGlslBlockStructReads(source, result, message)) << message;
		EXPECT_TRUE(message.empty());
		EXPECT_EQ(lineCount(source), lineCount(result));
		return result;
	}

	// the message of a source that is left as it is
	std::string refused(const std::string &source)
	{
		std::string result, message;
		EXPECT_FALSE(NL3D::rewriteGlslBlockStructReads(source, result, message)) << source;
		EXPECT_FALSE(message.empty());
		EXPECT_EQ(source, result);
		return message;
	}

	bool has(const std::string &text, const char *part)
	{
		return text.find(part) != std::string::npos;
	}
};

TEST_F(CUT3DGlslBlockStructReads, nothingToRewrite)
{
	const std::string plain = "#version 300 es\nprecision highp float;\nuniform vec4 color;\nout vec4 o;\nvoid main() { o = color; }\n";
	EXPECT_EQ(plain, rewritten(plain));

	// a struct that is in no uniform block: a local, and a uniform of its own
	const std::string local = "struct S { float a; };\nuniform S one;\nvoid main() { S s = S(1.0); S t = s; S u = one; gl_Position = vec4(t.a + u.a); }\n";
	EXPECT_EQ(local, rewritten(local));

	// reads of members only
	const std::string members = std::string(s_LightBlock) + "void main() { int i = 2; gl_Position = lights[i].color + vec4(lights[i + 1].pos, 1.0); }\n";
	EXPECT_EQ(members, rewritten(members));

	EXPECT_EQ(std::string(), rewritten(std::string()));
}

TEST_F(CUT3DGlslBlockStructReads, wholeStructRead)
{
	const std::string body = "void main() {\n"
	                         "  int idx = 3;\n"
	                         "  Light li = lights[idx];\n"
	                         "  gl_Position = li.color;\n"
	                         "}\n";
	const std::string expected = "void main() {\n"
	                             "  int idx = 3;\n"
	                             "  Light li = nlBlockRead_lights(int(idx));\n"
	                             "  gl_Position = li.color;\n"
	                             "}\n";
	EXPECT_EQ(withAccessor(expected), rewritten(std::string(s_LightBlock) + body));
}

TEST_F(CUT3DGlslBlockStructReads, everyKindOfUse)
{
	// an argument, both sides of a comparison, a return value, an assignment, an index that is an expression
	const std::string body = "float f(Light l) { return l.color.x; }\n"
	                         "Light g(uint u) { return lights[u]; }\n"
	                         "void main() {\n"
	                         "  Light a;\n"
	                         "  a = lights[ 2 * 3 + 1 ];\n"
	                         "  bool same = lights[0] == lights[1];\n"
	                         "  gl_Position = vec4(f(lights[4]), same ? 1.0 : 0.0, a.pos.x, lights[5].color.w);\n"
	                         "}\n";
	const std::string expected = "float f(Light l) { return l.color.x; }\n"
	                             "Light g(uint u) { return nlBlockRead_lights(int(u)); }\n"
	                             "void main() {\n"
	                             "  Light a;\n"
	                             "  a = nlBlockRead_lights(int(2 * 3 + 1));\n"
	                             "  bool same = nlBlockRead_lights(int(0)) == nlBlockRead_lights(int(1));\n"
	                             "  gl_Position = vec4(f(nlBlockRead_lights(int(4))), same ? 1.0 : 0.0, a.pos.x, lights[5].color.w);\n"
	                             "}\n";
	EXPECT_EQ(withAccessor(expected), rewritten(std::string(s_LightBlock) + body));
}

TEST_F(CUT3DGlslBlockStructReads, readInsideAnIndex)
{
	const std::string body = "void main() { Light a = lights[lights[1].mode]; Light b = lights[f(lights[2])]; }\n";
	const std::string expected = "void main() { Light a = nlBlockRead_lights(int(lights[1].mode)); Light b = nlBlockRead_lights(int(f(nlBlockRead_lights(int(2))))); }\n";
	EXPECT_EQ(withAccessor(expected), rewritten(std::string(s_LightBlock) + body));

	// an index with a comma in it is one argument
	const std::string comma = "void main() { int i = 0; int j = 1; Light a = lights[i++, j]; Light b = lights[max(i, j)]; }\n";
	const std::string commaExpected = "void main() { int i = 0; int j = 1; Light a = nlBlockRead_lights(int((i++, j))); Light b = nlBlockRead_lights(int(max(i, j))); }\n";
	EXPECT_EQ(withAccessor(commaExpected), rewritten(std::string(s_LightBlock) + comma));
}

TEST_F(CUT3DGlslBlockStructReads, readOverSeveralLines)
{
	// the lines keep their numbers
	const std::string body = "void main() {\n"
	                         "  Light a = lights // the table\n"
	                         "    [\n"
	                         "      1 +\n"
	                         "      2\n"
	                         "    ];\n"
	                         "  gl_Position = a.color;\n"
	                         "}\n";
	const std::string expected = "void main() {\n"
	                             "  Light a = nlBlockRead_lights(int(1 +\n"
	                             "      2))\n"
	                             "\n"
	                             "\n"
	                             ";\n"
	                             "  gl_Position = a.color;\n"
	                             "}\n";
	EXPECT_EQ(withAccessor(expected), rewritten(std::string(s_LightBlock) + body));

	// a preprocessor line inside a read
	const std::string message = refused(std::string(s_LightBlock) + "void main() {\n  Light a = lights[\n#ifdef FIRST\n    0\n#else\n    1\n#endif\n  ];\n}\n");
	EXPECT_TRUE(has(message, "line 4"));
}

TEST_F(CUT3DGlslBlockStructReads, commentsAndFieldsOfOtherStructs)
{
	const std::string body = "struct Other { Light lights; };\n"
	                         "// Light li = lights[idx];\n"
	                         "/* lights[1] */\n"
	                         "void main() { Other o; Light a = o.lights; gl_Position = a.color; }\n";
	EXPECT_EQ(std::string(s_LightBlock) + body, rewritten(std::string(s_LightBlock) + body));
}

TEST_F(CUT3DGlslBlockStructReads, instanceName)
{
	const std::string source = "struct S { vec4 a; float b; };\n"
	                           "uniform Block { S items[4]; S one; float scale; } blk;\n"
	                           "void main() { S x = blk.items[1]; S y = blk.one; gl_Position = x.a * y.b * blk.scale + blk.items[2].a; }\n";
	const std::string expected = "struct S { vec4 a; float b; };\n"
	                             "uniform Block { S items[4]; S one; float scale; } blk;"
	                             " S nlBlockRead_blk_items(int nlBlockIndex0) { return S(blk.items[nlBlockIndex0].a, blk.items[nlBlockIndex0].b); }"
	                             " S nlBlockRead_blk_one() { return S(blk.one.a, blk.one.b); }\n"
	                             "void main() { S x = nlBlockRead_blk_items(int(1)); S y = nlBlockRead_blk_one(); gl_Position = x.a * y.b * blk.scale + blk.items[2].a; }\n";
	EXPECT_EQ(expected, rewritten(source));
}

TEST_F(CUT3DGlslBlockStructReads, nestedStructs)
{
	const std::string source = "struct Inner { vec2 p; float w[2]; };\n"
	                           "struct Outer { Inner one; Inner two[2]; mat4 m; };\n"
	                           "uniform Block { layout(row_major) mat4 first; highp vec4 second; Outer[8] outers; };\n"
	                           "void main() {\n"
	                           "  Outer o = outers[1];\n"
	                           "  Inner i = outers[2].one;\n"
	                           "  Inner j = outers[3].two[1];\n"
	                           "  gl_Position = o.m * first * vec4(i.p, j.w[0], outers[4].two[0].w[1]) + second;\n"
	                           "}\n";
	const std::string expected = "struct Inner { vec2 p; float w[2]; };\n"
	                             "struct Outer { Inner one; Inner two[2]; mat4 m; };\n"
	                             "uniform Block { layout(row_major) mat4 first; highp vec4 second; Outer[8] outers; };"
	                             " Outer nlBlockRead_outers(int nlBlockIndex0) { return Outer(Inner(outers[nlBlockIndex0].one.p, outers[nlBlockIndex0].one.w), Inner[2](Inner(outers[nlBlockIndex0].two[0].p, outers[nlBlockIndex0].two[0].w), Inner(outers[nlBlockIndex0].two[1].p, outers[nlBlockIndex0].two[1].w)), outers[nlBlockIndex0].m); }"
	                             " Inner nlBlockRead_outers_one(int nlBlockIndex0) { return Inner(outers[nlBlockIndex0].one.p, outers[nlBlockIndex0].one.w); }"
	                             " Inner nlBlockRead_outers_two(int nlBlockIndex0, int nlBlockIndex1) { return Inner(outers[nlBlockIndex0].two[nlBlockIndex1].p, outers[nlBlockIndex0].two[nlBlockIndex1].w); }\n"
	                             "void main() {\n"
	                             "  Outer o = nlBlockRead_outers(int(1));\n"
	                             "  Inner i = nlBlockRead_outers_one(int(2));\n"
	                             "  Inner j = nlBlockRead_outers_two(int(3), int(1));\n"
	                             "  gl_Position = o.m * first * vec4(i.p, j.w[0], outers[4].two[0].w[1]) + second;\n"
	                             "}\n";
	EXPECT_EQ(expected, rewritten(source));
}

TEST_F(CUT3DGlslBlockStructReads, severalBlocksAndDirectives)
{
	const std::string source = "#version 300 es\r\n"
	                           "precision highp float;\r\n"
	                           "struct A { vec4 v; };\r\n"
	                           "struct B { ivec4 i; };\r\n"
	                           "#define TWICE(x) \\\r\n"
	                           "    ((x) + (x))\r\n"
	                           "layout(std140) uniform First {\r\n"
	                           "    A as[2];\r\n"
	                           "    vec4 plain;\r\n"
	                           "};\r\n"
	                           "#ifdef SOMETHING\r\n"
	                           "uniform vec4 other;\r\n"
	                           "#endif\r\n"
	                           "layout(std140) uniform Second {\r\n"
	                           "    B bs[60];\r\n"
	                           "};\r\n"
	                           "out vec4 o;\r\n"
	                           "void main() { A a = as[1]; B b = bs[int(a.v.x)]; o = TWICE(a.v) + vec4(b.i) + plain; }\r\n";
	const std::string expected = "#version 300 es\r\n"
	                             "precision highp float;\r\n"
	                             "struct A { vec4 v; };\r\n"
	                             "struct B { ivec4 i; };\r\n"
	                             "#define TWICE(x) \\\r\n"
	                             "    ((x) + (x))\r\n"
	                             "layout(std140) uniform First {\r\n"
	                             "    A as[2];\r\n"
	                             "    vec4 plain;\r\n"
	                             "}; A nlBlockRead_as(int nlBlockIndex0) { return A(as[nlBlockIndex0].v); }\r\n"
	                             "#ifdef SOMETHING\r\n"
	                             "uniform vec4 other;\r\n"
	                             "#endif\r\n"
	                             "layout(std140) uniform Second {\r\n"
	                             "    B bs[60];\r\n"
	                             "}; B nlBlockRead_bs(int nlBlockIndex0) { return B(bs[nlBlockIndex0].i); }\r\n"
	                             "out vec4 o;\r\n"
	                             "void main() { A a = nlBlockRead_as(int(1)); B b = nlBlockRead_bs(int(int(a.v.x))); o = TWICE(a.v) + vec4(b.i) + plain; }\r\n";
	EXPECT_EQ(expected, rewritten(source));

	// line ends of a carriage return alone
	std::string classic = std::string(s_LightBlock) + "// a comment\n#define ONE 1\nvoid main() { Light l = lights[ONE]; }\n";
	std::string classicExpected = withAccessor("// a comment\n#define ONE 1\nvoid main() { Light l = nlBlockRead_lights(int(ONE)); }\n");
	std::replace(classic.begin(), classic.end(), '\n', '\r');
	std::replace(classicExpected.begin(), classicExpected.end(), '\n', '\r');
	std::string result, message;
	EXPECT_TRUE(NL3D::rewriteGlslBlockStructReads(classic, result, message)) << message;
	EXPECT_EQ(classicExpected, result);
}

TEST_F(CUT3DGlslBlockStructReads, namesThatAreAdded)
{
	// two paths with one name, and names with underscores at their ends: no name has two in a row
	const std::string source = "struct Inner { vec4 v; };\n"
	                           "struct Outer { Inner x; Inner _y_; };\n"
	                           "uniform Block { Outer light; Inner light_x; Inner _pad__; };\n"
	                           "void main() { Inner a = light.x; Inner b = light_x; Inner c = light._y_; Inner d = _pad__; Inner e = light_x; }\n";
	const std::string expected = "struct Inner { vec4 v; };\n"
	                             "struct Outer { Inner x; Inner _y_; };\n"
	                             "uniform Block { Outer light; Inner light_x; Inner _pad__; };"
	                             " Inner nlBlockRead_light_x() { return Inner(light.x.v); }"
	                             " Inner nlBlockRead_light_x_2() { return Inner(light_x.v); }"
	                             " Inner nlBlockRead_light_y() { return Inner(light._y_.v); }"
	                             " Inner nlBlockRead_pad() { return Inner(_pad__.v); }\n"
	                             "void main() { Inner a = nlBlockRead_light_x(); Inner b = nlBlockRead_light_x_2(); Inner c = nlBlockRead_light_y(); Inner d = nlBlockRead_pad(); Inner e = nlBlockRead_light_x_2(); }\n";
	EXPECT_EQ(expected, rewritten(source));

	// a source that has such a name itself, as a name or in a preprocessor line
	EXPECT_TRUE(has(refused(std::string(s_LightBlock) + "float nlBlockRead_lights;\nvoid main() { Light l = lights[0]; }\n"), "nlBlockRead"));
	refused(std::string(s_LightBlock) + "#define nlBlockIndex0 4\nvoid main() { Light l = lights[0]; }\n");
	// which does not matter when nothing is added
	const std::string unused = std::string(s_LightBlock) + "float nlBlockRead_lights;\nvoid main() { gl_Position = lights[0].color; }\n";
	EXPECT_EQ(unused, rewritten(unused));
}

TEST_F(CUT3DGlslBlockStructReads, arrayLength)
{
	const std::string body = "void main() { int n = lights.length(); gl_Position = vec4(float(n)); }\n";
	EXPECT_EQ(std::string(s_LightBlock) + body, rewritten(std::string(s_LightBlock) + body));
}

TEST_F(CUT3DGlslBlockStructReads, nameDeclaredAgain)
{
	// a local variable, a later name of a declaration, a parameter: the reads cannot be told from uses of those
	const std::string message = refused(std::string(s_LightBlock) + "void main() {\n  Light lights = Light(vec3(0.0), 0, vec4(0.0));\n  Light l = lights;\n}\n");
	EXPECT_TRUE(has(message, "line 4"));
	EXPECT_TRUE(has(message, "'lights'"));
	refused(std::string(s_LightBlock) + "void main() { float a = 1.0, lights = 2.0; gl_Position = vec4(a + lights); }\n");
	refused(std::string(s_LightBlock) + "float f(float lights) { return lights; }\nvoid main() { Light l = lights[0]; }\n");
	refused(std::string(s_LightBlock) + "void main() { for (int i = 0, lights = 1; i < lights; ++i) { } }\n");
	refused(std::string(s_LightBlock) + "void main() { float[2] lights = float[2](1.0, 2.0); gl_Position = vec4(lights[0]); }\n");

	// behind a label and behind a condition, as a variable of a struct declared in place, as a type
	const std::string one = "struct S { vec4 a; };\nuniform Block { S one; };\n";
	refused(one + "void main() { switch (1) { case 1: S t = S(vec4(1.0)), one = t; gl_Position = one.a; } }\n");
	refused(one + "void main() { if (true) S t = S(vec4(1.0)), one = t; }\n");
	refused(one + "void main() { struct T { vec4 a; } one; one.a = vec4(1.0); gl_Position = one.a; }\n");
	refused(one + "void main() { struct T { vec4 a; } t, one; one.a = vec4(1.0); gl_Position = one.a; }\n");
	refused(one + "void main() { struct one { vec4 a; }; gl_Position = vec4(1.0); }\n");

	// the instance name of a block
	refused("struct S { vec4 a; };\nuniform Block { S items[4]; } blk;\nvoid main() { float blk = 1.0; gl_Position = vec4(blk); }\n");

	// a comma that separates no declarations
	const std::string commas = one + "void f(S a, S b) { }\nvoid main() { S t; int i = 0; i = 1, t = one; f(t, one); }\n";
	EXPECT_TRUE(has(rewritten(commas), "i = 1, t = nlBlockRead_one(); f(t, nlBlockRead_one());"));
}

TEST_F(CUT3DGlslBlockStructReads, sourcesThatAreNotTakenApart)
{
	EXPECT_TRUE(has(refused(std::string(s_LightBlock) + "void main() { Light l = lights[0]; \n"), "line 3"));
	refused(std::string(s_LightBlock) + "void main() { Light l = lights[0); }\n");
	refused(std::string(s_LightBlock) + "/* void main() { Light l = lights[0]; }\n");
	refused(std::string(s_LightBlock) + "void main() { Light l = lights[]; }\n");

	// a struct that is declared twice, one with a struct declared inside, one that depends on the preprocessor
	refused("struct S { vec4 a; };\nstruct S { vec3 a; };\nuniform Block { S items[4]; };\nvoid main() { S s = items[0]; }\n");
	refused("struct S { struct T { float x; } t; };\nuniform Block { S items[4]; };\nvoid main() { S s = items[0]; }\n");
	refused("struct S { vec4 a;\n#ifdef MORE\n  vec4 b;\n#endif\n};\nuniform Block { S items[4]; };\nvoid main() { S s = items[0]; }\n");

	// a member that two uniform blocks declare
	const std::string message = refused("struct S { vec4 a; };\n#ifdef SMALL\nuniform Block { S items[4]; };\n#else\nuniform Block { S items[64]; };\n#endif\nvoid main() { S s = items[0]; }\n");
	EXPECT_TRUE(has(message, "line 5"));

	// an array of structs inside a struct whose size is no plain number
	refused("#define N 2\nstruct T { float x; };\nstruct S { T ts[N]; };\nuniform Block { S items[4]; };\nvoid main() { S s = items[0]; }\n");

	// structs that multiply out to more than a function may hold
	refused("struct A { vec4 v; };\nstruct B { A a[64]; };\nstruct C { B b[64]; };\nstruct D { C c[64]; };\nuniform Block { D items[2]; };\nvoid main() { D d = items[0]; }\n");

	// reads in the indices of reads, a hundred deep
	std::string deep = std::string(s_LightBlock) + "int f(Light l) { return l.mode; }\nvoid main() { Light l = ";
	for (int i = 0; i < 100; ++i)
		deep += "lights[f(";
	deep += "lights[0]";
	for (int i = 0; i < 100; ++i)
		deep += ")]";
	deep += "; }\n";
	refused(deep);
}

TEST_F(CUT3DGlslBlockStructReads, whatIsLeftAloneWithAMessage)
{
	// the array as a whole: the read of one struct beside it is rewritten
	std::string source = std::string(s_LightBlock) + "float f(Light all[16]) { return all[0].color.x; }\nvoid main() { Light l = lights[1]; gl_Position = vec4(f(lights)) + l.color; }\n";
	std::string result, message;
	EXPECT_FALSE(NL3D::rewriteGlslBlockStructReads(source, result, message));
	EXPECT_TRUE(has(message, "line 4"));
	EXPECT_TRUE(has(result, "Light l = nlBlockRead_lights(int(1));"));
	EXPECT_TRUE(has(result, "f(lights)"));

	// the body of a #define
	source = std::string(s_LightBlock) + "#define LIGHT(i) lights[i]\nvoid main() { Light l = LIGHT(1); Light m = lights[2]; }\n";
	EXPECT_FALSE(NL3D::rewriteGlslBlockStructReads(source, result, message));
	EXPECT_TRUE(has(message, "#define"));
	EXPECT_TRUE(has(result, "#define LIGHT(i) lights[i]\n"));
	EXPECT_TRUE(has(result, "Light m = nlBlockRead_lights(int(2));"));

	// a block that is an array of blocks, and one with a preprocessor line between its braces
	source = "struct S { vec4 a; };\nuniform Block { S items[4]; } blocks[2];\nvoid main() { S s = blocks[1].items[0]; }\n";
	EXPECT_FALSE(NL3D::rewriteGlslBlockStructReads(source, result, message));
	EXPECT_EQ(source, result);
	EXPECT_TRUE(has(message, "'Block'"));
	source = "struct S { vec4 a; };\nuniform Block {\n#ifdef MANY\n  S items[64];\n#else\n  S items[4];\n#endif\n};\nvoid main() { S s = items[0]; }\n";
	EXPECT_FALSE(NL3D::rewriteGlslBlockStructReads(source, result, message));
	EXPECT_EQ(source, result);

	// the instance name of a block without a member behind it
	source = "struct S { vec4 a; };\nuniform Block { S items[4]; } blk;\n#define FIRST(b) b.items[0]\nvoid main() { S s = FIRST(blk); S t = blk.items[1]; }\n";
	EXPECT_FALSE(NL3D::rewriteGlslBlockStructReads(source, result, message));
	EXPECT_TRUE(has(message, "'blk'"));
	EXPECT_TRUE(has(result, "S t = nlBlockRead_blk_items(int(1));"));

	// a member whose type is a name of the preprocessor
	source = "struct S { vec4 a; };\n#define AliasOfS S\nuniform Block { AliasOfS items[4]; vec4 plain; };\nvoid main() { S s = items[0]; }\n";
	EXPECT_FALSE(NL3D::rewriteGlslBlockStructReads(source, result, message));
	EXPECT_EQ(source, result);
	EXPECT_TRUE(has(message, "'AliasOfS'"));
}
