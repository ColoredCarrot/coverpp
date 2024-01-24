import { Row, RowData, RowModel, Table, memo } from "@tanstack/react-table";
import { isDev } from "#/environment";

/**
 * Same as the default `getExpandedRowModel`,
 * except that non-expanded rows aren't actually filtered out.
 * Useful if you want to manually filter out those rows,
 * e.g. to implement animations.
 */
export default function getExpandedRowModelNoFilter<TData extends RowData>(): (
    table: Table<TData>,
) => () => RowModel<TData> {
    // Reference: https://github.com/TanStack/table/blob/764d5db0a644880576f8bc23db298849fcb3c324/packages/table-core/src/utils/getExpandedRowModel.ts
    return table =>
        memo(
            () => [
                table.getState().expanded,
                table.getPreExpandedRowModel(),
                table.options.paginateExpandedRows,
            ],
            (expanded, rowModel, paginateExpandedRows) => {
                if (!rowModel.rows.length) {
                    return rowModel;
                }

                if (!paginateExpandedRows) {
                    // Only expand rows at this point if they are being paginated
                    return rowModel;
                }

                return expandRows(rowModel);
            },
            {
                key: isDev() && "getExpandedRowModelNoFilter",
                debug: () => table.options.debugAll ?? table.options.debugTable,
            },
        );
}

function expandRows<TData extends RowData>(rowModel: RowModel<TData>) {
    const expandedRows: Row<TData>[] = [];

    const handleRow = (row: Row<TData>) => {
        expandedRows.push(row);

        // Here is the only difference to the default getExpandedRowModel:
        // We don't have the `row.getIsExpanded()` condition!
        if (row.subRows?.length /*&& row.getIsExpanded()*/) {
            row.subRows.forEach(handleRow);
        }
    };

    rowModel.rows.forEach(handleRow);

    return {
        rows: expandedRows,
        flatRows: rowModel.flatRows,
        rowsById: rowModel.rowsById,
    };
}
