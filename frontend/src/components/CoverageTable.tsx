import { Dialog, Transition } from "@headlessui/react";
import {
    IconArrowsSort,
    IconChevronDown,
    IconChevronRight,
    IconSortAscending,
    IconSortDescending,
} from "@tabler/icons-react";
import {
    CellContext,
    ExpandedState,
    Row,
    Table,
    TableMeta,
    createColumnHelper,
    flexRender,
    getCoreRowModel,
    useReactTable,
    getSortedRowModel,
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
    const pathSeparator = tableMeta.pathSeparator;

    const fullPath = [...ctx.row.getParentRows(), ctx.row]
        .map(row => row.original.path)
        .join(pathSeparator);

    const path = <code>{ctx.getValue()}</code>;
    return !ctx.row.originalSubRows?.length ? (
        <>
            <LinkButton onClick={() => setOpen(true)}>{path}</LinkButton>
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
                                            ctx.row.original.leafId!,
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
        path
    );
}

const helper = createColumnHelper<Scope>();
const columns = [
    helper.accessor("path", {
        header: "Source file",
        cell: ctx => <ScopePathCell ctx={ctx} />,
    }),
    helper.accessor("totalCovered", {
        header: "Covered",
        cell: ctx => <div className={styles.Number}>{ctx.getValue()}</div>,
    }),
    helper.accessor("totalReachable", {
        header: "Total",
        cell: ctx => <div className={styles.Number}>{ctx.getValue()}</div>,
    }),
    helper.accessor(scope => scope.totalCovered / scope.totalReachable, {
        header: "Percent",
        cell: ctx => (
            <div className={styles.Number}>
                {percentNumberFormat.format(ctx.getValue())}
            </div>
        ),
    }),
];

type CommonProps = { table: Table<Scope> };

function Head({ table }: CommonProps) {
    return (
        <div className={cls(styles.Row, styles.Head)}>
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
        getSortedRowModel: getSortedRowModel(),
        meta: {
            pathSeparator: props.pathSeparator,
            getCoverageLines: props.getCoverageLines,
        } satisfies CoverageTableMeta,
    });

    return (
        <div className={styles.CoverageTable}>
            <Head table={table} />
            <Body table={table} />
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
        scope.children = child.children;
    }
}
