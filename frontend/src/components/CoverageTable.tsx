import { Dialog, Transition } from "@headlessui/react";
import {
    IconArrowsSort,
    IconChevronDown,
    IconChevronRight,
    IconSearch,
    IconSortAscending,
    IconSortDescending,
} from "@tabler/icons-react";
import {
    CellContext,
    ExpandedState,
    FilterFn,
    Row,
    Table,
    TableMeta,
    createColumnHelper,
    flexRender,
    getCoreRowModel,
    getFilteredRowModel,
    getSortedRowModel,
    useReactTable,
} from "@tanstack/react-table";
import { Fragment, useMemo, useState } from "react";
import styles from "./CoverageTable.module.css";
import LinkButton from "#/components/LinkButton";
import RemoteCoverageCodeBlock from "#/components/RemoteCoverageCodeBlock";
import { LineCoverage } from "#/coverage/FileCoverage";
import cls from "#/util/cls";
import getExpandedRowModelNoFilter from "#/util/getExpandedRowModelNoFilter";

export type Scope = {
    path: string;
    fullPath: string;
    totalCovered: number;
    totalReachable: number;
    children?: Scope[];
    leafId?: number;
};

const percentNumberFormat = new Intl.NumberFormat("en-US", {
    style: "percent",
    minimumFractionDigits: 1,
    maximumFractionDigits: 1,
});

interface CoverageTableMeta extends TableMeta<Scope> {
    pathSeparator: string;

    getCoverageLines(leafId: number): LineCoverage[];
}

function ScopePathCell({ ctx }: { ctx: CellContext<Scope, string> }) {
    const [open, setOpen] = useState(false);

    const tableMeta = ctx.table.options.meta as CoverageTableMeta;

    const fullPath = ctx.row.original.fullPath;
    const leafId = ctx.row.original.leafId;

    return leafId !== undefined ? (
        <>
            <LinkButton onClick={() => setOpen(true)}>
                {<code>{ctx.getValue()}</code>}
            </LinkButton>
            <Transition show={open} as={Fragment}>
                <Dialog
                    onClose={() => setOpen(false)}
                    className={styles.Dialog}
                >
                    <Transition.Child
                        as={Fragment}
                        enter={styles.DialogTransitionBackdrop_enter}
                        enterFrom={styles.DialogTransitionBackdrop_enterFrom}
                        enterTo={styles.DialogTransitionBackdrop_enterTo}
                        leave={styles.DialogTransitionBackdrop_leave}
                        leaveFrom={styles.DialogTransitionBackdrop_leaveFrom}
                        leaveTo={styles.DialogTransitionBackdrop_leaveTo}
                    >
                        <div className={styles.DialogBackdrop} aria-hidden />
                    </Transition.Child>

                    <div className={styles.DialogPanelContainer}>
                        <Transition.Child
                            as={Fragment}
                            enter={styles.DialogTransition_enter}
                            enterFrom={styles.DialogTransition_enterFrom}
                            enterTo={styles.DialogTransition_enterTo}
                            leave={styles.DialogTransition_leave}
                            leaveFrom={styles.DialogTransition_leaveFrom}
                            leaveTo={styles.DialogTransition_leaveTo}
                        >
                            <Dialog.Panel className={styles.DialogPanel}>
                                <Dialog.Title className={styles.DialogTitle}>
                                    <code>{fullPath}</code>
                                </Dialog.Title>
                                <RemoteCoverageCodeBlock
                                    coverage={{
                                        filePath: fullPath,
                                        lines: tableMeta.getCoverageLines(
                                            leafId,
                                        ),
                                    }}
                                />
                            </Dialog.Panel>
                        </Transition.Child>
                    </div>
                </Dialog>
            </Transition>
        </>
    ) : (
        <code>{ctx.getValue()}</code>
    );
}

const pathFilterFn: FilterFn<Scope> = (row, _, filterValue: string) => {
    return row.original.fullPath
        .toLowerCase()
        .includes(filterValue.toLowerCase());
};

const helper = createColumnHelper<Scope>();
const columns = [
    helper.accessor("path", {
        header: "Source file",
        cell: ctx => <ScopePathCell key={ctx.cell.id} ctx={ctx} />,
        filterFn: pathFilterFn,
    }),
    helper.accessor("totalCovered", {
        header: "Covered",
        cell: ctx => <div className={styles.Number}>{ctx.getValue()}</div>,
        enableColumnFilter: false,
    }),
    helper.accessor("totalReachable", {
        header: "Total",
        cell: ctx => <div className={styles.Number}>{ctx.getValue()}</div>,
        enableColumnFilter: false,
    }),
    helper.accessor(scope => scope.totalCovered / scope.totalReachable, {
        header: "Percent",
        cell: ctx => (
            <div className={styles.Number}>
                {percentNumberFormat.format(ctx.getValue())}
            </div>
        ),
        enableColumnFilter: false,
    }),
];

type CommonProps = { table: Table<Scope> };

