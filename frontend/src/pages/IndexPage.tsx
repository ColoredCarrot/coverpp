import {Suspense} from "react";
import CodeBlock from "#/components/CodeBlock";

export default function IndexPage() {
    return <Suspense fallback={"Highlighting..."}>
        <CodeBlock content={'int main()\n{\n    return 1;\n}'}/>
    </Suspense>;
}
