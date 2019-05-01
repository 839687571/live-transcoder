import { SingleServerRegistry } from "./SingleServerRegistry/SingleServerRegistry";
import { DistributedRegistry } from "./DistributedRegistry/DistributedRegistry";
import {ICache, IRegistry} from "../interfaces/interfaces";
import * as config from "config";
import {LocalCache} from "../LocalCache";

let registry:IRegistry = null;
let cache:ICache = null;

export function initializeRegistry(): Promise<boolean> {

    if (config.get("registry")=== "SingleServerRegistry") {
        registry = new SingleServerRegistry();
        cache=new LocalCache();
    } else {
        registry = new DistributedRegistry();
        cache=new LocalCache();
    }
    return registry.initialize();
}

export function getRegistry(): IRegistry {
    if (!registry) {
        throw new Error("Registry not initialized yet!");
    }
    return registry;
}
export function getCache(): ICache {
    if (!cache) {
        throw new Error("Cache not initialized yet!");
    }
    return cache;
}