import { common, createStarryNight } from "@wooorm/starry-night";
import { atom, useAtomValue } from "jotai";

const starryNightAtom = atom(() => createStarryNight(common));

export default function useStarryNight() {
    return useAtomValue(starryNightAtom);
}
