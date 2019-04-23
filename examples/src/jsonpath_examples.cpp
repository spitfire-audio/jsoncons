// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <fstream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>

namespace jc = jsoncons;
namespace jp = jsoncons::jsonpath;

void json_query_examples()
{
    std::ifstream is("./input/booklist.json");
    jc::json booklist;
    is >> booklist;

    // The authors of books that are cheaper than $10
    jc::json result1 = jp::json_query(booklist, "$.store.book[?(@.price < 10)].author");
    std::cout << "(1) " << result1 << "\n";

    // The number of books
    jc::json result2 = jp::json_query(booklist, "$..book.length");
    std::cout << "(2) " << result2 << "\n";

    // The third book
    jc::json result3 = jp::json_query(booklist, "$..book[2]");
    std::cout << "(3)\n" << pretty_print(result3) << "\n";

    // All books whose author's name starts with Evelyn
    jc::json result4 = jp::json_query(booklist, "$.store.book[?(@.author =~ /Evelyn.*?/)]");
    std::cout << "(4)\n" << pretty_print(result4) << "\n";

    // The titles of all books that have isbn number
    jc::json result5 = jp::json_query(booklist, "$..book[?(@.isbn)].title");
    std::cout << "(5) " << result5 << "\n";

    // All authors and titles of books
    jc::json result6 = jp::json_query(booklist, "$['store']['book']..['author','title']");
    std::cout << "(6)\n" << pretty_print(result6) << "\n";

    // Normalized path expressions
    jc::json result7 = jp::json_query(booklist, "$.store.book[?(@.author =~ /Evelyn.*?/)]", jp::result_type::path);
    std::cout << "(7)\n" << pretty_print(result7) << "\n";

    // All titles whose author's second name is 'Waugh'
    jc::json result8 = jp::json_query(booklist,"$.store.book[?(tokenize(@.author,'\\\\s+')[1] == 'Waugh')].title");
    std::cout << "(8)\n" << result8 << "\n";

    // All keys in the second book
    jc::json result9 = jp::json_query(booklist,"keys($.store.book[1])[*]");
    std::cout << "(9)\n" << result9 << "\n";
}

void json_replace_example1()
{
    std::ifstream is("./input/booklist.json");
    jc::json booklist = jc::json::parse(is);

    jp::json_replace(booklist,"$.store.book[?(@.isbn == '0-553-21311-3')].price",10.0);
    std::cout << pretty_print(booklist) << "\n";
}

void json_replace_example2()
{
    jc::json j;
    try
    {
        j = jc::json::parse(R"(
{"store":
{"book": [
{"category": "reference",
"author": "Margaret Weis",
"title": "Dragonlance Series",
"price": 31.96}, 
{"category": "reference",
"author": "Brent Weeks",
"title": "Night Angel Trilogy",
"price": 14.70
}]}}
)");
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << "\n";
    }

    std::cout << ("1\n") << pretty_print(j) << "\n";

    jp::json_replace(j,"$..book[?(@.price==31.96)].price", 30.9);

    std::cout << ("2\n") << pretty_print(j) << "\n";
}

void jsonpath_complex_examples()
{
    const jc::json j = jc::json::parse(R"(
    [
      {
        "root": {
          "id" : 10,
          "second": [
            {
                 "names": [
                   2
              ],
              "complex": [
                {
                  "names": [
                    1
                  ],
                  "panels": [
                    {
                      "result": [
                        1
                      ]
                    },
                    {
                      "result": [
                        1,
                        2,
                        3,
                        4
                      ]
                    },
                    {
                      "result": [
                        1
                      ]
                    }
                  ]
                }
              ]
            }
          ]
        }
      },
      {
        "root": {
          "id" : 20,
          "second": [
            {
              "names": [
                2
              ],
              "complex": [
                {
                  "names": [
                    1
                  ],
                  "panels": [
                    {
                      "result": [
                        1
                      ]
                    },
                    {
                      "result": [
                        3,
                        4,
                        5,
                        6
                      ]
                    },
                    {
                      "result": [
                        1
                      ]
                    }
                  ]
                }
              ]
            }
          ]
        }
      }
    ]
    )");

    // Find all arrays of elements where result.length is 4
    jc::json result1 = jp::json_query(j,"$..[?(@.result.length == 4)].result");
    std::cout << "(1) " << result1 << "\n";

    // Find array of elements that has id 10 and result.length is 4
    jc::json result2 = jp::json_query(j,"$..[?(@.id == 10)]..[?(@.result.length == 4)].result");
    std::cout << "(2) " << result2 << "\n";

    // Find all arrays of elements where result.length is 4 and that have value 3 
    jc::json result3 = jp::json_query(j,"$..[?(@.result.length == 4 && (@.result[0] == 3 || @.result[1] == 3 || @.result[2] == 3 || @.result[3] == 3))].result");
    std::cout << "(3) " << result3 << "\n";
}

void jsonpath_union()
{
    const jc::json root = jc::json::parse(R"(
    {
          "firstName": "John",
          "lastName" : "doe",
          "age"      : 26,
          "address"  : {
            "streetAddress": "naist street",
            "city"         : "Nara",
            "postalCode"   : "630-0192"
          },
          "phoneNumbers": [
            {
              "type"  : "iPhone",
              "number": "0123-4567-8888"
            },
            {
              "type"  : "home",
              "number": "0123-4567-8910"
            }
          ]
}
    )");

    std::string path = "$.[firstName,address.city]";
    jc::json result = jp::json_query(root,path);

    std::cout << result << "\n";
}

void jsonpath_examples()
{
    std::cout << "\nJsonPath examples\n\n";
    json_query_examples();
    std::cout << "Here\n";
    json_replace_example1();
    json_replace_example2();
    jsonpath_complex_examples();
    jsonpath_union();
    std::cout << "\n";
}

