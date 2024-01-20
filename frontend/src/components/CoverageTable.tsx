import styles from "./CoverageTable.module.css";
import { IconChevronDown, IconChevronRight } from "@tabler/icons-react";
import {
    createColumnHelper,
    ExpandedState,
    flexRender,
    getCoreRowModel,
    Row,
    RowData,
    RowModel,
    Table,
    useReactTable,
    memo as tanStackTableMemo,
} from "@tanstack/react-table";
import { useState } from "react";
import { isDev } from "#/environment";

function cls(...classes: readonly string[]): string {
    return classes.join(" ");
}

type Entry = {
    path: string;
    totalCovered: number;
    totalReachable: number;
    children?: Entry[];
};

const percentNumberFormat = new Intl.NumberFormat("en-US", {
    style: "percent",
    minimumFractionDigits: 1,
    maximumFractionDigits: 1,
});

const helper = createColumnHelper<Entry>();
const columns = [
    helper.accessor("path", {
        header: "Source file",
        cell: ctx => <code>{ctx.getValue()}</code>,
    }),
    helper.accessor("totalCovered", { header: "Covered" }),
    helper.accessor("totalReachable", { header: "Total" }),
    helper.display({
        header: "Percent",
        cell: ctx =>
            percentNumberFormat.format(
                ctx.row.original.totalCovered / ctx.row.original.totalReachable,
            ),
    }),
];

type CommonProps = { table: Table<Entry> };

function Head({ table }: CommonProps) {
    return (
        <div className={cls(styles.Row, styles.Head)}>
            <div />
            {table.getLeafHeaders().map(header => (
                <div key={header.id}>
                    {flexRender(
                        header.column.columnDef.header,
                        header.getContext(),
                    )}
                </div>
            ))}
        </div>
    );
}

function DataRow({ row }: { row: Row<Entry> }) {
    return (
        <div
            className={styles.Row}
            style={{
                "--indent": row.depth,
                height: row.getIsAllParentsExpanded() ? "1.5rem" : 0,
                opacity: row.getIsAllParentsExpanded() ? 1 : 0,
            }}
        >
            <div>
                {row.getCanExpand() ? (
                    row.getIsExpanded() ? (
                        <IconChevronDown
                            className={styles.ExpandChevron}
                            size={"1.2rem"}
                            onClick={row.getToggleExpandedHandler()}
                        />
                    ) : (
                        <IconChevronRight
                            className={styles.ExpandChevron}
                            size={"1.2rem"}
                            onClick={row.getToggleExpandedHandler()}
                        />
                    )
                ) : (
                    <IconChevronRight
                        className={styles.ExpandChevron}
                        size={"1.2rem"}
                        style={{ opacity: 0 }}
                    />
                )}
            </div>
            {row.getVisibleCells().map(cell => (
                <div key={cell.id}>
                    {flexRender(cell.column.columnDef.cell, cell.getContext())}
                </div>
            ))}
        </div>
    );
}

function Body({ table }: { table: Table<Entry> }) {
    return table
        .getRowModel()
        .rows.map(row => <DataRow key={row.id} row={row} />);
}

const testData: Entry[] = [
    {
        path: "foo/bar",
        totalCovered: 12,
        totalReachable: 36,
        children: [
            { path: "baz.cpp", totalCovered: 10, totalReachable: 22 },
            {
                path: "lib",
                totalCovered: 2,
                totalReachable: 14,
                children: [
                    {
                        path: "util.cpp",
                        totalCovered: 2,
                        totalReachable: 14,
                    },
                ],
            },
        ],
    },
    { path: "src/main.cpp", totalCovered: 12, totalReachable: 36 },
];

export default function CoverageTable() {
    const [expanded, setExpanded] = useState<ExpandedState>(true);

    const table = useReactTable({
        data: testData,
        columns,
        state: { expanded },
        onExpandedChange: setExpanded,
        getSubRows: row => row.children,
        getCoreRowModel: getCoreRowModel(),
        getExpandedRowModel: getExpandedRowModelNoFilter(),
    });

    return (
        <div className={styles.CoverageTable}>
            <Head table={table} />
            <Body table={table} />
        </div>
    );
}

/**
 * Same as the default `getExpandedRowModel`,
 * except that non-expanded rows aren't actually filtered out.
 * Useful if you want to manually filter out those rows,
 * e.g. to implement animations.
 */
export function getExpandedRowModelNoFilter<TData extends RowData>(): (
    table: Table<TData>,
) => () => RowModel<TData> {
    return table =>
        tanStackTableMemo(
            () => [
                table.getState().expanded,
                table.getPreExpandedRowModel(),
                table.options.paginateExpandedRows,
            ],
            (expanded, rowModel, paginateExpandedRows) => {
                if (
                    !rowModel.rows.length ||
                    (expanded !== true && !Object.keys(expanded ?? {}).length)
                ) {
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

export function expandRows<TData extends RowData>(rowModel: RowModel<TData>) {
    const expandedRows: Row<TData>[] = [];

    const handleRow = (row: Row<TData>) => {
        expandedRows.push(row);

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
