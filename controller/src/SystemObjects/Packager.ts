import { Network } from "../utils";
import {SystemObject} from "./SystemObject";

export class Packager extends  SystemObject{

    async setup():Promise<boolean> {

        let body:any =  {
            "sid": "1"
        }
        let x=await Network.post({ url: `${this.baseUrl}/set` , body: body});

        return Promise.resolve(true);
    }
}
