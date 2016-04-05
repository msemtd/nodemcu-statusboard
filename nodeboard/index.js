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
    if (bval === undefined) {
        next();
        return;
    }
    if((style) && (style == 'nodemcu')) {
        // NodeMCU style plain text response
        var hexval = hexByte(bval);
        res.send('BOARD:' + bnum + ':' + hexval);
    } else {
        // HTML style as a nice table
        avoidCache(res)
        var labels = app.locals.boards.getBoardLabels(bnum-1);
        res.send(getBoardHtml(bnum, bval, labels));
    }
});
// GET setter - BAD API!!!!
// requires board id of digits and a hex value
sbr.get('/set/:board(\\d+)/:newval(0x[0-9a-fA-F]{2})', function(req, res, next){
    var bnum = req.params.board;
    var newval = parseInt(req.params.newval, 16);
    var hexval = hexByte(newval);
    console.log('sb set '+ bnum + ' to ' + newval + ' = hex ' + hexval);
    if (isNaN(newval)) {
        next();
        return;
    } else {
		app.locals.boards.setBoardVal(bnum-1, newval);
        res.send('OK set board '+ bnum + ' to decimal ' + newval + ' = hex ' + hexval);
        app.locals.boards.save();
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

function getBoardHtml(bnum, val, labels) {
    // get binary values from 2 hex digits
    var hexval = hexByte(val);
    var binval_str = binByte(val);
    var rowtext = "";
    for (var i = 0; i < 8; i++) {
        var b = binval_str.charAt(7-i);
        var lab = labels ? labels[i] : "Not Set";
        var cla = (b == '1') ? 'binon' : 'binoff';
        rowtext += `<tr> <td>BIT ${i}</td> <td>${lab}</td>  <td class="${cla}">${b}</td> </tr>
        `;
    }
    var css = `
        body {background-color: lightgrey; color: #000050; border-width: 3em; font-family: Arial,Helvetica,sans-serif;}
        h1   {color: blue;}
        p    {color: green;}
        table.bins {border: 1px solid black; border-spacing: 5px;}
        th, td { padding: 5px; }
        td.binon  {background-color: #33FF33;}
        td.binoff  {background-color: red;}
    `;
    var table = `
    <table class="bins">
    <tr>
    <th>Item</th>
    <th>Label</th>
    <th>Value</th>
    </tr>
    ${rowtext}
    </table>
    `;
    var html = `
    <HTML>
    <head>
    <title>STATUSBOARD ${bnum}</title>
    <style>
    ${css}
    </style>
    </head>
    <BODY>
    <H1>STATUSBOARD ${bnum}</H1>
    <DIV>
    <pre>Value (HEX) = 0x${hexval}</pre>
    <pre>Value (BIN) = ${binval_str}</pre>
    </DIV>
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

function hexByte(x) {
    x = Number(x);
    if (Number.isNaN(x)) x = 0;    
    return ("00" + x.toString(16).toUpperCase()).slice(-2);
}

function binByte(x) {
    x = Number(x);
    if (Number.isNaN(x)) x = 0;    
    return ("00000000" + x.toString(2)).slice(-8);
}
