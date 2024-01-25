import { Dialog, Transition } from "@headlessui/react";
import {
    IconArrowsSort,
    IconChevronDown,
    IconChevronRight,
    IconChevronsDown,
    IconChevronsRight,
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
import { Fragment, Ref, forwardRef, useMemo, useState } from "react";
import styles from "./CoverageTable.module.css";
import LinkButton, { InlineLinkButton } from "#/components/LinkButton";
import RemoteCoverageCodeBlock from "#/components/RemoteCoverageCodeBlock";
import { LineCoverage } from "#/coverage/FileCoverage";
import useKeyDownState from "#/hooks/useKeyDownState";
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

    flattenCompletely: boolean;

    shiftDown: boolean;
}

const CoverageLinesDialogPanel = forwardRef(function CoverageLinesDialogPanel(
    { ctx }: { ctx: CellContext<Scope, string> },
    ref: Ref<HTMLDivElement>,
) {
    const tableMeta = ctx.table.options.meta as CoverageTableMeta;

    const fullPath = ctx.row.original.fullPath;
    const leafId = ctx.row.original.leafId!;

    return (
        <Dialog.Panel className={styles.DialogPanel} ref={ref}>
            <Dialog.Title className={styles.DialogTitle}>
                <code>{fullPath}</code>
            </Dialog.Title>
            <RemoteCoverageCodeBlock
                coverage={{
                    filePath: fullPath,
                    lines: tableMeta.getCoverageLines(leafId),
                }}
            />
        </Dialog.Panel>
    );
});

