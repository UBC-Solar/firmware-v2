var d3 = require("d3");
var sp = require("serialport");
var Readline = require("@serialport/parser-readline");

var ser_sel = d3.select("#sel").append("select").attr("class", "select");
var refresh = d3.select("#sel").append("button").attr("id", "refresh").text("Refresh").attr("onclick", "refresh_list()");
var connect = d3.select("#sel").append("button").attr("id", "connect").text("Connect").attr("onclick", "connect_to_port()");
var disconnect = d3.select("#sel").append("button").attr("id", "disconnect").text("Disconnect").attr("onclick", "manual_disconnect()").attr("disabled", true);
refresh_list();

var port = null;

function refresh_list()
{
    ser_sel.selectAll("option").remove();
    sp.list(
        function(err, ports)
        {
            ser_sel.selectAll('option').data(ports).enter().append('option').text(function(p) {return p.comName + " (" + p.manufacturer + ")";});
        }
    );
}

function connect_to_port()
{
    document.getElementById("refresh").disabled = true;
    document.getElementById("connect").disabled = true;
    document.getElementById("disconnect").disabled = false;
    
    var com = ser_sel.node().value.split("(")[0];
    
    port = new sp(com);
    port.on('close', function(err) {if (err != null) {disconnect_from_port();alert(err);}});
    
    var parser = port.pipe(new Readline({delimiter: '\r\n'}));
    parser.on('data', console.log);
}

function manual_disconnect()
{
    disconnect_from_port();
    port.close();
}

function disconnect_from_port()
{
    refresh_list();
    document.getElementById("refresh").disabled = false;
    document.getElementById("connect").disabled = false;
    document.getElementById("disconnect").disabled = true;
}