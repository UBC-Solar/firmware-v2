var d3 = require("d3");

/**
 * A class that holds data in a table. Each row corresponds to each unique timestamp, and it 
 * contains each data reading at that timestamp. Since each timestamp corresponds to a 8-byte 
 * CAN message, and the CAN message may not hold each and every type of data kept track by 
 * the table, if certain data does not have valid readings at certain timestamps, the cell is 
 * left blank (there will never be a row with all data cells left blank.)
 * 
 * Each table row is clickable, and it invokes a callback event with the row's timestamp as 
 * the parameter.
 * 
 */
class timetable extends widget
{
    /**
     * Constructor of the table class
     * 
     * @param {dict} opt:
     *      options/configurations for the table. In addition to the options for the parent 
     *      class (the abstract widget class), each table must have the following options 
     *      specified in this parameter:
     *          - headers: list of headers for the table. By default, the first column of the 
     *                     table is for the timestamp, so this list, plus a timestamp column 
     *                     will make up the headers of the table.
     *          - column_width: width of each column of the table. By default, the first 
     *                          column (timestamp) is 100px wide, so this option sets the 
     *                          width of the other columns of the table.
     * 
     */
    constructor(opt)
    {
        // call superclass constructor to resize div, set autoscroll, and save parameters
        super(opt);

        // define constants and save parameters
        const HEADER_HEIGHT = 30;
        this.TIME_COL_WIDTH = 100;
        this.column_width = opt.column_width;

        // add header to div using the parameters
        // the time column is added in addition to the list of specified headers
        this.div.append("tr").attr("class", "header").selectAll("th").data(["Time"].concat(opt.headers)).enter().append("th").text((d) => {return d;}).attr("width", (d, i) => {return (i == 0) ? this.TIME_COL_WIDTH : this.column_width;}).attr("align", "center").attr("height", HEADER_HEIGHT);

        // create seperate div for data entries so when div overflows, the header will still 
        // remain visible at the top
        this.table = this.div.append("div").style("overflow-y", "scroll").style("height", opt.height - HEADER_HEIGHT);
    }
    
    /**
     * Implementation of the add_data function for the abstract widget class. Function 
     * updates the table with the newest dataset. Table rows will bec reated/updated/removed 
     * based on the newest dataset.
     * 
     * @param {list} data:
     *      newest dataset used to update the table. This parameter is a list of 
     *      timestamp-payload pairs. 
     *      e.g [{t: <timestamp>, p: <payload>}, {t: <timestamp>, p: <payload>}, ...]
     * 
     *      The payload is a list of either strings for valid values, or the boolean value 
     *      "false" for invalid values. Since payloads are essentially parsed CAN messages, 
     *      and each CAN message is only 8 bytes long, not every data kept track of by this   
     *      table can fit in a single message. Therefore, at certain timestamps, some data
     *      values are invalid.
     * 
     */
    add_data(data)
    {
        // inject data to table as-is, using the timestamp as the key
        var data = this.table.selectAll(".data").data(data, (d) => {return d.t;});

        // for any new data entries, create new row and attach click event
        // Each data entry in the parameter is a timestamp-payload pair, and each payload 
        // essentially a list of values that make up each row
        // If the value is a string, the value is valid, and the value becomes the text of 
        // the table cell. If the value is the boolean value "false," the value is invalid, 
        // and the text of the table cell is left blank.
        data.enter().append("tr").attr("class", "data").on("click", (d) => {this.click_event(d.t);}).selectAll("td").data((d) => {return d.p;}).enter().append("td").text((d) => {return d ? d : " ";}).attr("align", "center").attr("width", (d, i) => { return (i == 0) ? this.TIME_COL_WIDTH : this.column_width;});
        data.exit().remove();

        // if autoscroll is enabled, scroll to bottom of table
        if (this.autoscroll)
        {
            this.table.node().scrollTop = this.table.node().scrollHeight;
        }
    }

    /**
     * Implementation of the highlight function for the abstract widget class. Function 
     * locates the table row with the timestamp matching the parameter, and 
     * highlights/de-highlights the row depending on the highlighted state of the row.
     * 
     * @param {timestamp} time:
     *      timestamp of the table row that is highlighted/de-highlighted
     *  
     */
    highlight(time)
    {
        // isolate row to be highlighted using the timestamp
        var row = this.div.selectAll(".data").filter((d) => {return d.t == time;});
        row.transition().style("background-color", (row.style("background-color") == this.highlight_color) ? "white" : this.highlight_color);
    }
    
    /**
     * Implementation of the clear function for the abstract widget class.
     * Function removes all table entries.
     * 
     */
    clear()
    {
        // remove all data
        this.div.selectAll(".data").transition().remove();
    }
}