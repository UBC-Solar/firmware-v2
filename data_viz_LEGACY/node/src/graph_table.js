var d3 = require("d3");

class graph_table
{
    /**
     *  Constructor of the graph_table class
     * 
     * @param {dict} opt:
     *      options/configurations for the graph_table. Each graph_table must have the 
     *      following options specified in this parameter:
     *          - id: id of the div that holds the graph_table
     *          - width: full width of the graph_table
     *          - height: full height of the graph_table
     *          - title: title of the graph_table
     *          - table_col_width: width of each column of the table. By default, the first 
     *                             column (timestamp) is 100px wide, so this option sets the 
     *                             width of the other columns of the table.
     *          - range: Range of the values of the data types. Every data type tracked and 
     *                   visualized with the same graph_table should have the same range.
     *          - points: List of dictionaries that holds information of each type of data to 
     *                    be recorded and visualized. Each dictionary should have the 
     *                    following key-value pairs:
     *                      - filter function (f): function that accepts the CAN payload ID 
     *                                             as a parameter returns whether or not the 
     *                                             CAN payload contains valid information 
     *                                             about the data type. (Normally, only 1 CAN 
     *                                             id would give information about 1 data  
     *                                             type, e.g voltage or current of a certain 
     *                                             cell)
     *                      - value function (v): function that accepts the CAN payload 
     *                                            (payload[0] is id, payload[1]-payload[8] is 
     *                                            bytes 0-7,) and returns the calculated 
     *                                            value of the tracked data
     *                      - label (l): data label (name of the data for the table)
     *                      - primary color (pc): color of the data on the graph unit of the 
     *                                            graph
     *                      - secondary color (sc): color of the data on the scroll unit of 
     *                                              the graph
     *
     */
    constructor(opt)
    {
        // define constants
        const HEADER_HEIGHT = 30;
        const FOOTER_HEIGHT = 25;
        const TIME_COL_WIDTH = 100;

        // add this keyword to the end of all all divs defined in this class to make it unique
        var id_end = opt.id.replace("#", "_");

        // create and handle divs
        // size main div
        d3.select(opt.id).style("width", opt.width).style("height", opt.height);

        // add header div
        d3.select(opt.id).append("div").attr("id", "header" + id_end).style("width", opt.width).style("height", HEADER_HEIGHT).append("h2").style("margin", 0).style("border", "none").text(opt.title).style("text-align", "center"); 
        
        // add divs for scroll graph and table (sizing handled by class constructors, only need to handle float)
        d3.select(opt.id).append("div").attr("id", "scroll_graph" + id_end).style("float", "left");
        d3.select(opt.id).append("div").attr("id", "table" + id_end).style("float", "right");

        // add footer for global buttons (autoscroll, clear data)
        d3.select(opt.id).append("div").attr("id", "footer" + id_end).style("width", opt.width).style("height", FOOTER_HEIGHT).style("text-align", "left");
        
        // save point info
        this.points = opt.points;

        // calculate width of table using default column width for the timestamp column and the user-defined column width
        var table_width = TIME_COL_WIDTH + this.points.length*opt.table_col_width;
        
        // create scroll graph
        this.scroll_graph = new scroll_graph({id: "#scroll_graph" + id_end, 
                                              width: opt.width - table_width,
                                              height: opt.height - FOOTER_HEIGHT - HEADER_HEIGHT, 
                                              margin: {top:10, left: 50, right: 10, bottom: 30}, 
                                              p_color: opt.points.map((d) => {return d.pc;}), 
                                              s_color: opt.points.map((d) => {return d.sc;}), 
                                              highlight_color: "rgb(255, 0, 0)", // must pass rgb values for transition interpolations
                                              range: opt.range,
                                              click_event: null}); // for now, initalize click_event with null (will be replaced by wrapper function that calls both scroll_graph and table functions)

        // create table
        this.table = new timetable({id: "#table" + id_end, 
                                    width: table_width,
                                    height: opt.height - FOOTER_HEIGHT - HEADER_HEIGHT,
                                    headers: opt.points.map((d) => {return d.l;}), 
                                    highlight_color: "rgb(255, 255, 0)", // must pass rgb values for transition interpolations
                                    click_event: null, // for now, initalize click_event with null (will be replaced by wrapper function that calls both scroll_graph and table functions)
                                    column_width: opt.table_col_width}); // first column (time) fixed to 100

        // after both objects are created, event callbacks of both objects can be defined
        this.scroll_graph.click_event = (d) => {this.table.highlight(d);this.scroll_graph.highlight(d);};
        this.table.click_event = (d) => {this.table.highlight(d);this.scroll_graph.highlight(d);};
        
        // keep track of all incoming timestamps and payloads
        this.data = [];
        
        // add buttons to footer
        // clear data should clear canvas in scroll graph and table
        d3.select("#footer" + id_end).append("button").text("Clear Data").on("click", () => {this.scroll_graph.clear();this.table.clear();this.data = [];});

        // autoscroll button should set object fields and object itself will handle enabled/disabled autoscrolling
        this.autoscroll_button = d3.select("#footer" + id_end).append("button").text("Disable Autoscroll").on("click", 
            () => {if (this.scroll_graph.autoscroll) 
                   {
                       this.autoscroll_button.text("Enable Autoscroll");
                   } 
                   else {
                       this.autoscroll_button.text("Disable Autoscroll");
                   }
                   this.scroll_graph.autoscroll = !this.scroll_graph.autoscroll;
                   this.table.autoscroll = !this.table.autoscroll;
                   });

    }
    
    /**
     * Function appends a new timestamp-payload pair to the existing dataset and adds the new 
     * data to the graph and table.
     * 
     * @param {Date()} time:
     *                  - timestamp of the new payload
     * @param {list} payload:
     *                  - new payload to be tracked and visualized
     *  
     */
    add_data(time, raw_payload)
    {
        // All widget objects should be receiving a list of timestamp-payload pairs. The 
        // payload should give information about every point being displayed and kept track 
        // of by this object at each timestamp. (length of payload should match length of 
        // this.points)
        // Each data unit at each index of the payload should correspond to each point of 
        // this.points with the same index
        // For each point, if the CAN message (the raw_payload) holds valid information about 
        // the point (the point's filter function returns true when the CAN Id is passed,) 
        // the data unit at the same index in the payload would be the calculated value of 
        // the point (using the value function.) Else, the data unit at the same index of the 
        // payload would take up the boolean value "false" to indicate that it is not valid.
        var payload = this.points.map((d) => {return d.f(raw_payload[0]) ? d.v(raw_payload) : false;});
      
        // only store most recent 100 timestamps
        // TODO: explore storing phased out info in a local db in case user wants to view old data
        if (this.data.length > 100)
        {
            this.data.shift();
        }

        // push timestamp-payload pair to data
        this.data.push({t:time, p:payload});

        // directly pass data into objects
        this.scroll_graph.add_data(this.data);

        // append timestamp in string form to the list of payloads
        this.table.add_data(this.data.map((d) => {var date = new Date(d.t); return {t:d.t, p:[date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds() + "." + date.getMilliseconds()].concat(d.p)};}));
    }
}  