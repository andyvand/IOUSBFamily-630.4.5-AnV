/*! @typedef IOAsyncCallback0
    @abstract standard callback function for asynchronous I/O requests with
    no extra arguments beyond a refcon and result code.
    @param refcon The refcon passed into the original I/O request
    @param result The result of the I/O operation
*/
typedef void (*IOAsyncCallback0)(void *refcon, IOReturn result);

/*! @typedef IOAsyncCallback1
    @abstract standard callback function for asynchronous I/O requests with
    one extra argument beyond a refcon and result code.
    This is often a count of the number of bytes transferred
    @param refcon The refcon passed into the original I/O request
    @param result The result of the I/O operation
    @param arg0	Extra argument
*/
typedef void (*IOAsyncCallback1)(void *refcon, IOReturn result, void *arg0);

/*! @typedef IOAsyncCallback2
    @abstract standard callback function for asynchronous I/O requests with
    two extra arguments beyond a refcon and result code.
    @param refcon The refcon passed into the original I/O request
    @param result The result of the I/O operation
    @param arg0	Extra argument
    @param arg1	Extra argument
*/
typedef void (*IOAsyncCallback2)(void *refcon, IOReturn result, void *arg0, void *arg1);

/*! @typedef IOAsyncCallback
    @abstract standard callback function for asynchronous I/O requests with
    lots of extra arguments beyond a refcon and result code.
    @param refcon The refcon passed into the original I/O request
    @param result The result of the I/O operation
    @param args	Array of extra arguments
    @param numArgs Number of extra arguments
*/
typedef void (*IOAsyncCallback)(void *refcon, IOReturn result, void **args,
                                uint32_t numArgs);
