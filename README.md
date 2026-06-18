# Gastos personales — patrones de diseño en C++20

Herramienta de consola para ordenar y exportar una lista de gastos personales,
construida con **Factory Method**, **Decorator** y **Strategy** sobre C++20
(concepts, ranges, RAII).

## Compilar y probar

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build --output-on-failure
```

`expenses_app` corre el flujo demostrativo (carga, orden, exportación).
`expenses_tests` corre las 7 pruebas vía `ctest`.

> **Nota sobre C++20:** `std::string::contains` es una función de C++23
> (P1679), no de C++20. Como `CMakeLists.txt` fija `CMAKE_CXX_STANDARD 20`
> estrictamente, `tests/test_expenses.cpp` usa un pequeño helper
> `has_substring` (basado en `std::string::find`) en lugar de `.contains()`
> para no romper el estándar declarado por el proyecto.

## ¿Qué patrón desacopla qué parte del diseño?

**Factory Method** desacopla *qué formato de exportación se necesita* de
*cómo se construye un exportador concreto*. El cliente solo conoce
`make_exporter<E>()` y el tipo resultante `ExportFunction`; nunca ve
`CsvExporter`, `JsonExporter` ni `TextExporter` directamente, así que agregar
un nuevo formato no toca el código que ya exporta.

**Decorator** desacopla *el formato de exportación* de *los pasos extra sobre
la salida* (auditoría, resumen). `AuditedExporter<Inner>` y
`SummaryExporter<Inner>` envuelven cualquier tipo que cumpla el concept
`ExpenseExporter` —incluyendo a otro decorador— sin modificar ni el
exportador envuelto ni el flujo principal. Por eso se pueden anidar en
cualquier orden (`AuditedExporter{SummaryExporter{CsvExporter{}}}` o al
revés) sin tocar una sola línea de `CsvExporter`.

**Strategy** desacopla *el criterio de orden* del *algoritmo de
ordenamiento*. `sort_with` solo exige, vía el concept `SortStrategy`, que el
comparador sea compatible con `std::ranges::sort` sobre el contenedor
recibido; no conoce `SortByDate`, `SortByCategory` ni
`SortByAmountDescending` en particular. Agregar un nuevo criterio es
escribir un nuevo comparador, sin tocar `sort_with` ni el resto del flujo.
