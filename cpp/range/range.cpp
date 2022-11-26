#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iostream>
#include <list>
#include <range/v3/algorithm.hpp>
#include <range/v3/algorithm/sort.hpp>
#include <range/v3/functional/invoke.hpp>
#include <range/v3/iterator/unreachable_sentinel.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/range_fwd.hpp>
#include <range/v3/view.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>

using ranges::views::transform;

void sort_list() {
    std::list<int>   list{3, 2, 1};
    std::vector<int> v{1, 2, 3};
    // ranges::sort(list);
    ranges::sort(v);
    auto xv = v | transform([](int i) { return i + 1; });

    std::string s = "abcdefggg";
    ranges::find(s, 'a');
    // NOTE: help compiler know there is an 'a' in s
    ranges::find(s.begin(), ranges::unreachable_sentinel_t(), 'a');

    struct Employee {
        std::string name;
        int         id;
    };

    struct Payslip {
        std::string pay_info;
        int         employee_id;
    };

    std::vector<Employee> employees;
    std::vector<Payslip>  payslips;

    auto old_stl = [employees, payslips]() mutable {
        std::sort(
            employees.begin(), employees.end(),
            [](const Employee &x, const Employee &y) { return x.id < y.id; });
        std::sort(payslips.begin(), payslips.end(),
                  [](const Payslip &x, const Payslip &y) {
                      return x.employee_id < y.employee_id;
                  });
        std::equal(employees.begin(), employees.end(), payslips.begin(),
                   payslips.end(), [](const Employee &a, const Payslip &b) {
                       return a.id == b.employee_id;
                   });
    };

    auto range_projection = [employees, payslips]() mutable {
        ranges::sort(employees, ranges::less{},
                     [](const Employee &e) { return e.id; });
        ranges::sort(payslips, ranges::less{},
                     [](const Payslip &e) { return e.employee_id; });
        ranges::equal(
            employees, payslips, ranges::equal_to{},
            [](const Employee &e) { return e.id; },
            [](const Payslip &p) { return p.employee_id; });
    };

    auto range_magic_invoke = [employees, payslips]() mutable {
        ranges::sort(employees, ranges::less{}, &Employee::id);
        ranges::sort(payslips, ranges::less{}, &Payslip::employee_id);
        ranges::equal(employees, payslips, ranges::equal_to{}, &Employee::id,
                      &Payslip::employee_id);
    };
}

void view() {
    std::vector<int> vec{1, 2, 3};
    for (int i : vec) {
        if ((i % 2) == 0) {
            std::cout << i * i;
        }
    }
    auto view = vec | ranges::views::filter([](int i) { return i % 2 == 0; }) |
                ranges::views::transform([](int i) { return i * i; });
    fmt::print("{}", view);

    std::string s = "abcd";
    std::string b = "qwerttytyy";
    ranges::views::zip(s, b);
}
