import { routes } from "#/routes";
import CoverageReport from "#/components/CoverageReport";

export default function IndexPage() {
    const data = routes.index.useLoaderData();

    if (data.status === "error") {
        return <p>{data.statusCode + " " + data.statusText}</p>;
    }

    return <CoverageReport report={data.report} />;
}
