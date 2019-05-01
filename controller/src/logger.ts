import { configure, getLogger } from 'log4js';

configure({
    appenders: { 'out': { type: 'stdout' } },
    categories: { default: { appenders: ['out'], level: 'debug' } }
});

export class Logger
{
    prefix:string;
    internalLogger:any;

    constructor(category:string="",prefix:string="") {
        this.prefix=prefix;
        this.internalLogger=getLogger(category);
    }



    debug(...args) {
        args[0]="%s "+args[0];
        args.splice(1,0,this.prefix);
        return this.internalLogger.debug.apply(this.internalLogger, args);
    }
    info(...args) {
        args[0]="%s "+args[0];
        args.splice(1,0,this.prefix);
        return this.internalLogger.debug.apply(this.internalLogger, args);
    }
    warn(...args) {
        args[0]="%s "+args[0];
        args.splice(1,0,this.prefix);
        return this.internalLogger.debug.apply(this.internalLogger, args);
    }
    error(...args) {
        args[0]="%s "+args[0];
        args.splice(1,0,this.prefix);
        return this.internalLogger.debug.apply(this.internalLogger, args);
    }
}