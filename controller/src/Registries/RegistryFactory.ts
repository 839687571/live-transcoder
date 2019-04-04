import { SingleServerRegistry } from "./SingleServerRegistry/SingleServerRegistry";
import { KubernetesRegistry } from "./KubernetesRegistry/KubernetesRegistry";
import {IRegistry} from "../interfaces/interfaces";


const config:any = { "registry": "SingleServerRegistry"}

export function getRegistry(): IRegistry {
    if (config.registry=== "SingleServerRegistry") {
        return new SingleServerRegistry();
    } else {
        return new KubernetesRegistry();
    }
}