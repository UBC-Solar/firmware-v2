var express    = require("express");
var bodyParser = require("body-parser");
var app        = express();
var server     = require("http").Server(app);
const io       = require('socket.io')(server);

app.use(bodyParser.urlencoded({extended: false}));
app.use(bodyParser.json());

server.listen(process.env.PORT || 5000, () => {console.log("Started");});

app.get('/', (req, res) => {res.sendFile(__dirname + "/index.html")});

app.post('/data', (req, res) =>
  {
    console.log("New data");
    console.log(req.body.payload);

    //TODO implement payload parsing on server
    io.emit("data", req.body.payload);

    res.end();
  });

io.on('connection', socket => {
  console.log("New user connected");
});