/* 
TODO: -
* Get info from sources
* Web form page generator
* security - user logins, tokens etc.
*
*/
var port = 9912;
require('console-stamp')( console, {  } );
var express = require('express');
var app = express();
var path = require('path');
var favicon = require('serve-favicon');
app.disable('x-powered-by'); // security by obscurity: mask attack surface
app.use(favicon(path.join(__dirname, 'favicon.ico')));
app.locals.boards = require('./boards');
app.locals.boards.load();

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
    var bnum = req.params.board;
    var style = req.params.style;
    var remote = req.connection.remoteAddress;
    console.log('req for '+ bnum + ' on ' + req.originalUrl + ' with style ' + style + ' for ' + remote);
    // lookup board value and serve in chosen style
	var bval = app.locals.boards.getBoardVal(bnum-1);
    console.log('value for board '+ bnum + ' is ' + bval);
    if((style) && (style == 'nodemcu')) {
        // NodeMCU style plain text response
        var hexval = ("00" + bnum.toString(16)).slice(-2);
        res.send('BOARD:' + bnum + ':' + hexval);
    } else {
        // HTML style as a nice table
        avoidCache(res)
        res.send(getBoardHtml(bnum, bval));
    }
});
// GET setter - BAD API!!!!
// requires board id of digits and a hex value
sbr.get('/set/:board(\\d+)/:newval(0x[0-9a-fA-F]{2})', function(req, res, next){
    var bnum = req.params.board;
    var newval = parseInt(req.params.newval, 16);
    console.log('sb set '+ bnum + ' to ' + newval);
    if (isNaN(newval)) {
        next();
    } else {
		app.locals.boards.setBoardVal(bnum-1, newval);
        res.send('OK set board '+ bnum + ' to decimal ' + newval);
    }
});
app.use("/sb", sbr);

app.use(function(err, req, res, next) {
    console.error(err.stack);
    res.status(500).send('Something broke!');
});

app.listen(port, function () {
    console.log('app listening on port '+ port);
});

function getBoardHtml(bnum, val) {
    // get binary values from 2 hex digits
    var hexval = ("00" + val.toString(16)).slice(-2);
    var binval_str = ("00000000" + val.toString(2)).slice(-8);
    var rowtext = "";
    var bitc = 0;
    for (var v of binval_str) {
        rowtext += `
        <tr> <td>BIT ${bitc}</td> <td>${v}</td> </tr>
        `;
        bitc++;
    }
    var table = `
    <table border="1">
    <tr>
    <th>Item</th>
    <th>Value</th>
    </tr>
    ${rowtext}
    </table>
    `;
    var html = `
    <HTML>
    <head>
    <title>STATUSBOARD ${bnum}</title>
    </head>
    <BODY>
    <H1>STATUSBOARD ${bnum}</H1>
    <DIV>VALUE = 0x${hexval}</DIV>
    <DIV>VALUE(BIN) = ${binval_str}</DIV>
    ${table}
    </BODY>
    </HTML>
    `;
    return html;
}

function avoidCache(res) {
    res.setHeader("Cache-Control", "no-cache, no-store, must-revalidate"); // HTTP 1.1.
    res.setHeader("Pragma", "no-cache"); // HTTP 1.0.
    res.setHeader("Expires", "0"); // Proxies.
}