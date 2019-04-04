import * as request from "request-promise-native";
import {SystemObject} from "./SystemObject";


export class Transcoder extends  SystemObject{

    setup():Promise<boolean> {
        return Promise.resolve(true);
    }

}
