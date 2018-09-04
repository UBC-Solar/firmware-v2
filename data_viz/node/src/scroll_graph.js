var d3 = require("d3");

/**
 * A class that visualizes data over time in a graph. This class consists of 3 parts: the 
 * scroll unit, the configuration panel, and the graph unit.
 * 
 * The scroll unit is essentially a graph as well, but with a horizontally adjustable brush. 
 * Whatever is covered by the brush is graphed onto the graph unit (in a sense, the scroll 
 * unit is the full graph while the graph unit is the zoomed in version of the full graph.) 
 * This allows users to zoom and scroll through the graphed data at whatever suitable domain.
 * 
 * The configuration panel is a panel beside the scroll and graph unit. It displays relavent 
 * information, such as the start and end times of the current domain of the graph unit, as 
 * well as the span of the current domain. It also allows users to decide if they want to 
 * display the curve fit for each data type, as well as the type of curve fit on the graph 
 * unit.
 *
 * Each point is clickable, and it invokes a callback event with the point's timestamp as 
 * the parameter.
 * 
 */
class scroll_graph extends widget
{
    /**
     * Constructor of the scroll_graph class
     * 
     * @param {dict} opt:
     *      options/configurations for the scroll_graph. In addition to the options for the 
     *      parent class (the abstract widget class), each table must have the following 
     *      options specified in this parameter:
     *          - p_color: A list of colors for the points on the graph unit. The color at 
     *                     each index of the list corresponds to the color of the data at the 
     *                     same index in the payload from the dataset.
     *          - s_color: A list of colors for the points on the scroll unit. The color at 
     *                     each index of the list corresponds to the color of the data at the 
     *                     same index in the payload from the dataset.
     *          - margin: A dictionary that holds margin values for the widget. It should 
     *                    contain the following values:
     *              - left: left margin (margin from the y-axis of the scroll and graph unit 
     *                      to the border of the div)
     *              - right: right margin (margin from the right border of the graph and 
     *                       scroll unit to the left border of the configuration panel)
     *              - top: top margin (margin from the top border of the graph unit to the 
     *                     top border of the div)
     *              - bottom: bottom margin (margin from the bottom border of the scroll unit 
     *                        to the bottom border of the div)
     * 
     */
    constructor(opt)
    {
        // call superclass constructor to resize div, set autoscroll, and save parameters
        super(opt);
        
        // define constants
        const CONFIG_WIDTH = 120;  // width of config panel
        const AXIS_MARGIN = 20;    // margin in graph svg to allow x-axis to be seen
        const TOP_CLIP_OFF_MARGIN = 5; // margin in svgs to allow points mapped close to the edge of svg to not be cut off
        this.SIDE_CLIP_OFF_MARGIN = 20; // margin in svgs to allow points mapped close to the edge of svg to not be cut off

        // create divs to hold visual (graph and scroll unit) and config panel
        var visual = this.div.append("div").style("height", opt.height).style("width", opt.width - CONFIG_WIDTH).style("float", "left");
        this.config = this.div.append("div").attr("class", "config").style("height", opt.height).style("width", CONFIG_WIDTH).style("float", "left").style("overflow-y", "scroll");
        
        // save options
        this.p_color = opt.p_color;
        this.s_color = opt.s_color;

        // append line breaks and add labels for starting time, ending time, and domain
        this.config.append("br");
        this.config.append("br");
        this.config.append("b").text("Starting time:").append("br");
        this.start_time_text = this.config.append("t").text("N/A");
        this.start_time = null;

        this.config.append("br");
        this.config.append("br");
        this.config.append("b").text("Ending time:").append("br");
        this.end_time_text = this.config.append("t").text("N/A");
        this.end_time = null;

        this.config.append("br");
        this.config.append("br");
        this.config.append("b").text("Domain:").append("br");
        this.domain_text = this.config.append("t").text("5000ms");
        this.config.append("br");
        this.config.append("br");
        this.domain = 5000;

        // lookup table for curves
        this.curves = { "Linear":d3.curveLinear, "Monotone x":d3.curveMonotoneX, "Natural":d3.curveNatural, "Step":d3.curveStep};

        // for each type of point to be graphed, append a selectbox for the curve and a 
        // checkbox for the visiblity of the line
        for (var i = 0; i < this.p_color.length; i++)
        {
            this.config.append("b").text("Line" + i + ":").style("color", this.p_color[i]).append("br");
            this.config.append("select").selectAll("option").data(Object.keys(this.curves)).enter().append("option").text((d) => {return d;});
            this.config.append("br");
            this.config.append("input").attr("type", "checkbox");
            this.config.append("t").text("Show line").append("br");
            this.config.append("br");
        }
        
        // attach callback events for each selectbox and clickbox
        this.config.selectAll("input").on("click", (d, i) => {this.hide_line(i);});
        this.config.selectAll("select").on("change", (d, i) => {this.change_line(i);});

        this.svg = visual.append("svg").attr("width", opt.width).attr("height", opt.height);
        
        // graph init
        // graph should take up 3 quarters of the div (excluding margins)
        var graph_height = (opt.height - opt.margin.top - opt.margin.bottom)*3/4;
        this.graph_width = opt.width - opt.margin.left - opt.margin.right - CONFIG_WIDTH;
        
        // y-axis
        // y-scale should map range of values to a value between the height of the graph, 
        // excluding the margin reserved for the x-axis (for the minimum value,) and a thin 
        // margin at the top (for the max value)
        // 0 is not used for the max value, since points at max value would be cut off halfway
        this.graph_y_scale = d3.scaleLinear().domain(opt.range).range([graph_height - AXIS_MARGIN, TOP_CLIP_OFF_MARGIN]);
        this.svg.append("g").attr("class", "graph_y_axis").call(d3.axisLeft().scale(this.graph_y_scale)).attr("transform", "translate(" + opt.margin.left + ", " + opt.margin.top + ")");

        // create svg for graph
        this.graph = this.svg.append("svg").attr("class", "graph").attr("x", opt.margin.left).attr("y", opt.margin.top).attr("width", this.graph_width).attr("height", graph_height);
        
        // x-axis
        // axis is moved to where the margin reserved for the axis begins
        this.graph.append("g").attr("class", "graph_x_axis").attr("transform", "translate(0, " + (graph_height - AXIS_MARGIN) + ")");
        this.graph_x_axis = d3.axisBottom();

        // lines
        // create path objects for each set of points to be graphed
        // for now, set datum for each line to be an empty list
        this.graph.selectAll(".graph_line").data(this.p_color.map((d) => {return [];})).enter().append("path").attr("class", "graph_line").style("stroke", "none").style("fill", "none");
        
        // create a line object for each point to be graphed (this just maps the data in the 
        // path into a line so it is different from the path)
        // the only difference between each line object is the curve fit selected from the 
        // config panel. Everything else is identical.
        // since the data held by the path will be a list of timestamp-value pairs, the "x" 
        // and "y" portions are mapped by using the graph's x and y-axis scales (since the 
        // x-axis scale is not defined yet, and would change according to the brush, we 
        // initalize it to some callable function for now)
        this.lines = [];
        for (var i = 0; i < this.p_color.length; i++)
        {
            this.lines.push(d3.line().x((d) => {return d.t;}).y((d) => {return this.graph_y_scale(d.y);}));
        }

        // scroll init
        // scroll unit should take up the remain quarter of the div
        var scroll_height = (opt.height - opt.margin.top - opt.margin.bottom)/4;
        this.scroll_width = opt.width - opt.margin.left - opt.margin.right - CONFIG_WIDTH;
        
        // y-axis
        // same implementation as the y-axis of the graph, but the range mapped to is smaller 
        // (margins are all the same)
        this.scroll_y_scale = d3.scaleLinear().domain(opt.range).range([scroll_height - AXIS_MARGIN, TOP_CLIP_OFF_MARGIN]);
        // for the scroll unit's y-axis, only show ticks for the max, mid, and min value of 
        // the range
        var scroll_y_axis = d3.axisLeft().scale(this.scroll_y_scale).tickValues([opt.range[0], opt.range[1]/2, opt.range[1]]);
        this.svg.append("g").attr("class", "scroll_y_axis").call(scroll_y_axis).attr("transform", "translate(" + opt.margin.left + ", " + (graph_height + opt.margin.top) + ")");
      
        // create svg for the scroll unit
        this.scroll = this.svg.append("svg").attr("class", "scroll").attr("x", opt.margin.left).attr("y", graph_height + opt.margin.top).attr("width", this.scroll_width).attr("height", scroll_height);

        // x-axis
        // same implementation as the x-axis of the graph
        this.scroll.append("g").attr("class", "scroll_x_axis").attr("transform", "translate(0 ," + (scroll_height - AXIS_MARGIN) + ")");
        this.scroll_x_scale = null; // defined later TODO: change to some default value

        // brush init
        // set bounds of brush to exclude margins. Also attach event callback to brush.
        this.brush = d3.brushX().extent([[0, 0], [this.scroll_width - this.SIDE_CLIP_OFF_MARGIN, scroll_height - AXIS_MARGIN]]).on("brush end", () => {this.adjust_brush();});
        this.brush_g = this.scroll.append("g").attr("class", "brush_group").call(this.brush);
        
        this.brush.move(this.brush_g, this.scroll_width);
        
        this.domain = 5000;

        // function is used to convert data injected into the group into list of data 
        // injected into the circles within the group
        // Since each payload is essentially a parsed CAN message, it is possible that each 
        // CAN message cannot hold every single type of data visualized in this widget. In 
        // that case, some data type values would be invalid, and will be filtered out before 
        // the payload is injected into the points. However, the colors of the points of data 
        // is determined by its index in the payload, which is inaccurate if payloads may be 
        // filtered. Therefore, each data payload is transformed into value-index pairs first 
        // before it is filtered in order to perserve the original index.
        this.convert = (p) => {return p.map((d, i) => {return {v:d, i:i};}).filter((d) => {return d.v;})};
    }
    
