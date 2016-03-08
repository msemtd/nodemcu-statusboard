/* 
TODO: -
* Get info from sources
* Web form page generator
* multiple boards
* cache values between runs or just get from source again
* security - user logins, tokens etc.
*
*/
require('console-stamp')( console, {  } );
var express = require('express');
var app = express();
var path = require('path');
var port = 9912;
var favicon = require('serve-favicon');
app.disable('x-powered-by');
app.use(favicon(path.join(__dirname, 'favicon.ico')));
app.locals.boards = require('./boards');
app.locals.boards.load();

// TODO: currently only supporting one board
var boardval = 77;

app.get('/', function (req, res) {
    console.log('serving root');
    res.send('Hello! Nothing to see here!');
});

// This is an attempt at a better express middleware to handle any URL starting "/sb"
// as a REST-ful API or somesuch - TODO 
// For handy tester see http://forbeslindesay.github.io/express-route-tester/
var sbr = express.Router();
// GET handler for board number
sbr.get('/:board(\\d+)/:style?', function(req, res, next){
    var board = req.params.board;
    var style = req.params.style;
    console.log('sbr for '+ board + ' on ' + req.originalUrl + ' with style ' + style);
    // TODO lookup board value and serve in style
    if((style) && (style == 'nodemcu')) {
        res.send('TADA for ' + req.baseUrl + ':::' + board + '===0x' + boardval.toString(16));
    } else {
        // TODO dish up default HTML style
        next();
    }
});
// GET setter - BAD API!!!!
// requires board id of digits and a hex value
sbr.get('set/:board(\\d+)/:newval(0x[0-9a-fA-F]{2})', function(req, res, next){
    var board = req.params.board;
    var newval = parseInt(req.params.newval, 10);
    console.log('sbr set '+ board + ' to ' + newval);
    // TODO lookup board value and serve in style
    if (isNaN(newval)) {
        next();
    } else {
        boardval = newval;
        res.send('OK set board '+ board + ' to ' + newval);
    }
});
app.use("/sb", sbr);

app.get('/statusboards/1/nodemcutest', function (req, res) {
    console.log('serving nodemcu');
    res.send('/statusboards/1/nodemcutest:'+boardval);
});

app.get(/^\/statusboards\/set\/(\d+)\/(\d+)$/, function (req, res) {
    var board = req.params[0];
    var val = req.params[1];
    console.log("setting a board status: '"+ board + "' to '" + val + "'");
    boardval = val;
    res.send(' OK '+'/statusboards set to '+boardval);
});

app.use(function(err, req, res, next) {
    console.error(err.stack);
    res.status(500).send('Something broke!');
});

app.listen(port, function () {
    console.log('app listening on port '+ port);
});

