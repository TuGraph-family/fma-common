## TextParser Manual

Parsing text files can become a bottleneck for high performance programs: single threaded parsing with fscanf() yields ~
30MB/s for edge files with two integers per line. `fma_common::TextParser` is designed to solve the problem. It can
parse at over **200MB/s with single thread**, and **supports parsing with multiple threads**. In our test environment,
we can parse more than 1GB/s with 4 threads.

- [Features](#features-)
- [Header File](#header-file-)
- [MakeCsvParser/MakeTsvParser Helper Functions](#makecsvparsermaketsvparser-helper-functions-)
- [Using TextParser Directly](#using-textparser-directly-)
- [TextParserUtils](#textparserutils-)

### Features [^](#textparser-manual)

- Fast single thread performace (>200MB/s)
- Multi-thread parsing support (over 1GB/s with 4 threads)
- Out-of-box CSV/TSV support
- Customizable parsing interface
- Featured parsing utilities

### Header File [^](#textparser-manual)

TextParser is implemented in `fma-common/text_parser.h`.

### MakeCsvParser/MakeTsvParser Helper Functions [^](#textparser-manual)

The simplest way to use TextParser is to use the `MakeCsvParser/MakeTsvParser` helper functions. Just create
an [`InputFmaStream`](/docs/fma_stream.md) and then use `MakeCsvParser()/MakeTsvParser()` to create the parser. You can
then use `TextParser::ReadBlock(std::vector<ElementType>&)` to read a block of data.

Suppose you are parsing an edge file which contains (`int64_t srcId`, `int64_t dstId`) in each line seperated with tab
or blankspace, the following code shows how to use parse the file with `TextParser`. (This code can be found
in [test/test_text_parser.cpp](https://github.com/fma-cloud/fma-common/blob/master/test/test_text_parser.cpp)).

```c++
#include "fma-common/text_parser.h"
using namespace fma_common;

InputFmaStream stream("/path/to/text/file", 0);
auto text_parser = 
    MakeTsvParser<int64_t, int64_t> (    // specify type of each field
         stream,            // stream to read from
         block_size,        // size of each text block to parse with different threads
         n_parse_threads);  // number of parser threads
std::vector<decltype(text_parser)::ElementType> buf;  // ElementType is std::tuple<int64_t, int64_t> here
size_t n = 0;
while (text_parser->ReadBlock(buf)) {
    n += buf.size();
    for (auto& e : buf) {
        // e is of type std::tuple<int64_t, int64_t>
        int64_t src = std::get<0>(e);
        int64_t dst = std::get<1>(e);
    }
}
```

`MakeTsvParser` is declared as:

```c++
template<typename... Ts>
std::shared_ptr< TextParser<std::tuple<Ts...>, TextParserUtils::TupleParser<false, Ts...>> >
    MakeTsvParser(InputFileStream& stream,
                  size_t block_size = 65536,
                  size_t n_threads = 1);
```

As you can see, `MakeTsvParser` accepts variable template parameters. You must specify the type of each field in the
template parameter. The function returns a `TextParser` which parses a tuple of the field types from each text line,
assuming fields are seperated with tab or blankspace. Similarly, `MakeCsvParser` returns a parser which parses Comma
Seperated Vectors (CSV) files.

----------

*Hint: If the input contains some fields that you don't need. You can specify `TextParserUtils::DropField` as the type
of that field. This saves the time to parse the unnecessary data, and makes parsing faster.*

----------

### Using TextParser Directly [^](#textparser-manual)

Sometimes we need to customize the parsing behavior. In this case, you can use the `TextParser` class directly. It is
declared as follows:

```c++
template<typename T,   // type of data for each line, usually a struct or tuple
         typename F>   // the type of function used to parse one text line
class TextParser {
public:
    typedef T ElementType;
    typedef F ParseOneLine;

    TextParser(InputFileStream& stream,   // input text stream
        const ParseOneLine& func,         // function to parse one text line
        size_t block_size = 65536,        // text block size
        size_t n_threads = 1);            // number of parser threads

    /* Read a block of data into block, return true on success */
    bool ReadBlock(std::vector<ElementType>& block);

    /* Read one element into data, return true on success */
    bool Read(ElementType& data);
}
```

The function `ParseOneLine` is declared as:

```c++
size_t ParseOneLine(const char* begin, const char* end, T& data);
```

The input is a text string from `begin` to `end`, which might contain multiple lines. `ParseOneLine` must parse the
first line in the text string, store the result in `data`, and then return the number of characters in the first line.

***
***NOTE: The return value of `ParseOneLine` should also count any unparsed fields in the first line. For example, if the
first line contains three fields with totally 30 characters (position of the second line), then the return value should
be 30, no matter how many fields you actually need.***
***

Suppose we have an edge file in the following format:

```text
    12345    67890    type_a
    23456    78901    type_b
    ...
```

And we want to parse each line into an edge of type:

```c++
struct Edge{
    int64_t src;
    int64_t dst;
    int type;
};
```

in which `type` is determined by a table lookup `int GetTypeWithString(const char* beg, const char* end)`. We can define
the ParseOneLine function as:

```c++
size_t ParseOneEdge(const char* beg, const char* end, Edge& e) {
	const char* p = beg;
	p += TextParserUtils::ParseInt64(p, end, e.src);
	p += TextParserUtils::ParseInt64(p, end, e.dst);
	while (p != end && (*p == ' ' || *p == '\t')) p++;	// skip blankspaces
	const char* type_beg = p;
	while (p != end && TextParserUtils::IsGraphical(*p)) p++;	// scan the type string
	e.type = GetTypeWithString(type_beg, p);
	return p - beg;
}
```

And then you can parse the edge file with the following code:

```c++
        TextParser<Edge, decltype(ParseOneEdge)> edge_parser(
                 stream, 
                 ParseOneEdge,
                 65536,          // 64KB for each text block
                 4);             // use 4 parsing threads
	std::vector<Edge> edges;
	while (edge_parser.ReadBlock(edges)) {
		// deal with the edges...
	}
```

There is also a helper function `MakeTextParser` to help create text parsers.

### TextParserUtils [^](#textparser-manual)

The `atoi`, `atoll` and `atof` functions are slow, especially when used in multi-threaded programs. To parse text
strings efficiently, we provides our own implementation of these functionalities in `fma_common::TextParserUtils`.

Here are some commonly used functions:

```c++
/* parse an int64_t */
size_t ParseInt64(const char* b, const char* e, int64_t& d);

/* parse a double */
size_t ParseDouble(const char* b, const char* e, double& d);

/* parse a string that contains [a-z, A-Z, 0-9] */
size_t ParseAlphabetNumeric(const char*b, const char* e, std::string& s);

/* parse a string with visiable characters, that is, no tab or blankspace */
size_t ParseGraphicalString(const char*b, const char* e, std::string& s);

/* parse to the end of graphical string, but don't return it */
size_t DropGraphicalField(const char*b, const char* e);

/* parse a printable string that contains visiable characters as well as tab and blankspace */
size_t ParsePrintableString(const char*b, const char* e, std::string& s);

/* parse a string till comma */
size_t ParseCsvString(const char*b, const char* e, std::string& s);

/* parse csv field, but don't return it */
size_t DropCsvField(const char*b, const char* e);

/* parse any of the above types. 
Use TextParserUtils::DropField for T to drop the field.
If IS_CSV=true, then parse as CSV, otherwise, parse as TSV */
template<typename T, bool IS_CSV = false>
    inline size_t ParseT(const char* b, const char* e, T& d);
```