    /**
     * Implementation of the add_data function for the abstract widget class. Function 
     * updates the graph with the newest dataset. Points will be created/updated/removed 
     * based on the newest dataset.
     * 
     * @param {list} data:
     *      newest dataset used to update the graph. This parameter is a list of 
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
        // get list of timestamps
        var times = data.map((d) => {return d.t;});

        // get default start and end times if they are not initalized
        if (this.start_time == null)
        {
            this.start_time = times[0];
            this.end_time = times[0] + this.domain;
        }

        // update scroll scale
        // Ideally, the scroll unit's domain should span from the first data unit's timestamp 
        // to the last data unit's timestamp, while having the brush span the specified graph 
        // domain. However, if the difference between the first and last timestamp is less 
        // than the graph domain, or if only one point exists in the dataset, the domain 
        // should span from the first timestamp to (first timestamp + graph domain.) With 
        // that in mind, the scroll unit's scale is set accordingly
        this.scroll_x_scale = d3.scaleTime().domain([times[0], Math.max(times[0] + this.domain, times[times.length - 1])]).range([0, this.scroll_width - this.SIDE_CLIP_OFF_MARGIN]);
        this.svg.select(".scroll_x_axis").call(d3.axisBottom().scale(this.scroll_x_scale).ticks(d3.timeSecond));
        
        // update scroll data
        // Use timestamp as the key, inject the dataset into points graphed onto the scroll 
        // unit.
        // For payloads at new timestamps, filter out invalid data (valued as "false",) 
        // create points for each valid payload, and scale its position using the y scale 
        // declared in the constructor. Payloads with the same timestamps should be in the 
        // same group.
        // For all payloads at all timestamps, transform the group using the updated x scale 
        // from above (hence the reason points at each timestamp are grouped together.)
        // Remove any payloads that are phased out (key no longer exists in the dataset)
        var scroll_data = this.scroll.selectAll(".scroll_data").data(data, (d) => {return d.t;});
        scroll_data.enter().append("g").attr("class", "scroll_data").attr("transform", (d) => {return "translate(" + this.scroll_x_scale(d.t) + ", 0)";}).selectAll("circle").data((d) => {return this.convert(d.p);}).enter().append("circle").attr("cy", (d) => {return this.scroll_y_scale(d.v);}).attr("r", 2).attr("fill", (d) => {return this.s_color[d.i];});
        scroll_data.attr("transform", (d) => {return "translate(" + this.scroll_x_scale(d.t) + ", 0)";});
        scroll_data.exit().remove();
        
        // update graph data
        // For payloads at new timestamps, filter out invalid data (valued as "false",) 
        // create points for each new valid payload, and scale this position using the y 
        // scale declared in the constructor. Payload with the same timestamps should be in 
        // the same group.
        // Remove any payloads that are phased out (key no longer exists in the dataset)
        // groups are not horizontally transformed here, since the brush determines the transform values
        var graph_data = this.graph.selectAll(".graph_data").data(data, (d) => {return d.t;});
        graph_data.enter().append("g").attr("class", "graph_data").on("click", (d) => {this.click_event(d.t);}).selectAll("circle").data((d) => {return this.convert(d.p);}).enter().append("circle").attr("cy", (d) => {return this.graph_y_scale(d.v);}).attr("r", 5).attr("fill", (d) => {return this.p_color[d.i];});
        graph_data.exit().remove();
        
        // update datum for each path
        // datum of each path should be a list of timestamp-value pairs (extracted by mapping 
        // dataset into individual lists and filtering out invalid values)
        // line is not redrawn yet since x scale of graph is not defined yet 
        this.graph.selectAll(".graph_line").datum((d, i) => {return data.map((m) => {return m.p[i] ? {t:m.t, y:m.p[i]} : false;}).filter((f) => {return f;});});

        // update the ticks so new points have a tick as well
        this.graph_x_axis.tickValues(times);

        // update graph x scale via brush
        // move and stretch/shrink brush to trigger event callback, where the graph x scale 
        // is determined and the graph points and lines are transformed and generated 
        // accordingly.
        // If autoscroll is enabled, resize and move brush to cover the last n seconds of the 
        // scroll domain, where n is the specified graph domain (unless the scroll unit's 
        // domain is less than the specified graph domain. In that case, set the brush to 
        // span the whole scroll unit) 
        if (this.autoscroll)
        {
            // find new start of the brush using the scroll unit's x scale 
            var new_start = this.scroll_x_scale((times[times.length - 1] - this.domain));
            // move brush to the rightmost portion of the scroll unit
            this.brush.move(this.brush_g,[Math.max(0, new_start) ,this.scroll_width - this.SIDE_CLIP_OFF_MARGIN]);
        }
        // If autoscroll is disabled, find the new start and end points of the brush by using 
        // the exact same starting and ending timestamps, but with the updated scroll x scale.
        // If the starting timestamp has been phased out, set the new left bound of the brush 
        // to the left bound of the scroll unit, and set the brush to span the graph domain.
        else
        {
            // find the new start and end points of the brush using the same starting and 
            // ending timestamps
            var new_start = this.scroll_x_scale(this.start_time);
            var new_end   = this.scroll_x_scale(this.end_time);

            // If the new start bound is less than 0, the start timestamp has been phased out
            if (new_start < 0)
            {
                this.brush.move(this.brush_g, [0, this.scroll_x_scale(times[0] + this.domain)]);
            }
            // Else, move brush to new start and end points
            else
            {
                this.brush.move(this.brush_g, [new_start, new_end]);
            }
        }
    }
    
    /**
     * This is a callback function for when the brush is moved and/or resized. The function does the following:
     *      - creates new x-axis scale for the graph unit using the new bounds of the brush
     *      - shifts each point on the graph unit using the new x-axis scale
     *      - redraws each line on the graph unit using the new x-axis scale
     *      - updates text in the configuration panel with the new starting/ending times and 
     *        span of the new graph domain
     * 
     */
    adjust_brush()
    {
        // if scroll x scale is null, it means no points have been registered, and there is 
        // no domain
        // in this case, just return
        if (this.scroll_x_scale == null) return;

        // get the current bounds of brush and convert to time
        var brush_edge = d3.brushSelection(this.brush_g.node());
        var time_interval = brush_edge.map(this.scroll_x_scale.invert);
        
        // update graph scale such that brush bounds are mapped to graph axis
        // update axis with new scale
        var graph_x_scale = d3.scaleTime().domain(time_interval).range([0, this.graph_width - this.SIDE_CLIP_OFF_MARGIN]);
        this.graph_x_axis.scale(graph_x_scale);
        this.graph.select(".graph_x_axis").call(this.graph_x_axis);

        // shift all points in graph to new graph scale
        this.graph.selectAll(".graph_data").attr("transform", (d) => {return "translate(" + graph_x_scale(d.t) + ", 0)";});
        
        // before redrawing lines, set x component of each line to new scale
        for (var i = 0; i < this.lines.length; i++)
        {
            this.lines[i].x((d) => {return graph_x_scale(d.t);});
        }

        // reddraw all lines using new graph scale
        this.graph.selectAll(".graph_line").attr("d", (d, i) => {return this.lines[i](d);});
        
        // update new graph domain, start time and end time
        this.start_time = time_interval[0];
        this.end_time = time_interval[1];
        this.domain = (this.end_time - this.start_time);

        this.start_time_text.text(time_interval[0].getHours() + ":" + time_interval[0].getMinutes() + ":" + time_interval[0].getSeconds() + "." + time_interval[0].getMilliseconds());
        this.end_time_text.text(time_interval[1].getHours() + ":" + time_interval[1].getMinutes() + ":" + time_interval[1].getSeconds() + "." + time_interval[1].getMilliseconds());
        this.domain_text.text(this.domain + "ms");
    }
    
