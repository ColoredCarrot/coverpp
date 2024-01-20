import styles from "./CoverageTable.module.css";
import { IconChevronDown, IconChevronRight } from "@tabler/icons-react";
import {
    createColumnHelper,
    ExpandedState,
    flexRender,
    getCoreRowModel,
    Row,
    Table,
    useReactTable,
} from "@tanstack/react-table";
import { useState } from "react";
import cls from "#/util/cls";
import getExpandedRowModelNoFilter from "#/util/getExpandedRowModelNoFilter";

export type Scope = {
    path: string;
    totalCovered: number;
    totalReachable: number;
    children?: Scope[];
};

const percentNumberFormat = new Intl.NumberFormat("en-US", {
    style: "percent",
    minimumFractionDigits: 1,
    maximumFractionDigits: 1,
});

const helper = createColumnHelper<Scope>();
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

type CommonProps = { table: Table<Scope> };

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

function DataRow({ row }: { row: Row<Scope> }) {
    return (
        <div
            className={cls(styles.Row, [
                styles.contracted,
                !row.getIsAllParentsExpanded(),
            ])}
            style={{ "--indent": row.depth }}
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
                        className={styles.HiddenChevron}
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

function Body({ table }: { table: Table<Scope> }) {
    return table
        .getRowModel()
        .rows.map(row => <DataRow key={row.id} row={row} />);
}

export default function CoverageTable(props: { scopes: Scope[] }) {
    const [expanded, setExpanded] = useState<ExpandedState>(true);

    const table = useReactTable({
        data: props.scopes,
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
