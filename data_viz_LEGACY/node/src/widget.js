var d3 = require("d3");

/**
 * An abstract class for "widgets," an object that visualizes data over time.
 * It should be a collection of visual units, where each unit represents a certain quantity 
 * at a given time. The distinction between each visual unit is the timestamp of the 
 * visualized data. 
 * 
 * Qualities of the widget include (but not limited to):
 *  - clickable visual units (events are determined by parameters)
 *  - visual units can be highlighted (usually occured when visual units are clicked)
 *  - can be "autoscrolled" (when autoscroll is enabled, the most recent visual units are 
 *    shown)
 *  - visual units can be cleared, or removed
 * 
 */
class widget
{
    /**
     * constructor of the abstract class
     * 
     * @param {dict} opt: 
     *      options/configurations for the widget class. Each widget must have the following 
     *      options specified in this parameter:
     *          - id:              id of the div that holds the widget
     *          - width:           full width of the widget
     *          - height:          full height of the widget
     *          - click_event:     event callback when certain units are clicked. The 
     *                             callback should have only one parameter, which is the 
     *                             timestamp of the clicked unit (this is usually a set of 
     *                             highlight functions of numerous related widgets)
     *          - highlight_color: color of the visual unit when it is highlighted. It should 
     *                             be in the RGB() form, since the units usually fade in and 
     *                             out of the highlighted color, and numerical values are 
     *                             required for interpolation         
     */
    constructor(opt)
    {
        // resize div
        this.div = d3.select(opt.id).style("width", opt.width).style("height", opt.height);

        // save paramters
        this.click_event = opt.click_event;
        this.highlight_color = opt.highlight_color;

        // by default, set autoscroll to true
        this.autoscroll = true;
    }
    
    /**
     * Function updates the visual units with the newest dataset. Visual units will be 
     * created/updated/removed based on the newest dataset. 
     * This is implemented by the sub-class.
     * 
     * @param {list} data:
     *      newest dataset used to update the current visual units. This parameter is a list  
     *      of timestamp-payload pairs. The format of the payload and how it is processed is 
     *      determined by the sub-class.
     *      e.g [{t: <timestamp>, p: <payload>}, {t: <timestamp>, p: <payload>}, ...]
     * 
     */
    add_data(data) {}
    
    /**
     * Function locates the visual unit with the timestamp matching the parameter, and  
     * highlights/de-highlights the visual unit depending on the highlighted state of the 
     * visual unit. 
     * This is implemented by the sub-class.
     * 
     * @param {timestamp} time:
     *      timestamp of the visual unit that is highlighted/de-highlighted
     *  
     */
    highlight(time) {}
    
    /**
     * Function removes all visual units.
     * This is implemented by the sub-class.
     * 
     */
    clear() {}
}