    /**
     * Implementation of the highlight function for the abstract widget class. Function 
     * locates the points with the timestamp matching the parameter, and 
     * highlights/de-highlights the points depending on the highlighted state of the points.
     * During the highlighted state, the point value is shown on the graph as well (as a text)
     * 
     * @param {timestamp} time:
     *      timestamp of the points that is highlighted/de-highlighted
     *  
     */
    highlight(time) 
    {
        // identify groups that are clicked
        var graph_g = this.svg.select(".graph").selectAll(".graph_data").filter((d) => {return d.t == time;});
        var scroll_g = this.svg.select(".scroll").selectAll(".scroll_data").filter((d) => {return d.t == time;});

        // check to see if group is highlighted already
        var highlighted = graph_g.select("circle").attr("fill") == this.highlight_color;
        
        // if it is already highlighted, shrink the points back to original size and change 
        // them back to their original colors
        // Also, shift the texts up and remove
        if (highlighted)
        {
            graph_g.selectAll("circle").data((d) => {return this.convert(d.p);}).transition().attr("fill", (d) => {return this.p_color[d.i];}).attr("r", 5);
            graph_g.selectAll("text").transition().attr("y", -10).remove();
            scroll_g.selectAll("circle").data((d) => {return this.convert(d.p);}).transition().attr("fill", (d) => {return this.s_color[d.i];}).attr("r", 2);
        }
        
        // if the points are not highlighted yet, highlight them by enlarging them and change 
        // their color to the highlighted color
        // Add text to each point. If the point holds a large value, shift the position of 
        // the text down by a small margin so it will still be visible and will not be cut 
        // off by the graph unit's border
        else
        {
            graph_g.selectAll("circle").transition().attr("fill", this.highlight_color).attr("r", 6);
            graph_g.selectAll("text").data((d) => {return this.convert(d.p);}).enter().append("text").text((d) => {return d.v;}).transition().attr("y", (d) => {return (this.graph_y_scale(d.v) < 10) ? this.graph_y_scale(d.v) + 10 : this.graph_y_scale(d.v);});
            scroll_g.selectAll("circle").transition().attr("fill", this.highlight_color).attr("r", 3);
        }
    }
    
