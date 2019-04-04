import {EntryInfo} from './interfaces/interfaces'
import {
    AuthenticateRequest,
    BusinessLogic,
    SetupRequest,
    SetupResponse,
    TearDownRequest,
    TearDownResponse
} from './BusinessLogic'

import * as express from 'express'
import { configure, getLogger } from 'log4js';
import {KalturaAPIException} from "./KalturaAPI";
const logger = getLogger();
const  bodyParser = require('body-parser')

configure({
    appenders: { 'out': { type: 'stdout' } },
    categories: { default: { appenders: ['out'], level: 'debug' } }
});



process.on('uncaughtException', function (err: Error) {
    try {
        logger.error(err);
    } catch (err) {

    }
});
process.on('unhandledRejection', function (err: Error) {
    try {
        logger.error(err);
    } catch (err) {

    }
});

const asyncMiddleware = fn =>
    (req, res, next) => {
        Promise.resolve(fn(req, res, next))
            .catch(next);
    };



export class Application {

    app: any;
    server:any;
    bl: BusinessLogic;
    ready: Promise<any>;

    static instance:Application=new Application();

    constructor() {
        this.app = express();
        this.app.use(bodyParser.json());
        this.app.use(bodyParser.urlencoded({extended: false}));
        this.register();
        this.app.use((err, req, res, next)=> {
            if (err!=null) {
                res.status(200);
                res.send(err);
                logger.error(err);
                return
            }
            next();

        });

        this.bl=new BusinessLogic();
    }

    private register() {

        this.app.post('/authenticate', asyncMiddleware(async (req, res) => {
            let authRequest = req.body as AuthenticateRequest;
            let result: EntryInfo = await this.bl.authenticate(authRequest);
            res.send(result);
        }));

        this.app.post('/setup', asyncMiddleware(async (req, res) => {
            let setupRequest = req.body as SetupRequest;
            let result: SetupResponse = await this.bl.setup(setupRequest);

            res.send(result);
        }));

        this.app.post('/teardown', asyncMiddleware(async (req, res) => {
            let teardownRequest = req.body as TearDownRequest;
            let result: TearDownResponse = await this.bl.teardown(teardownRequest);
            res.send(result);
        }));

        this.app.post('/reportAlert', asyncMiddleware(async (req, res) => {
            res.send("{}");
        }));
    }

    start() {
        this.ready=new Promise( (resolve,reject)=> {
            this.server = this.app.listen(8084,  ()=> {
                let host = this.server.address().address;
                let port = this.server.address().port;

                logger.info("Example app listening at http://%s:%s", host, port);

                resolve();
            })
        })
    }
}


Application.instance.start();