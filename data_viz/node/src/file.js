var d3 = require("d3");

// create graph for voltage with dummy values
var voltage = new graph_table({id: "#voltage", 
                               title: "Voltages",
                               width: 1200,
                               height: 400, 
                               table_col_width: 75,
                               range: [0, 255],
                               points:[{f: (id) => {return true;}, v:(p) => {return p[1];}, l: "p1", pc: "green", sc: "blue"},
                                       {f: (id) => {return id%2 == 1;}, v:(p) => {return p[2];}, l: "p2", pc: "purple", sc: "orange"},
                                       {f: (id) => {return true;}, v:(p) => {return (Number(p[3]) + Number(p[4]))/2;}, l: "avg", pc: "grey", sc: "green"}]});
            
// create graph for current with dummy values
var current = new graph_table({id: "#current", 
                               title: "Currents",
                               width: 1200,
                               height: 400, 
                               table_col_width: 75,
                               range: [0, 300],
                               points:[{f: (p) => {return true;}, v:(p) => {return p[4];}, l: "p1", pc: "green", sc: "blue"},
                                       {f: (p) => {return p[0]%2 == 0;}, v:(p) => {return p[5];}, l: "p2", pc: "purple", sc: "orange"}]});

var ser = new port_select({funct: incoming_payload});

// add graph functions to serial callback function
function incoming_payload(payload)
{
    if (payload.trim().split(" ").length != 9) return;
    var timestamp = Date.now();
    voltage.add_data(timestamp, payload.trim().split(" "));
    current.add_data(timestamp, payload.trim().split(" "));
}