    /**
     * Implementation of the clear function for the abstract widget class.
     * Function removes all points on both the scroll and graph unit.
     * 
     */
    clear()
    {
        // shift everything up first before removing
        this.graph.selectAll(".graph_data").selectAll("circle").transition().attr("cy", -10);
        this.graph.selectAll(".graph_data").selectAll("text").transition().attr("cy", -10);
        this.graph.selectAll(".graph_data").transition().remove();

        this.scroll.selectAll(".scroll_data").selectAll("circle").transition().attr("cy", -10);
        this.scroll.selectAll(".scroll_data").transition().remove();

        this.graph.selectAll(".graph_line").datum([]).attr("d", (d, i) => {return this.lines[i](d);});
    }

    /**
     * This is a callback function for the checkboxes in the configuration panel. Given an 
     * index, it toggles the visibility of the curve-fit line for the data with the same 
     * index in the payload. 
     * 
     * @param {integer} index:
     *      The index of the toggled line. The index of the line matches the index of the 
     *      data that make up the line in the payload.
     */
    hide_line(index)
    {
        // filter out line whose corresponding checkbox has been toggled and apply stroke 
        // change
        // since it is a style change, line does not need to be redrawn
        var check_state = this.config.selectAll("input").filter((d, i) => {return i == index;}).property("checked");
        this.graph.selectAll(".graph_line").filter((d, i) => {return i == index;}).style("stroke", check_state ? this.p_color[index] : "none");
    }

    /**
     * This is a callback function for the selectboxes in the configuration panel. Given an 
     * index, it changes the curve-fit line for the data with the same index in the payload. 
     * 
     * @param {integer} index:
     *      The index of the changed line. The index of the line matches the index of the 
     *      data that make up the line in the payload.
     */
    change_line(index)
    {
        // filter out line whose corresponding selectbox has been toggled
        // update curve property of line and redraw
        this.lines[index].curve(this.curves[this.config.selectAll("select").filter((d, i) => {return i == index;}).node().value]);
        this.graph.selectAll(".graph_line").filter((d, i) => {return i == index;}).attr("d", this.lines[index]);
    }
}