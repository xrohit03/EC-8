#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <iomanip>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

// =======================================================================
// Modelo de dominio
// =======================================================================

struct Expense {
    std::string date;     // "2026-06-04"
    std::string category; // "food", "transport", "books"
    std::string detail;
    double amount{};
};

using ExpenseList = std::vector<Expense>;

// =======================================================================
// Strategy: criterios de ordenamiento
// =======================================================================
// Cada criterio es un objeto función independiente. sort_with no conoce
// ninguno de ellos en particular: solo exige que el comparador sea
// compatible con std::ranges::sort sobre el contenedor recibido. Agregar
// un nuevo criterio (p. ej. por descripcion) no requiere tocar sort_with
// ni el código cliente que ya usa otras estrategias.

struct SortByDate {
    bool operator()(const Expense& a, const Expense& b) const {
        return a.date < b.date;
    }
};

struct SortByCategory {
    bool operator()(const Expense& a, const Expense& b) const {
        return std::tie(a.category, a.date) < std::tie(b.category, b.date);
    }
};

struct SortByAmountDescending {
    bool operator()(const Expense& a, const Expense& b) const {
        return a.amount > b.amount;
    }
};

template<class R, class Cmp>
concept SortStrategy =
    std::ranges::random_access_range<R> &&
    std::sortable<std::ranges::iterator_t<R>, Cmp>;

void sort_with(auto& values, auto cmp)
    requires SortStrategy<decltype(values), decltype(cmp)>
{
    std::ranges::sort(values, cmp);
}

// =======================================================================
// Factory Method: creación de exportadores
// =======================================================================
// El concept ExpenseExporter es el "producto" abstracto: cualquier tipo
// que sepa exportar una ExpenseList a std::string lo satisface. La
// fábrica make_exporter<E>() es el "Factory Method": encapsula la
// construcción de E y la envuelve en un ExportFunction homogéneo, para
// que el cliente nunca necesite un if/else sobre el formato.

template<class T>
concept ExpenseExporter =
    requires(T exporter, const ExpenseList& expenses) {
        { exporter.export_expenses(expenses) } -> std::same_as<std::string>;
    };

using ExportFunction = std::function<std::string(const ExpenseList&)>;

struct CsvExporter {
    std::string export_expenses(const ExpenseList& expenses) const {
        std::ostringstream out;
        out << "date,category,detail,amount\n";
        for (const auto& e : expenses) {
            out << e.date << ',' << e.category << ',' << e.detail << ','
                << std::fixed << std::setprecision(2) << e.amount << '\n';
        }
        return out.str();
    }
};

struct JsonExporter {
    std::string export_expenses(const ExpenseList& expenses) const {
        std::ostringstream out;
        out << '[';
        for (std::size_t i = 0; i < expenses.size(); ++i) {
            const auto& e = expenses[i];
            out << "{\"date\":\"" << e.date << "\","
                << "\"category\":\"" << e.category << "\","
                << "\"detail\":\"" << e.detail << "\","
                << "\"amount\":" << std::fixed << std::setprecision(2) << e.amount
                << '}';
            if (i + 1 < expenses.size()) out << ',';
        }
        out << ']';
        return out.str();
    }
};

struct TextExporter {
    std::string export_expenses(const ExpenseList& expenses) const {
        std::ostringstream out;
        for (const auto& e : expenses) {
            out << e.date << " [" << e.category << "] " << e.detail << ": "
                << std::fixed << std::setprecision(2) << e.amount << '\n';
        }
        return out.str();
    }
};

template<ExpenseExporter E, class... Args>
ExportFunction make_exporter(Args&&... args) {
    return [exporter = E(std::forward<Args>(args)...)](const ExpenseList& expenses) {
        return exporter.export_expenses(expenses);
    };
}

// =======================================================================
// Decorator: pasos adicionales sobre la salida
// =======================================================================
// AuditedExporter y SummaryExporter envuelven cualquier ExpenseExporter
// (concreto o ya decorado) y le agregan comportamiento sin modificar su
// código ni el del exportador que envuelven. Como ambos decoradores
// también satisfacen ExpenseExporter, se pueden anidar libremente.

template<ExpenseExporter Inner>
struct AuditedExporter {
    Inner inner;

    std::string export_expenses(const ExpenseList& expenses) const {
        std::ostringstream out;
        out << inner.export_expenses(expenses)
            << "\n--- audit ---\n"
            << "entries: " << expenses.size() << '\n';
        return out.str();
    }
};

template<ExpenseExporter Inner>
struct SummaryExporter {
    Inner inner;

    std::string export_expenses(const ExpenseList& expenses) const {
        double total = 0.0;
        for (const auto& e : expenses) total += e.amount;

        std::ostringstream out;
        out << inner.export_expenses(expenses)
            << "\n--- summary ---\n"
            << "total: " << std::fixed << std::setprecision(2) << total << '\n';
        return out.str();
    }
};
