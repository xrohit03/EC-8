#include "expenses.h"

#include <cassert>
#include <string>
#include <tuple>

// std::string::contains llegó en C++23 (P1679). Este proyecto se compila
// con CMAKE_CXX_STANDARD 20 (ver CMakeLists.txt), así que se usa este
// pequeño helper en su lugar para no depender de una característica de
// la biblioteca que no existe todavía en C++20.
bool has_substring(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

// ---------------------------------------------------------------------
// Strategy
// ---------------------------------------------------------------------

void test_strategy_sort_by_amount_descending() {
    ExpenseList expenses{
        {"2026-06-01", "food", "lunch", 18.50},
        {"2026-06-02", "transport", "bus", 3.20},
        {"2026-06-03", "books", "cpp", 45.00}
    };
    auto by_amount = [](const Expense& a, const Expense& b) {
        return a.amount > b.amount;
    };
    sort_with(expenses, by_amount);
    assert(expenses.front().amount == 45.00);
    assert(expenses.back().amount == 3.20);
}

void test_strategy_sort_by_category_then_date() {
    ExpenseList expenses{
        {"2026-06-03", "transport", "bus", 3.20},
        {"2026-06-01", "books", "cpp", 45.00},
        {"2026-06-02", "books", "algorithms", 38.00}
    };
    auto by_category_then_date = [](const Expense& a, const Expense& b) {
        return std::tie(a.category, a.date) < std::tie(b.category, b.date);
    };
    sort_with(expenses, by_category_then_date);
    assert(expenses[0].category == "books");
    assert(expenses[0].date == "2026-06-01");
    assert(expenses[2].category == "transport");
}

// ---------------------------------------------------------------------
// Factory Method
// ---------------------------------------------------------------------

void test_factory_create_csv_exporter() {
    ExpenseList expenses{
        {"2026-06-01", "food", "lunch", 18.50},
        {"2026-06-02", "transport", "bus", 3.20}
    };
    auto exporter = make_exporter<CsvExporter>();
    auto output = exporter(expenses);
    assert(has_substring(output, "date,category,detail,amount"));
    assert(has_substring(output, "food"));
    assert(has_substring(output, "transport"));
}

void test_factory_create_json_exporter() {
    ExpenseList expenses{
        {"2026-06-01", "food", "lunch", 18.50}
    };
    auto exporter = make_exporter<JsonExporter>();
    auto output = exporter(expenses);
    assert(has_substring(output, "\"category\""));
    assert(has_substring(output, "\"food\""));
    assert(has_substring(output, "\"amount\""));
}

// ---------------------------------------------------------------------
// Decorator
// ---------------------------------------------------------------------

void test_decorator_audit_preserves_inner_export() {
    ExpenseList expenses{
        {"2026-06-01", "food", "lunch", 18.50},
        {"2026-06-02", "transport", "bus", 3.20}
    };
    AuditedExporter exporter{CsvExporter{}};
    auto output = exporter.export_expenses(expenses);
    assert(has_substring(output, "date,category,detail,amount"));
    assert(has_substring(output, "transport"));
    assert(has_substring(output, "audit"));
}

void test_decorator_summary_adds_total() {
    ExpenseList expenses{
        {"2026-06-01", "food", "lunch", 18.50},
        {"2026-06-02", "transport", "bus", 3.20}
    };
    SummaryExporter exporter{TextExporter{}};
    auto output = exporter.export_expenses(expenses);
    assert(has_substring(output, "food"));
    assert(has_substring(output, "total"));
    assert(has_substring(output, "21.7") || has_substring(output, "21.70"));
}

// ---------------------------------------------------------------------
// Integración
// ---------------------------------------------------------------------

void test_integration_export_sorted_expenses() {
    ExpenseList expenses{
        {"2026-06-01", "food", "lunch", 18.50},
        {"2026-06-02", "transport", "bus", 3.20},
        {"2026-06-03", "books", "cpp", 45.00}
    };
    auto by_amount = [](const Expense& a, const Expense& b) {
        return a.amount > b.amount;
    };
    sort_with(expenses, by_amount);

    auto exporter =
        AuditedExporter{
            SummaryExporter{
                CsvExporter{}
            }
        };
    auto output = exporter.export_expenses(expenses);
    assert(expenses.front().amount == 45.00);
    assert(has_substring(output, "date,category,detail,amount"));
    assert(has_substring(output, "total"));
    assert(has_substring(output, "audit"));
}

int main() {
    test_strategy_sort_by_amount_descending();
    test_strategy_sort_by_category_then_date();
    test_factory_create_csv_exporter();
    test_factory_create_json_exporter();
    test_decorator_audit_preserves_inner_export();
    test_decorator_summary_adds_total();
    test_integration_export_sorted_expenses();
    return 0;
}
