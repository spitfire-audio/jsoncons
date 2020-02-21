// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <iomanip>
#include <jsoncons/json.hpp>

namespace json_type_traits_macros_examples_ns {

    class Foo
    {
    public:
        virtual ~Foo() = default;
    };

    class Bar : public Foo
    {
        static const bool bar = true;
        JSONCONS_TYPE_TRAITS_FRIEND
    };

    class Baz : public Foo 
    {
        static const bool baz = true;
        JSONCONS_TYPE_TRAITS_FRIEND
    };

    enum class BookCategory {fiction,biography};

    // #1 Class with public member data and default constructor   
    struct Book1
    {
        BookCategory category;
        std::string author;
        std::string title;
        double price;
    };

    // #2 Class with private member data and default constructor   
    class Book2
    {
        BookCategory category;
        std::string author;
        std::string title;
        double price;
        Book2() = default;

        JSONCONS_TYPE_TRAITS_FRIEND
    public:
        BookCategory get_category() const {return category;}

        const std::string& get_author() const {return author;}

        const std::string& get_title() const{return title;}

        double get_price() const{return price;}
    };

    // #3 Class with getters and initializing constructor
    class Book3
    {
        BookCategory category_;
        std::string author_;
        std::string title_;
        double price_;
    public:
        Book3(BookCategory category,
              const std::string& author,
              const std::string& title,
              double price)
            : category_(category), author_(author), title_(title), price_(price)
        {
        }

        Book3(const Book3&) = default;
        Book3(Book3&&) = default;
        Book3& operator=(const Book3&) = default;
        Book3& operator=(Book3&&) = default;

        BookCategory category() const {return category_;}

        const std::string& author() const{return author_;}

        const std::string& title() const{return title_;}

        double price() const{return price_;}
    };

    class Employee
    {
        std::string firstName_;
        std::string lastName_;
    public:
        Employee(const std::string& firstName, const std::string& lastName)
            : firstName_(firstName), lastName_(lastName)
        {
        }
        virtual ~Employee() = default;

        virtual double calculatePay() const = 0;

        const std::string& firstName() const {return firstName_;}
        const std::string& lastName() const {return lastName_;}
    };

    class HourlyEmployee : public Employee
    {
        double wage_;
        unsigned hours_;
    public:
        HourlyEmployee(const std::string& firstName, const std::string& lastName, 
                       double wage, unsigned hours)
            : Employee(firstName, lastName), 
              wage_(wage), hours_(hours)
        {
        }
        HourlyEmployee(const HourlyEmployee&) = default;
        HourlyEmployee(HourlyEmployee&&) = default;
        HourlyEmployee& operator=(const HourlyEmployee&) = default;
        HourlyEmployee& operator=(HourlyEmployee&&) = default;

        double wage() const {return wage_;}

        unsigned hours() const {return hours_;}

        double calculatePay() const override
        {
            return wage_*hours_;
        }
    };

    class CommissionedEmployee : public Employee
    {
        double baseSalary_;
        double commission_;
        unsigned sales_;
    public:
        CommissionedEmployee(const std::string& firstName, const std::string& lastName, 
                             double baseSalary, double commission, unsigned sales)
            : Employee(firstName, lastName), 
              baseSalary_(baseSalary), commission_(commission), sales_(sales)
        {
        }
        CommissionedEmployee(const CommissionedEmployee&) = default;
        CommissionedEmployee(CommissionedEmployee&&) = default;
        CommissionedEmployee& operator=(const CommissionedEmployee&) = default;
        CommissionedEmployee& operator=(CommissionedEmployee&&) = default;

        double baseSalary() const
        {
            return baseSalary_;
        }

        double commission() const
        {
            return commission_;
        }

        unsigned sales() const
        {
            return sales_;
        }

        double calculatePay() const override
        {
            return baseSalary_ + commission_*sales_;
        }
    };
#if defined(JSONCONS_HAS_STD_OPTIONAL)
    class MetaDataReplyTest 
    {
    public:
        MetaDataReplyTest()
            : description()
        {
        }
        const std::string& GetStatus() const 
        {
            return status;
        }
        const std::string& GetPayload() const 
        {
            return payload;
        }
        const std::optional<std::string>& GetDescription() const 
        {
            return description;
        }
    private:
        JSONCONS_TYPE_TRAITS_FRIEND;
        std::string status;
        std::string payload;
        std::optional<std::string> description;
    };
#endif

} // json_type_traits_macros_examples_ns

namespace ns = json_type_traits_macros_examples_ns;

// Declare the traits at global scope
JSONCONS_ENUM_TRAITS(ns::BookCategory,fiction,biography)

JSONCONS_ALL_MEMBER_TRAITS(ns::Book1,category,author,title,price)
JSONCONS_ALL_MEMBER_TRAITS(ns::Book2,category,author,title,price)
JSONCONS_ALL_GETTER_CTOR_TRAITS(ns::Book3,category,author,title,price)

JSONCONS_N_GETTER_CTOR_TRAITS(ns::HourlyEmployee, 3, firstName, lastName, wage, hours)
JSONCONS_N_GETTER_CTOR_TRAITS(ns::CommissionedEmployee, 4, firstName, lastName, baseSalary, commission, sales)
JSONCONS_POLYMORPHIC_TRAITS(ns::Employee, ns::HourlyEmployee, ns::CommissionedEmployee)

JSONCONS_N_MEMBER_TRAITS(ns::Bar,1,bar)
JSONCONS_N_MEMBER_TRAITS(ns::Baz,1,baz)
JSONCONS_POLYMORPHIC_TRAITS(ns::Foo, ns::Bar, ns::Baz)

