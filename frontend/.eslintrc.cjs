module.exports = {
    rules: {
        "react-refresh/only-export-components": "warn",
        "@typescript-eslint/no-namespace": "off",

        "sort-imports": ["warn", { ignoreDeclarationSort: true }],
        "import/order": [
            "warn",
            {
                "newlines-between": "never",
                alphabetize: { order: "asc", caseInsensitive: true },
                warnOnUnassignedImports: true,
            },
        ],
    },
    reportUnusedDisableDirectives: true,
    ignorePatterns: ["dist/*"],
    env: { browser: true, es2020: true, node: true },
    parserOptions: { ecmaVersion: "latest", sourceType: "module" },
    settings: {
        react: { version: "detect" },
        "import/resolver": { typescript: true },
    },
    plugins: ["react-refresh"],
    extends: [
        "eslint:recommended",
        "plugin:react/recommended",
        "plugin:react/jsx-runtime",
        "plugin:react-hooks/recommended",
        "plugin:@typescript-eslint/recommended",
        "plugin:import/recommended",
        "plugin:import/typescript",
        "prettier",
    ],
    parser: "@typescript-eslint/parser",
};
