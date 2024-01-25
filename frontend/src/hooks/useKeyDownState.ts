import { useEffect, useState } from "react";

export default function useKeyDownState(key: string) {
    const [down, setDown] = useState(false);

    useEffect(() => {
        const downHandler = (ev: KeyboardEvent) =>
            ev.key === key && setDown(true);
        const upHandler = (ev: KeyboardEvent) =>
            ev.key === key && setDown(false);

        window.addEventListener("keydown", downHandler);
        window.addEventListener("keyup", upHandler);

        return () => {
            window.removeEventListener("keydown", downHandler);
            window.removeEventListener("keyup", upHandler);
        };
    }, []);

    return down;
}
