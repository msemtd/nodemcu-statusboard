var express = require('express');
var app = express();
var port = 9912;
var boardval = 77;

app.get('/', function (req, res) {
  res.send('Hello!');
});

app.get('/statusboards/1/nodemcutest', function (req, res) {
  res.send('/statusboards/1/nodemcutest:'+boardval);
});

app.use(function(err, req, res, next) {
  console.error(err.stack);
  res.status(500).send('Something broke!');
});

app.listen(port, function () {
  console.log('app listening on port '+ port);
});