function Head({ table }: CommonProps) {
    return (
        <div className={styles.Head}>
            <Headers table={table} />
            <Filter table={table} />
        </div>
    );
}

function Headers({ table }: CommonProps) {
    return (
        <div className={styles.Row}>
            <div />
            {table.getLeafHeaders().map(header => {
                const sorted = header.column.getIsSorted();
                const SortIcon =
                    sorted === false
                        ? IconArrowsSort
                        : sorted === "asc"
                          ? IconSortAscending
                          : IconSortDescending;

                const sortIconColor = sorted
                    ? "var(--color-sort-icon-active)"
                    : undefined;

                return (
                    <div
                        key={header.id}
                        className={styles.HeadColumn}
                        onClick={header.column.getToggleSortingHandler()}
                    >
                        <SortIcon size={"1rem"} color={sortIconColor} />
                        <div style={{ width: "0.2rem" }} />
                        {flexRender(
                            header.column.columnDef.header,
                            header.getContext(),
                        )}
                    </div>
                );
            })}
        </div>
    );
}

function Filter({ table }: CommonProps) {
    return (
        <div className={styles.Row}>
            <div>
                <IconSearch className={styles.RowIcon} />
            </div>

            {table.getLeafHeaders().map(header => (
                <div key={header.id}>
                    {header.column.getCanFilter() && (
                        <input
                            type="text"
                            className={styles.FilterInput}
                            value={
                                (header.column.getFilterValue() ?? "") as string
                            }
                            onChange={e =>
                                header.column.setFilterValue(
                                    e.currentTarget.value,
                                )
                            }
                        />
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
                styles.collapsed,
                !row.getIsAllParentsExpanded(),
            ])}
            style={{ "--indent": row.depth }}
        >
            <div>
                {row.getCanExpand() ? (
                    row.getIsExpanded() ? (
                        <IconChevronDown
                            className={cls(
                                styles.ExpandChevron,
                                styles.RowIcon,
                            )}
                            onClick={row.getToggleExpandedHandler()}
                        />
                    ) : (
                        <IconChevronRight
                            className={cls(
                                styles.ExpandChevron,
                                styles.RowIcon,
                            )}
                            onClick={row.getToggleExpandedHandler()}
                        />
                    )
                ) : (
                    <IconChevronRight
                        className={cls(styles.HiddenChevron, styles.RowIcon)}
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

export default function CoverageTable(props: {
    scopes: Scope[];
    pathSeparator: string;
    flattenSingleChildScopes?: boolean;
    getCoverageLines: (leafId: number) => LineCoverage[];
}) {
    const [expanded, setExpanded] = useState<ExpandedState>(true);

    const data = useMemo(() => {
        if (!(props.flattenSingleChildScopes ?? true)) {
            return props.scopes;
        }
        const flattened = props.scopes.slice();
        flattenScopeTree(flattened, props.pathSeparator);
        return flattened;
    }, [props.scopes, props.pathSeparator, props.flattenSingleChildScopes]);

    const table = useReactTable({
        data,
        columns,
        state: { expanded },
        onExpandedChange: setExpanded,
        getSubRows: row => row.children,
        getCoreRowModel: getCoreRowModel(),
        getExpandedRowModel: getExpandedRowModelNoFilter(),
        getFilteredRowModel: getFilteredRowModel(),
        getSortedRowModel: getSortedRowModel(),
        filterFromLeafRows: true,
        meta: {
            pathSeparator: props.pathSeparator,
            getCoverageLines: props.getCoverageLines,
        } satisfies CoverageTableMeta,
    });

    const numRowsFilteredOut =
        table.getPreFilteredRowModel().flatRows.length -
        table.getFilteredRowModel().flatRows.length;

    const filteredOutWarning =
        numRowsFilteredOut > 0 ? (
            <>
                <div style={{ height: "0.6rem" }} />
                <div className={styles.Row}>
                    <div />
                    <div className={styles.RowsHiddenWarningText}>
                        {numRowsFilteredOut}{" "}
                        {numRowsFilteredOut === 1 ? "row" : "rows"} hidden
                    </div>
                </div>
            </>
        ) : undefined;

    return (
        <div className={styles.CoverageTable}>
            <Head table={table} />
            <Body table={table} />
            {filteredOutWarning}
        </div>
    );
}

/**
 * Flatten the given scope tree.
 *
 * Specifically, if a scope contains only one child, the parent scope
 * is merged with the child scope (their paths are concatenated).
 *
 * @param scopes Scope tree to flatten
 * @param pathSeparator Separator to be placed between concatenated paths
 */
function flattenScopeTree(scopes: Scope[], pathSeparator: string) {
    for (const scope of scopes) {
        if (scope.children === undefined) {
            continue;
        }

        flattenScopeTree(scope.children, pathSeparator);

        if (scope.children.length !== 1) {
            continue;
        }
        const child = scope.children[0];

        // Merge scope with its child
        scope.path += pathSeparator + child.path;
        scope.fullPath = child.fullPath;
        scope.children = child.children;
    }
}
