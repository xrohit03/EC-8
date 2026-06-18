#include "expenses.h"

#include <iostream>

namespace {

ExpenseList load_sample_expenses() {
    return ExpenseList{
        {"2026-06-04", "food", "almuerzo", 18.50},
        {"2026-06-02", "transport", "pasaje bus", 3.20},
        {"2026-06-10", "books", "libro c++", 45.00},
        {"2026-06-07", "books", "libro algoritmos", 38.00},
        {"2026-06-15", "food", "cena", 22.30},
    };
}

} // namespace

int main() {
    ExpenseList expenses = load_sample_expenses();

    // --- Strategy: el criterio de orden se elige en tiempo de ejecución
    // sin que sort_with conozca SortByAmountDescending en particular. ---
    std::cout << "== Gastos ordenados por monto (descendente) ==\n";
    sort_with(expenses, SortByAmountDescending{});
    for (const auto& e : expenses) {
        std::cout << "  " << e.date << "  " << e.category << "  "
                  << e.detail << "  " << e.amount << '\n';
    }

    std::cout << "\n== Mismos gastos ordenados por categoria y fecha ==\n";
    sort_with(expenses, SortByCategory{});
    for (const auto& e : expenses) {
        std::cout << "  " << e.date << "  " << e.category << "  "
                  << e.detail << "  " << e.amount << '\n';
    }

    // --- Factory Method: el cliente pide un formato por tipo, sin
    // ningún if/else sobre csv/json/text. ---
    auto csv_export = make_exporter<CsvExporter>();
    auto json_export = make_exporter<JsonExporter>();

    std::cout << "\n== Exportacion CSV (via Factory Method) ==\n";
    std::cout << csv_export(expenses);

    std::cout << "\n== Exportacion JSON (via Factory Method) ==\n";
    std::cout << json_export(expenses) << '\n';

    // --- Decorator: auditoría y resumen se agregan envolviendo el
    // exportador base, sin modificar CsvExporter ni el flujo principal.
    // El orden del anidamiento es libre. ---
    auto reporting_export =
        AuditedExporter{
            SummaryExporter{
                CsvExporter{}
            }
        };

    std::cout << "\n== Exportacion CSV + resumen + auditoria (Decorator) ==\n";
    std::cout << reporting_export.export_expenses(expenses);

    return 0;
}
