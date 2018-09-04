var d3 = require("d3");
var sp = require("serialport");
var Readline = require("@serialport/parser-readline");

class port_select
{
    constructor(opt)
    {
        this.ser_sel    = d3.select("#sel").append("select").attr("class", "select");
        this.refresh    = d3.select("#sel").append("button").attr("id", "refresh").text("Refresh").on("click", () => {this.refresh_list();});
        this.connect    = d3.select("#sel").append("button").attr("id", "connect").text("Connect").on("click", () => {this.connect_to_port();});
        this.disconnect = d3.select("#sel").append("button").attr("id", "disconnect").text("Disconnect").on("click", () => {this.manual_disconnect();}).attr("disabled", true);
        
        this.port   = null;
        this.parser = null;
        
        this.refresh_list();
        this.incoming = opt.funct;
    }
    
    refresh_list()
    {
        this.ser_sel.selectAll("option").remove();
        //log_event(ACTION, "List Refreshed.");
        sp.list((err, ports) => {this.ser_sel.selectAll('option').data(ports).enter().append('option').text((p) => {return p.comName + " (" + p.manufacturer + ")";})});
    }
    
    connect_to_port()
    {
        this.refresh.node().disabled    = true;
        this.connect.node().disabled    = true;
        this.disconnect.node().disabled = false;
        
        var com = this.ser_sel.node().value.split("(")[0];
        
        //log_event(ACTION, "Connecting to " + com);
        this.port = new sp(com);
        this.port.on('close', function(err) {if (err != null) {this.disconnect_from_port(); alert(err); /*log_event(ERROR, com + " unexpectedly disconnected.")*/}});
        this.parser = this.port.pipe(new Readline({delimiter: '\r\n'}));
        this.parser.on('data', this.incoming);
        //log_event(EVENT, "Connected to " + com);
    }
    
    manual_disconnect()
    {
        this.disconnect_from_port();
        this.port.close();
        // log_event(ACTION, "Port Disconnected");
    }
    
    disconnect_from_port()
    {
        this.refresh_list();
        this.refresh.node().disabled = false;
        this.connect.node().disabled = false;
        this.disconnect.node().disabled = true;
    }
}