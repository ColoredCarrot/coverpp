import { IconMoonFilled, IconSunFilled } from "@tabler/icons-react";
import { ReactNode } from "@tanstack/react-router";
import styles from "./GlobalThemeToggle.module.css";
import {
    Theme,
    toggleGlobalTheme,
    useGlobalTheme,
} from "#/hooks/useGlobalTheme";
import cls from "#/util/cls";

const iconMap: Record<Theme, ReactNode> = {
    light: <IconSunFilled className={cls(styles.Icon, styles.IconSun)} />,
    dark: <IconMoonFilled className={cls(styles.Icon, styles.IconMoon)} />,
} as const;

export default function GlobalThemeToggle() {
    const currentTheme = useGlobalTheme();

    return (
        <div className={styles.GlobalThemeToggle} onClick={toggleGlobalTheme}>
            {iconMap[currentTheme]}
        </div>
    );
}