function ScopePathCell({ ctx }: { ctx: CellContext<Scope, string> }) {
    const [open, setOpen] = useState(false);

    const leafId = ctx.row.original.leafId;

    return leafId !== undefined ? (
        <>
            <InlineLinkButton onClick={() => setOpen(true)}>
                {
                    <code>
                        {(ctx.table.options.meta as CoverageTableMeta)
                            .flattenCompletely
                            ? ctx.row.original.fullPath
                            : ctx.getValue()}
                    </code>
                }
            </InlineLinkButton>
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
                            <CoverageLinesDialogPanel ctx={ctx} />
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
    helper.accessor(scope => scope.totalReachable - scope.totalCovered, {
        header: "Miss.",
        cell: ctx => <div className={styles.Number}>{ctx.getValue()}</div>,
        enableColumnFilter: false,
    }),
    helper.accessor(
        scope =>
            scope.totalReachable === 0
                ? 1
                : scope.totalCovered / scope.totalReachable,
        {
            header: "Percent",
            cell: ctx => (
                <div className={styles.Number}>
                    {percentNumberFormat.format(ctx.getValue())}
                </div>
            ),
            enableColumnFilter: false,
        },
    ),
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
                        <div style={{ width: "0.2rem", height: 0 }} />
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

function fullyExpandOrContractRow(row: Row<Scope>, expanded: boolean) {
    row.toggleExpanded(expanded);
    for (const subRow of row.subRows) {
        fullyExpandOrContractRow(subRow, expanded);
    }
}

function ExpandRowIcon({
    row,
    shiftDown,
}: {
    row: Row<Scope>;
    shiftDown: boolean;
}) {
    if (!row.getCanExpand()) {
        return (
            <IconChevronRight
                className={cls(styles.HiddenChevron, styles.RowIcon)}
            />
        );
    }

    const iconProps = {
        className: cls(styles.ExpandChevron, styles.RowIcon),
    };
    return row.getIsExpanded() ? (
        shiftDown ? (
            <IconChevronsDown
                {...iconProps}
                onClick={() => fullyExpandOrContractRow(row, false)}
            />
        ) : (
            <IconChevronDown
                {...iconProps}
                onClick={row.getToggleExpandedHandler()}
            />
        )
    ) : shiftDown ? (
        <IconChevronsRight
            {...iconProps}
            onClick={() => fullyExpandOrContractRow(row, true)}
        />
    ) : (
        <IconChevronRight
            {...iconProps}
            onClick={row.getToggleExpandedHandler()}
        />
    );
}

function DataRow({ row, shiftDown }: { row: Row<Scope>; shiftDown: boolean }) {
    return (
        <div
            className={cls(styles.Row, [
                styles.collapsed,
                !row.getIsAllParentsExpanded(),
            ])}
            style={{ "--indent": row.depth }}
        >
            <div>
                <ExpandRowIcon row={row} shiftDown={shiftDown} />
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
        .rows.map(row => (
            <DataRow
                key={row.id}
                row={row}
                shiftDown={(table.options.meta as CoverageTableMeta).shiftDown}
            />
        ));
}

function TitleRow({
    table,
    ...props
}: {
    table: Table<Scope>;
    flattenCompletely: boolean;
    setFlattenCompletely: (flattenCompletely: boolean) => void;
}) {
    const flattenCompletelyButton = props.flattenCompletely ? (
        <LinkButton onClick={() => props.setFlattenCompletely(false)}>
            Coarsen
        </LinkButton>
    ) : (
        <LinkButton onClick={() => props.setFlattenCompletely(true)}>
            Flatten
        </LinkButton>
    );

    return (
        <div className={styles.TitleRow}>
            <LinkButton onClick={() => table.toggleAllRowsExpanded(true)}>
                Expand all
            </LinkButton>
            <LinkButton onClick={() => table.toggleAllRowsExpanded(false)}>
                Contract all
            </LinkButton>
            {flattenCompletelyButton}
        </div>
    );
}

export default function CoverageTable(props: {
    scopes: Scope[];
    pathSeparator: string;
    flattenSingleChildScopes?: boolean;
    getCoverageLines: (leafId: number) => LineCoverage[];
}) {
    const [flattenCompletely, setFlattenCompletely] = useState(false);

    const [expanded, setExpanded] = useState<ExpandedState>(true);

    const shiftDown = useKeyDownState("Shift");

    const data = useMemo(() => {
        if (!(props.flattenSingleChildScopes ?? true)) {
            return props.scopes;
        }
        return flattenCompletely
            ? flattenScopeTreeCompletely(props.scopes)
            : flattenScopeTree(props.scopes, props.pathSeparator);
    }, [
        props.scopes,
        props.pathSeparator,
        props.flattenSingleChildScopes,
        flattenCompletely,
    ]);

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
            flattenCompletely,
            shiftDown,
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
            <TitleRow
                table={table}
                flattenCompletely={flattenCompletely}
                setFlattenCompletely={setFlattenCompletely}
            />
            <Head table={table} />
            <Body table={table} />
            {filteredOutWarning}
        </div>
    );
}

function flattenScopeTree(
    scopes: readonly Scope[],
    pathSeparator: string,
): Scope[] {
    const flattened = scopes.slice();
    flattenScopeTreeInplace(flattened, pathSeparator);
    return flattened;
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
function flattenScopeTreeInplace(scopes: Scope[], pathSeparator: string) {
    for (const scope of scopes) {
        if (scope.children === undefined) {
            continue;
        }

        flattenScopeTreeInplace(scope.children, pathSeparator);

        if (scope.children.length !== 1) {
            continue;
        }
        const child = scope.children[0];

        // Merge scope with its child
        scope.path += pathSeparator + child.path;
        scope.fullPath = child.fullPath;
        scope.children = child.children;
        scope.leafId = child.leafId;
    }
}

function flattenScopeTreeCompletely(scopes: readonly Scope[]): Scope[] {
    const leafs: Scope[] = [];
    findLeafs(scopes, leafs);
    return leafs;
}

function findLeafs(scopes: readonly Scope[], result: Scope[]): void {
    for (const scope of scopes) {
        if (scope.leafId !== undefined) {
            result.push(scope);
        }
        if (scope.children !== undefined) {
            findLeafs(scope.children, result);
        }
    }
}