#if defined(JSONCONS_HAS_STD_OPTIONAL)
JSONCONS_N_MEMBER_TRAITS(ns::MetaDataReplyTest, 2, status, payload, description)
#endif

using namespace jsoncons;

#if defined(JSONCONS_HAS_STD_OPTIONAL)
static void json_type_traits_optional_examples()
{
    std::string input1 = R"({
      "status": "OK",
      "payload": "Modified",
      "description": "TEST"
    })";
    std::string input2 = R"({
      "status": "OK",
      "payload": "Modified"
    })";

    auto val1 = decode_json<ns::MetaDataReplyTest>(input1);
    assert(val1.GetStatus() == "OK");
    assert(val1.GetPayload() == "Modified");
    assert(val1.GetDescription());
    assert(val1.GetDescription() == "TEST");

    auto val2 = decode_json<ns::MetaDataReplyTest>(input2);
    assert(val2.GetStatus() == "OK");
    assert(val2.GetPayload() == "Modified");
    assert(!val2.GetDescription());

    std::string output1;
    std::string output2;

    encode_json(val2,output2,indenting::indent);
    encode_json(val1,output1,indenting::indent);

    std::cout << "(1)\n";
    std::cout << output1 << "\n\n";

    std::cout << "(2)\n";
    std::cout << output2 << "\n\n";
}
#endif

static void json_type_traits_book_examples()
{
    const std::string input = R"(
    [
        {
            "category" : "fiction",
            "author" : "Haruki Murakami",
            "title" : "Kafka on the Shore",
            "price" : 25.17
        },
        {
            "category" : "biography",
            "author" : "Robert A. Caro",
            "title" : "The Path to Power: The Years of Lyndon Johnson I",
            "price" : 16.99
        }
    ]
    )";

    std::cout << "(1)\n\n";
    auto books1 = decode_json<std::vector<ns::Book1>>(input);
    for (const auto& item : books1)
    {
        switch(item.category)
        {
            case ns::BookCategory::fiction: std::cout << "fiction, "; break;
            case ns::BookCategory::biography: std::cout << "biography, "; break;
        }
        std::cout << item.author << ", " 
                  << item.title << ", " 
                  << item.price << "\n";
    }
    std::cout << "\n";
    encode_json(books1, std::cout, indenting::indent);
    std::cout << "\n\n";

    std::cout << "(2)\n\n";
    auto books2 = decode_json<std::vector<ns::Book2>>(input);
    for (const auto& item : books2)
    {
        switch(item.get_category())
        {
            case ns::BookCategory::fiction: std::cout << "fiction, "; break;
            case ns::BookCategory::biography: std::cout << "biography, "; break;
        }
        std::cout << item.get_author() << ", " 
                  << item.get_title() << ", " 
                  << item.get_price() << "\n";
    }
    std::cout << "\n";
    encode_json(books2, std::cout, indenting::indent);
    std::cout << "\n\n";

    std::cout << "(3)\n\n";
    auto books3 = decode_json<std::vector<ns::Book3>>(input);
    for (const auto& item : books3)
    {
        switch(item.category())
        {
            case ns::BookCategory::fiction: std::cout << "fiction, "; break;
            case ns::BookCategory::biography: std::cout << "biography, "; break;
        }
        std::cout << item.author() << ", " 
                  << item.title() << ", " 
                  << item.price() << "\n";
    }
    std::cout << "\n";
    encode_json(books3, std::cout, indenting::indent);
    std::cout << "\n\n";
}

void employee_polymorphic_example()
{
    std::string input = R"(
[
    {
        "firstName": "John",
        "hours": 1000,
        "lastName": "Smith",
        "wage": 40.0
    },
    {
        "baseSalary": 30000.0,
        "commission": 0.25,
        "firstName": "Jane",
        "lastName": "Doe",
        "sales": 1000
    }
]
    )"; 

    auto v = decode_json<std::vector<std::unique_ptr<ns::Employee>>>(input);

    std::cout << "(1)\n";
    for (const auto& p : v)
    {
        std::cout << p->firstName() << " " << p->lastName() << ", " << p->calculatePay() << "\n";
    }

    std::cout << "\n(2)\n";
    encode_json(v, std::cout, indenting::indent);

    std::cout << "\n\n(3)\n";
    json j(v);
    std::cout << pretty_print(j) << "\n\n";
}

void foo_bar_baz_example()
{
    std::vector<std::unique_ptr<ns::Foo>> u;
    u.emplace_back(new ns::Bar());
    u.emplace_back(new ns::Baz());

    std::string buffer;
    encode_json(u, buffer);
    std::cout << "(1)\n" << buffer << "\n\n";

    auto v = decode_json<std::vector<std::unique_ptr<ns::Foo>>>(buffer);

    std::cout << "(2)\n";
    for (const auto& ptr : v)
    {
        if (dynamic_cast<ns::Bar*>(ptr.get()))
        {
            std::cout << "A bar\n";
        }
        else if (dynamic_cast<ns::Baz*>(ptr.get()))
        {
            std::cout << "A baz\n";
        } 
    }
}

void json_traits_macros_examples()
{
    std::cout << "\njson_type_traits macro examples\n\n";

    std::cout << std::setprecision(6);

    json_type_traits_book_examples();
    employee_polymorphic_example();
    foo_bar_baz_example();

#if defined(JSONCONS_HAS_STD_OPTIONAL)
    json_type_traits_optional_examples();
#endif

    std::cout << std::endl;
}

